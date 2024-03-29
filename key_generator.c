/* key_generator.c - Debian weak key generator
 * Written by Rob Stradling
 * Copyright (C) 2008-2022 Sectigo Limited
 */

#include <arpa/inet.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "openssl/bio.h"
#include "openssl/bn.h"
#include "openssl/evp.h"
#include "openssl/ec.h"
#include "openssl/rand.h"
#include "openssl/rsa.h"


/* Override getpid(), so we can control the PID seen by crypto/rand/md_rand.c */
static pid_t g_pid;

pid_t getpid(
)
{
	return g_pid;
}


/* These functions are taken/adapted from apps/app_rand.c */

static int seeded = 0;
static int egdsocket = 0;

int app_RAND_load_file(
	const char* file
)
{
	char buffer[200];
	
	if (file == NULL)
		file = RAND_file_name(buffer, sizeof buffer);
	else if (RAND_egd(file) > 0) {
		/* we try if the given filename is an EGD socket.
		   if it is, we don't write anything back to the file. */
		egdsocket = 1;
		return 1;
	}
	if (file == NULL || !RAND_load_file(file, -1))
		if (RAND_status() == 0)
			return 0;
	seeded = 1;
	return 1;
}

int app_RAND_write_file(
	const char* file
)
{
	char buffer[200];
	
	if (egdsocket || !seeded)
		/* If we did not manage to read the seed file,
		 * we should not write a low-entropy seed file back --
		 * it would suppress a crucial warning the next time
		 * we want to use it. */
		return 0;

	if (file == NULL)
		file = RAND_file_name(buffer, sizeof buffer);
	if (file == NULL || !RAND_write_file(file))
		return 0;
	return 1;
}


int main(
	int argc,
	char** argv
)
{
	int t_returnCode = 1;

	if (argc != 4) {
		fprintf(stderr, "Usage: %s <key_size_in_bits_or_curve_name> <process_id> <rnd|nornd|noreadrnd>\n", argv[0]);
		exit(1);
	}

	/* Get and check the <process_id> */
	char* t_endPtr = NULL;
	g_pid = strtol(argv[2], &t_endPtr, 10);
	if ((*t_endPtr != '\0') || (g_pid < 0) || (g_pid > 32767)) {
		fprintf(stderr, "Process ID must be an integer between 0 and 32767\n");
		exit(1);
	}

	/* Configure RNG state */
	char t_randFilename[1024];
	if (getenv("RANDFILE"))
		strcpy(t_randFilename, getenv("RANDFILE"));
	else
		sprintf(t_randFilename, "%s/.rnd", getenv("HOME"));

	if (strcmp(argv[3], "nornd") == 0)
		/* Simulate first runs */
		(void)remove(t_randFilename);
	else if (strcmp(argv[3], "noreadrnd") == 0) {
		/* With 'sudo openssl ...', the randfile would have been unreadable */
		if (truncate(t_randFilename, 0) != 0) {
			fprintf(stderr, "truncate() => %s\n", strerror(errno));
			exit(1);
		}
	}
	else if (strcmp(argv[3], "rnd") != 0) {
		fprintf(stderr, "You must specify rnd, nornd or noreadrnd!\n");
		exit(1);
	}
	app_RAND_load_file(NULL);

	BIGNUM* t_publicExponent = NULL;
	RSA* t_rsa = NULL;
	EC_GROUP* t_group = NULL;
	EC_KEY* t_ecKey = NULL;
	if (atoi(argv[1]) > 0) {
		/* Generate predictable RSA keypair */
		t_publicExponent = BN_new();
		t_rsa = RSA_new();
		if (!t_publicExponent || !t_rsa)
			goto done;
		if (!BN_set_word(t_publicExponent, 65537))
			goto done;
		if (!RSA_generate_key_ex(t_rsa, atoi(argv[1]), t_publicExponent, NULL))
			goto done;
	}
	else {
		int t_nid;
		if (!strcmp(argv[1], "secp256r1"))
			t_nid = NID_X9_62_prime256v1;
		else if (!strcmp(argv[1], "secp384r1"))
			t_nid = NID_secp384r1;
		else {
			fprintf(stderr, "'%s' does not denote an RSA key size or a supported elliptic curve!\n", argv[1]);
			exit(1);
		}

		/* Generate predictable ECDSA keypair */
		t_group = EC_GROUP_new_by_curve_name(t_nid);
		EC_GROUP_set_asn1_flag(t_group, OPENSSL_EC_NAMED_CURVE);
		EC_GROUP_set_point_conversion_form(t_group, POINT_CONVERSION_UNCOMPRESSED);
		t_ecKey = EC_KEY_new();
		if (!t_ecKey)
			goto done;
		if (EC_KEY_set_group(t_ecKey, t_group) == 0)
			goto done;
		if (!EC_KEY_generate_key(t_ecKey))
			goto done;
	}

	/* Create output file */
	BIO* t_outFile = NULL;
	if ((t_outFile = BIO_new(BIO_s_file())) == NULL)
		goto done;
	char t_outFilename[255];
	sprintf(t_outFilename, "%s_%d_%s_%s%s.key", argv[1], g_pid, argv[3],
		((htonl(0x1234) == 0x1234) != (getenv("FLIP_ENDIAN") != NULL)) ? "be" : "le",
#ifdef __LP64__
		"64"
#else
		"32"
#endif
	);
	if (!BIO_write_filename(t_outFile, t_outFilename))
		goto done;

	/* Write key to output file */
	unsigned char* t_key_data = NULL;
	int t_key_size = 0;
	if (t_rsa != NULL)
		t_key_size = i2d_RSAPrivateKey(t_rsa, &t_key_data);
	else if (t_ecKey != NULL)
		t_key_size = i2d_ECPrivateKey(t_ecKey, &t_key_data);
	if (BIO_write(t_outFile, t_key_data, t_key_size) <= 0)
		goto done;

	t_returnCode = 0;

done:
	app_RAND_write_file(NULL);

	if (t_outFile)
		BIO_free(t_outFile);
	if (t_key_data)
		OPENSSL_free(t_key_data);
	if (t_publicExponent)
		BN_free(t_publicExponent);
	if (t_rsa)
		RSA_free(t_rsa);
	if (t_group)
		EC_GROUP_free(t_group);
	if (t_ecKey)
		EC_KEY_free(t_ecKey);

	return t_returnCode;
}

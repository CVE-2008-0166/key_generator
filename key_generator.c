

//#define _XOPEN_SOURCE 500

#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "openssl/bio.h"
#include "openssl/bn.h"
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
	if (argc != 4) {
		fprintf(stderr, "Usage: %s <key_size_in_bits> <process_id> <rnd|nornd|noreadrnd>\n", argv[0]);
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

	/* Generate predictable RSA keypair */
	BIGNUM* t_publicExponent = BN_new();
	RSA* t_rsa = RSA_new();
	if (!t_publicExponent || !t_rsa)
		goto done;
	if (!BN_set_word(t_publicExponent, 65537))
		goto done;
	if (!RSA_generate_key_ex(t_rsa, atoi(argv[1]), t_publicExponent, NULL))
		goto done;

	/* Create output file */
	BIO* t_outFile = NULL;
	if ((t_outFile = BIO_new(BIO_s_file())) == NULL)
		goto done;
	char t_outFilename[255];
	sprintf(t_outFilename, "%d.%s.key", g_pid, argv[3]);
	if (!BIO_write_filename(t_outFile, t_outFilename))
		goto done;

	/* Write key to output file */
	unsigned char* t_key_data = NULL;
	int t_key_size = i2d_RSAPrivateKey(t_rsa, &t_key_data);
	if (BIO_write(t_outFile, t_key_data, t_key_size) <= 0)
		goto done;

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

	return 0;
}

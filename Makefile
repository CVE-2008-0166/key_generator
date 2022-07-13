OPENSSLDIR=openssl-0.9.8f_with_CVE-2008-0166

all: key_generator

clean:
	rm -f key_generator

distclean: clean
	rm -f .openssl_is_built
	cd $(OPENSSLDIR); $(MAKE) clean; rm -f certs/*.0 test/dummytest; git checkout -- Makefile apps/CA.pl crypto/opensslconf.h tools/c_rehash

.openssl_is_built:
	cd $(OPENSSLDIR); ./config --prefix=. no-asm no-idea no-mdc2 no-rc5 zlib; $(MAKE)
	touch .openssl_is_built

key_generator: .openssl_is_built
	gcc -o key_generator key_generator.c -I$(OPENSSLDIR)/include $(OPENSSLDIR)/libcrypto.a

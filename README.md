# key_generator
[Debian weak key](https://wiki.debian.org/SSLkeys) generator.

## Introduction
CABForum Baseline Requirements 4.9.1.1 (Reasons for Revoking a Subscriber Certificate) and 6.1.1.3 (Subscriber Key Pair Generation) expect Certification Authorities to check that there is not a [proven method that can easily compute the Subscriber's Private Key based on the Public Key](https://cabforum.org/baseline-requirements-documents/), citing the example of [Debian weak keys](https://cve.mitre.org/cgi-bin/cvename.cgi?name=CVE-2008-0166) (CVE-2008-0166). Due to CVE-2008-0166, OpenSSL 0.9.8c-1 up to versions before 0.9.8g-9 on Debian-based operating systems can only produce 294,912 possible RSA keys of any given keysize.

Back in 2008, Debian published an [openssl-blacklist](https://packages.debian.org/search?keywords=openssl-blacklist) package, which contained complete blocklists for RSA keysizes that were commonly in use at the time, as well as an `openssl-vulnkey` tool for checking whether or not any given key is blocklisted.

## Problems
It's not an easy task to correctly set up old, vulnerable Debian versions, for all 3 word size / endianness combinations, in order to generate complete weak key blocklists for RSA keysizes that aren't detected by `openssl-vulnkey` (as shipped by Debian). Additionally, many modern Linux systems no longer even provide the openssl-blacklist package. Nonetheless, CAs should expect that they will need to check for Debian weak keys indefinitely.

## Purpose
This repository provides tools - to generate complete sets of Debian weak keys (for all of the affected architectures), and to generate the corresponding blocklists - that can be run on a modern 64-bit Linux system. The `key_generator` tool uses a bundled version of [OpenSSL 0.9.8f](https://github.com/CVE-2008-0166/key_generator/commit/9fbb1ecbd9fee3a59c829657c639ba663f2706b5) that has been [modified](https://github.com/CVE-2008-0166/key_generator/commit/c39d4c0e82879314f0a44e55f0212bd12c291e3e) to make it vulnerable to CVE-2008-0166. Multiple architectures are simulated thanks to [64-bit Linux being able to execute 32-bit binaries](#Prebuilt-Binaries) and with the help of a [further modification](https://github.com/CVE-2008-0166/key_generator/commit/90078bea3596b1783c4ea5796d7299139c6c0e94) that provides a mechanism to emulate the opposite endianness by reversing the byte order of certain variables used within the affected OpenSSL RNG code.

## Pregenerated Keys and Blocklists
The [openssl_blocklists](https://github.com/CVE-2008-0166/openssl_blocklists) repository contains complete blocklists of Debian weak keys for various RSA keysizes, using the same format as `openssl-vulnkey`.

The [private_keys](https://github.com/CVE-2008-0166/private_keys) repository contains complete sets of Debian weak keys for various RSA keysizes, using the PKCS#1 private key format. Using these key sets, CAs can implement weak/compromised key checks without having to be tied to the proprietary, RSA-specific format used by `openssl-vulnkey`.

## Key Generator Tools

### Prebuilt key_generator Binaries
[bin/linux](bin/linux) contains a [32-bit](bin/linux/x86-32/key_generator) `key_generator` binary that was prebuilt on Debian Buster (i386), and a [64-bit](bin/linux/x86-64/key_generator) `key_generator` binary that was prebuilt on Ubuntu 20.10 (x64).

To be able to run the 32-bit `key_generator` binary on Ubuntu 20.10 (x64), do this:
``` bash
sudo apt install libc6:i386
```

These prebuilt binaries have been tested on Ubuntu 20.10 (x64) and Gentoo Linux (x64).

### Build key_generator from Source
To build the bundled version of OpenSSL and the `key_generator` tool from source, run
``` bash
make
```
To cleanup the `key_generator` build, run
``` bash
make clean
```
To cleanup both the `key_generator` and bundled OpenSSL builds, run
``` bash
make distclean
```

### Testing key_generator
To check that `key_generator` functions correctly in your environment, run
``` bash
./run_tests.sh
```
You should see the following output:
```
Generating weak RSA-2048 keys for process ID 32767 of 32767: le64(rnd,nornd,noreadrnd) le32(rnd,nornd,noreadrnd) be32(rnd,nornd,noreadrnd)
Generating test blocklist
Tests Successful
```
(IMPORTANT: If you see "Tests FAILED!", then unfortunately `key_generator` will not produce Debian weak keys in your environment).

### Using the Tools

To use `key_generator` to generate all of the Debian weak keys for a particular RSA keysize, run
``` bash
./generate_weak_keys.sh <key_size>
```

To put those keys into a set of .zip files, run
``` bash
./zip_weak_keys.sh <key_size>
```

To generate blocklists for those keys that are compatible with `openssl-vulnkey`, run
``` bash
./generate_blocklists.sh <key_size>
```

NOTE: Key and blocklist generation takes a long, long time!

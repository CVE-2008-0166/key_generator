package main

import (
	"crypto/sha256"
	"crypto/x509"
	"encoding/hex"
	"fmt"
	"log"
	"os"
	"strings"
)

func main() {
	if len(os.Args) != 2 {
		log.Fatalf("Usage: %s <private_keys_directory>\n", os.Args[0])
	} else if files, err := os.ReadDir(os.Args[1]); err != nil {
		log.Fatalf("os.ReadDir() => %v\n", err)
	} else {
		for _, file := range files {
			if !strings.HasSuffix(file.Name(), ".key") {
				continue
			} else if key, err := os.ReadFile(os.Args[1] + "/" + file.Name()); err != nil {
				log.Fatalf("os.ReadFile(%s) => %v\n", file.Name(), err)
			} else if rsakey, err := x509.ParsePKCS1PrivateKey(key); err == nil {
				sha256_modulus := sha256.Sum256(rsakey.N.Bytes())
				fmt.Printf("%s\n", hex.EncodeToString(sha256_modulus[:]))
			} else if eckey, err2 := x509.ParseECPrivateKey(key); err2 == nil {
				sha256_xcoord := sha256.Sum256(eckey.X.Bytes())
				fmt.Printf("%s\n", hex.EncodeToString(sha256_xcoord[:]))
			} else {
				log.Fatalf("%s: x509.ParsePKCS1PrivateKey() => %v; x509.ParseECPrivateKey() => %v\n", file.Name(), err, err2)
			}
		}
	}
}

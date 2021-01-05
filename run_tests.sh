#!/bin/bash

TMPDIR=`mktemp -d`
./generate_weak_keys.sh 2048 32767 $TMPDIR

echo Generating test blocklist
cd $TMPDIR/2048
for f in *.key; do
  openssl rsa -in $f -inform der -modulus -noout | sha1sum >> sha1sums.txt
done

SHA1SUM=`sort sha1sums.txt | sed "s/ *-$//" | sha1sum | sed "s/ *-$//"`
if [[ $SHA1SUM == "2a910e906624766546114d300ca6b671c7acc08c" ]]; then
  echo "Tests Successful"
else
  echo "Tests FAILED!"
  cat sha1sums.txt
fi

rm -rf $TMPDIR



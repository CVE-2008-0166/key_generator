#!/bin/bash

if [ -n "$1" ] && [ "$1" -eq "$1" ] 2>/dev/null; then
  mkdir -p blocklists/$1/be32 blocklists/$1/le32 blocklists/$1/le64
  rm -f blocklists/$1/be32/blocklist-$1.db blocklists/$1/le32/blocklist-$1.db blocklists/$1/le64/blocklist-$1.db
  TMPFILE=$(mktemp)

  for f in keys/$1/$1_*_be32.key; do
    echo -n $f | sed "s/_/ /g" | sed "s/^keys\/$1\///" | sed "s/\.key//" | sed "s/be32/ppc64 /" >> $TMPFILE
    openssl rsa -inform der -in $f -modulus -noout | sha1sum | sed "s/  -$//" >> $TMPFILE
  done
  sort -k 3,3r -k 2,2n $TMPFILE > blocklists/$1/be32/blocklist-$1.db
  echo -n > $TMPFILE

  for f in keys/$1/$1_*_le32.key; do
    echo -n $f | sed "s/_/ /g" | sed "s/^keys\/$1\///" | sed "s/\.key//" | sed "s/le32/i386 /" >> $TMPFILE
    openssl rsa -inform der -in $f -modulus -noout | sha1sum | sed "s/  -$//" >> $TMPFILE
  done
  sort -k 3,3r -k 2,2n $TMPFILE > blocklists/$1/le32/blocklist-$1.db
  echo -n > $TMPFILE

  for f in keys/$1/$1_*_le64.key; do
    echo -n $f | sed "s/_/ /g" | sed "s/^keys\/$1\///" | sed "s/\.key//" | sed "s/le64/x86_64 /" >> $TMPFILE
    openssl rsa -inform der -in $f -modulus -noout | sha1sum | sed "s/  -$//" >> $TMPFILE
  done
  sort -k 3,3r -k 2,2n $TMPFILE > blocklists/$1/le64/blocklist-$1.db
  rm -f $TMPFILE
else
  echo "Usage: $0 <key_size>"
fi

#!/bin/bash

check() {
  if [ "$(which "$1")" = "" ]; then
    echo "This script requires $1; please resolve and try again."
    exit 1
  fi
}

check dwk_blocklist_generator

if [ -n "$1" ] && [ -n "$2" ] 2>/dev/null; then
  TMPFILE=$(mktemp)
  mkdir -p dwk_blocklists
  dwk_blocklist_generator $2/le32/$1 > $TMPFILE
  dwk_blocklist_generator $2/le64/$1 >> $TMPFILE
  dwk_blocklist_generator $2/be32/$1 >> $TMPFILE
  if [[ "$1" =~ ^[0-9]+$ ]]; then
    sort $TMPFILE > dwk_blocklists/sha256_modulus_$1.csv
  else
    sort $TMPFILE > dwk_blocklists/sha256_xcoord_$1.csv
  fi
  rm $TMPFILE
else
  echo "Usage: $0 <key_size> <private_keys_directory>"
fi

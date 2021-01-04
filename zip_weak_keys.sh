#!/bin/bash

if [ -n "$1" ] && [ "$1" -eq "$1" ] 2>/dev/null; then
  mkdir -p zips/be32 zips/le32 zips/le64
  rm -f zips/be32/keys-$1.zip zips/le32/keys-$1.zip zips/le64/keys-$1.zip
  find keys/$1 -iname "*_be32.key" -type f | sort -n -k2 -t_ | zip -r zips/be32/keys-$1.zip -@
  find keys/$1 -iname "*_le32.key" -type f | sort -n -k2 -t_ | zip -r zips/le32/keys-$1.zip -@
  find keys/$1 -iname "*_le64.key" -type f | sort -n -k2 -t_ | zip -r zips/le64/keys-$1.zip -@
else
  echo "Usage: $0 <key_size>"
fi

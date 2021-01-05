#!/bin/bash

START_PID=0
if [ -n "$2" ]; then
  START_PID=$2
fi

if [ -n "$1" ] && [ "$1" -eq "$1" ] 2>/dev/null; then
  mkdir -p keys
  cd keys
  mkdir $1 2>/dev/null
  if [ $? -eq 0 ]; then
    cd $1
    for (( i=$START_PID; i<=32767; i++ )); do
      echo -n "Generating weak RSA-$1 keys for process ID $i of 32767: "
      echo -n "le64(rnd"
      ../../bin/linux/x86-64/key_generator $1 $i rnd
      echo -n ",nornd"
      ../../bin/linux/x86-64/key_generator $1 $i nornd
      echo -n ",noreadrnd) "
      ../../bin/linux/x86-64/key_generator $1 $i noreadrnd
      echo -n "le32(rnd"
      ../../bin/linux/x86-32/key_generator $1 $i rnd
      echo -n ",nornd"
      ../../bin/linux/x86-32/key_generator $1 $i nornd
      echo -n ",noreadrnd) "
      ../../bin/linux/x86-32/key_generator $1 $i noreadrnd
      echo -n "be32(rnd"
      FLIP_ENDIAN=1 ../../bin/linux/x86-32/key_generator $1 $i rnd
      echo -n ",nornd"
      FLIP_ENDIAN=1 ../../bin/linux/x86-32/key_generator $1 $i nornd
      echo ",noreadrnd)"
      FLIP_ENDIAN=1 ../../bin/linux/x86-32/key_generator $1 $i noreadrnd
    done
  else
    echo "The keys/$1 directory already exists"
  fi
else
  echo "Usage: $0 <key_size> [<start_pid>]"
fi

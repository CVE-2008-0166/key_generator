#!/bin/bash

START_PID=0
if [ -n "$2" ]; then
  START_PID=$2
fi

CURRENT_DIR=`pwd`
OUTPUT_DIR=keys
if [ -n "$3" ]; then
  OUTPUT_DIR=$3
fi

RANDFILE=`mktemp`

if [ -n "$1" ] && [ "$1" -eq "$1" ] 2>/dev/null; then
  mkdir -p $OUTPUT_DIR
  cd $OUTPUT_DIR
  mkdir $1 2>/dev/null
  if [ $? -eq 0 ]; then
    cd $1
    for (( i=$START_PID; i<=32767; i++ )); do
      echo -n "Generating weak RSA-$1 keys for process ID $i of 32767: "
      # We must do noreadrnd on the first run, because our RANDFILE will be empty.
      echo -n "le64(noreadrnd"
      RANDFILE=$RANDFILE $CURRENT_DIR/bin/linux/x86-64/key_generator $1 $i noreadrnd
      echo -n ",nornd"
      RANDFILE=$RANDFILE $CURRENT_DIR/bin/linux/x86-64/key_generator $1 $i nornd
      echo -n ",rnd) "
      RANDFILE=$RANDFILE $CURRENT_DIR/bin/linux/x86-64/key_generator $1 $i rnd
      echo -n "le32(noreadrnd"
      RANDFILE=$RANDFILE $CURRENT_DIR/bin/linux/x86-32/key_generator $1 $i noreadrnd
      echo -n ",nornd"
      RANDFILE=$RANDFILE $CURRENT_DIR/bin/linux/x86-32/key_generator $1 $i nornd
      echo -n ",rnd) "
      RANDFILE=$RANDFILE $CURRENT_DIR/bin/linux/x86-32/key_generator $1 $i rnd
      echo -n "be32(noreadrnd"
      FLIP_ENDIAN=1 RANDFILE=$RANDFILE $CURRENT_DIR/bin/linux/x86-32/key_generator $1 $i noreadrnd
      echo -n ",nornd"
      FLIP_ENDIAN=1 RANDFILE=$RANDFILE $CURRENT_DIR/bin/linux/x86-32/key_generator $1 $i nornd
      echo ",rnd)"
      FLIP_ENDIAN=1 RANDFILE=$RANDFILE $CURRENT_DIR/bin/linux/x86-32/key_generator $1 $i rnd
    done
  else
    echo "The $OUTPUT_DIR/$1 directory already exists"
  fi
else
  echo "Usage: $0 <key_size> [<start_pid>] [<output_directory>]"
fi

rm -f $RANDFILE

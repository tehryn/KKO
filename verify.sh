#!/bin/bash

# Setup variables
BINARY=./huff_codec
TMPDIR=./testtmp
IMGDIR=./data
INPUTS="hd01 hd02 hd07 hd08 hd09 hd12 nk01"
MODES="static adaptive"

# Colours
RESTORE="\e[0m"
GREEN="\e[92m"
RED="\e[91m"

# Functions
function PrintOK() {
    echo -e -n "$GREEN"
    echo -e -n "OK"
    echo -e "$RESTORE"
}

function PrintError() {
    echo -e -n "$RED"
    echo -e -n "ERROR"
    echo -e "$RESTORE"
}

function TestFileEmpty() {
    echo -e -n "\t$1: "
    if [ -s "$TMPDIR/$2" ]; then
        PrintError
    else
        PrintOK
    fi
}

function TestFilesDiffer() {
    echo -e -n "\tOutput diff  : "

    diff $IMGDIR/$1.raw $TMPDIR/$1.$2.raw >/dev/null 2>/dev/null
    if [ $? -ne 0 ]; then
        PrintError
    else
        PrintOK
    fi
}


# Bootstrap
# Clean testdir
if [ -d "$TMPDIR" ]; then
    cd $TMPDIR && rm -f * && cd -
else
    mkdir $TMPDIR
fi

# Clean build
make clean >/dev/null 2>&1

echo -n "Compiling: "
make >$TMPDIR/compile.log 2>&1
if [ $? -ne 0 ]; then
    PrintError
    exit 1
else
    PrintOK
fi

# Loop over inputs and test them
for input in $INPUTS
do
    for mode in $MODES
    do
        echo "$BINARY -c -i $IMGDIR/$input.raw -o $TMPDIR/$input.$mode.huffman -h $mode -m"
        $BINARY -c -i $IMGDIR/$input.raw -o $TMPDIR/$input.$mode.model.huffman -h $mode -m

        echo "$BINARY -d -i $TMPDIR/$input.$mode.huffman -o $TMPDIR/$input.$mode.raw -h $mode -m"
        $BINARY -d -i $TMPDIR/$input.$mode.model.huffman -o $TMPDIR/$input.$mode.model.raw -h $mode -m

        TestFilesDiffer $input $mode.model

        echo "$BINARY -c -i $IMGDIR/$input.raw -o $TMPDIR/$input.$mode.huffman -h $mode"
        $BINARY -c -i $IMGDIR/$input.raw -o $TMPDIR/$input.$mode.huffman -h $mode

        echo "$BINARY -d -i $TMPDIR/$input.$mode.huffman -o $TMPDIR/$input.$mode.raw -h $mode"
        $BINARY -d -i $TMPDIR/$input.$mode.huffman -o $TMPDIR/$input.$mode.raw -h $mode

        TestFilesDiffer $input $mode
    done
done

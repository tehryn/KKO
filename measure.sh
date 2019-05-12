#!/bin/bash

# Setup variables
BINARY=./huff_codec
TMPDIR=./testtmp
IMGDIR=./data
INPUTS="hd01 hd02 hd07 hd08 hd09 hd12 nk01"
MODES="static adaptive"
EFFORTS="9"

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

    diff $IMGDIR/$1.raw $TMPDIR/$1.raw >$TMPDIR/$1.diff.out 2>&1 >/dev/out
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

## Clean build
#make clean >/dev/null 2>&1
#
#echo -n "Compiling: "
#make >$TMPDIR/compile.log 2>&1
#if [ $? -ne 0 ]; then
#    PrintError
#    exit 1
#else
#    PrintOK
#fi

# Loop over modes, models and inputs and measure them
echo "file;mode;model,effort;maximum compression memory used [KB];compression time [s];maximum decompression memory used [KB];decomression time [s];original size [B];compressed size [B];compression ration"
    for mode in $MODES
    do
        for model_en in `seq 0 1`
        do
            model=""
            model_str="0"
            if [ $model_en -ne 0 ]; then
                model="-m"
                model_str="1"
            fi
            for input in $INPUTS
            do
                t_out=`TIME="%M %e" time $BINARY -c -i $IMGDIR/$input.raw -o $TMPDIR/$input_tmp.raw -h $mode $model -$effort 2>&1`
                comp_max_kbytes=`python3 -c "print(\"$t_out\".split(\" \")[0])"`
                comp_seconds=`python3 -c "print(\"$t_out\".split(\" \")[1])"`
                orig_size=`wc -c $IMGDIR/$input.raw | cut -d " " -f 1`
                new_size=`wc -c $TMPDIR/$input_tmp.raw | cut -d " " -f 1`
                comp_ration=`python3 -c "print($new_size / $orig_size)"`

                t_out=`TIME="%M %e" time $BINARY -d -i $TMPDIR/$input_tmp.raw -o $TMPDIR/$input.raw -h $mode $model -$effort 2>&1`
                decomp_max_kbytes=`python3 -c "print(\"$t_out\".split(\" \")[0])"`
                decomp_seconds=`python3 -c "print(\"$t_out\".split(\" \")[1])"`

                echo "$input;$mode;$model_str;$effort;$comp_max_kbytes;$comp_seconds;$decomp_max_kbytes;$decomp_seconds;$orig_size;$new_size;$comp_ration"
            done
        done
    done
done

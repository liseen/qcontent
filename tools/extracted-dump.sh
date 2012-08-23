#!/bin/env bash


export PATH=/opt/qcontent/bin:$PATH
export LD_LIBRARY_PATH=/opt/qcontent/lib:$LD_LIBRARY_PATH

BIN=$(readlink -f -- $(dirname -- "$0"))

DUMP_DIR=/export/m4/extracted-dump
test -d $DUMP_DIR || mkdir -p $DUMP_DIR

cd $DUMP_DIR

while true
do
    now_date=`date +%s`
    extracted_file="$now_date.extracted"
    echo -e "dump_extracted\t5000" | hubmgr -h localhost > "$extracted_file" 2>"$extracted_file.err"
    if [ ! -s "$extracted_file.err" ];
    then
        rm  -rf "$extracted_file.err"
    fi
    sleep  30
done



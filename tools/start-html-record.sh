#!/usr/bin/env bash

export TZ=Asia/Shanghai
export LC_ALL='en_US.UTF8'

export LD_LIBRARY_PATH=/opt/qcontent/lib:$LD_LIBRARY_PATH
export PATH=/opt/qcontent/bin:$PATH

ROOT_DIR=/export/m1
DATA_DIR=$ROOT_DIR/qcrawler-html/data
LOG_DIR=$ROOT_DIR/qcrawler-html/log
ULOG_DIR=$ROOT_DIR/qcrawler-html/ulog

test -d $DATA_DIR || mkdir -p $DATA_DIR
test -d $LOG_DIR || mkdir -p $LOG_DIR
test -d $ULOG_DIR || mkdir -p $ULOG_DIR

LOG_FILE=$LOG_DIR/qcrawler-html.log
DAEMON="-le -log ${LOG_FILE}"

exec setuidgid xunxin.wan ttserver -port 9860 -sid 60 -thnum 32 -mul 16 ${DAEMON} \
    "${DATA_DIR}/qcrawler-html.tch#bnum=100000000#opts=ld#rcnum=5000#xmsiz=134217728"

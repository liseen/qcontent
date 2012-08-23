#!/usr/bin/env bash

export TZ=Asia/Shanghai
export LC_ALL='en_US.UTF8'

export LD_LIBRARY_PATH=/opt/qcrawler-thirdparty/lib:$LD_LIBRARY_PATH
export PATH=/opt/qcrawler-thirdparty/bin:$PATH

ROOT_DIR=/tmp
DATA_DIR=$ROOT_DIR/qcrawler-html/data
LOG_DIR=$ROOT_DIR/qcrawler-html/log
ULOG_DIR=$ROOT_DIR/qcrawler-html/ulog


test -d $DATA_DIR || mkdir -p $DATA_DIR
test -d $LOG_DIR || mkdir -p $LOG_DIR
test -d $ULOG_DIR || mkdir -p $ULOG_DIR

LOG_FILE=$LOG_DIR/qcrawler-html.log
DAEMON="-dmn -pid ${LOG_DIR}/qcrawler-html.pid  -le -log ${LOG_FILE}"
exec ttserver -port 9860 "${DATA_DIR}/qcrawler-html.tch#bnum=100000#opts=ld#rcnum=500#xmsiz=536870"

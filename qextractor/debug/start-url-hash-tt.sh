#!/usr/bin/env bash

export TZ=Asia/Shanghai
export LC_ALL='en_US.UTF8'

export LD_LIBRARY_PATH=/opt/qcrawler-thirdparty/lib:$LD_LIBRARY_PATH
export PATH=/opt/qcrawler-thirdparty/bin:$PATH

DATA_DIR=/tmp/qcrawler/data
LOG_DIR=/tmp/qcrawler/log

test -d $DATA_DIR || mkdir -p $DATA_DIR
test -d $LOG_DIR || mkdir -p $LOG_DIR

LOG_FILE=$LOG_DIR/qcrawler-url.log

exec ttserver -port 9850 -ld "${DATA_DIR}/qcrawler-url.tch#bnum=100000#opts=l#rcnum=500#opts=l#xmsiz=536870"

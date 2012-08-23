#!/usr/bin/env bash

export TZ=Asia/Shanghai
export LC_ALL='en_US.UTF8'

export LD_LIBRARY_PATH=/opt/qcontent/lib:$LD_LIBRARY_PATH
export PATH=/opt/qcontent/bin:$PATH

ROOT_DIR=/export/m2/
DATA_DIR=$ROOT_DIR/qcrawler-url/data
LOG_DIR=$ROOT_DIR/qcrawler-url/log

test -d $DATA_DIR || mkdir -p $DATA_DIR
test -d $LOG_DIR || mkdir -p $LOG_DIR

LOG_FILE=$LOG_DIR/qcrawler-url.log


DAEMON="-dmn -pid ${LOG_DIR}/qcrawler-url.pid -le -log ${LOG_FILE} "
exec ttserver -port 9850 -thnum 40 $DAEMON "${DATA_DIR}/qcrawler-url.tch#bnum=400000000#opts=l#rcnum=5000#xmsiz=4294967296"

#!/bin/sh

export TZ=Asia/Shanghai
export LC_ALL='en_US.UTF8'

export LD_LIBRARY_PATH=/opt/qcrawler-thirdparty/lib:$LD_LIBRARY_PATH
export PATH=/opt/qcrawler-thirdparty/bin:$PATH

ROOT_DIR=/export/m3/
DATA_DIR=$ROOT_DIR/qcrawler-record/data
LOG_DIR=$ROOT_DIR/qcrawler-record/log
ULOG_DIR=$ROOT_DIR/qcrawler-record/ulog

test -d $DATA_DIR || mkdir -p $DATA_DIR
test -d $LOG_DIR || mkdir -p $LOG_DIR
test -d $ULOG_DIR || mkdir -p $ULOG_DIR

LOG_FILE=$LOG_DIR/qcrawler-record.log
RTS_FILE=$DATA_DIR/qcrawler-record/71.rts

DAEMON="-dmn -pid ${LOG_DIR}/qcrawler-record.pid -le -log ${LOG_FILE} "

exec ttserver -port 9870 -ulog ${ULOG_DIR} -sid 71 -thnum 32 -mul 16 ${DAEMON} -mhost 192.168.10.176 -mport 9870 -rts $RTS_FILE \
    "${DATA_DIR}/qcrawler-record.tct#bnum=10000000#opts=lb#rcnum=2000#xmsiz=536870912#idx=download_time:dec#idx=host:lex"

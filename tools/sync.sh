#!/usr/bin/env bash

BIN=$(readlink -f -- $(dirname -- "$0"))
cd $BIN/..

test -f Makefile && make distclean

rsync  --exclude 'pipeline/*' --exclude '*.t' --exclude '*.vdom' --exclude '*~' --exclude '*.tgz'  --exclude '*.dat' --exclude '*.data' -auvz * xunxin.wan@l-crwl1.s.cn5.qunar.com:/home/xunxin.wan/qcontent/

#!/usr/bin/env bash

BIN=$(readlink -f -- $(dirname -- "$0"))
cd $BIN/..

test -f Makefile && make distclean

rsync --exclude '*~' -auvz * xunxin.wan@crwl3:/home/xunxin.wan/qcontent/

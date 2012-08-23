#!/usr/bin/env bash

BIN=$(readlink -f -- $(dirname -- "$0"))
cd $BIN/..

test -f Makefile && make distclean

rsync --exclude 'pipeline/*' --exclude '*~' --exclude '*.t' --exclude '*.vdom' -auvz * xunxin.wan@l-crwl12.s.cn5.qunar.com:/home/xunxin.wan/qcontent/

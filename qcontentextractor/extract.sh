#!/bin/sh

BIN=$(readlink -f -- $(dirname -- "$0"))

cd $BIN/

GLOG_minloglevel=1 $BIN/extract $@

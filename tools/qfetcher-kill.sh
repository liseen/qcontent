#!/bin/env bash

PATH=$PATH:$HOME/bin
PATH=/opt/perl/bin/:$PATH
#PATH=/opt/qcrawler-thirdparty/bin:$PATH
PATH=/opt/qt/bin:$PATH
PATH=/opt/qcontent/bin:$PATH

export PATH
export LD_LIBRARY_PATH=/opt/qcontent/lib:$LD_LIBRARY_PATH

atnodes 'pgrep qfetcher > /tmp/qfetcher_kill.pid' l-crwl[2-7]
sleep 1
atnodes 'kill `cat /tmp/qfetcher_kill.pid`' l-crwl[2-7]
sleep 300;
atnodes 'kill -9 `cat /tmp/qfetcher_kill.pid`' l-crwl[2-7]


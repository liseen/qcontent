#!/bin/env bash

PATH=$PATH:$HOME/bin
PATH=/opt/perl/bin/:$PATH
#PATH=/opt/qcrawler-thirdparty/bin:$PATH
PATH=/opt/qt/bin:$PATH
PATH=/opt/qcontent/bin:$PATH

export PATH
export LD_LIBRARY_PATH=/opt/qcontent/lib:$LD_LIBRARY_PATH

atnodes 'pkill qcrawler' l-crwl[2-7]
#atnodes -tty 'sudo /etc/init.d/named restart' l-crwl[2-6]

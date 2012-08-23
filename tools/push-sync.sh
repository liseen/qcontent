#!/bin/env bash


BIN=$(readlink -f -- $(dirname -- "$0"))

cd $BIN/../

#atnodes -tty 'sudo chown xunxin.wan:qunarengineer /opt/qcontent' 'l-crwl[2-7,9,10]'
for host in  3 4 5 6 7 9 10
do
    rsync --exclude ’*~’ --exclude '*haproxy.cfg' -auvz  bin  bin32  etc include  include32  lib  lib32  libexec  man  sbin  share  tools  var xunxin.wan@l-crwl$host:/opt/qcontent/
done

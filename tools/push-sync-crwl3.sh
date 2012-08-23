#!/bin/env bash


BIN=$(readlink -f -- $(dirname -- "$0"))

cd $BIN/../

#atnodes -tty 'sudo chown xunxin.wan:qunarengineer /opt/qcontent' 'l-crwl[2-6]'
for host in `seq 4 6`
do
    rsync --exclude ’*~’ -auvz  bin  bin32  etc  include  include32  lib  lib32  libexec  man  sbin  share  tools  var xunxin.wan@l-crwl$host:/opt/qcontent/
done

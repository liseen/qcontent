#!/bin/sh

source /home/$USER/.bash_profile
source /opt/qunar-tools/qbash.sh
source /opt/qunar-tools/qlog.sh

execCmd "atnodes 'find /export/m4/extracted-dump/ -name \"*.extracted*\" -mtime +14 -print -delete' l-crwl[2-7]"
execCmd "atnodes 'find /export/m4/newurl-dump/ -name \"*.newurl*\" -mtime +14 -print -delete' l-crwl[2-7]"
execCmd "atnodes 'find /export/m4/*log -name \"*.log*\" -a -mtime +30 -print -delete' l-crwl[1-7]"

infoOut "DONE: $0"


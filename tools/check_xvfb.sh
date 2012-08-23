#!/bin/sh

atnodes 'top -b -n 1 -p `pgrep Xvfb` | grep Xvfb' -L l-crwl[1-7]

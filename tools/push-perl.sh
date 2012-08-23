#!/bin/sh

export PATH=/opt/perl/bin:$PATH
tonodes -r /opt/perl/ l-crwl[2-7,9-10]:/opt/perl -rsync -archive -update -compress

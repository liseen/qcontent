#!/usr/bin/env bash

BIN=$(readlink -f -- $(dirname -- "$0"))
rm -f /opt/qcontent/etc/haproxy.cfg

ln -s /opt/qcontent/etc/haproxy/haproxy.cfg.`hostname -s` /opt/qcontent/etc/haproxy.cfg

#!/bin/sh

BIN=$(readlink -f -- $(dirname -- "$0"))

DIST=qcontent-1.0.0.tar.gz
rm $BIN/qcontent-*.tar.gz
rm $BIN/qc-qcontent-*.rpm

cd "$BIN"/../../
tar --exclude '*.svn*' --exclude '*pipeline*' --exclude '*qurldb*' --exclude '*.vdom' -czvf $DIST qcontent/
mv $DIST qcontent/spec/
cd "$BIN"

sudo cp qcontent-1.0.0.tar.gz /usr/src/redhat/SOURCES/qcontent-1.0.0.tar.gz

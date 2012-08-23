INSTALLDIR=/opt/qcontent/

DESTDIR=$$PWD/.objs

INCLUDEPATH +=  $$PWD \
    $$PWD/qcontentcommon \
    $$INSTALLDIR/include \
    $$INCLUDEPATH

LIBS += -Wl,-rpath-link,$$PWD/.objs/ -L$$PWD/.objs/ \
        -Wl,-rpath,$$INSTALLDIR/lib/ -L$$INSTALLDIR/lib

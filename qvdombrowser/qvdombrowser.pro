TEMPLATE = app

SOURCES += iteratorconfigdialog.cpp \
           aboutdialog.cpp \
           hunterconfigdialog.cpp \
           lineedit.cpp \
           urlloader.cpp \
           mainwindow.cpp \
           webview.cpp \
           webpage.cpp \
           fielddialog.cpp \
           main.cpp
HEADERS += iterator.h \
           iteratorconfigdialog.h \
           aboutdialog.h \
           hunterconfigdialog.h \
           version.h \
           lineedit.h \
           urlloader.h \
           mainwindow.h \
           webview.h \
           webpage.h \
           fielddialog.h

include($$PWD/../qcontent.pri)
INCLUDEPATH += ../qwebvdom

CONFIG -= app_bundle
CONFIG += qt warn_on uitools
DESTDIR = $$PWD
LIBS += -lvdom

BASE_DIR = $$PWD
QT+=xml network webkit
QMAKE_RPATHDIR = $$OUTPUT_DIR/lib $$QMAKE_RPATHDIR

isEmpty(OUTPUT_DIR) {
    CONFIG(release):OUTPUT_DIR=$$PWD
    CONFIG(debug):OUTPUT_DIR=$$PWD
}

DEFINES += USE_SYSTEM_MALLOC
CONFIG(release) {
    DEFINES += NDEBUG
}
DEFINES += BUILDING_QT__=1

isEmpty(QJSON_DIR) {
} else {
    INCLUDEPATH = $$QJSON_DIR/include $$INCLUDEPATH
    QMAKE_LIBDIR_FLAGS += -L$$QJSON_DIR/lib
    QMAKE_RPATHDIR += $$QJSON_DIR/lib
}

INCLUDEPATH += $$QTDIR/include/QtWebKit

CONFIG += $$(QT_WEBKIT)
webkit_trunk {
    WEBKITDIR = $$(WEBKITDIR)
    WEBKITBRANCH = $$(WEBKITBRANCH)
    isEmpty(WEBKITBRANCH) {
        CONFIG(release):WEBKITBUILD = $$WEBKITDIR/WebKitBuild/Release/lib
        CONFIG(debug):WEBKITBUILD = $$WEBKITDIR/WebKitBuild/Debug/lib
    } else {
        CONFIG(release):WEBKITBUILD = $$WEBKITDIR/WebKitBuild/$$WEBKITBRANCH/Release/lib
        CONFIG(debug):WEBKITBUILD = $$WEBKITDIR/WebKitBuild/$$WEBKITBRANCH/Debug/lib
    }
    message(Using WebKit Trunk at $$WEBKITDIR)
    message(Using WebKit Build at $$WEBKITBUILD)
    QT -= webkit
    DEFINES += WEBKIT_TRUNK
    QMAKE_LIBDIR_FLAGS += -L$$WEBKITBUILD
    LIBS += -L$$WEBKITBUILD -lQtWebKit
    INCLUDEPATH = $$WEBKITDIR/WebKit/qt/Api $$INCLUDEPATH
    QMAKE_RPATHDIR = $$WEBKITBUILD $$QMAKE_RPATHDIR
}

target.path = $$OUTPUT_DIR/bin
INSTALLS += target


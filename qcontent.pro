TEMPLATE=subdirs

VERSION = 1.0.0
DISTFILES += qcontent.pri
CONFIG += ordered

SUBDIRS = googleurl\
        qhost2site\
        qnormalurl\
        qcontentcommon\
        qdedup_rpc\
        qcontenthub\
        libqcontenthub\
        qurlqueue\
        libqurlqueue\
        hubmgr\
        uqmgr\
        qhtml2vdom\
        qstore\
        libqtitlefilter\
        qcontentextractor\
        qurlfilter\
        qcrawler\
        qurlparse\
        qthumbscale\
        qws\
        libqws\
        qpredict\
        libqpredict\
        qextractor\
        qtitlefilter


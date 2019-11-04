QT += core gui widgets xml
CONFIG += dll warn_on

TEMPLATE = lib
TARGET = rastercanvas

SOURCES = rastermainwindow.cpp \
          mypaintview.cpp \
          mpbrushselector.cpp

HEADERS = rastermainwindow.h \
          mypaintview.h \
          mpbrushselector.h

# --- QTMyPaint ---
win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../qtmypaint/release/ -lqtmypaint
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../qtmypaint/debug/ -lqtmypaint
else:unix: LIBS += -L$$OUT_PWD/../qtmypaint/ -lqtmypaint

INCLUDEPATH += ../libmypaint
INCLUDEPATH += ../qtmypaint
DEPENDPATH += ../qtmypaint

FRAMEWORK_DIR = "../../framework"
include($$FRAMEWORK_DIR/framework.pri)

unix {
    INSTALLS += target
    target.path = /lib/raster

    !include(../../../tupiglobal.pri) {
        error("Run ./configure first!")
    }
}

win32:CONFIG(release, debug|release): LIBS += -L../json-c/release -ljson-c
else:win32:CONFIG(debug, debug|release): LIBS += -L../json-c/debug -ljson-c
else:unix: LIBS += -L../json-c -ljson-c

# win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../qtmypaint/release/libqtmypaint.a
# else:win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../qtmypaint/debug/libqtmypaint.a
# else:win32:!win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../qtmypaint/release/qtmypaint.lib
# else:win32:!win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../qtmypaint/debug/qtmypaint.lib
# else:unix: PRE_TARGETDEPS += $$OUT_PWD/../qtmypaint/libqtmypaint.a

RESOURCES += resources.qrc

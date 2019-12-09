QT += core gui widgets

TEMPLATE = lib
CONFIG += dll warn_on

TARGET = qtmypaint

HEADERS += mpbrush.h \
           mphandler.h \
           mpsurface.h \
           mptile.h

SOURCES += mpbrush.cpp \
           mphandler.cpp \
           mpsurface.cpp \
           mptile.cpp

FRAMEWORK_DIR = "../../framework"
include($$FRAMEWORK_DIR/framework.pri)

LIBS += -L../json-c -ljson-c
LIBS += -L../libmypaint -llibmypaint

# --- json-c ---
win32:CONFIG(release, debug|release): LIBS += -L../json-c/release/ -ljson-c
else:win32:CONFIG(debug, debug|release): LIBS += -L../json-c/debug/ -ljson-c
else:unix: LIBS += -L../json-c -ljson-c

INCLUDEPATH += ../json-c
DEPENDPATH += ../json-c

# --- libmypaint ---
win32:CONFIG(release, debug|release): LIBS += -L../libmypaint/release/ -llibmypaint
else:win32:CONFIG(debug, debug|release): LIBS += -L../libmypaint/debug/ -llibmypaint
else:unix: LIBS += -L../libmypaint -llibmypaint

INCLUDEPATH += ../libmypaint
DEPENDPATH += ../libmypaint

macx {
    INSTALLS += target
    target.path = /lib

    !include(../../../tupiglobal.pri) {
        error("Run ./configure first!")
    }
}

unix:!mac {
    INSTALLS += target
    target.path = /lib/raster

    !include(../../../tupiglobal.pri) {
        error("Run ./configure first!")
    }
}   

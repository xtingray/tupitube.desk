QT += core gui widgets

TEMPLATE = lib
CONFIG += dll warn_on

TARGET = libmypaint

macx {
    INSTALLS += target
    target.path = /lib
	
    INCLUDEPATH += ../json-c
    LIBS += -L../json-c -ljson-c

    !include(../../../tupiglobal.pri) {
        error("Run ./configure first!")
    }
}

unix:!mac {
    INSTALLS += target
    target.path = /lib/raster
	
    INCLUDEPATH += ../json-c
    LIBS += -L../json-c -ljson-c

    !include(../../../tupiglobal.pri) {
        error("Run ./configure first!")
    }
}   

win32 {
    include(../../../win.pri)
    INCLUDEPATH += ../json-c
    LIBS += -L../json-c/release -ljson-c
}

HEADERS += brushmodes.h \
           brushsettings-gen.h \
           fifo.h \
           helpers.h \
           mapping.h \
           mypaint.h \
           mypaint-brush.h \
           mypaint-brush-settings.h \
           mypaint-brush-settings-gen.h \
           mypaint-config.h \
           mypaint-config.h.in \
           mypaint-fixed-tiled-surface.h \
           mypaint-glib-compat.h \
           mypaint-rectangle.h \
           mypaint-surface.h \
           mypaint-tiled-surface.h \
           operationqueue.h \
           rng-double.h \
           tiled-surface-private.h \
           tilemap.h \
           utils.h

SOURCES += brushmodes.c \
           fifo.c \
           helpers.c \
           libmypaint.c \
           mapping.c \
           mypaint.c \
           mypaint-brush.c \
           mypaint-brush-settings.c \
           mypaint-fixed-tiled-surface.c \
           mypaint-rectangle.c \
           mypaint-surface.c \
           mypaint-tiled-surface.c \
           operationqueue.c \
           rng-double.c \
           tilemap.c \
           utils.c

# for C files, we need to allow C99 mode.
QMAKE_CFLAGS += -std=c99
QMAKE_CFLAGS += -Wno-unknown-pragmas

FRAMEWORK_DIR = "../../framework"
include($$FRAMEWORK_DIR/framework.pri)


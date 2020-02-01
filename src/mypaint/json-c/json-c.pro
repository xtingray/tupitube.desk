TEMPLATE = lib
CONFIG += dll warn_on

TARGET = json-c

HEADERS += arraylist.h \
           bits.h \
           debug.h \
           json.h \
           json_c_version.h \
           json_inttypes.h \
           json_object.h \
           json_object_iterator.h \
           json_object_private.h \
           json_tokener.h \
           json_util.h \
           linkhash.h \
           math_compat.h \
           printbuf.h \
           random_seed.h \
           config.h \
           json_config.h

SOURCES += arraylist.c \
           debug.c \
           json_c_version.c \
           json_object.c \
           json_object_iterator.c \
           json_tokener.c \
           json_util.c \
           libjson.c \
           linkhash.c \
           printbuf.c \
           random_seed.c

# for C files, we need to allow C99 mode.
QMAKE_CFLAGS += -std=c99
QMAKE_CFLAGS += -D_XOPEN_SOURCE=600

FRAMEWORK_DIR = "../../framework"
include($$FRAMEWORK_DIR/framework.pri)

win32 {
    include(../../../win.pri)
}

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

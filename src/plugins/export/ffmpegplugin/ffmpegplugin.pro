QT += opengl core gui svg xml network
CONFIG += plugin warn_on
TEMPLATE = lib
TARGET = tupiffmpegplugin

unix {
    !include(../../../../tupiglobal.pri) {
        error("Please run configure first")
    }
}

win32 {
    include(../../../../win.pri)
    include(../../../../ffmpeg.win.pri)
}

mac {
    LIBS += -lavutil
}

INSTALLS += target
target.path = /plugins/

HEADERS += tffmpegmoviegenerator.h \
           ffmpegplugin.h

SOURCES += tffmpegmoviegenerator.cpp \
           ffmpegplugin.cpp

FRAMEWORK_DIR = "../../../framework"
include($$FRAMEWORK_DIR/framework.pri)

LIBBASE_DIR = ../../../libbase
STORE_DIR = ../../../store
LIBTUPI_DIR = ../../../libtupi

include($$LIBBASE_DIR/libbase.pri)
include($$STORE_DIR/store.pri)
include($$LIBTUPI_DIR/libtupi.pri)


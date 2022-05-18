QT += opengl core gui svg xml network
CONFIG += plugin warn_on
TEMPLATE = lib
TARGET = tupiffmpegplugin

unix {
    !include(../../../../tupiglobal.pri) {
        error("Please run configure first")
    }
}

INSTALLS += target
target.path = /plugins/

contains("DEFINES", "ADD_HEADERS") {
    INSTALLS += headers
    headers.target = .
    headers.commands = /bin/cp *.h $(INSTALL_ROOT)/include/tupiffmpeg
    headers.path = /include/tupiffmpeg
}

win32 {
    include(../../../../win.pri)
    include(../../../../ffmpeg.win.pri)
}

mac {
    LIBS += -lavutil
}

HEADERS += tupaudiomixer.h \
           tupaudiotranscoder.h \
           tffmpegmoviegenerator.h \
           ffmpegplugin.h

SOURCES += tupaudiomixer.cpp \
           tupaudiotranscoder.cpp \
           tffmpegmoviegenerator.cpp \
           ffmpegplugin.cpp

FRAMEWORK_DIR = "../../../framework"
include($$FRAMEWORK_DIR/framework.pri)

LIBBASE_DIR = ../../../libbase
STORE_DIR = ../../../store
LIBTUPI_DIR = ../../../libtupi

include($$LIBBASE_DIR/libbase.pri)
include($$STORE_DIR/store.pri)
include($$LIBTUPI_DIR/libtupi.pri)


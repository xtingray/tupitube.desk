QT += widgets svgwidgets opengl core gui svg xml network core5compat
CONFIG += plugin warn_on
TEMPLATE = lib
TARGET = tupipapagayotool

unix {
    !include(../../../../tupiglobal.pri){
        error("Please run configure first")
    }
}

win32 {
    include(../../../../win.pri)
}

INSTALLS += target 
target.path = /plugins/

FRAMEWORK_DIR = "../../../framework"
include($$FRAMEWORK_DIR/framework.pri)

HEADERS += papagayosettings.h \
           # buttonspanel.h \
           papagayoconfigurator.h \
           lipsyncmanager.h \
           papagayotool.h \
           mouthsdialog.h

SOURCES += papagayosettings.cpp \
           # buttonspanel.cpp \
           papagayoconfigurator.cpp \
           lipsyncmanager.cpp \
           papagayotool.cpp \
           mouthsdialog.cpp

LIBBASE_DIR = ../../../libbase
STORE_DIR = ../../../store
LIBTUPI_DIR = ../../../libtupi
COMMON_DIR = ../common

include($$LIBBASE_DIR/libbase.pri)	
include($$STORE_DIR/store.pri)
include($$LIBTUPI_DIR/libtupi.pri)
include($$COMMON_DIR/common.pri)

QT += opengl core gui svg xml network
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
           buttonspanel.h \
           configurator.h \
           lipsyncmanager.h \
           papagayotool.h 

SOURCES += papagayosettings.cpp \
           buttonspanel.cpp \
           configurator.cpp \
           lipsyncmanager.cpp \
           papagayotool.cpp

LIBBASE_DIR = ../../../libbase
STORE_DIR = ../../../store
LIBTUPI_DIR = ../../../libtupi

include($$LIBBASE_DIR/libbase.pri)	
include($$STORE_DIR/store.pri)
include($$LIBTUPI_DIR/libtupi.pri)

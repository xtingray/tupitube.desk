QT += opengl core gui svg xml network
TEMPLATE = lib
TARGET = rastercolor

CONFIG += dll warn_on

HEADERS += rastercolorwidget.h
SOURCES += rastercolorwidget.cpp 

FRAMEWORK_DIR = "../../../framework"
include($$FRAMEWORK_DIR/framework.pri)

LIBCOLOR_DIR = "../../../libcolor"
include($$LIBCOLOR_DIR/libcolor.pri)

STORE_DIR = ../../../store/
INCLUDEPATH += $$STORE_DIR
LIBS += -L$$STORE_DIR -ltupistore

LIBBASE_DIR = ../../../libbase/
INCLUDEPATH += $$LIBBASE_DIR
LIBS += -L$$LIBBASE_DIR -ltupibase

LIBTUPI_DIR = ../../../libtupi/
INCLUDEPATH += $$LIBTUPI_DIR
LIBS += -L$$LIBTUPI_DIR -ltupi

macx {
    INSTALLS += target
    target.path = /lib

    !include(../../../../tupiglobal.pri) {
             error("Run ./configure first!")
    }
}

unix:!mac {
    INSTALLS += target
    target.path = /lib/raster

    !include(../../../../tupiglobal.pri) {
             error("Run ./configure first!")
    }
}

win32 {
    include(../../../../win.pri)
}

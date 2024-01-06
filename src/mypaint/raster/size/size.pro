QT += opengl core gui svg xml network
TEMPLATE = lib
TARGET = rastersize

CONFIG += dll warn_on

HEADERS += rastersizewidget.h
SOURCES += rastersizewidget.cpp 

FRAMEWORK_DIR = "../../../framework"
include($$FRAMEWORK_DIR/framework.pri)

LIBCOLOR_DIR = "../../../libcolor"
include($$LIBCOLOR_DIR/libcolor.pri)

macx {
    INSTALLS += target
    target.path = /lib

	STORE_DIR = ../../../store/
    INCLUDEPATH += $$STORE_DIR
    LIBS += -L$$STORE_DIR -ltupistore

    LIBBASE_DIR = ../../../libbase/
    INCLUDEPATH += $$LIBBASE_DIR
    LIBS += -L$$LIBBASE_DIR -ltupibase

    LIBTUPI_DIR = ../../../libtupi/
    INCLUDEPATH += $$LIBTUPI_DIR
    LIBS += -L$$LIBTUPI_DIR -ltupi
	
    !include(../../../../tupiglobal.pri) {
             error("Run ./configure first!")
    }
}

unix:!mac {
    INSTALLS += target
    target.path = /lib/raster

    STORE_DIR = ../../../store/
    INCLUDEPATH += $$STORE_DIR
    LIBS += -L$$STORE_DIR -ltupistore

    LIBBASE_DIR = ../../../libbase/
    INCLUDEPATH += $$LIBBASE_DIR
    LIBS += -L$$LIBBASE_DIR -ltupibase

    LIBTUPI_DIR = ../../../libtupi/
    INCLUDEPATH += $$LIBTUPI_DIR
    LIBS += -L$$LIBTUPI_DIR -ltupi
	
    !include(../../../../tupiglobal.pri) {
             error("Run ./configure first!")
    }
}

win32 {
    include(../../../../win.pri)

	STORE_DIR = ../../../store
    INCLUDEPATH += $$STORE_DIR
    LIBS += -L$$STORE_DIR/release -ltupistore

    LIBBASE_DIR = ../../../libbase
    INCLUDEPATH += $$LIBBASE_DIR
    LIBS += -L$$LIBBASE_DIR/release -ltupibase

    LIBTUPI_DIR = ../../../libtupi
    INCLUDEPATH += $$LIBTUPI_DIR
    LIBS += -L$$LIBTUPI_DIR/release -ltupi
}

QT += opengl core gui svg xml network multimedia
TEMPLATE = lib
TARGET = tupilibrary

INSTALLS += target
target.path = /lib/

CONFIG += dll warn_on

HEADERS += tupsounddialog.h \
           tuplibrarywidget.h \
           tupitemmanager.h  \
           tupsymboleditor.h \
           tuptreedelegate.h \
           tupnewitemdialog.h \
           tuplibrarydisplay.h \
           tupsoundplayer.h \
           tupsearchdialog.h \
           tupvideocutter.h \
           tupvideoimporterdialog.h 

SOURCES += tupsounddialog.cpp \
           tuplibrarywidget.cpp \
           tupitemmanager.cpp  \
           tupsymboleditor.cpp \
           tuptreedelegate.cpp \
           tupnewitemdialog.cpp \
           tuplibrarydisplay.cpp \
           tupsoundplayer.cpp \
           tupsearchdialog.cpp \
           tupvideocutter.cpp \
           tupvideoimporterdialog.cpp
           
FRAMEWORK_DIR = "../../framework"
include($$FRAMEWORK_DIR/framework.pri)

MICMANAGER_DIR = ../micmanager/
include($$MICMANAGER_DIR/micmanager.pri)

unix {
    STORE_DIR = ../../store/
    INCLUDEPATH += $$STORE_DIR
    LIBS += -L$$STORE_DIR -ltupistore

    LIBBASE_DIR = ../../libbase/
    INCLUDEPATH += $$LIBBASE_DIR
    LIBS += -L$$LIBBASE_DIR -ltupibase

    LIBTUPI_DIR = ../../libtupi/
    INCLUDEPATH += $$LIBTUPI_DIR
    LIBS += -L$$LIBTUPI_DIR -ltupi

    # LIBS += -L$$MICMANAGER_DIR -ltupmicmanager

    !include(../../../tupiglobal.pri) {
             error("Run ./configure first!")
    }
}

win32 {
    include(../../../win.pri)

    STORE_DIR = ../../store/
    INCLUDEPATH += $$STORE_DIR
    LIBS += -L$$STORE_DIR/release/ -ltupistore

    LIBBASE_DIR = ../../libbase/
    INCLUDEPATH += $$LIBBASE_DIR
    LIBS += -L$$LIBBASE_DIR/release/ -ltupibase

    LIBTUPI_DIR = ../../libtupi/
    INCLUDEPATH += $$LIBTUPI_DIR
    LIBS += -L$$LIBTUPI_DIR/release/ -ltupi 

    # LIBS += -L$$MICMANAGER_DIR/release/ -ltupmicmanager
}

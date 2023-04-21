QT += widgets opengl core gui svg xml network
TEMPLATE = lib
TARGET = tupicolor

INSTALLS += tpal 

tpal.target = palettes/*.tpal
tpal.commands = test -d $(INSTALL_ROOT)/data/palettes/ || mkdir $(INSTALL_ROOT)/data/palettes; \
                cp palettes/*.tpal $(INSTALL_ROOT)/data/palettes/
tpal.path = /data/palettes/

INSTALLS += target
target.path = /lib/

CONFIG += dll warn_on

HEADERS += tupcolorpicker.h \
           tupviewcolorcells.h \
           tupcellscolor.h \
           tuppaletteparser.h \
           tupformitem.h \
           tupcolorform.h \
           tupcolorbutton.h \
           tupcolorbuttonpanel.h

SOURCES += tupcolorpicker.cpp \
           tupviewcolorcells.cpp \
           tupcellscolor.cpp \
           tuppaletteparser.cpp \
           tupformitem.cpp \
           tupcolorform.cpp \
           tupcolorbutton.cpp \
           tupcolorbuttonpanel.cpp

FRAMEWORK_DIR = "../framework"
include($$FRAMEWORK_DIR/framework.pri)

unix {
    STORE_DIR = ../store/
    INCLUDEPATH += $$STORE_DIR
    LIBS += -L$$STORE_DIR -ltupistore

    LIBBASE_DIR = ../libbase/
    INCLUDEPATH += $$LIBBASE_DIR
    LIBS += -L$$LIBBASE_DIR -ltupibase

    LIBTUPI_DIR = ../libtupi/
    INCLUDEPATH += $$LIBTUPI_DIR
    LIBS += -L$$LIBTUPI_DIR -ltupi

    !include(../../tupiglobal.pri) {
             error("libcolor: Run ./configure first!")
    }
}

win32 {
    include(../../win.pri)

    STORE_DIR = ../store/
    INCLUDEPATH += $$STORE_DIR
    LIBS += -L$$STORE_DIR/release/ -ltupistore

    LIBBASE_DIR = ../libbase/
    INCLUDEPATH += $$LIBBASE_DIR
    LIBS += -L$$LIBBASE_DIR/release/ -ltupibase

    LIBTUPI_DIR = ../libtupi/
    INCLUDEPATH += $$LIBTUPI_DIR
    LIBS += -L$$LIBTUPI_DIR/release/ -ltupi 
}

QT += core gui widgets xml
CONFIG += dll warn_on

TEMPLATE = lib
TARGET = rastermain

SOURCES = rastermainwindow.cpp \
          rastercanvasbase.cpp \
          rastercanvas.cpp

HEADERS = rastermainwindow.h \
          rastercanvasbase.h \
          rastercanvas.h

# --- QTMyPaint ---
win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../../qtmypaint/release/ -lqtmypaint
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../../qtmypaint/debug/ -lqtmypaint
else:unix: LIBS += -L$$OUT_PWD/../../qtmypaint/ -lqtmypaint

INCLUDEPATH += ../../libmypaint
INCLUDEPATH += ../../qtmypaint

INCLUDEPATH += ../brushes
LIBS += -L../brushes -lrasterbrushes

INCLUDEPATH += ../color
LIBS += -L../color -lrastercolor

# DEPENDPATH += ../../qtmypaint

FRAMEWORK_DIR = "../../../framework"
include($$FRAMEWORK_DIR/framework.pri)

LIBCOLOR_DIR = "../../../libcolor"
include($$LIBCOLOR_DIR/libcolor.pri)

macx {
    INSTALLS += target
    target.path = /lib

    INSTALLS += brushes
    brushes.target = .
    brushes.commands = cp -r brushes $(INSTALL_ROOT)/themes/raster
    brushes.path = /themes/raster/brushes

    INSTALLS += resources
    resources.target = .
    resources.commands = cp -r resources $(INSTALL_ROOT)/themes/raster
    resources.path = /themes/raster/resources

    STORE_DIR = ../../../store/
    INCLUDEPATH += $$STORE_DIR
    LIBS += -L$$STORE_DIR/ -ltupistore

    LIBBASE_DIR = ../../../libbase/
    INCLUDEPATH += $$LIBBASE_DIR
    LIBS += -L$$LIBBASE_DIR/ -ltupibase

    LIBTUPI_DIR = ../../../libtupi/
    INCLUDEPATH += $$LIBTUPI_DIR
    LIBS += -L$$LIBTUPI_DIR/ -ltupi

    !include(../../../../tupiglobal.pri) {
        error("Run ./configure first!")
    }
}

unix:!mac {
    INSTALLS += target
    target.path = /lib/raster

    INSTALLS += brushes
    brushes.target = .
    brushes.commands = cp -r brushes $(INSTALL_ROOT)/themes/raster
    brushes.path = /themes/raster/brushes

    INSTALLS += resources
    resources.target = .
    resources.commands = cp -r resources $(INSTALL_ROOT)/themes/raster
    resources.path = /themes/raster/resources

    STORE_DIR = ../../../store/
    INCLUDEPATH += $$STORE_DIR
    LIBS += -L$$STORE_DIR/release/ -ltupistore

    LIBBASE_DIR = ../../../libbase/
    INCLUDEPATH += $$LIBBASE_DIR
    LIBS += -L$$LIBBASE_DIR/release/ -ltupibase

    LIBTUPI_DIR = ../../../libtupi/
    INCLUDEPATH += $$LIBTUPI_DIR
    LIBS += -L$$LIBTUPI_DIR/release/ -ltupi

    !include(../../../../tupiglobal.pri) {
        error("Run ./configure first!")
    }
}

win32 {
    include(../../../../win.pri)
}

win32:CONFIG(release, debug|release): LIBS += -L../../json-c/release -ljson-c
else:win32:CONFIG(debug, debug|release): LIBS += -L../../json-c/debug -ljson-c
else:unix: LIBS += -L../../json-c -ljson-c

# win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../qtmypaint/release/libqtmypaint.a
# else:win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../qtmypaint/debug/libqtmypaint.a
# else:win32:!win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../qtmypaint/release/qtmypaint.lib
# else:win32:!win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../qtmypaint/debug/qtmypaint.lib
# else:unix: PRE_TARGETDEPS += $$OUT_PWD/../../qtmypaint/libqtmypaint.a


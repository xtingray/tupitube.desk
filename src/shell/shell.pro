QT += opengl core gui svg xml network
QT += multimedia multimediawidgets printsupport
QT += widgets
CONFIG += warn_on
TEMPLATE = app

macx {
    CONFIG += console static
    ICON = ../../launcher/icons/tupitube.desk.icns
    QMAKE_INFO_PLIST = ./Info.plist
    QMAKE_LFLAGS += -Wl,-rpath,@loader_path/../,-rpath,@executable_path/../,-rpath,@executable_path/../Frameworks
    TARGET = ../../bin/TupiTube

    INSTALLS += icons \
                target \
                tupidata \
                html

    icons.target = ../../launcher/icons/tupitube.desk.png
    icons.commands = cp ../../launcher/icons/tupitube.desk.png $(INSTALL_ROOT)/pixmaps
    icons.path = /pixmaps/

    target.path = /bin/

    tupidata.target = data
    tupidata.commands = cp -r data/* $(INSTALL_ROOT)/data
    tupidata.path = /data/

    html.target = html
    html.files = html
    html.commands = cp -r html $(INSTALL_ROOT)/share/tupitube/data
    html.path = /data/
}

unix:!mac {
    INSTALLS += tupidata \
                launcher \
                mime \
                target \
                desktop \
                icons \
                html \
                copyright

    tupidata.target = data
    tupidata.commands = cp -r data/* $(INSTALL_ROOT)/data
    tupidata.path = /data/

    launcher.target = ../../launcher/tupitube.desk
    launcher.commands = cp ../../launcher/tupitube.desk $(INSTALL_ROOT)/bin; chmod 755 $(INSTALL_ROOT)/bin/tupitube.desk
    launcher.path = /bin/

    mime.target = ../../launcher/tupitube.xml
    mime.commands = cp ../../launcher/tupitube.xml $(INSTALL_ROOT)/share/mime/packages
    mime.path = /share/mime/packages/

    desktop.target = ../../launcher/tupitube.desktop
    desktop.commands = cp ../../launcher/tupitube.desktop $(INSTALL_ROOT)/applications
    desktop.path = /applications/

    icons.target = ../../launcher/icons/tupitube.desk.png
    icons.commands = cp ../../launcher/icons/tupitube.desk.png $(INSTALL_ROOT)/pixmaps
    icons.path = /pixmaps/

    html.target = html 
    html.files = html 
    html.commands = cp -r html $(INSTALL_ROOT)/share/tupitube/data
    html.path = /data/

    copyright.target = man/copyright
    copyright.commands = cp man/copyright $(INSTALL_ROOT)/share/doc/tupitube
    copyright.path = /tupitube/

    target.path = /bin/
    TARGET = ../../bin/tupitube.bin
}

HEADERS += tupmainwindow.h \
           tupstatusbar.h \
           tupnewproject.h \
           tupapplication.h \
           tuplocalprojectmanagerhandler.h

SOURCES += main.cpp \
           tupmainwindow.cpp \
           tupstatusbar.cpp \
           tupnewproject.cpp \
           tupapplication.cpp \
           tupmainwindow_gui.cpp \
           tuplocalprojectmanagerhandler.cpp

unix {
    HEADERS += tupcrashhandler.h \
               tupcrashwidget.h
    SOURCES += tupcrashhandler.cpp \
               tupcrashwidget.cpp 
}
		   		   
FRAMEWORK_DIR = ../framework
include($$FRAMEWORK_DIR/framework.pri)

unix {
    !include(../../tupiglobal.pri) {
        error("Please run configure first")
    }
}

win32 {
    TARGET = tupitube.desk
    include(../../win.pri)
    include(../../quazip.win.pri)
}

include(shell_config.pri)

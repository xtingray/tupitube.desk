QT += opengl core gui svg xml network
QT += multimedia multimediawidgets printsupport
QT += widgets
CONFIG += warn_on
TEMPLATE = app

macx {
    CONFIG += console static
    # ICON = ../../launcher/icons/tupitube.desk.icns
    ICON = tupitube.desk.icns
    QMAKE_INFO_PLIST = ./Info.plist
    QMAKE_LFLAGS += -Wl,-rpath,@loader_path/../,-rpath,@executable_path/../,-rpath,@executable_path/../Frameworks
    TARGET = ../../bin/TupiTube

    INSTALLS += icons \
                target \
                tupidata \
                html

    icons.target = ../../launcher/icons/tupitube.desk.png
    icons.commands = cp -v ../../launcher/icons/tupitube.desk.png $(INSTALL_ROOT)/pixmaps
    icons.path = /pixmaps/

    target.path = /bin/

    tupidata.target = data
    tupidata.commands = cp -r data/* $(INSTALL_ROOT)/data
    tupidata.path = /data/

    html.target = html
    html.files = html
    html.commands = cp -rv html $(INSTALL_ROOT)/share/tupitube/data
    html.path = /data/
}

unix:!mac {
    INSTALLS += tupidata \
                launcher \
                mime \
                target \
                desktop \
                appdata \
                icons \
                html \
                copyright

    tupidata.target = data
    tupidata.commands = cp -rv data/* $(INSTALL_ROOT)/data
    tupidata.path = /data/

    launcher.target = ../../launcher/tupitube.desk
    launcher.commands = cp -v ../../launcher/tupitube.desk $(INSTALL_ROOT)/bin; chmod 755 $(INSTALL_ROOT)/bin/tupitube.desk
    launcher.path = /bin/

    mime.target = ../../launcher/tupitube.xml
    mime.commands = cp -v ../../launcher/tupitube.xml $(INSTALL_ROOT)/share/mime/packages
    mime.path = /share/mime/packages/

    desktop.target = ../../launcher/tupitube.desktop
    desktop.commands = cp -v ../../launcher/tupitube.desktop $(INSTALL_ROOT)/applications
    desktop.path = /applications/

    appdata.target = ../../launcher/tupitube.appdata.xml
    appdata.commands = cp -v ../../launcher/tupitube.appdata.xml $(INSTALL_ROOT)/metainfo
    appdata.path = /metainfo/

    icons.target = ../../launcher/icons/tupitube.desk.png
    icons.commands = cp -v ../../launcher/icons/tupitube.desk.png $(INSTALL_ROOT)/pixmaps
    icons.path = /pixmaps/

    html.target = html 
    html.files = html 
    html.commands = cp -rv html $(INSTALL_ROOT)/share/tupitube/data
    html.path = /data/

    copyright.target = man/copyright
    copyright.commands = cp -v man/copyright $(INSTALL_ROOT)/share/doc/tupitube
    copyright.path = /tupitube/

    target.path = /bin/
    TARGET = ../../bin/tupitube.bin
}

HEADERS += tupsigndialog.h \
           tupstatusbar.h \
           tupnewproject.h \
           tupapplication.h \
           tuplocalprojectmanagerhandler.h \
           tupmainwindow.h

SOURCES += tupsigndialog.cpp \
           tupstatusbar.cpp \
           tupnewproject.cpp \
           tupapplication.cpp \
           tuplocalprojectmanagerhandler.cpp \
           tupmainwindow.cpp \
           tupmainwindow_gui.cpp \
           main.cpp

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
	RC_ICONS = ../../tools/windows/tupitube.ico
    include(../../win.pri)
    include(../../quazip.win.pri)
}

include(shell_config.pri)

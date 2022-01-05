TEMPLATE = lib 
TARGET = tupmicmanager

INSTALLS += target
target.path = /lib/

QT += widgets multimedia

FRAMEWORK_DIR = "../../framework"
include($$FRAMEWORK_DIR/framework.pri)

unix {
  !include(../../../tupiglobal.pri) {
    error("Run ./configure first!")
  }
}

HEADERS = tupmicmanager.h \
          tupmiclevel.h

SOURCES = tupmicmanager.cpp \
          tupmiclevel.cpp

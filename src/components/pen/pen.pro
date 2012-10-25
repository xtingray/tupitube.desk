# File generated by kdevelop's qmake manager. 
# ------------------------------------------- 
# Subdir relative project main directory: ./src/components/pen
# Target is a library:  

INSTALLS += target
target.path = /lib/

macx {
    CONFIG += staticlib warn_on
}

HEADERS += tuppenwidget.h 
           # tuppenthicknesswidget.h
SOURCES += tuppenwidget.cpp 
           # tuppenthicknesswidget.cpp

*:!macx{
    CONFIG += dll warn_on
}
TEMPLATE = lib
TARGET = tupipen

FRAMEWORK_DIR = "../../framework"
include($$FRAMEWORK_DIR/framework.pri)
include(../components_config.pri)

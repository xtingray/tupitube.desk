TEMPLATE = app
CONFIG -= moc
TARGET = libpng 

macx {
    CONFIG -= app_bundle
    CONFIG += warn_on static console
}

DEPENDPATH += .
INCLUDEPATH += .
LIBS += -lpng

# Input
SOURCES += main.cpp

TEMPLATE = app
CONFIG -= moc
TARGET = libpng 

macx {
    CONFIG -= app_bundle
    CONFIG += warn_on static console
    INCLUDEPATH += /usr/local/include
    LIBS += -L/usr/local/lib -lpng
}

DEPENDPATH += .
INCLUDEPATH += .
# LIBS += -lpng

# Input
SOURCES += main.cpp

TEMPLATE = app
CONFIG -= moc
TARGET = quazip 
DEPENDPATH += .

macx {
    INCLUDEPATH += /usr/local/include
    CONFIG -= app_bundle
    CONFIG += warn_on static console
}

unix:!macx {
  INCLUDEPATH += /usr/local/quazip/include
}

LIBS += -L/usr/local/lib -lz

# Input
SOURCES += main.cpp

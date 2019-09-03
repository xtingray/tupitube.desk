TEMPLATE = app
CONFIG -= moc
TARGET = ffmpeg 

macx {
    CONFIG -= app_bundle
    CONFIG += warn_on static console
}

INCLUDEPATH += . /usr/local/ffmpeg/include
DEFINES += __STDC_CONSTANT_MACROS
LIBS += -L/usr/local/ffmpeg/lib -lavformat -lavcodec -lavutil -lavresample -lswresample #-lswscale

# Input
SOURCES += main.cpp

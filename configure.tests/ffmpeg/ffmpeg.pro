TEMPLATE = app
CONFIG -= moc
TARGET = ffmpeg 

!mac {
    INCLUDEPATH += . /usr/local/ffmpeg/include
    DEFINES += __STDC_CONSTANT_MACROS
    LIBS += -L/usr/local/ffmpeg/lib -lavformat -lavcodec -lavutil #-lswscale
}

macx {
    CONFIG -= app_bundle
    CONFIG += warn_on static console
    INCLUDEPATH += . /usr/local/ffmpeg/include
    LIBS += -L/usr/local/ffmpeg/lib -lavformat -lavcodec -lavutil #-lswscale
}

# Input
SOURCES += main.cpp

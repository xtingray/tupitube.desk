TEMPLATE = app
CONFIG -= moc
TARGET = libsndfile 

!mac {
    INCLUDEPATH += . /usr/local/libsndfile/include
    LIBS += -L/usr/local/libsndfile/lib -lsndfile
}

macx {
    CONFIG -= app_bundle
    CONFIG += warn_on static console
    INCLUDEPATH += . /usr/local/libsndfile/include
    LIBS += -L/usr/local/libsndfile/lib -lsndfile
}

DEPENDPATH += .

# Input
SOURCES += main.cpp

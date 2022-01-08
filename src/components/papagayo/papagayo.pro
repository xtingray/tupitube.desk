QT += core gui widgets multimedia xml
TEMPLATE = lib 
TARGET = tupipapagayo

CONFIG += dll warn_on

INSTALLS += target
target.path = /lib/

FRAMEWORK_DIR = "../../framework"
include($$FRAMEWORK_DIR/framework.pri)

win32 {
    STORE_DIR = ../../store/
    INCLUDEPATH += $$STORE_DIR
    LIBS += -L$$STORE_DIR/release/ -ltupistore

    INCLUDEPATH += C:/devel/sources/libsndfile/include
    LIBS += -LC:/devel/sources/libsndfile/lib -llibsndfile-1
} else { # unix
    !include(../../../tupiglobal.pri) {
             error("Run ./configure first!")
    }

    STORE_DIR = ../../store/
    INCLUDEPATH += $$STORE_DIR
    LIBS += -L$$STORE_DIR -ltupistore
}

HEADERS +=  tupaudioextractor.h \
            tuplipsyncdoc.h \
            tupbreakdowndialog.h \
            tupwaveformview.h \
            tupmouthview.h \
            tupcustomizedmouthview.h \
            tuppg_config.h \
            tuppapagayoimporter.h \
            tuppapagayoapp.h
		   
SOURCES += tupaudioextractor.cpp \
           tuplipsyncdoc.cpp \
           tupbreakdowndialog.cpp \
           tupwaveformview.cpp \
           tupmouthview.cpp \
           tupcustomizedmouthview.cpp \
           tuppapagayoimporter.cpp \
           tuppapagayoapp.cpp

# HEADERS += tupaudioextractor.h \
#           tuplipsyncdoc.h \
#           tupwaveformview.h \
#           tupmouthview.h \
#           tupcustomizedmouthview.h \
#           tuppg_config.h \
#           tupbreakdowndialog.h \
#           tuppapagayoimporter.h \
#           tuppapagayoapp.h

# SOURCES += tupaudioextractor.cpp \
#           tuplipsyncdoc.cpp \
#           tupwaveformview.cpp \
#           tupmouthview.cpp \
#           tupcustomizedmouthview.cpp \
#           tupbreakdowndialog.cpp \
#           tuppapagayoimporter.cpp \
#           tuppapagayoapp.cpp

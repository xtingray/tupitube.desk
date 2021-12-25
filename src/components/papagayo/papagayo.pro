#-------------------------------------------------
#
# Project created by QtCreator 2014-06-03T18:13:04
#
#-------------------------------------------------

QT += core gui widgets multimedia

TARGET = tupipapagayo
TEMPLATE = lib 

INSTALLS += target
target.path = /lib/

win32 {
    STORE_DIR = ../../store/
    INCLUDEPATH += $$STORE_DIR
    LIBS += -L$$STORE_DIR/release/ -ltupistore

    INCLUDEPATH += libsndfile_1.0.25/Win32/include

    LIBS += -LC:\Users\mclifton\Documents\QtDevelopment\Papagayo\libsndfile_1.0.25\Win32\lib -llibsndfile-1
    # LIBS += -Llibsndfile_1.0.25\Win32\lib -llibsndfile-1

    SOUND_FILES += LC:\Users\mclifton\Documents\QtDevelopment\Papagayo\libsndfile_1.0.25\Win32\bin\libsndfile-1.dll

    extra_libs.files = SOUND_FILES
    INSTALLS += extra_libs
} else { # unix
    !include(../../../tupiglobal.pri) {
             error("Run ./configure first!")
    }

    STORE_DIR = ../../store/
    INCLUDEPATH += $$STORE_DIR
    LIBS += -L$$STORE_DIR -ltupistore
}

FRAMEWORK_DIR = "../../framework"
include($$FRAMEWORK_DIR/framework.pri)

HEADERS += tuppapagayoapp.h \
           tuplipsyncdoc.h \
           tupwaveformview.h \
           tupaudioextractor.h \
           tupmouthview.h \
           tupcustomizedmouthview.h \
           tuppg_config.h \
           tupbreakdowndialog.h \
           tuppapagayoimporter.h

SOURCES += tuppapagayoapp.cpp \
           tuplipsyncdoc.cpp \
           tupwaveformview.cpp \
           tupmouthview.cpp \
           tupcustomizedmouthview.cpp \
           tupaudioextractor.cpp \
           tupbreakdowndialog.cpp \
           tuppapagayoimporter.cpp

INCLUDEPATH += src

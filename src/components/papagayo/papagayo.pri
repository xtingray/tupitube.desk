INCLUDEPATH += $$PAPAGAYO_DIR

unix {
    INCLUDEPATH += /usr/local/libsndfile/include $$PAPAGAYO_DIR
    LIBS += -L/usr/local/libsndfile/lib -lsndfile -L$$PAPAGAYO_DIR -ltupipapagayo
}

win32 {
    LIBS += -L$$PAPAGAYO_DIR/release/ -ltupipapagayo
}

linux-g++ {
    INCLUDEPATH += /usr/local/libsndfile/include $$PAPAGAYO_DIR
    LIBS += -L/usr/local/libsndfile/lib -lsndfile -L$$PAPAGAYO_DIR -ltupipapagayo

    PRE_TARGETDEPS += $$PAPAGAYO_DIR/libtupipapagayo.so
}

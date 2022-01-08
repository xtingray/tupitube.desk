INCLUDEPATH += $$PAPAGAYO_DIR

win32 {
    INCLUDEPATH += C:/devel/sources/libsndfile/include $$PAPAGAYO_DIR
    # LIBS += -LC:/devel/sources/libsndfile/lib -llibsndfile-1 -L$$PAPAGAYO_DIR/release -ltupipapagayo
    LIBS += -L$$PAPAGAYO_DIR/release -ltupipapagayo
}

unix {
    INCLUDEPATH += /usr/local/libsndfile/include $$PAPAGAYO_DIR
    LIBS += -L/usr/local/libsndfile/lib -lsndfile -L$$PAPAGAYO_DIR -ltupipapagayo
}

linux-g++ {
    INCLUDEPATH += /usr/local/libsndfile/include $$PAPAGAYO_DIR
    LIBS += -L/usr/local/libsndfile/lib -lsndfile -L$$PAPAGAYO_DIR -ltupipapagayo

    PRE_TARGETDEPS += $$PAPAGAYO_DIR/libtupipapagayo.so
}

INCLUDEPATH += $$NEWS_DIR

unix {
    LIBS += -L$$NEWS_DIR -ltupinews
}

win32 {
    LIBS += -L$$NEWS_DIR/release/ -ltupinews
}

linux-g++ {
    PRE_TARGETDEPS += $$NEWS_DIR/libtupinews.so
}

INCLUDEPATH += $$LIBRARY_DIR

unix {
    LIBS += -L$$LIBRARY_DIR -ltupilibrary -lpng
}

win32 {
    LIBPNG_DIR = C:/devel/sources/libpng
    LIBS += -L$$LIBRARY_DIR/release/ -L$$LIBPNG_DIR/lib -lpng -ltupilibrary
}

linux-g++ {
    PRE_TARGETDEPS += $$LIBRARY_DIR/libtupilibrary.so
}

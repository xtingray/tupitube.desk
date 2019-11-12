INCLUDEPATH += $$LIBCOLOR_DIR

unix {
    LIBS += -L$$LIBCOLOR_DIR -ltupicolor
}

win32 {
    LIBS += -L$$LIBCOLOR_DIR/release -ltupicolor
}

linux-g++ {
    PRE_TARGETDEPS += $$LIBCOLOR_DIR/libtupicolor.so
}


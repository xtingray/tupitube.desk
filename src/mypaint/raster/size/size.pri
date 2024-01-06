INCLUDEPATH += $$RASTERCOLOR_DIR

unix {
    LIBS += -L$$RASTERCOLOR_DIR -lrastercolor
}

win32 {
    LIBS += -L$$RASTERCOLOR_DIR/release -lrastercolor
}

linux-g++ {
    PRE_TARGETDEPS += $$RASTERCOLOR_DIR/librastercolor.so
}

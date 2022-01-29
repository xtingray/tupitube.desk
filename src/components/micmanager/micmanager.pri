INCLUDEPATH += $$MICMANAGER_DIR

unix {
    LIBS += -L$$MICMANAGER_DIR -ltupimicmanager
}

win32 {
    LIBS += -L$$MICMANAGER_DIR/release/ -ltupimicmanager
}

linux-g++ {
    PRE_TARGETDEPS += $$MICMANAGER_DIR/libtupimicmanager.so
}

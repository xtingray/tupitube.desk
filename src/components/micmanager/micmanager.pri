INCLUDEPATH += $$MICMANAGER_DIR

unix {
    LIBS += -L$$MICMANAGER_DIR -ltupmicmanager
}

win32 {
    LIBS += -L$$MICMANAGER_DIR/release/ -ltupmicmanager
}

linux-g++ {
    PRE_TARGETDEPS += $$MICMANAGER_DIR/libtupmicmanager.so
}

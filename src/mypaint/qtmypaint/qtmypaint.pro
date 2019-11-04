QT += core gui widgets

TEMPLATE = lib
CONFIG += dll warn_on

TARGET = qtmypaint

HEADERS += mpbrush.h \
           mphandler.h \
           mpsurface.h \
           mptile.h

SOURCES += mpbrush.cpp \
           mphandler.cpp \
           mpsurface.cpp \
           mptile.cpp

LIBS += -L../json-c -ljson-c
LIBS += -L../libmypaint -llibmypaint

# --- json-c ---
win32:CONFIG(release, debug|release): LIBS += -L../json-c/release/ -ljson-c
else:win32:CONFIG(debug, debug|release): LIBS += -L../json-c/debug/ -ljson-c
else:unix: LIBS += -L../json-c -ljson-c

INCLUDEPATH += ../json-c
DEPENDPATH += ../json-c

# win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += ../json-c/release/libjson-c.a
# else:win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += ../json-c/debug/libjson-c.a
# else:win32:!win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += ../json-c/release/json-c.lib
# else:win32:!win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += ../json-c/debug/json-c.lib
# else:unix: PRE_TARGETDEPS += ../json-c/libjson-c.a

# --- libmypaint ---
win32:CONFIG(release, debug|release): LIBS += -L../libmypaint/release/ -llibmypaint
else:win32:CONFIG(debug, debug|release): LIBS += -L../libmypaint/debug/ -llibmypaint
else:unix: LIBS += -L../libmypaint -llibmypaint

INCLUDEPATH += ../libmypaint
DEPENDPATH += ../libmypaint

# win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += ../libmypaint/release/libmypaint.a
# else:win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += ../libmypaint/debug/libmypaint.a
# else:win32:!win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += ../libmypaint/release/libmypaint.lib
# else:win32:!win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += ../libmypaint/debug/libmypaint.lib
# else:unix: PRE_TARGETDEPS += ../libmypaint/libmypaint.a

unix {
    INSTALLS += target
    target.path = /lib/raster
}

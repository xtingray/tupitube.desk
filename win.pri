CONFIG += release

#DEFINES += TUP_DEBUG HAVE_LIBAV
DEFINES += HAVE_LIBAV

DEFINES += VERSION=\\\"0.2\\\" CODE_NAME=\\\"Abequar\\\" REVISION=\\\"11\\\" CONFIG_VERSION=\\\"2\\\"

contains(DEFINES, TUP_DEBUG) {
    CONFIG += console
}

CONFIG += release

#DEFINES += TUP_DEBUG HAVE_LIBAV
DEFINES += HAVE_LIBAV

DEFINES += VERSION=\\\"0.2\\\" CODE_NAME=\\\"Garapa\\\" REVISION=\\\"13\\\" CONFIG_VERSION=\\\"2\\\"

contains(DEFINES, TUP_DEBUG) {
    CONFIG += console
}

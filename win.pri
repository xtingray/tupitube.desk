CONFIG += release
DESTDIR = release

DEFINES += TUP_DEBUG HAVE_FFMPEG
# DEFINES += HAVE_FFMPEG
# DEFINES += TUP_32BIT

DEFINES += TUPITUBE_VERSION=\\\"0.2\\\" REVISION=\\\"22\\\" CODE_NAME=\\\"Xamã\\\" CONFIG_VERSION=\\\"5\\\"

contains(DEFINES, TUP_DEBUG) {
    CONFIG += console
}

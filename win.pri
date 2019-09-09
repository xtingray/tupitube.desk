CONFIG += release

DEFINES += TUP_DEBUG HAVE_FFMPEG
# DEFINES += HAVE_FFMPEG

DEFINES += VERSION=\\\"Community\\\" REVISION=\\\"I\\\" CODE_NAME=\\\"Free_Version\\\" CONFIG_VERSION=\\\"2\\\"

contains(DEFINES, TUP_DEBUG) {
    CONFIG += console
}

CONFIG += release

# DEFINES += TUP_DEBUG HAVE_FFMPEG
DEFINES += HAVE_FFMPEG

DEFINES += VERSION=\\\"0.2\\\" REVISION=\\\"14\\\" CODE_NAME=\\\"Uberaba\\\" CONFIG_VERSION=\\\"3\\\"

contains(DEFINES, TUP_DEBUG) {
    CONFIG += console
}

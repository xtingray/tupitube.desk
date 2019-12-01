CONFIG += release

# DEFINES += TUP_DEBUG HAVE_FFMPEG
DEFINES += HAVE_FFMPEG

DEFINES += VERSION=\\\"Artist\\\" REVISION=\\\"11-19\\\" CODE_NAME=\\\"Uberaba\\\" CONFIG_VERSION=\\\"3\\\"

contains(DEFINES, TUP_DEBUG) {
    CONFIG += console
}

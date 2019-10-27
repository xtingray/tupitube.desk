CONFIG += release

# DEFINES += TUP_DEBUG HAVE_FFMPEG
DEFINES += HAVE_FFMPEG

DEFINES += VERSION=\\\"Artist\\\" REVISION=\\\"10-19\\\" CODE_NAME=\\\"Mameluco\\\" CONFIG_VERSION=\\\"2\\\"

contains(DEFINES, TUP_DEBUG) {
    CONFIG += console
}

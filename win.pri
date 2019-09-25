CONFIG += release

# DEFINES += TUP_DEBUG HAVE_FFMPEG
DEFINES += HAVE_FFMPEG

DEFINES += VERSION=\\\"Artist\\\" REVISION=\\\"09-19\\\" CODE_NAME=\\\"Cuore\\\" CONFIG_VERSION=\\\"2\\\"

contains(DEFINES, TUP_DEBUG) {
    CONFIG += console
}

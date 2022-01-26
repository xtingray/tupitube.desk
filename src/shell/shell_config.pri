QT += xml opengl svg

INCLUDEPATH += ../plugins/tools/common

MICMANAGER_DIR = ../components/micmanager
MYPAINT_DIR = ../mypaint
LIBCOLOR_DIR = ../libcolor
COLORPALETTE_DIR = ../components/colorpalette
PLAYER_DIR = ../components/player
WORKSPACE_DIR = ../components/workspace
PAPAGAYO_DIR = ../components/papagayo
BRUSH_DIR = ../components/brush
# KINAS_DIR = ../components/kinas 
IMPORT_DIR = ../components/import 
EXPORT_DIR = ../components/export 
EXPOSURE_DIR = ../components/exposure 
TIMELINE_DIR = ../components/timeline 
LIBRARY_DIR = ../components/library
SCENES_DIR = ../components/scenes 
NEWS_DIR = ../components/news
LIBUI_DIR = ../libui
STORE_DIR = ../store 
LIBTUPI_DIR = ../libtupi
LIBBASE_DIR = ../libbase
# NET_DIR = ../net

include($$MICMANAGER_DIR/micmanager.pri)
include($$MYPAINT_DIR/mypaint.pri)
include($$LIBCOLOR_DIR/libcolor.pri)
include($$COLORPALETTE_DIR/colorpalette.pri)
include($$WORKSPACE_DIR/workspace.pri)
include($$PAPAGAYO_DIR/papagayo.pri)
include($$PLAYER_DIR/player.pri)
include($$BRUSH_DIR/brush.pri)
# include($$KINAS_DIR/kinas.pri)
include($$IMPORT_DIR/import.pri)
include($$EXPORT_DIR/export.pri)
include($$EXPOSURE_DIR/exposure.pri)
include($$TIMELINE_DIR/timeline.pri)
include($$LIBRARY_DIR/library.pri)
include($$SCENES_DIR/scenes.pri)
include($$NEWS_DIR/news.pri)
include($$LIBUI_DIR/libui.pri)
include($$STORE_DIR/store.pri)
include($$LIBTUPI_DIR/libtupi.pri)
include($$LIBBASE_DIR/libbase.pri)
# include($$NET_DIR/net.pri)

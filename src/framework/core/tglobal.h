/***************************************************************************
 *   Project TUPITUBE DESK                                                *
 *   Project Contact: info@maefloresta.com                                 *
 *   Project Website: http://www.maefloresta.com                           *
 *   Project Leader: Gustav Gonzalez <info@maefloresta.com>                *
 *                                                                         *
 *   Developers:                                                           *
 *   2010:                                                                 *
 *    Gustavo Gonzalez / xtingray                                          *
 *                                                                         *
 *   KTooN's versions:                                                     * 
 *                                                                         *
 *   2006:                                                                 *
 *    David Cuadrado                                                       *
 *    Jorge Cuadrado                                                       *
 *   2003:                                                                 *
 *    Fernado Roldan                                                       *
 *    Simena Dinas                                                         *
 *                                                                         *
 *   Copyright (C) 2010 Gustav Gonzalez - http://www.maefloresta.com       *
 *   License:                                                              *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>. *
 ***************************************************************************/

#ifndef TGLOBAL_H
#define TGLOBAL_H

#if defined(QT_SHARED) || defined(QT_PLUGIN)
  #define T_GUI_EXPORT Q_GUI_EXPORT
  #define T_CORE_EXPORT Q_DECL_EXPORT
  #define T_SOUND_EXPORT Q_DECL_EXPORT
  #define TUPITUBE_EXPORT Q_GUI_EXPORT
  #define TUPITUBE_PLUGIN Q_DECL_EXPORT
#else
  #define T_GUI_EXPORT
  #define T_CORE_EXPORT
  #define T_SOUND_EXPORT
  #define TUPITUBE_EXPORT
  #define TUPITUBE_PLUGIN
#endif

#ifdef QT_GUI_LIB
  #include <QGuiApplication>
#endif

#ifdef TUP_DEBUG
  #include <QDebug>
  #define SHOW_VAR(arg) qDebug() << #arg << " = " << arg;
#endif

#define COMPANY "MaeFloresta"
#define CACHE_DB "TupiTube"

#define LIBRARY_DIR CONFIG_DIR+"/libraries"
#define MAEFLORESTA_URL "https://www.maefloresta.com/"

#define TUPITUBE_URL "https://tupitube.com"
#define TUPITUBE_POST "https://www.tupitube.com" // SQA: change for post.tupitube.com
#define LIBRARY_URL "https://library.tupitube.com"
#define MEDIA_URL "media.tupitube.com"
#define BROWSER_FINGERPRINT "Tupi_Browser 2.0"
#define MOZILLA_FINGERPRINT "Mozilla/5.0"

#define SECRET_KEY "923B479F-12324679-30A0E076-34E82C77-5"

#define ZLAYER_LIMIT 10000
#define BG_LAYERS 4
#define ZLAYER_BASE 100000 // Initial zValue for Frames Mode layers
#define ITEMS_PER_FRAME 100 // Maximum items per frame

#define DEFAULT_FONT_SIZE 36

enum PlayMode { PlayAll = 0, OneScene };
enum PlayDirection { Forward = 0, Backward };
enum MediaType {Video = 0, Audio};
enum SoundType {NoSound = 0, Lipsync = 1, Effect = 2};
struct SoundResource {
    QString key;
    int sceneIndex;
    int frameIndex;
    QString path;
    bool muted;
    SoundType type;
};

enum PapagayoAppMode { Insert = 0, Update, VoiceRecorded, AudioFromLibrary };
enum PapagayoAppLanguage { English = 0, Spanish, OtherLang };
enum PapagayoAppViewType { Predefined = 0, Customized };

enum ModuleSource { Library = 0, PapagayoApp };

enum ProjectDimension {
    FREE = 0,
    FORMAT_520,
    FORMAT_640,
    FORMAT_480,
    FORMAT_576,
    FORMAT_720,
    FORMAT_MOBILE,
    FORMAT_1080_VERTICAL,
    FORMAT_1080
};

#define CUSTOMIZED_MOUTH 5
#define MOUTHS_PACKAGE_SIZE 10
#define MOUTH_WIDTH 200
#define MOUTH_HEIGHT 200

#define DARK_THEME 0
#define LIGHT_THEME 1

#ifdef Q_OS_WIN
  #define __STDC_FORMAT_MACROS 1
  #include <inttypes.h>
#endif

enum SafeLevel { Background = 0, Foreground };
enum ImportAction {VideoAction=0, FolderAction};

enum PenTool { PencilMode = 0, EraserMode };

enum NodeLocation { FirstNode = 0, MiddleNode, RandomNode, LastNode };
enum NodeContext { SelectionNode = 0, PapagayoNode, TextNode };
enum NodePosition { TopLeftNode  = 0, TopRightNode, BottomLeftNode, BottomRightNode, CenterNode };
enum NodeAction { NoAction = 0, NodeScale, NodeRotate };
enum NodeType { CurveNode = 0, LineNode };

#endif

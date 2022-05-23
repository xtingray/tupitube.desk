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
// #define BG_LAYERS_TOTAL 10
#define ZLAYER_BASE 100000 // Initial zValue for Frames Mode layers

#define DEFAULT_FONT_SIZE 36

enum MediaType {Video = 0, Audio};
struct SoundResource {
    QString key;
    int frame;
    QString path;
    bool muted;
};

#define DARK_THEME 0
#define LIGHT_THEME 1

#endif

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

#ifndef TUPEXPORTINTERFACE_H
#define TUPEXPORTINTERFACE_H

#include "tglobal.h"
#include "tupscene.h"
#include "qplugin.h" // Q_EXPORT_PLUGIN
#include "tuplibrary.h"

#include <QStringList>
#include <QDir>
#include <QColor>

class TUPITUBE_EXPORT TupExportInterface
{
    public:
        enum Plugin
             {
               VideoFormats = 0,
               OpenVideoFormat = 1,
               ImageSequence = 2,
               AnimatedImage = 3,
               MobileFormats = 4
             };

        enum Format 
             {
               NONE = 0,
               WEBM = 1 << 0,
               OGV  = 1 << 1,
               MP4  = 1 << 2,
               MPEG = 1 << 3,
               AVI  = 1 << 4,
               MOV  = 1 << 5,
               GIF  = 1 << 6,
               PNG  = 1 << 7,
               JPEG = 1 << 8,
               XPM  = 1 << 9,
               SVG  = 1 << 10, 
               APNG = 1 << 11
             };

        Q_DECLARE_FLAGS(Formats, Format);

        virtual ~TupExportInterface() {};
        virtual QString formatName() const = 0;
        virtual Plugin key() = 0;
        virtual Formats availableFormats() = 0;
        virtual bool exportToFormat(const QColor color, const QString &filePath, const QList<TupScene *> &scenes, 
                                    Format format, const QSize &size, const QSize &newSize, int fps, TupLibrary *library = nullptr, 
                                    bool waterMark = false) = 0;
        virtual bool exportFrame(int frameIndex, const QColor color, const QString &filePath, TupScene *scene, const QSize &size, 
                                 TupLibrary *library = nullptr, bool waterMark = false) = 0;
        virtual bool exportToAnimatic(const QString &filePath, const QList<QImage> images, const QList<int> indexes,
                                      TupExportInterface::Format format, const QSize &size, int fps) = 0;
        virtual QString getExceptionMsg() const = 0;
};
        Q_DECLARE_OPERATORS_FOR_FLAGS(TupExportInterface::Formats);
        Q_DECLARE_INTERFACE(TupExportInterface, "com.maefloresta.tupi.ExportInterface/0.1");

#endif

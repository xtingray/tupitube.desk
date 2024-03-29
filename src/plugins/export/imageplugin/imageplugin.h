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

#ifndef	IMAGEPLUGIN_H
#define IMAGEPLUGIN_H

#include "tglobal.h"
#include "tupexportpluginobject.h"
#include "tuplayer.h"
#include "tupanimationrenderer.h"

#include <QColor>
#include <QImage>
#include <QPainter>
#include <QSvgGenerator>

class TUPITUBE_PLUGIN ImagePlugin : public TupExportPluginObject
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "com.maefloresta.tupi.TupToolInterface" FILE "imageplugin.json")

    public:
        ImagePlugin();
        virtual ~ImagePlugin();

        QString formatName() const;
        virtual TupExportInterface::Plugin key();
        TupExportInterface::Formats availableFormats();
        virtual bool exportToFormat(const QColor color, const QString &filePath, const QList<TupScene *> &scenes,
                                    TupExportInterface::Format format, const QSize &size, const QSize &newSize,
                                    int fps, TupProject *project, bool waterMark = false);
        virtual bool exportFrame(int frameIndex, const QColor color, const QString &filePath, TupScene *scene,
                                 const QSize &size, TupProject *project, bool waterMark = false);

        virtual bool exportToAnimatic(const QString &filePath, const QList<QImage> images, const QList<int> indexes,
                                      TupExportInterface::Format format, const QSize &size, int fps);
        virtual QString getExceptionMsg() const;
        QString errorMsg;

    signals:
        void messageChanged(const QString &msg);
        void progressChanged(int percent);

    private:
        QString m_baseName;
};

#endif

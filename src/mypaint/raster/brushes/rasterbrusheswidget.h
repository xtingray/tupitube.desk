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

#ifndef RASTERBRUSHESWIDGET_H
#define RASTERBRUSHESWIDGET_H

#include "tglobal.h"
#include "tupmodulewidgetbase.h"
#include "tconfig.h"
#include "tuppaintareaevent.h"
#include "rasterbutton.h"

#include <QStackedWidget>
#include <QListWidgetItem>

class TUPITUBE_EXPORT RasterBrushesWidget : public TupModuleWidgetBase
{
    Q_OBJECT

    public:
        RasterBrushesWidget(const QString &brushLibPath, QWidget *parent = nullptr);
        ~RasterBrushesWidget();

        void loadInitSettings();
        bool isValid() { return !brushLib.isEmpty(); }

    public slots:
        // Give the brush name (no extension) i.e. : "classic/blend+paint"
        void selectBrush(QString brushName = QString());
        void updateBrushesPanel(int index);

    signals:
        void brushSelected(const QByteArray &content);

    protected:
        QMap<QString, QStringList> brushLib;
        const QString brushesPath;

    protected slots:
        void itemClicked(QListWidgetItem *);

    private:
        QStackedWidget * stackedWidget;
        QList<RasterButton *> buttonsList;
        QList<QListWidget*> brushesSet;

        int groupIndex;
        int brushIndex;
};

#endif

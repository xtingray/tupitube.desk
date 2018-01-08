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

#ifndef TUPCOLORBUTTONPANEL_H
#define TUPCOLORBUTTONPANEL_H

#include "tglobal.h"

#include <QWidget>

class TUPITUBE_EXPORT TupColorButtonPanel : public QWidget
{
    Q_OBJECT

    public:
        TupColorButtonPanel(Qt::BrushStyle style, const QSize &baseColorCellSize, int spacing,
                            const QString &buttonParams, QWidget *parent = 0);
        ~TupColorButtonPanel();
        void resetPanel();
        void enableTransparentColor(bool flag);

    signals:
        void clickColor(const QColor &color);

    private slots:
        void updateSelection(int index);
        void customizeColors();
        void resetColors();

    private:
       void setPanel(const QSize &cellSize, const QString &buttonParams);
       void setState(int index, bool isSelected);

       struct Private;
       Private *const k;
       int spacing;
};

#endif

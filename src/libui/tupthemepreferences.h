/***************************************************************************
 *   Project TUPITUBE DESK                                                 *
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

#ifndef TUPTHEMEPREFERENCES_H
#define TUPTHEMEPREFERENCES_H

#include "tglobal.h"
#include "tcolorbutton.h"
#include "tconfig.h"
#include "tradiobutton.h"
#include "tupcolorbutton.h"
#include "tslider.h"

#include <QHBoxLayout>

class TUPITUBE_EXPORT TupThemePreferences : public QWidget
{
    Q_OBJECT

    public:
        TupThemePreferences(QWidget *parent = nullptr);
        ~TupThemePreferences();

        void saveValues();
        bool showWarning();

    signals:
        void colorPicked(const QColor&);

    private slots:
        void updateCurrentRow(int row);
        void updateCurrentColor(const QColor &color);
        void restoreDefaultTheme();

    private:
        void setupPage();
        void addColorEntry(int id, const QString &label, const QColor &initColor, const QColor &endColor);

        QGridLayout *formLayout;
        QList<TRadioButton *> radioList;
        QList<TupColorButton *> cellList;
        QList<TSlider *> sliderList;
        int currentRow;
        int oldRow;
        int colorPos;
        QColor currentColor;
        QColor oldColor;
        bool themeChanged;
};

#endif

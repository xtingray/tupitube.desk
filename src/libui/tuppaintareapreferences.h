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

#ifndef TUPPAINTAREAPREFERENCES_H
#define TUPPAINTAREAPREFERENCES_H

#include "tglobal.h"
#include "tconfig.h"

#include <QWidget>
#include <QSpinBox>
#include <QPushButton>
#include <QGridLayout>
#include <QTabWidget>

class TUPITUBE_EXPORT TupPaintAreaPreferences : public QWidget
{
    Q_OBJECT

    public:
        enum ColorType {Grid, ROT};
        TupPaintAreaPreferences(QWidget *parent = nullptr);
        ~TupPaintAreaPreferences();

        void saveValues();        

    private slots:
        void setGridColor();
        void setRotColor();
        void setSafeAreaRectColor();
        void setSafeAreaLineColor();
        QColor setButtonColor(QPushButton *button, const QColor &currentColor) const;
        void restoreValues();
 
    private:
        void setupPage();
        QGridLayout * gridPanel();
        QGridLayout * ruleOfThirdsPanel();
        QGridLayout * safeAreaPanel();

    private:
        QTabWidget *tabWidget;
        QColor gridColor;
        QColor rotColor;
        QColor safeAreaRectColor;
        QColor safeAreaLineColor;

        QPushButton *gridColorButton;
        QSpinBox *gridSeparation;
        QSpinBox *gridThickness;

        QPushButton *rotColorButton;
        QSpinBox *rotSeparation;
        QSpinBox *rotThickness;

        QPushButton *safeRectColorButton;
        QPushButton *safeLineColorButton;
        QSpinBox *safeThickness;

        QFont labelFont;
};

#endif

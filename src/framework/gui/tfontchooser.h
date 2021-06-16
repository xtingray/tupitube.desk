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

#ifndef TFONTCHOOSER_H
#define TFONTCHOOSER_H

#include "tglobal.h"
#include "tformfactory.h"

#include <QFrame>
#include <QComboBox>
#include <QHBoxLayout>
#include <QFontDatabase>
#include <QFontComboBox>
#include <QPushButton>

class T_GUI_EXPORT TFontChooser : public QFrame
{
    Q_OBJECT

    public:
        TFontChooser(QWidget *parent = nullptr);
        ~TFontChooser();

        void setCurrentFont(const QFont &font);
        void updateFontSettings(const QFont &itemFont);

        void initFont();
        QFont currentFont() const;
        int currentSize() const;

    signals:
        void fontChanged();
        void alignmentUpdated(Qt::Alignment);

    public slots:
        void loadFontInfo(const QFont &newFont);

    private slots:
        void emitFontChanged(int = 0);

        void setBoldFlag();
        void setItalicFlag();
        void setUnderlineFlag();
        void setOverlineFlag();

        void alignTextToLeft();
        void alignTextToCenter();
        void alignTextToRight();

    private:
        QFontComboBox *m_families;
        QComboBox *m_fontSize;
        
        QFont m_currentFont;
        QPushButton *boldButton;
        QPushButton *italicButton;
        QPushButton *underlineButton;
        QPushButton *overlineButton;

        QPushButton *leftButton;
        QPushButton *centerButton;
        QPushButton *rightButton;
};

#endif

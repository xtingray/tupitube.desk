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

#ifndef TUPVIEWCOLORCELLS_H
#define TUPVIEWCOLORCELLS_H

#include "tglobal.h"
#include "tconfig.h"
#include "timagebutton.h"
#include "tupcellscolor.h"
#include "tuppaletteparser.h"
#include "tapplicationproperties.h"
#include "tupcolorbuttonpanel.h"

#include <QFrame>
#include <QComboBox>
#include <QStackedWidget>
#include <QVBoxLayout>
#include <QBrush>
#include <QScrollArea>
#include <QGroupBox>

class TUPITUBE_EXPORT TupViewColorCells : public QFrame
{
    Q_OBJECT

    public:
        TupViewColorCells(QWidget *parent = nullptr);
        ~TupViewColorCells();

        void readPaletteFile(const QString &file);
        void setColor(const QBrush & brush);
        void clearSelection();
        void resetBasicPanel();
        void enableTransparentColor(bool flag);

    signals:
        void colorSelected(const QBrush &);

    public slots:
        void addCurrentColor();
        void removeCurrentColor();
        void addPalette(const QString & name, const QList<QBrush> & brushes, bool editable);
        void changeColor(QTableWidgetItem*);

    private slots:
        void updateColorFromPanel(const QColor &color);

    private:
        void setupForm();
        void setupButtons();
        void addDefaultColor(int i, int j, const QColor &);
        void fillNamedColor();
        void readPalettes(const QString &paletteDir);
        void addPalette(TupCellsColor *palette);

        QComboBox *chooserPalette;
        QStackedWidget *containerPalette;

        TupCellsColor *defaultPalette;
        TupCellsColor *qtColorPalette;
        TupCellsColor *customColorPalette;
        TupCellsColor *customGradientPalette;

        int numColorRecent;
        QBrush currentColor;
        QTableWidgetItem* currentCell;
        QVBoxLayout *viewLayout;
        TupColorButtonPanel *buttonPanel;
};

#endif

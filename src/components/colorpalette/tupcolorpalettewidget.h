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

#ifndef TUPCOLORPALETTE_H
#define TUPCOLORPALETTE_H

#include "tglobal.h"
#include "tupmodulewidgetbase.h"
#include "tcolorcell.h"
#include "tupcolorform.h"
#include "tupcolorpicker.h"
#include "tupviewcolorcells.h"
#include "tslider.h"
#include "tupgradientcreator.h"

#include <QBoxLayout>
#include <QSplitter>
#include <QTabWidget>
#include <QLineEdit>

class TUPITUBE_EXPORT TupColorPaletteWidget : public TupModuleWidgetBase
{
    Q_OBJECT

    public:
        enum BrushType { Solid = 0, Gradient };

        TupColorPaletteWidget(QWidget *parent = nullptr);
        ~TupColorPaletteWidget();

        // SQA: change this for QBrush
        QPair<QColor, QColor> color();
        void parsePaletteFile(const QString &file);
        void setBgColor(const QColor &color);

    public slots:
        void init();
        void updateContourColor(const QColor &color);
        void updateFillColor(const QColor &color);
        void updateBgColor(const QColor &color);
        void updateColorMode(TColorCell::FillType mode);
        void checkColorButton(TColorCell::FillType mode);

    private slots:
        void initBg();
        void setColorOnAppFromHTML(const QBrush &brush);
        void updateColorFromHTML();
        void updateBgColorFromHTML();
        void syncColor(const QColor &color);
        void setHS(int h, int s);

        void updateColorFromPalette(const QBrush& brush);
        void updateColorFromDisplay(const QBrush& brush);
        void updateGradientColor(const QBrush &brush);
        void switchColors();
        void updateColorType(int index);

        void activateEyeDropper();
        void activateBgEyeDropper();

    signals:
        void paintAreaEventTriggered(const TupPaintAreaEvent *event);
        void colorSpaceChanged(TColorCell::FillType type);
        void eyeDropperActivated(TColorCell::FillType type);

    private:
        void setupButtons();
        void setupColorDisplay();
        void setupMainPalette();
        void setupColorChooser();
        void setupGradientManager();
        void setGlobalColors(const QBrush &brush);
        void updateLuminancePicker(const QColor &color);
        void saveTextColor(const QColor &color);

        QSplitter *splitter;
        QTabWidget *tab;

        TupViewColorCells *paletteContainer;
        TupColorForm *colorForm;
        TupColorPicker *colorPickerArea;
        TSlider *luminancePicker;
        TupGradientCreator *gradientManager;

        QLineEdit *htmlField;
        QLineEdit *bgHtmlField;

        QBrush currentContourBrush;
        QBrush currentFillBrush;

        TColorCell *contourColorCell;
        TColorCell *fillColorCell;
        TColorCell *bgColor;

        bool flagGradient;
        BrushType type;

        TColorCell::FillType currentSpace;
        TupColorPaletteWidget::BrushType fgType;
        TupColorPaletteWidget::BrushType bgType;
};

#endif

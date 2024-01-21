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

#include "rastersizewidget.h"

RasterSizeWidget::RasterSizeWidget(double thickness, const QColor &penColor, QWidget *parent) : TupModuleWidgetBase(parent)
{
    #ifdef TUP_DEBUG
        qDebug() << "[RasterSizeWidget()] - thickness ->" << thickness;
    #endif

    setWindowIcon(QIcon(ICONS_DIR + "brush_size.png"));
    setWindowTitle(tr("Brush Size"));

    QWidget *borderWidget = new QWidget;
    QVBoxLayout *borderLayout = new QVBoxLayout(borderWidget);

    thickPreview = new TupPenThicknessWidget;
    thickPreview->setColor(penColor);
    thickPreview->setBrush(Qt::SolidPattern);
    double input = (thickness*10);
    int size = (int) input;
    thickPreview->render(size);
    thickPreview->setFixedHeight(120);

    thicknessControl = new TDoubleSpinBoxControl(thickness, 0.1, 5, 0.1, tr("Thickness"));
    thicknessControl->setFixedWidth(180);

    connect(thicknessControl, SIGNAL(valueChanged(double)), this, SLOT(setBrushThickness(double)));

    borderLayout->addWidget(thickPreview);
    borderLayout->addWidget(thicknessControl);
    borderLayout->addStretch();
    borderLayout->setAlignment(Qt::AlignTop);

    addChild(borderWidget);

    boxLayout()->addStretch();
}

RasterSizeWidget::~RasterSizeWidget()
{
    #ifdef TUP_DEBUG
        qDebug() << "[~RasterSizeWidget()]";
    #endif
}

void RasterSizeWidget::setBrushThickness(double size)
{
    #ifdef TUP_DEBUG
        qDebug() << "[RasterSizeWidget::setBrushThickness()] - size ->" << size;
    #endif

    if (size > 0) {
        TCONFIG->beginGroup("BrushParameters");
        TCONFIG->setValue("RasterThickness", QString::number(size, 'g', 2));
        TCONFIG->sync();

        double param = size*10;
        int value = (int)param;
        thickPreview->render(value);

        emit brushSizeChanged(size);
    }
}

void RasterSizeWidget::setPenColor(const QColor &color)
{
    #ifdef TUP_DEBUG
        qDebug() << "[RasterSizeWidget::setPenColor()] - color ->" << color;
    #endif

    thickPreview->setColor(color);
}

void RasterSizeWidget::updateColor(QColor color)
{
    thickPreview->setColor(color);
}

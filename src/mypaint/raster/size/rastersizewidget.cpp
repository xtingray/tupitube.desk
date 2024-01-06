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
#include "tseparator.h"

RasterSizeWidget::RasterSizeWidget(QWidget *parent) : TupModuleWidgetBase(parent)
{
    setWindowIcon(QIcon(ICONS_DIR + "brush.png"));
    setWindowTitle(tr("Brush Size"));

    QWidget *borderWidget = new QWidget;
    QVBoxLayout *borderLayout = new QVBoxLayout(borderWidget);

    TCONFIG->beginGroup("BrushParameters");
    int thicknessValue = TCONFIG->value("Thickness", 3).toInt();
    if (thicknessValue > 100)
        thicknessValue = 3;

    thickPreview = new TupPenThicknessWidget(this);
    thickPreview->render(thicknessValue);

    thickness = new TEditSpinBox(thicknessValue, 1, 100, 1, tr("Thickness"));
    thickness->setValue(thicknessValue);
    thickness->setFixedWidth(180);

    connect(thickness, SIGNAL(valueChanged(int)), this, SLOT(setThickness(int)));
    connect(thickness, SIGNAL(valueChanged(int)), thickPreview, SLOT(render(int)));

    borderLayout->addWidget(thickPreview);
    borderLayout->addWidget(thickness);
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

void RasterSizeWidget::setThickness(int width)
{
    #ifdef TUP_DEBUG
        qDebug() << "[RasterSizeWidget::setThickness()]";
        qDebug() << "*** thickness: " << width;
    #endif

    if (width > 0) {
        pen.setWidth(width);
        TCONFIG->beginGroup("BrushParameters");
        TCONFIG->setValue("Thickness", width);
        updatePenProperties();
    }
}

void RasterSizeWidget::setPenColor(const QColor color)
{
    borderBrush.setColor(color);
    thickPreview->setColor(color);
}

void RasterSizeWidget::setPenThickness(int width)
{
    #ifdef TUP_DEBUG
        qDebug() << "[RasterSizeWidget::setPenThickness()]";
        qDebug() << "*** thickness: " << width;
    #endif

    pen.setWidth(width);
    TCONFIG->beginGroup("BrushParameters");
    TCONFIG->setValue("Thickness", width);
    thickPreview->render(width);

    thickness->blockSignals(true);
    thickness->setValue(width);
    thickness->blockSignals(false);
}

void RasterSizeWidget::init(int width)
{
    #ifdef TUP_DEBUG
        qDebug() << "[RasterSizeWidget::init()]";
        qDebug() << "*** thickness: " << width;
    #endif

    blockSignals(true);
    setPenColor(QColor(0, 0, 0));
    blockSignals(false);
    setThickness(width);
}

QPen RasterSizeWidget::getPen() const
{
    return pen;
}

void RasterSizeWidget::updatePenProperties()
{
    pen.setBrush(borderBrush);

    TupPaintAreaEvent event(TupPaintAreaEvent::ChangePen, pen);
    emit paintAreaEventTriggered(&event);
}

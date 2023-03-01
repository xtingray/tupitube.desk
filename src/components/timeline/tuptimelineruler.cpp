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

#include "tuptimelineruler.h"
#include "tconfig.h"

TupTimeLineRuler::TupTimeLineRuler(int fps, QWidget *parent): QHeaderView(Qt::Horizontal, parent)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupTimeLineRuler()]";
    #endif

    this->fps = fps;
    TCONFIG->beginGroup("General");
    themeName = TCONFIG->value("Theme", "Light").toString();

    setSectionResizeMode(QHeaderView::Custom);
    setHighlightSections(true);
    setStyleSheet("QHeaderView { background-color: #cccccc; }");
}

TupTimeLineRuler::~TupTimeLineRuler()
{
    #ifdef TUP_DEBUG
        qDebug() << "[~TupTimeLineRuler()]";
    #endif
}

void TupTimeLineRuler::updateFPS(int value)
{
    fps = value;
}

void TupTimeLineRuler::paintSection(QPainter *painter, const QRect & rect, int logicalIndex) const
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupTimeLineRuler::paintSection()] - RECT SIZE -> " << rect.size();
    #endif

    if (!model() || !rect.isValid())
        return;

    QModelIndex currentSelection = currentIndex(); 
    int column = currentSelection.row(); 

    // Cell is selected
    if (selectionModel()->isSelected(model()->index(column, logicalIndex))) {
        painter->fillRect(rect, QBrush(QColor(0, 135, 0, 80))); // Light Green
    } else {
        // Cell contains a number
        if ((logicalIndex + 1) == 1 || (logicalIndex + 1) % 5 == 0) {
            painter->fillRect(rect, QBrush(QColor(150, 150, 150, 255))); // Gray
        } else if ((logicalIndex + 1) % fps == 0) {
            painter->fillRect(rect, QBrush(QColor(48, 140, 198)));
        }
    }

    logicalIndex++;

    int x = rect.bottomRight().x();
    int topY = rect.topRight().y();
    int bottomY = rect.bottomRight().y();
    painter->drawLine(x, bottomY, x, bottomY - 6);
    painter->drawLine(x, topY, x, topY + 4);

    // Drawing numbers on the ruler
    if (logicalIndex == 1 || logicalIndex % 5 == 0) {
        QFont font = this->font();
        font.setPointSize(7);
        QFontMetrics fm(font);

        QString number = QString::number(logicalIndex);
        painter->setFont(font);	
        painter->drawText((rect.center().x() - (fm.horizontalAdvance(number)/2)),
                          (rect.center().y() + (fm.height()/2)) - 2, number);
    }

    QPen pen = painter->pen();
    pen.setWidth(1);
    pen.setColor(QColor(150, 150, 150));
    painter->setPen(pen); 
    painter->drawLine(rect.bottomLeft(), rect.bottomRight());
}

void TupTimeLineRuler::mousePressEvent(QMouseEvent *event)
{
    emit headerSelectionChanged(logicalIndexAt(event->pos()));
}

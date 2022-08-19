/***************************************************************************
 *   Project TUPITUBE DESK                                                 *
 *   Project Contact: info@maefloresta.com                                 *
 *   Project Website: http://www.maefloresta.com                           *
 *   Project Leader: Gustav Gonzalez <info@maefloresta.com>                *
 *                                                                         *
 *   Developers:                                                           *
 *   2019:                                                                 *
 *    Alejandro Carrasco Rodríguez                                         *
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

#include "trulerbase.h"

TRulerBase::TRulerBase(Qt::Orientation direction, QWidget *parent) : QFrame(parent)
{
    position = 0;
    ruleDirection = direction;
    drawPointer = false;
    ruleSeparation = 10;

    ruleZero = QPointF(0,0);
    pArrow = QPolygonF(3);

    sFactor = 1.0;

    if (ruleDirection == Qt::Horizontal) {

        setMaximumHeight(20);
        setMinimumHeight(20);

        ruleWidth = width();
        ruleHeight = height();

        pArrow << QPointF(0.0, 0.0);
        pArrow << QPointF(5.0, 5.0);
        pArrow << QPointF(10.0, 0.0);

        pArrow.translate(0,13);

    } else {

        setMaximumWidth(20);
        setMinimumWidth(20);

        ruleWidth = height();
        ruleHeight =  width();

        pArrow << QPointF(0.0, 0.0);
        pArrow << QPointF(5.0, 5.0);
        pArrow << QPointF(0.0, 10.0);

        pArrow.translate(13,0);
    }

    setMouseTracking(true);

    connect(this, SIGNAL(displayMenu(TRulerBase *, QPoint)), this, SLOT(showMenu(TRulerBase *, QPoint)));

    menu = new QMenu(this);

    QAction *to5 = menu->addAction(tr("Change scale to 5..."));
    QAction *to10 = menu->addAction(tr("Change scale to 10..."));

    connect(to5, SIGNAL(triggered()), this, SLOT(changeScaleTo5pts()));
    connect(to10, SIGNAL(triggered()), this, SLOT(changeScaleTo10pts()));
}


TRulerBase::~TRulerBase()
{
    // delete k;
}

void TRulerBase::paintEvent(QPaintEvent *)
{
    QPainter p(this);

    if (ruleDirection == Qt::Vertical)
        p.scale(1.0, sFactor);
    else
        p.scale(sFactor, 1.0);

    drawScale(&p);
    // p.setBrush(palette().color(QPalette::Foreground));
    p.setBrush(palette().color(QPalette::QPalette::WindowText));

    p.save();

    p.drawConvexPolygon(pArrow);
    p.restore();

    p.end();
	
}

void TRulerBase::drawScale(QPainter *painter)
{
    painter->save();
    QFont tfont(font().family(), 7);
    QFontMetrics fm(tfont);
    painter->setFont(tfont);

    int fact = 1;

    if (ruleDirection == Qt::Horizontal) {
        painter->translate(ruleZero.x(), 0);
        painter->drawLine(-390, height()-1, width(), height()-1);
    } else {
        painter->drawLine(width()-1, 0, width()-1, height());
        fact = -1;
        painter->translate(0, ruleZero.y());
        painter->rotate(90);
    }

    int ypos = ruleHeight*fact;
    int ytext = ruleHeight/2;

    for (int i = 0; i < ruleWidth; i += ruleSeparation) {
         QSize sizeFont = fm.size(Qt::TextSingleLine, QString::number(i));
         if (i % 100 == 0) { // FIX ME
             painter->drawLine (i, ypos, i, 0);
             int dx = i + 3;
             if (ruleDirection == Qt::Vertical)
                 painter->drawText(QPoint(dx, ytext - sizeFont.height()), QString::number(i));
             else
                 painter->drawText(QPoint(dx, ytext), QString::number(i));
         } else {
                 painter->drawLine (i, ypos, i, ypos - ruleHeight/4*fact);
         }
    }

   for (int i = 0; i > -390 ; i -= ruleSeparation) {
         //cout << "Var: " << i << endl;
         QSize sizeFont = fm.size (Qt::TextSingleLine, QString::number(i));
         if (i % 100  == 0) { // FIX ME
             painter->drawLine(i, ypos, i, 0);
             int dx = i + 3;
             if (ruleDirection == Qt::Vertical)
                 painter->drawText(QPoint(dx, ytext - sizeFont.height()), QString::number(i));
             else
                 painter->drawText(QPoint(dx, ytext), QString::number(i));
         } else {
             painter->drawLine(i, ypos, i, ypos - ruleHeight/4*fact);
         }
    }

    painter->restore();
}

void TRulerBase::resizeEvent(QResizeEvent *)
{
    if (ruleDirection == Qt::Horizontal) {
        ruleWidth = width();
        ruleHeight = height();
    } else if (ruleDirection == Qt::Vertical) {
        ruleWidth = height();
        ruleHeight =  width();
    }

    update();
}

void TRulerBase::mouseMoveEvent(QMouseEvent *event)
{
    if (drawPointer)
        movePointers(event->pos());
}

void TRulerBase::setDrawPointer(bool flag)
{
    drawPointer = flag;
    update();
}

void TRulerBase::setSeparation(int sep)
{
    if (sep > 0 && sep <= 10000) {
        ruleSeparation = sep;
        update();
    } else {
        #ifdef TUP_DEBUG
            qDebug() << "TRulerBase::setSeparation() - Error: Can't assign separation : " + QString::number(sep);
        #endif
    }
}

void TRulerBase::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::RightButton)
        emit displayMenu(this, event->globalPos());
}

Qt::Orientation TRulerBase::orientation()
{
    return ruleDirection;
}

int TRulerBase::separation() const
{
    return ruleSeparation;
}

double TRulerBase::scaleFactor() const
{
    return sFactor;
}

void TRulerBase::showMenu(TRulerBase *ruler, QPoint pos)
{
    if (ruler)
        menu->popup(pos);
}

void TRulerBase::slide(int value)
{
    int distance = -value + ruleHeight;

    if (ruleDirection == Qt::Horizontal)
        move(distance, pos().y());
    else
        move(pos().x(), distance);
}

QPointF TRulerBase::zero() const
{
    return ruleZero;
}

void TRulerBase::translateArrow(double dx, double dy)
{
    pArrow.translate(dx, dy);
}

void TRulerBase::setZeroAt(const QPointF & pos)
{
    ruleZero = pos;
    update();
}

void TRulerBase::scale(double factor)
{
    sFactor = factor;
    update();
}

QSize TRulerBase::sizeHint() const
{
    int distance = ruleWidth/3;

    if (ruleDirection == Qt::Horizontal)
        return QSize(distance, height());

    return QSize(width(), distance);
}

void TRulerBase::changeScaleTo5pts()
{
    setSeparation(5);
}

void TRulerBase::changeScaleTo10pts()
{
    setSeparation(10);
}

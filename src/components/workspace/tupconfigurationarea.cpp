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

#include "tupconfigurationarea.h"

#include <QMainWindow>
#include <QApplication>
#include <QToolTip>
#include <QPainter>
#include <QStyle>
#include <QStyleOptionButton>
#include <QMouseEvent>

TupConfigurationArea::TupConfigurationArea(QWidget *parent) : QDockWidget(parent)
{
    toolTipShowed = false;
    setAllowedAreas(Qt::RightDockWidgetArea);

    connect(&shower, SIGNAL(timeout()), this, SLOT(showConfigurator()));
}

TupConfigurationArea::~TupConfigurationArea()
{
    if (widget()) {
        widget()->hide();
        widget()->setParent(0);
    }
}

QSize TupConfigurationArea::sizeHint() const
{
    if (widget())
        return widget()->minimumSizeHint();

    return QDockWidget::sizeHint();
}

void TupConfigurationArea::setConfigurator(QWidget *w, int minWidth)
{
    Q_CHECK_PTR(w);

    QWidget *old = this->widget();

    if (!w || old == w) 
        return;

    setWidget(w);

    if (minWidth > 0) {
        setMinimumWidth(minWidth);
        setMaximumWidth(minWidth);
    }

    showConfigurator();
}

void TupConfigurationArea::toggleLock()
{
    locker.stop();
    hideConfigurator();
}

void TupConfigurationArea::shrink()
{
    #ifdef TUP_DEBUG
        #ifdef Q_OS_WIN
            qDebug() << "[TupConfigurationArea::shrink()]";
        #else
            T_FUNCINFO;
        #endif
    #endif

    QMainWindow *mainWindow = dynamic_cast<QMainWindow *>(parentWidget());
    if (!mainWindow || !widget()) {
        #ifdef TUP_DEBUG
            QString msg = "TupConfigurationArea::shrink() - Fatal error!";
            #ifdef Q_OS_WIN
                qDebug() << msg;
            #else
                tError() << msg;
            #endif
        #endif
        return;
    }

    bool mouseTrackingFlag = mainWindow->hasMouseTracking();
    int pm = style()->pixelMetric(QStyle::PM_DockWidgetSeparatorExtent);

    mainWindow->setMouseTracking(true);

    int wOffset = 0;
    int hOffset= 0;

    Qt::DockWidgetArea position = mainWindow->dockWidgetArea(this);

    if (position == Qt::BottomDockWidgetArea) {
        wOffset = 20;
        hOffset = -(y() * 2 + pm - 1); // SQA: FIXME FIXME FIXME
    } else if (position == Qt::LeftDockWidgetArea) {
        wOffset = width()+(pm/2)+1;
        hOffset = height() / 2;
    } else if (position == Qt::RightDockWidgetArea) {
        wOffset = -(pm/2)+1;
        hOffset = height() / 2;
    }

    QMouseEvent press(QEvent::MouseButtonPress,
                      mapToParent( QPoint(this->x(), this->y()))/2 + QPoint(wOffset, hOffset),
                      Qt::LeftButton, 0, 0);

    if (! QApplication::sendEvent(mainWindow, &press))
        qWarning("Fail pressing");

    qApp->processEvents();

    int df = 0;
    int x1 = 0;
    int x2 = 0;
    int y1 = 0;
    int y2 = 0;
    int xRelease = 0;
    int yRelease = 0;

    if (position == Qt::BottomDockWidgetArea) {
        df = widget()->height();
        x1 = press.pos().x();
        y1 = press.pos().y() + df;

        x2 = press.globalPos().x();
        y2 = press.globalPos().y() + df;

        xRelease = this->x();
        yRelease = 10;
    } else if (position == Qt::LeftDockWidgetArea) {
               df = widget()->width();
               x1 = press.pos().x() - df;
               y1 = press.pos().y();

               x2 = press.globalPos().x() - df;
               y2 = press.globalPos().y();

               xRelease = 10;
               yRelease = this->y();
    } else if (position == Qt::RightDockWidgetArea) {
               df = widget()->width();
               x1 = press.pos().x() + df;
               y1 = press.pos().y();

               x2 = press.globalPos().x() + df;
               y2 = press.globalPos().y();

               xRelease = mainWindow->width();
               yRelease = this->y();
    }

    QMouseEvent move(QEvent::MouseMove,
                     QPoint(x1, y1),
                     QPoint(x2, y2),
                     Qt::LeftButton, 0, 0);

    if (! QApplication::sendEvent(mainWindow, &move)) {
        #ifdef TUP_DEBUG
            QString msg = "TupConfigurationArea::shrink() - Error while moving!";
            #ifdef Q_OS_WIN
                qWarning() << msg;
            #else
                tWarning() << msg;
            #endif
        #endif
    }

    qApp->processEvents();

    QMouseEvent release(QEvent::MouseButtonRelease,
                        QPoint(xRelease, yRelease),
                        Qt::LeftButton, 0, 0);

    if (! QApplication::sendEvent(mainWindow, &release)) {
        #ifdef TUP_DEBUG
            QString msg = "TupConfigurationArea::shrink() - Error while releasing!";
            #ifdef Q_OS_WIN
                qWarning() << msg;
            #else
                tWarning() << msg;
            #endif
        #endif
    }

    qApp->processEvents();
    mainWindow->setMouseTracking(mouseTrackingFlag);
}

void TupConfigurationArea::enterEvent(QEvent *event)
{
    Q_UNUSED(event);

    if (locker.isActive())
        locker.stop();

    if (shower.isActive())
        return;

    shower.start(300);
}

void TupConfigurationArea::leaveEvent(QEvent *event)
{
    Q_UNUSED(event);

    if (shower.isActive())
        shower.stop();

    if (locker.isActive() || rect().contains(mapFromGlobal(QCursor::pos())) || hasFocus())
        return;

    locker.start(1000);
}

void TupConfigurationArea::showConfigurator()
{
    QWidget *widget = this->widget();

    if (widget && !isFloating()) {
        widget->setVisible(true);

        QPalette pal = parentWidget()->palette();
        setPalette(pal);
        setAutoFillBackground(false);

        setFeatures(QDockWidget::AllDockWidgetFeatures);
    }

    shower.stop();
    mousePos = QCursor::pos();
}

void TupConfigurationArea::hideConfigurator()
{
    QWidget *widget = this->widget();

    if (widget && !isFloating ()) {
        widget->setVisible(false);
        setFeatures(QDockWidget::NoDockWidgetFeatures);

        QPalette pal = palette();
        pal.setBrush(QPalette::Background, pal.button());
        setPalette(pal);
        setAutoFillBackground(true);

        for (int i = 0; i < 2; ++i) 
             qApp->processEvents();

        shrink();

        if (!toolTipShowed) {
            QToolTip::showText (mousePos, tr("Cursor here for expand"), this);
            toolTipShowed = true;
        }
    }

    mousePos = QCursor::pos();
}

void TupConfigurationArea::paintEvent(QPaintEvent *event)
{
    QDockWidget::paintEvent(event);
    bool draw = false;

    if (widget()) {
        if (widget()->isVisible())
            draw = false;
        else
            draw = true;
    } else {
            draw = false;
    }

    if (draw) {
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing, true);
        painter.setRenderHint(QPainter::TextAntialiasing, true);

        QFont font("Times", 16, QFont::Bold);
        painter.setFont(font);

        QStyleOptionButton buttonOption;
        buttonOption.initFrom(this);
		
        buttonOption.text = tr("Properties");
        buttonOption.icon = QIcon();
        buttonOption.palette = palette();
        buttonOption.rect = rect();
        buttonOption.state = QStyle::State_On;

        buttonOption.features = QStyleOptionButton::DefaultButton;

        style()->drawControl(QStyle::CE_PushButton, &buttonOption, &painter, this);		
    }
}

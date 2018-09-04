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

#include "toolview.h"

ToolView::ToolView(const QString &title, const QIcon &icon, const QString &code, QWidget * parent)
          : QDockWidget(title, parent), m_size(-1), m_perspective(0)
{
    setFeatures(AllDockWidgetFeatures);
    setWindowIcon(icon);
    setup(title);
    setObjectName("ToolView-" + code);
    name = title;
    expanded = false;
}

ToolView::~ToolView()
{
}

void ToolView::setup(const QString &label)
{
    m_button = new TViewButton(this);
    m_button->setToolTip(label);

    connect(toggleViewAction(), SIGNAL(toggled(bool)), this, SLOT(saveSize(bool)));
}

TViewButton *ToolView::button() const
{
    return m_button;
}

void ToolView::expandDock(bool flag)
{
    #ifdef TUP_DEBUG
        #ifdef Q_OS_WIN
            qDebug() << "[ToolView::expandDock()]";
        #else
            T_FUNCINFO;
        #endif
    #endif

    expanded = flag;
    if (flag)
        show();
    else 
        close();

    m_button->setActivated(flag);
}

bool ToolView::isExpanded()
{
    return expanded;
}

void ToolView::setExpandingFlag() 
{
    #ifdef TUP_DEBUG
        #ifdef Q_OS_WIN
            qDebug() << "[ToolView::setExpandingFlag()]";
        #else
            T_FUNCINFO;
        #endif
    #endif

    if (expanded)
        expanded = false;
    else 
        expanded = true;

    // emit dockExpanded(expanded); 
}

void ToolView::setSizeHint() 
{
    #ifdef TUP_DEBUG
        #ifdef Q_OS_WIN
            qDebug() << "[ToolView::setSizeHint()]";
        #else
            T_FUNCINFO;
        #endif
    #endif

    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
}

void ToolView::saveSize(bool checked)
{
    #ifdef TUP_DEBUG
        #ifdef Q_OS_WIN
            qDebug() << "[ToolView::saveSize()]";
        #else
            T_FUNCINFO;
        #endif
    #endif

    Q_UNUSED(checked);

    if (m_button->area() == Qt::LeftToolBarArea || m_button->area() == Qt::RightToolBarArea)
        m_size = width();
    else
        m_size = height();
}

QSize ToolView::sizeHint() const
{
    #ifdef TUP_DEBUG
        #ifdef Q_OS_WIN
            qDebug() << "[ToolView::sizeHint()]";
        #else
            T_FUNCINFO;
        #endif
    #endif

    QSize size = QDockWidget::sizeHint();

    if (m_size < 0) 
        return size;

    if (m_button->area() == Qt::LeftToolBarArea || m_button->area()  == Qt::RightToolBarArea)
        size.setWidth(m_size);
    else
        size.setHeight(m_size);

    return size;
}

void ToolView::setDescription(const QString &description)
{
    m_button->setStatusTip(description);
}

void ToolView::setShortcut(QKeySequence shortcut)
{
    m_button->setShortcut(shortcut);
}

void ToolView::setPerspective(int wsp)
{
    m_perspective = wsp;
}

int ToolView::perspective() const
{
    return m_perspective;
}

int ToolView::fixedSize() const
{
    #ifdef TUP_DEBUG
        #ifdef Q_OS_WIN
            qDebug() << "[ToolView::fixedSize()]";
        #else
            T_FUNCINFO;
        #endif
    #endif

    return m_size;
}

void ToolView::setFixedSize(int s)
{
    #ifdef TUP_DEBUG
        #ifdef Q_OS_WIN
            qDebug() << "[ToolView::setFixedSize()]";
        #else
            T_FUNCINFO;
        #endif
    #endif

    m_size = s;
}

void ToolView::showEvent(QShowEvent *event)
{
    #ifdef TUP_DEBUG
        #ifdef Q_OS_WIN
            qDebug() << "[ToolView::showEvent()]";
        #else
            T_FUNCINFO << name;
        #endif
    #endif

    if (TMainWindow *mw = dynamic_cast<TMainWindow *>(parentWidget())) {
        if (!(mw->currentPerspective() & m_perspective)) {
            event->ignore(); // make sure!
            return;
        }
    }

    QDockWidget::showEvent(event);
}

void ToolView::enableButton(bool flag)
{
    m_button->setEnabled(flag);
}

QString ToolView::getObjectID() 
{
    return objectName();
}

bool ToolView::isChecked()
{
    return m_button->isChecked();
}

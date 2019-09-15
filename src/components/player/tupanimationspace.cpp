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

#include "tupanimationspace.h"

TupAnimationspace::TupAnimationspace(TupCameraWidget *playerUI, QWidget *parent): QWidget(parent)
{
    // TODO: Try a nice dark color for this window
    // setStyleSheet("QMainWindow { background-color: #d0d0d0; }");

    playerInterface = playerUI;
    playOn = false;
    setCameraWidget(playerInterface);
}

TupAnimationspace::~TupAnimationspace()
{
    #ifdef TUP_DEBUG
         qDebug() << "[~TupCameraWidget()]";
    #endif

    delete playerInterface;
    playerInterface = nullptr;
}

void TupAnimationspace::setCameraWidget(TupCameraWidget *playerUI) 
{
    QBoxLayout *layout = new QBoxLayout(QBoxLayout::TopToBottom, this);
    playerInterface = playerUI;
    layout->addWidget(playerInterface, 0, Qt::AlignCenter);
}

void TupAnimationspace::mousePressEvent(QMouseEvent *event)
{
    Q_UNUSED(event);
    setFocus();
}

void TupAnimationspace::keyPressEvent(QKeyEvent *event) 
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupAnimationspace::keyPressEvent(QKeyEvent)]";
    #endif

    switch (event->key()) {
        case Qt::Key_P:
        case Qt::Key_Space:
          {
              playerInterface->doPause();
          }
        break;
        case Qt::Key_Escape:
          {
              setFocus();
              playOn = false;
              playerInterface->doStop();
          }
        break;
        case Qt::Key_Right:
          {
              playerInterface->nextFrame();
          }
        break;
        case Qt::Key_Left:
          {
              playerInterface->previousFrame();
          }
        break;
        case Qt::Key_Return:
          {
              playOn = false;
              playerInterface->doStop();
              playerInterface->clearMemory();
              emit newPerspective(0);
          }
        break;
        case Qt::Key_1:
          {
              if (event->modifiers() == Qt::ControlModifier)
                  emit newPerspective(0);
          }
        break;
        case Qt::Key_3:
          {
              if (event->modifiers() == Qt::ControlModifier)
                  emit newPerspective(2);
          }
        break;
    }
}

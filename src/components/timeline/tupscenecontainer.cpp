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

#include "tupscenecontainer.h"

TupSceneContainer::TupSceneContainer(QWidget *parent) : QTabWidget(parent)
{
}

TupSceneContainer::~TupSceneContainer()
{
}

void TupSceneContainer::addScene(int sceneIndex, TupTimeLineTable *framesTable, const QString &sceneName)
{
    #ifdef TUP_DEBUG
        #ifdef Q_OS_WIN
            qDebug() << "[TupSceneContainer::addScene()]";
        #else
            T_FUNCINFO;
        #endif
    #endif

    scenes << framesTable;
    QTabWidget::insertTab(sceneIndex, framesTable, sceneName);
}

void TupSceneContainer::restoreScene(int sceneIndex, const QString &sceneName)
{
    TupTimeLineTable *framesTable = undoScenes.takeLast();
    scenes << framesTable;
    QTabWidget::insertTab(sceneIndex, framesTable, sceneName);
}

void TupSceneContainer::removeScene(int sceneIndex, bool withBackup)
{
    #ifdef TUP_DEBUG
        #ifdef Q_OS_WIN
            qDebug() << "[TupSceneContainer::removeScene()]";
        #else
            T_FUNCINFO;
        #endif
    #endif

    if (withBackup)
        undoScenes << scenes.takeAt(sceneIndex);
    else
        scenes.takeAt(sceneIndex);

    QTabWidget::removeTab(sceneIndex);
}

void TupSceneContainer::renameScene(int index, const QString &name)
{
    setTabText(index, name);
}

void TupSceneContainer::removeAllScenes()
{
    clear();
    scenes.clear();
    undoScenes.clear();
}

TupTimeLineTable * TupSceneContainer::currentScene()
{
    int index = currentIndex();
    TupTimeLineTable *framesTable = scenes.at(index);

    return framesTable;
}

TupTimeLineTable * TupSceneContainer::getTable(int index)
{
    TupTimeLineTable *framesTable = scenes.at(index);
    return framesTable;
}

int TupSceneContainer::count()
{
    return scenes.count();
}

bool TupSceneContainer::isTableIndexValid(int index)
{
    if (index > -1 && index < scenes.count())
        return true;

    return false;
}

#ifndef QT_NO_WHEELEVENT
void TupSceneContainer::wheelEvent(QWheelEvent *ev)
{
    QRect rect = tabBar()->rect();
    rect.setWidth(width());

    if (rect.contains(ev->pos()))
        wheelMove(ev->delta());
}

void TupSceneContainer::wheelMove(int delta)
{
    if (count() > 1) {
        int current = currentIndex();
        if (delta < 0) {
            current = (current + 1) % count();
        } else {
            current--;
            if (current < 0)
                current = count() - 1;
        }
        setCurrentIndex(current);
    }
}

#endif

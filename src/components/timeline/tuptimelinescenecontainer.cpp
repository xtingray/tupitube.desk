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

#include "tuptimelinescenecontainer.h"

TupTimelineSceneContainer::TupTimelineSceneContainer(QWidget *parent) : QTabWidget(parent)
{
}

TupTimelineSceneContainer::~TupTimelineSceneContainer()
{
}

void TupTimelineSceneContainer::addScene(int sceneIndex, TupTimeLineTable *framesTable, const QString &sceneName)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupTimelineSceneContainer::addScene()]";
    #endif

    scenes << framesTable;
    QTabWidget::insertTab(sceneIndex, framesTable, sceneName);
    connect(tabBar(), SIGNAL(tabBarDoubleClicked(int)), this, SIGNAL(sceneRenameRequested(int)));
}

void TupTimelineSceneContainer::restoreScene(int sceneIndex, const QString &sceneName)
{
    TupTimeLineTable *framesTable = undoScenes.takeLast();
    scenes << framesTable;
    QTabWidget::insertTab(sceneIndex, framesTable, sceneName);
    connect(tabBar(), SIGNAL(tabBarDoubleClicked(int)), this, SLOT(editSceneLabel(int)));
}

void TupTimelineSceneContainer::removeScene(int sceneIndex, bool withBackup)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupTimelineSceneContainer::removeScene()]";
    #endif

    if (withBackup)
        undoScenes << scenes.takeAt(sceneIndex);
    else
        scenes.takeAt(sceneIndex);

    QTabWidget::removeTab(sceneIndex);
}

void TupTimelineSceneContainer::renameScene(int index, const QString &name)
{
    setTabText(index, name);
}

void TupTimelineSceneContainer::removeAllScenes()
{
    blockSignals(true);
    clear();
    scenes.clear();
    undoScenes.clear();
    blockSignals(false);
}

TupTimeLineTable * TupTimelineSceneContainer::currentScene()
{
    int index = currentIndex();
    TupTimeLineTable *framesTable = nullptr;

    if (index < scenes.size())
        framesTable = scenes.at(index);

    return framesTable;
}

TupTimeLineTable * TupTimelineSceneContainer::getTable(int index)
{
    TupTimeLineTable *framesTable = scenes.at(index);

    return framesTable;
}

int TupTimelineSceneContainer::count()
{
    return scenes.count();
}

bool TupTimelineSceneContainer::isTableIndexValid(int index)
{
    if (index > -1 && index < scenes.count())
        return true;

    return false;
}

#ifndef QT_NO_WHEELEVENT
void TupTimelineSceneContainer::wheelEvent(QWheelEvent *ev)
{
    QRect rect = tabBar()->rect();
    rect.setWidth(width());

    // SQA: Evaluate this replacement (delta)
    if (rect.contains(ev->position().toPoint()))
        wheelMove(ev->angleDelta().y());
}

void TupTimelineSceneContainer::wheelMove(int delta)
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

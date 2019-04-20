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

#include "shifttool.h"
#include "tconfig.h"
#include "tuprectitem.h"
#include "tupellipseitem.h"
#include "tuplineitem.h"
#include "tupscene.h"
#include "tupinputdeviceinformation.h"
#include "tupgraphicsscene.h"

ShiftTool::ShiftTool()
{
    activeView = "WORKSPACE";
    rect = 0;
    scene = 0;

    setupActions();
}

ShiftTool::~ShiftTool()
{
}

void ShiftTool::init(TupGraphicsScene *gScene)
{
    scene = gScene;
}

QStringList ShiftTool::keys() const
{
    return QStringList() << tr("Shift");
}

void ShiftTool::setupActions()
{
    TAction *shiftAction = new TAction(QIcon(kAppProp->themeDir() + "icons/shift.png"), tr("Shift"), this);
    shiftAction->setShortcut(QKeySequence(tr("H")));
    shiftAction->setToolTip(tr("Shift") + " - " + "H");

    shiftCursor = QCursor(kAppProp->themeDir() + "cursors/shift.png");
    shiftAction->setCursor(shiftCursor);

    shiftActions.insert(tr("Shift"), shiftAction);
}

void ShiftTool::press(const TupInputDeviceInformation *input, TupBrushManager *brushManager, TupGraphicsScene *gScene)
{
    Q_UNUSED(input);
    Q_UNUSED(brushManager);
    Q_UNUSED(gScene);

    added = false;
    rect = new QGraphicsRectItem(QRectF(input->pos(), QSize(0,0)));
    rect->setPen(QPen(Qt::red, 1, Qt::SolidLine));

    firstPoint = input->pos();
}

void ShiftTool::move(const TupInputDeviceInformation *input, TupBrushManager *brushManager, TupGraphicsScene *gScene)
{
    Q_UNUSED(input);
    Q_UNUSED(brushManager);

    if (name() == tr("Shift")) 
        scene = gScene; // <- SQA: Trace this variable
}

void ShiftTool::release(const TupInputDeviceInformation *input, TupBrushManager *brushManager, TupGraphicsScene *gScene)
{
    Q_UNUSED(brushManager);

    if (name() == tr("Shift")) {
        currentCenter = input->pos();
        foreach (QGraphicsView *view, gScene->views()) {
            if (activeView.compare(view->accessibleName()) == 0) {
                view->centerOn(currentCenter);
                view->setSceneRect(input->pos().x() - (projectSize.width()/2), input->pos().y() - (projectSize.height()/2),
                                   projectSize.width(), projectSize.height());
                break;
            }  
        }
    } 
}

void ShiftTool::setProjectSize(const QSize size)
{
    projectSize = size;
}

QMap<QString, TAction *> ShiftTool::actions() const
{
    return shiftActions;
}

int ShiftTool::toolType() const
{
    return TupToolInterface::View;
}

QWidget *ShiftTool::configurator()
{
    return 0;
}

void ShiftTool::aboutToChangeScene(TupGraphicsScene *)
{
}

void ShiftTool::aboutToChangeTool()
{
    if (name() == tr("Shift")) {
       if (scene) {
           foreach (QGraphicsView * view, scene->views())
                    view->setDragMode(QGraphicsView::NoDrag);
       }
    }
}

void ShiftTool::saveConfig()
{
}

void ShiftTool::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_F11 || event->key() == Qt::Key_Escape) {
        emit closeHugeCanvas();
    } else {
        QPair<int, int> flags = TupToolPlugin::setKeyAction(event->key(), event->modifiers());
        if (flags.first != -1 && flags.second != -1)
            emit callForPlugin(flags.first, flags.second);
    }
}

QCursor ShiftTool::cursor() const
{
   if (name() == tr("Shift"))
       return shiftCursor;

   return QCursor(Qt::ArrowCursor);
}

void ShiftTool::setActiveView(const QString &viewID)
{
    activeView = viewID;
}

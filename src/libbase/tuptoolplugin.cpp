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

#include "tuptoolplugin.h"

TupToolPlugin::TupToolPlugin(QObject *parent) : QObject(parent)
{
}

TupToolPlugin::~TupToolPlugin()
{
}

void TupToolPlugin::init(TupGraphicsScene *scene)
{
    Q_UNUSED(scene)
}

void TupToolPlugin::updateScene(TupGraphicsScene *scene)
{
    Q_UNUSED(scene)
}

void TupToolPlugin::setCurrentToolName(const QString &tool)
{
    currentTool = tool;
}

QString TupToolPlugin::currentToolName() const
{
    return currentTool;
}

void TupToolPlugin::setToolId(TAction::ActionId code)
{
    currentId = code;
}

TAction::ActionId TupToolPlugin::toolId()
{
    return currentId;
}

void TupToolPlugin::begin()
{
    #ifdef TUP_DEBUG
        qWarning() << "[TupToolPlugin::begin()] - Begin: " << currentTool;
    #endif
}

void TupToolPlugin::end()
{
    #ifdef TUP_DEBUG
        qWarning() << "[TupToolPlugin::end()] - End: " << currentTool;
    #endif
}

void TupToolPlugin::sceneResponse(const TupSceneResponse *event)
{
    Q_UNUSED(event)
}

void TupToolPlugin::layerResponse(const TupLayerResponse *event)
{
    Q_UNUSED(event)
}

void TupToolPlugin::frameResponse(const TupFrameResponse *event)
{
    Q_UNUSED(event)
}

void TupToolPlugin::libraryResponse(const TupLibraryResponse *event)
{
    Q_UNUSED(event)
}

void TupToolPlugin::itemResponse(const TupItemResponse *event)
{
    Q_UNUSED(event)
}

void TupToolPlugin::doubleClick(const TupInputDeviceInformation *, TupGraphicsScene *)
{
}

void TupToolPlugin::aboutToChangeScene(TupGraphicsScene *scene)
{
    Q_UNUSED(scene)
}

void TupToolPlugin::keyPressEvent(QKeyEvent *event)
{
    Q_UNUSED(event)
}

void TupToolPlugin::keyReleaseEvent(QKeyEvent *event)
{
    Q_UNUSED(event)
}

QMenu *TupToolPlugin::menu() const
{
    QMenu *menu = new QMenu(new QWidget);
    return menu;
}

void TupToolPlugin::saveConfig() 
{
}

QCursor TupToolPlugin::toolCursor() // const
{
    return QCursor(Qt::ArrowCursor); 
}

void TupToolPlugin::updatePos(QPointF pos)
{
    Q_UNUSED(pos)
}

void TupToolPlugin::resizeNode(qreal factor)
{
    Q_UNUSED(factor)
}

void TupToolPlugin::updateZoomFactor(qreal factor)
{
    Q_UNUSED(factor)
}

void TupToolPlugin::setProjectSize(const QSize size)
{
    Q_UNUSED(size)
}

/*
QPair<int, int> TupToolPlugin::setKeyAction(int key, Qt::KeyboardModifiers modifiers)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupToolPlugin::setKeyAction()] - key -> " << key;
    #endif

    TAction::MenuId menu = TAction::BrushesMenu;
    int tool = TAction::Pencil;

    if (modifiers & Qt::ControlModifier) {
        switch (key) {
            case Qt::Key_Right:
            {
                menu = TAction::Arrows;
                tool = TAction::Right_QuickCopy;
            }
            break;
            case Qt::Key_Left:
            {
                menu = TAction::Arrows;
                tool = TAction::Left_Delete;
            }
            break;
            case Qt::Key_Up:
            {
                menu = TAction::Arrows;
                tool = TAction::Up_Delete;
            }
            break;
            case Qt::Key_Down:
            {
                menu = TAction::Arrows;
                tool = TAction::Down_QuickCopy;
            }
            break;
        }
    } else {
        switch (key) {
            case Qt::Key_P:
            {
                 if (modifiers == Qt::ShiftModifier) {
                     menu = TAction::ColorMenu;
                     tool = TAction::ColorPalette;
                 } else {
                     tool = TAction::Pencil;
                 }
            }
            break;

            case Qt::Key_K:
            {
                 tool = TAction::Ink;
            }
            break;

            case Qt::Key_S:
            {
                 tool = TAction::Polyline;
            }
            break;

            case Qt::Key_L:
            {
                 tool = TAction::Line;
            }
            break;

            case Qt::Key_R:
            {
                tool = TAction::Rectangle;
            }
            break;

            case Qt::Key_C:
            {
                tool = TAction::Ellipse;
            }
            break;

            case Qt::Key_T:
            {
                tool = TAction::Text;
            }
            break;

            case Qt::Key_O:
            {
                menu = TAction::SelectionMenu;
                tool = TAction::ObjectSelection;
            }
            break;

            case Qt::Key_N:
            {
                menu = TAction::SelectionMenu;
                tool = TAction::NodesEditor;
            }
            break;

            case Qt::Key_F:
            {
                menu = TAction::FillMenu;
                tool = TAction::BucketTool;
            }
            break;

            case Qt::Key_Left:
            {
                menu = TAction::Arrows;
                tool = TAction::Left_Arrow;
            }
            break;

            case Qt::Key_Right:
            {
                menu = TAction::Arrows;
                tool = TAction::Right_Arrow;
            }
            break;

            case Qt::Key_PageUp:
            case Qt::Key_Up:
            {
                menu = TAction::Arrows;
                tool = TAction::Up_Arrow;
            }
            break;

            case Qt::Key_PageDown:
            case Qt::Key_Down:
            {
                menu = TAction::Arrows;
                tool = TAction::Down_Arrow;
            }
            break;

            case Qt::Key_Delete:
            {
                menu = TAction::SelectionMenu;
                tool = TAction::Delete;
            }
            break;

            case Qt::Key_E:
            {
                menu = TAction::ColorMenu;
                tool = TAction::EyeDropper;
            }
            break;

            default:
            {
                menu = TAction::InvalidMenu;
                tool = TAction::NoAction;
            }
        }
    }

    QPair<int, int> flags;
    flags.first = menu;
    flags.second = tool;

    return flags;
}
*/

TupToolPlugin::Mode TupToolPlugin::currentMode()
{
    return TupToolPlugin::View;
}

TupToolPlugin::EditMode TupToolPlugin::currentEditMode()
{
    return TupToolPlugin::None;
}

void TupToolPlugin::setActiveView(const QString &viewID)
{
    Q_UNUSED(viewID)
}

void TupToolPlugin::setCurrentItem(const QString &id)
{
    Q_UNUSED(id)
}

void TupToolPlugin::updateWorkSpaceContext()
{
}

void TupToolPlugin::clearSelection()
{
}

void TupToolPlugin::setColorMode(TColorCell::FillType mode)
{
    Q_UNUSED(mode)
}

void TupToolPlugin::updatePressure(qreal pressure)
{
    Q_UNUSED(pressure)
}

void TupToolPlugin::updateTextColor(const QColor &color)
{
    Q_UNUSED(color)
}

void TupToolPlugin::updateColorType(TColorCell::FillType fillType)
{
    Q_UNUSED(fillType)
}

void TupToolPlugin::refreshEyeDropperPanel()
{
}

void TupToolPlugin::updateColorOnSelection(TupProjectRequest::Action action, const QColor &color)
{
    Q_UNUSED(action)
    Q_UNUSED(color)
}

void TupToolPlugin::updatePenOnSelection(const QPen &pen)
{
    Q_UNUSED(pen)
}

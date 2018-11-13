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

TupToolPlugin::TupToolPlugin(QObject * parent) : QObject(parent) // , k(new Private)
{
}

TupToolPlugin::~TupToolPlugin()
{
}

void TupToolPlugin::init(TupGraphicsScene *scene)
{
    Q_UNUSED(scene);
}

void TupToolPlugin::updateScene(TupGraphicsScene *scene)
{
    Q_UNUSED(scene);
}

void TupToolPlugin::setName(const QString &tool)
{
    currentTool = tool;
}

QString TupToolPlugin::name() const
{
    return currentTool;
}

void TupToolPlugin::begin()
{
#ifdef TUP_DEBUG
    QString msg = "TupToolPlugin::begin() - Begin: " + currentTool;
    #ifdef Q_OS_WIN
        qWarning() << msg;
    #else
        tWarning() << msg;
    #endif
#endif
}

void TupToolPlugin::end()
{
#ifdef TUP_DEBUG
    QString msg = "TupToolPlugin::end() - End: " + currentTool;
    #ifdef Q_OS_WIN
        qWarning() << msg;
    #else
        tWarning("tools") << msg;
    #endif
#endif
}

void TupToolPlugin::sceneResponse(const TupSceneResponse *event)
{
    Q_UNUSED(event);
}

void TupToolPlugin::layerResponse(const TupLayerResponse *event)
{
    Q_UNUSED(event);
}

void TupToolPlugin::frameResponse(const TupFrameResponse *event)
{
    Q_UNUSED(event);
}

void TupToolPlugin::itemResponse(const TupItemResponse *event)
{
    Q_UNUSED(event);
}

void TupToolPlugin::doubleClick(const TupInputDeviceInformation *, TupGraphicsScene *)
{
}

void TupToolPlugin::aboutToChangeScene(TupGraphicsScene *scene)
{
    Q_UNUSED(scene);
}

void TupToolPlugin::keyPressEvent(QKeyEvent *event)
{
    Q_UNUSED(event);
}

void TupToolPlugin::keyReleaseEvent(QKeyEvent *event)
{
    Q_UNUSED(event);
}

QMenu *TupToolPlugin::menu() const
{
    QMenu *menu = new QMenu(new QWidget);
    return menu;
}

void TupToolPlugin::saveConfig() 
{
}

QCursor TupToolPlugin::cursor() const
{
    return QCursor(Qt::ArrowCursor); 
}

void TupToolPlugin::updatePos(QPointF pos)
{
    Q_UNUSED(pos);
}

void TupToolPlugin::resizeNodes(qreal factor)
{
    Q_UNUSED(factor);
}

void TupToolPlugin::updateZoomFactor(qreal factor)
{
    Q_UNUSED(factor);
}

void TupToolPlugin::setProjectSize(const QSize size)
{
    Q_UNUSED(size);
}

QPair<int, int> TupToolPlugin::setKeyAction(int key, Qt::KeyboardModifiers modifiers)
{
    TupToolPlugin::MenuIndex menu = TupToolPlugin::BrushesMenu;
    int tool = TupToolPlugin::PencilTool;

    if (modifiers & Qt::ControlModifier) {
        switch (key) {
            case Qt::Key_Right:
                menu = TupToolPlugin::Arrows;
                tool = TupToolPlugin::QuickCopyRight;
            break;
            case Qt::Key_Down:
            case Qt::Key_PageDown:
                menu = TupToolPlugin::Arrows;
                tool = TupToolPlugin::QuickCopyDown;
            break;
            case Qt::Key_Up:
            case Qt::Key_PageUp:
                menu = TupToolPlugin::Arrows;
                tool = TupToolPlugin::DeleteUp; 
            break;
            case Qt::Key_Left:
                menu = TupToolPlugin::Arrows;
                tool = TupToolPlugin::DeleteLeft;              
            break;
        }
    } else {
        switch (key) {
            case Qt::Key_P:
                 if (modifiers == Qt::ShiftModifier) {
                     menu = TupToolPlugin::ColorMenu;
                     tool = TupToolPlugin::ColorTool;
                 } else {
                     tool = TupToolPlugin::PencilTool;
                 }
            break;

            case Qt::Key_K:
                 tool = TupToolPlugin::InkTool;
            break;

            /*
            case Qt::Key_E:
                 tool = TupToolPlugin::EraserTool;
            break;
            */

            case Qt::Key_S:
                 tool = TupToolPlugin::PolyLineTool;
            break;

            case Qt::Key_L:
                 tool = TupToolPlugin::LineTool;
            break;

            case Qt::Key_R:
                 tool = TupToolPlugin::RectangleTool;
            break;

            case Qt::Key_C:
                 tool = TupToolPlugin::EllipseTool;
            break;

            /* SQA: Temporarily disabled
            case Qt::Key_T:
                 tool = TupToolPlugin::TextTool;
            break;
            */

            case Qt::Key_O:
                 menu = TupToolPlugin::SelectionMenu;
                 tool = TupToolPlugin::ObjectsTool;
            break;

            case Qt::Key_N:
                 menu = TupToolPlugin::SelectionMenu;
                 tool = TupToolPlugin::NodesTool;
            break;

            /*
            case Qt::Key_B:
                 menu = TupToolPlugin::FillMenu;
                 tool = TupToolPlugin::ContourFill;
            break;
            */

            case Qt::Key_F:
                 menu = TupToolPlugin::FillMenu;
                 tool = TupToolPlugin::FillTool;
            break;

            case Qt::Key_Left:
                 menu = TupToolPlugin::Arrows;
                 tool = TupToolPlugin::LeftArrow;
            break;

            case Qt::Key_Right:
                 menu = TupToolPlugin::Arrows;
                 tool = TupToolPlugin::RightArrow;
            break;

            case Qt::Key_PageUp:
            case Qt::Key_Up:
                 menu = TupToolPlugin::Arrows;
                 tool = TupToolPlugin::UpArrow;
            break;

            case Qt::Key_PageDown:
            case Qt::Key_Down:
                 menu = TupToolPlugin::Arrows;
                 tool = TupToolPlugin::DownArrow;
            break;

            case Qt::Key_Delete:
                 menu = TupToolPlugin::SelectionMenu;
                 tool = TupToolPlugin::Delete;
            break;

            default:
            {
                 menu = TupToolPlugin::InvalidMenu;
                 tool = TupToolPlugin::InvalidBrush;
            }
        }
    }

    QPair<int, int> flags;
    flags.first = menu;
    flags.second = tool;

    return flags;
}

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
    Q_UNUSED(viewID);
}

void TupToolPlugin::setCurrentItem(const QString &id)
{
    Q_UNUSED(id);
}

void TupToolPlugin::updateWorkSpaceContext()
{
}

void TupToolPlugin::clearSelection()
{
}

void TupToolPlugin::setColorMode(TColorCell::FillType mode)
{
    Q_UNUSED(mode);
}

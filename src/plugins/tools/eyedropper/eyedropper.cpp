/***************************************************************************
 *   Project TUPITUBE DESK                                                *
 *   Project Contact: info@maefloresta.com                                 *
 *   Project Website: http://www.maefloresta.com                           *
 *   Project Leader: Gustav Gonzalez <info@maefloresta.com>                *
 *                                                                         *
 *   Developers:                                                           *
 *   2010:                                                                 *
 *    Gustav Gonzalez / xtingray                                           *
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

#include "eyedropper.h"
#include "tuprequestbuilder.h"

#include <QScreen>

EyeDropper::EyeDropper()
{
    #ifdef TUP_DEBUG
        qDebug() << "[EyeDropper::EyeDropper()]";
    #endif

    setupActions();
}

EyeDropper::~EyeDropper()
{
}

void EyeDropper::init(TupGraphicsScene *gScene)
{
    #ifdef TUP_DEBUG
        qDebug() << "[EyeDropper::init()]";
    #endif

    Q_UNUSED(gScene)
}

QList<TAction::ActionId> EyeDropper::keys() const
{
    return QList<TAction::ActionId>() << TAction::EyeDropper;
}

void EyeDropper::setupActions()
{
    settings = new EyeDropperSettings;
    cursor = QCursor(kAppProp->themeDir() + "cursors/eyedropper.png", 8, 8);

    TAction *action = new TAction(QIcon(kAppProp->themeDir() + "icons/eyedropper.png"), tr("Eye Dropper"), this);
    // action->setShortcut(QKeySequence(tr("E")));
    action->setToolTip(tr("Eye Dropper"));
    action->setCursor(cursor);
    action->setActionId(TAction::EyeDropper);

    eyedropperActions.insert(TAction::EyeDropper, action);
}

void EyeDropper::press(const TupInputDeviceInformation *input, TupBrushManager *brushManager, TupGraphicsScene *gScene)
{
    #ifdef TUP_DEBUG
        qDebug() << "[EyeDropper::press()]";
    #endif

    Q_UNUSED(input)
    Q_UNUSED(brushManager)
    Q_UNUSED(gScene)

    emit colorPicked(fillType, grabColorFromScreen());
}

void EyeDropper::move(const TupInputDeviceInformation *, TupBrushManager *, TupGraphicsScene *)
{
}

void EyeDropper::release(const TupInputDeviceInformation *, TupBrushManager *, TupGraphicsScene *)
{
}

QColor EyeDropper::grabColorFromScreen() const
{
    const QPoint p = QCursor::pos();
    const QPixmap pixmap = QGuiApplication::primaryScreen()->grabWindow(desktop->winId(), p.x(), p.y(), 1, 1);
    QImage i = pixmap.toImage();

    return i.pixel(0, 0);
}

QMap<TAction::ActionId, TAction *> EyeDropper::actions() const
{
    return eyedropperActions;
}

int EyeDropper::toolType() const
{
    return TupToolInterface::Color;
}

QWidget *EyeDropper::configurator()
{
    #ifdef TUP_DEBUG
        qDebug() << "[EyeDropper::configurator()]";
    #endif

    return settings;
}

void EyeDropper::aboutToChangeScene(TupGraphicsScene *)
{
}

void EyeDropper::aboutToChangeTool() 
{
}

void EyeDropper::saveConfig()
{
}

void EyeDropper::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_F11 || event->key() == Qt::Key_Escape) {
        emit closeHugeCanvas();
    } else {
        QPair<int, int> flags = TAction::setKeyAction(event->key(), event->modifiers());
        if (flags.first != -1 && flags.second != -1)
            emit callForPlugin(flags.first, flags.second);
    }
}

QCursor EyeDropper::toolCursor()
{
    return cursor;
}

void EyeDropper::updateColorType(TColorCell::FillType type)
{
    fillType = type;
}

void EyeDropper::refreshEyeDropperPanel()
{
    /*
    #ifdef TUP_DEBUG
        qDebug() << "[EyeDropper::refreshEyeDropperPanel()]";
    #endif
    */

    settings->updateColor(grabColorFromScreen());
}

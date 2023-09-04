/***************************************************************************
 *   Project TUPITUBE DESK                                                 *
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

#include "taction.h"

TAction::TAction(QObject *parent, const QString &id) : QAction(parent)
{
    if (TActionManager *m = dynamic_cast<TActionManager *>(parent))
        initWithManager(m, id);
}

TAction::TAction(const QString &text, QObject *parent, const QString &id) : QAction(text, parent)
{
    if (TActionManager *m = dynamic_cast<TActionManager *>(parent))
        initWithManager(m, id);
}

TAction::TAction(const QIcon &icon, const QString &text, QObject *parent, const QString &id) : QAction(icon, text, parent)
{
    if (TActionManager *m = dynamic_cast<TActionManager *>(parent))
        initWithManager(m, id);
}

TAction::TAction(const QIcon &icon, QObject *parent, const QString &id) : QAction(parent)
{
    setIcon(icon);

    if (TActionManager *m = dynamic_cast<TActionManager *>(parent))
        initWithManager(m, id);
}

TAction::TAction(const QIcon &icon, const QString &text, const QString &key, QObject *parent,
                 const QString &id) : QAction(icon, text, parent)
{
    setShortcut(QKeySequence(key));
    if (TActionManager *m = dynamic_cast<TActionManager *>(parent))
        initWithManager(m, id);
}

TAction::TAction(const QIcon &icon, const QKeySequence &key, QObject *parent, const QString &id) : QAction(parent)
{
    setIcon(icon);
    setShortcut(key);

    if (TActionManager *m = dynamic_cast<TActionManager *>(parent))
        initWithManager(m, id);
}

TAction::TAction(const QIcon &icon, const QString &text, const QKeySequence &key, QObject *reciever,
                 const char *slot, QObject *parent, const QString &id) : QAction(icon, text, parent)
{
    setShortcut(key);
    connect(this, SIGNAL(triggered()), reciever, slot);

    if (TActionManager *m = dynamic_cast<TActionManager *>(parent))
        initWithManager(m, id);
}

TAction::TAction(const QString &text, const QKeySequence &key, QObject *reciever,
                 const char *slot, QObject *parent, const QString &id) : QAction(text, parent)
{
    setShortcut(key);
    connect(this, SIGNAL(triggered()), reciever, slot);

    if (TActionManager *m = dynamic_cast<TActionManager *>(parent))
        initWithManager(m, id);
}

TAction::~TAction()
{
}

void TAction::initWithManager(TActionManager *parent, const QString &id)
{
    setIconVisibleInMenu(true);

    if (!id.isEmpty())
        parent->insert(this, id);
}

void TAction::init()
{
}

void TAction::setCursor(const QCursor &cursor)
{
    m_cursor = cursor;
}

QCursor TAction::cursor() const
{
    return m_cursor;
}

void TAction::setActionId(TAction::ActionId code)
{
    id = code;
}

TAction::ActionId TAction::actionId()
{
    return id;
}

QPair<int, int> TAction::setKeyAction(int key, Qt::KeyboardModifiers modifiers)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TAction::setKeyAction()] - key -> " << key;
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
                tool = TAction::PaintBucket;
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

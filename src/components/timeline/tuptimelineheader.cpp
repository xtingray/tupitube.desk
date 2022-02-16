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

#include "tuptimelineheader.h"
#include "tconfig.h"

TupTimeLineHeader::TupTimeLineHeader(QWidget * parent) : QHeaderView(Qt::Vertical, parent)
{
    TCONFIG->beginGroup("Theme");
    uiTheme = TCONFIG->value("UITheme", DARK_THEME).toInt();

    setSectionResizeMode(QHeaderView::Custom);
    setSectionsClickable(true);
    setSectionsMovable(true);
    setFixedWidth(115);
    viewIconOn = QPixmap(THEME_DIR + "icons/show_layer.png");
    viewIconOff = QPixmap(THEME_DIR + "icons/hide_layer.png");

    connect(this, SIGNAL(sectionDoubleClicked(int)), this, SLOT(showTitleEditor(int)));

    editorSection = -1;

    editor = new QLineEdit(this);
    editor->setFocusPolicy(Qt::ClickFocus);
    editor->setInputMask("");
    connect(editor, SIGNAL(editingFinished()), this, SLOT(hideTitleEditor()));
    editor->hide();
}

TupTimeLineHeader::~TupTimeLineHeader()
{
}

void TupTimeLineHeader::paintSection(QPainter * painter, const QRect & rect, int index) const
{
    if (!model() || !rect.isValid())
        return;

    painter->save();

    QStyleOptionHeader headerOption;
    headerOption.rect = rect;
    headerOption.orientation = Qt::Vertical;
    headerOption.position = QStyleOptionHeader::Middle;
    headerOption.text = "";

    style()->drawControl(QStyle::CE_HeaderSection, &headerOption, painter);

    if (currentLayer == index) {
        QColor color(0, 136, 0, 40);
        if (uiTheme == DARK_THEME)
            color = QColor(120, 120, 120, 80);

        painter->fillRect(rect, color);
    }

    QFont font = this->font();
    font.setPointSize(7);
    QFontMetrics fm(font);

    int y = rect.normalized().bottomLeft().y() - (1 + (rect.normalized().height() - fm.height())/2);
    painter->setFont(font);
    painter->setPen(QPen(Qt::black, 1, Qt::SolidLine));
    painter->drawText(10, y, layers[index].title);

    y = rect.y();

    QRectF viewRect = QRectF(0, 0, 13, 7); 
    int viewY = static_cast<int>((rect.height() - viewRect.height())/2);
    if (layers[index].isVisible)
        painter->drawPixmap(QPointF(rect.x() + 90, viewY + y), viewIconOn, viewRect);
    else
        painter->drawPixmap(QPointF(rect.x() + 90, viewY + y), viewIconOff, viewRect);

    painter->restore();
}

void TupTimeLineHeader::mousePressEvent(QMouseEvent *event)
{
    QPoint point = event->pos();
    int section = logicalIndexAt(point);

    if (section != currentLayer)
        emit headerSelectionChanged(section);

    int y = sectionViewportPosition(section);
    QRect rect(90, y, 20, sectionSize(section));
    if (rect.contains(point))
        emit visibilityChanged(section, !layers[section].isVisible);

    QHeaderView::mousePressEvent(event);
}

void TupTimeLineHeader::updateSelection(int index)
{
    if (currentLayer != index) {
        currentLayer = index;
        updateSection(index);
    }
}

void TupTimeLineHeader::insertSection(int index, const QString &name)
{
    TimeLineLayerItem layer;
    layer.title = name;
    layer.lastFrame = -1;
    layer.isVisible = true;
    layer.isLocked = false;
    layer.isSound = false;

    layers.insert(index, layer);
}

void TupTimeLineHeader::setSectionVisibility(int index, bool visibility)
{
    if (index >= 0 && index < layers.count()) {
        layers[index].isVisible = visibility;
        updateSection(index);
    }
}

void TupTimeLineHeader::setSectionTitle(int index, const QString &name)
{
    layers[index].title = name;
    updateSection(index);
}

void TupTimeLineHeader::showTitleEditor(int index)
{
    if (index >= 0) {
        QFont font = this->font();
        font.setPointSize(7);
        editor->setFont(font);
        int x = sectionViewportPosition(index);
        editor->setGeometry(0, x, width(), sectionSize(index));
        editorSection = index;
        editor->setText(layers[index].title);
        editor->show();
        editor->setFocus();
    }
}

void TupTimeLineHeader::hideTitleEditor()
{
    editor->hide();

    if (editorSection != -1 && editor->isModified())
        emit nameChanged(editorSection, editor->text());

    editorSection = -1;
}

void TupTimeLineHeader::removeSection(int index)
{
    layers.removeAt(index);
}

int TupTimeLineHeader::lastFrame(int index)
{
    if (index > -1 && index < layers.count())
        return layers[index].lastFrame;

    return -1;
}

void TupTimeLineHeader::updateLastFrame(int index, bool addition)
{
    if (addition)
        layers[index].lastFrame++;
    else
        layers[index].lastFrame--;
}

void TupTimeLineHeader::resetLastFrame(int index)
{
    layers[index].lastFrame = -1;
}

bool TupTimeLineHeader::isSound(int index)
{
    return layers[index].isSound;
}

void TupTimeLineHeader::setSoundFlag(int index, bool flag)
{
    layers[index].isSound = flag;
}

int TupTimeLineHeader::currentSectionIndex()
{
    return currentLayer;
}

void TupTimeLineHeader::moveHeaderSection(int position, int newPosition, bool isLocalRequest)
{
    if (isLocalRequest) {
        sectionOnMotion = true;
        moveSection(visualIndex(position), visualIndex(newPosition));
        layers.swapItemsAt(position, newPosition);
        sectionOnMotion = false;
    } else {
        layers.swapItemsAt(position, newPosition);
    }
}

bool TupTimeLineHeader::sectionIsMoving()
{
    return sectionOnMotion;
}

int TupTimeLineHeader::columnsTotal()
{
    return layers.size();
}

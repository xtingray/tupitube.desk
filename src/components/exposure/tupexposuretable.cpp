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

#include "tupexposuretable.h"
#include "tconfig.h"

#include <QPainterPath>

class TUPITUBE_EXPORT TupExposureVerticalHeader: public QHeaderView
{
    public:
        TupExposureVerticalHeader(int fps, QWidget *parent = nullptr);
        ~TupExposureVerticalHeader();

        void updateFPS(int fps);
        virtual void paintSection(QPainter *painter, const QRect &rect, int logicalIndex) const;

    private:
        int fps;
};

TupExposureVerticalHeader::TupExposureVerticalHeader(int value, QWidget *parent): QHeaderView(Qt::Vertical, parent)
{
    setFixedWidth(25);
    fps = value;
}

TupExposureVerticalHeader::~TupExposureVerticalHeader()
{
}

void TupExposureVerticalHeader::updateFPS(int value)
{
    fps = value;
}

void TupExposureVerticalHeader::paintSection(QPainter *painter, const QRect & rect, int logicalIndex) const
{
    Q_UNUSED(logicalIndex)

    if (!rect.isValid())
        return;

    QStyleOptionHeader headerOption;
    headerOption.rect = rect;
    headerOption.orientation = Qt::Vertical;
    headerOption.position = QStyleOptionHeader::Middle;
    headerOption.text = "";

    QStyle::State state = QStyle::State_None;

    if (isEnabled())
        state |= QStyle::State_Enabled;

    if (window()->isActiveWindow())
        state |= QStyle::State_Active;
 
    style()->drawControl(QStyle::CE_HeaderSection, &headerOption, painter);

    QString text;
    int label = logicalIndex + 1;
    text = text.setNum(label);
    QFont font = this->font();
    font.setPointSize(7);
    QFontMetrics fm(font);

    int x = rect.normalized().x() + ((rect.normalized().width() - fm.horizontalAdvance(text))/2);
    int y = rect.normalized().bottomLeft().y() - (1 + (rect.normalized().height() - fm.height())/2);

    painter->setFont(font);
    if (label % fps == 0) {

        painter->fillRect(rect, QBrush(QColor(120, 120, 120)));
        painter->setPen(QPen(Qt::white, 1, Qt::SolidLine));
    } else {
        painter->setPen(QPen(Qt::black, 1, Qt::SolidLine));
    }

    painter->drawText(x, y, text);
}

/////////////////

class TupExposureItemDelegate : public QItemDelegate
{
    public:
        TupExposureItemDelegate(QObject * parent = nullptr);
        ~TupExposureItemDelegate();

        virtual void paint(QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & index) const;

    private:
        QString themeName;
};

TupExposureItemDelegate::TupExposureItemDelegate(QObject * parent) :  QItemDelegate(parent)
{
    TCONFIG->beginGroup("General");
    themeName = TCONFIG->value("Theme", "Light").toString();
}

TupExposureItemDelegate::~TupExposureItemDelegate()
{
}

void TupExposureItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem & option, const QModelIndex & index) const
{
    QItemDelegate::paint(painter, option, index);
    TupExposureTable *table = qobject_cast<TupExposureTable *>(index.model()->parent());
    QTableWidgetItem *item = table->itemFromIndex(index);

    if (item) {
        /* SQA: Useful code
        if (item->data(TupExposureTable::IsLocked).toBool()) {
             QPixmap pixmap(THEME_DIR + "icons/padlock.png");
             painter->drawPixmap(1, 5, pixmap);
        } else 
        */

        int x = option.rect.topLeft().x() + 2;
        int y = option.rect.topLeft().y() + 2;
        int w = option.rect.bottomRight().x() - x - 2;
        int h = option.rect.bottomRight().y() - y - 2;

        if ((item->data(TupExposureTable::IsEmpty).toInt() == TupExposureTable::Empty)) {
            QColor color(100, 100, 100, 30);
            if (themeName.compare("Dark") == 0)
                color = QColor(120, 120, 120);

            QPen pen(color);
            pen.setStyle(Qt::DashLine);
            painter->setPen(pen);
            painter->drawRect(x, y, w, h);
        }

        if ((item->data(TupExposureTable::IsEmpty).toInt() == TupExposureTable::Used)) {
            QPainterPath path(QPointF(x, y));
            path.lineTo(x, y + h - 3);
            path.lineTo(x + 3, y + h);
            path.lineTo(x + w, y + h);
            path.lineTo(x + w, y + 4);
            path.lineTo(x + w - 8, y); 

            QColor color(0, 102, 255, 80);
            if (themeName.compare("Dark") == 0)
                color = QColor(0, 0, 0, 60);

            painter->fillPath(path, QBrush(color));

            if (themeName.compare("Dark") == 0) {
                QPen pen(QColor(100, 100, 100));
                painter->setPen(pen);
                painter->drawPath(path);
            }
        }
    }
}

////////////////////////

TupExposureTable::TupExposureTable(int fps, QWidget *parent) : QTableWidget(parent)
{
    TCONFIG->beginGroup("General");
    themeName = TCONFIG->value("Theme", "Light").toString();

    isLocalRequest = false;
    isEditing = false;

    TupExposureVerticalHeader *verticalHeader = new TupExposureVerticalHeader(fps, this);
    setVerticalHeader(verticalHeader);

    setItemDelegate(new TupExposureItemDelegate(this));
    removingLayer = false;

    QTableWidgetItem *prototype = new QTableWidgetItem();
    prototype->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEditable);
    prototype->setTextAlignment(Qt::AlignCenter);
    prototype->setData(IsEmpty, Unset);
    setItemPrototype(prototype);

    setRowCount(100);

    for(int i=0; i < 100; i++)
        setRowHeight(i, 20);

    header = new TupExposureHeader(this);
    connect(header, SIGNAL(visibilityChanged(int, bool)), this, SIGNAL(layerVisibilityChanged(int, bool)));
    connect(header, SIGNAL(nameChanged(int, const QString &)), this, SIGNAL(layerNameChanged(int, const QString & )));
    connect(header, SIGNAL(sectionMoved(int, int, int)), this, SLOT(requestLayerMove(int, int, int)));
    connect(header, SIGNAL(headerSelectionChanged(int)), this, SLOT(updateLayerSelection(int)));
    setHorizontalHeader(header);

    connect(this, SIGNAL(cellClicked(int, int)), this, SLOT(markUsedFrames(int, int)));
    connect(this, SIGNAL(currentCellChanged(int, int, int, int)), this, SLOT(requestFrameSelection(int, int, int, int)));

    setSelectionBehavior(QAbstractItemView::SelectItems);
    setSelectionMode(QAbstractItemView::ExtendedSelection);

    singleMenu = nullptr;
}

TupExposureTable::~TupExposureTable()
{
    delete header;
}

void TupExposureTable::updateFPS(int fps)
{
    TupExposureVerticalHeader *header = dynamic_cast<TupExposureVerticalHeader *>(this->verticalHeader());
    header->updateFPS(fps);
}

void TupExposureTable::requestFrameRenaming(QTableWidgetItem *item)
{
    QModelIndex index = indexFromItem(item);
    emit frameRenamed(index.column(), index.row(), item->text());
}

void TupExposureTable::requestFrameSelection(int currentSelectedRow, int currentColumn, int previousRow, int previousColumn)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupExposureTable::requestFrameSelection()] - current -> " << currentSelectedRow
                 << "," << currentColumn << " - previous -> " << previousRow << "," << previousColumn;
    #endif

    if (!removingLayer) {
        QTableWidgetItem *frame = item(currentSelectedRow, currentColumn);
        if (frame) {
            if (previousColumn != currentColumn || previousRow != currentSelectedRow)
                emit frameSelected(currentLayer(), currentRow());

            if ((previousColumn != currentColumn) || (columnCount() == 1))
                header->updateSelection(currentColumn);
        }
    } else { // A layer is being removed
        removingLayer = false;
        selectFrame(currentColumn, currentSelectedRow);

        if (previousColumn == 0)
            header->updateSelection(0);
        else
            header->updateSelection(currentColumn);
    }
}

void TupExposureTable::requestLayerMove(int logicalIndex, int oldVisualIndex, int newVisualIndex)
{
    Q_UNUSED(logicalIndex)

    if (!header->sectionIsMoving()) {
        int distance = newVisualIndex - oldVisualIndex;
        if (distance > 0) {
            if (distance > 1)
                newVisualIndex = oldVisualIndex + 1;
        } else {
            distance = abs(distance);
            if (distance > 1)
                newVisualIndex = oldVisualIndex - 1;
        }

        isLocalRequest = true;
        emit layerMoved(oldVisualIndex, newVisualIndex);
    }
}

void TupExposureTable::updateLayerSelection(int layerIndex)
{
    selectFrame(layerIndex, currentRow());
}

QString TupExposureTable::frameName(int layerIndex, int frameIndex)
{
    QTableWidgetItem *frame = item(frameIndex, layerIndex);
    if (frame)
        return frame->text();

    return "";
}

void TupExposureTable::setFrameName(int layerIndex, int frameIndex, const QString &name)
{
    QTableWidgetItem *frame = item(frameIndex, layerIndex);
    if (frame) { 
        QFont font = this->font();
        font.setPointSize(7);
        frame->setFont(font);
        if (frame->text() != name)
            frame->setText(name);
    }
}

void TupExposureTable::setLayerName(int layerIndex, const QString & name)
{
    header->setSectionTitle(header->logicalIndex(layerIndex), name);
}

bool TupExposureTable::layerIndexIsValid(int layerIndex)
{
    if (layerIndex >= 0 && layerIndex < columnCount())
        return true;

    return false; 
}

bool TupExposureTable::frameIndexIsValid(int frameIndex)
{
    if (frameIndex >= 0 && frameIndex < rowCount())
        return true;

    return false;  
}

TupExposureTable::FrameType TupExposureTable::frameState(int layerIndex, int frameIndex)
{ 
    TupExposureTable::FrameType type = TupExposureTable::Unset;
    if (layerIndexIsValid(layerIndex) && frameIndexIsValid(frameIndex)) {
        QTableWidgetItem *frame = item(frameIndex, layerIndex);
        if (frame)
            type = TupExposureTable::FrameType(frame->data(TupExposureTable::IsEmpty).toInt()); 
    }

    return type;
}

void TupExposureTable::updateFrameState(int layerIndex, int frameIndex, TupExposureTable::FrameType value)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupExposureTable::updateFrameState()] - layerIndex -> " << layerIndex;
        qDebug() << "[TupExposureTable::updateFrameState()] - frameIndex -> " << frameIndex;
    #endif

    if (layerIndex < 0 || frameIndex < 0)
        return;

    QTableWidgetItem *frame = item(frameIndex, layerIndex);
    if (frame) {
        frame->setData(IsEmpty, value);
    } else {
        #ifdef TUP_DEBUG
            qDebug() << "[TupExposureTable::updateFrameState()] - Error: No frame at ["
                     << layerIndex << "," << frameIndex << "]";
        #endif
    }
}

void TupExposureTable::selectFrame(int layerIndex, int frameIndex)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupExposureTable::selectFrame()] - layerIndex, frameIndex -> "
                 << layerIndex << "," << frameIndex;
    #endif

    if (header->currentSectionIndex() != layerIndex)
        header->updateSelection(layerIndex);

    setCurrentCell(frameIndex, layerIndex);
}

void TupExposureTable::selectFrame(int layerIndex, int frameIndex, const QString &selection)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupExposureTable::selectFrame()] - layerIndex -> "
                 << layerIndex << " - frameIndex -> " << frameIndex << " - selection -> " << selection;
    #endif

    if (selection.isEmpty()) {
        #ifdef TUP_DEBUG
            qDebug() << "[TupExposureTable::selectFrame()] - Selection area parameter is EMPTY!";
        #endif
        return;
    }

    if (header->currentSectionIndex() != layerIndex)
        header->updateSelection(layerIndex);

    QStringList coords = selection.split(",");
    if (coords.count() == 4) {
        int initLayer = coords.at(0).toInt();
        int lastLayer = coords.at(1).toInt();
        int initFrame = coords.at(2).toInt();
        int lastFrame = coords.at(3).toInt();

        selectionModel()->clearSelection();
        setCurrentCell(frameIndex, layerIndex);

        for (int i=initLayer; i<=lastLayer; i++) {
            for (int j=initFrame; j<=lastFrame; j++)
                selectionModel()->select(model()->index(j, i), QItemSelectionModel::Select);
        }
    } else {
        #ifdef TUP_DEBUG
            qDebug() << "[TupExposureTable::selectFrame()] - Selection area parameter is MISCONFIGURED!";
        #endif
    }
}

void TupExposureTable::updateSelection(int layerIndex, int frameIndex)
{
    selectionModel()->clearSelection();
    selectionModel()->select(model()->index(frameIndex, layerIndex), QItemSelectionModel::Select);
}

void TupExposureTable::setSinglePopUpMenu(QMenu *single)
{
    singleMenu = single;
    // multipleMenu = multiple;
}

int TupExposureTable::currentLayer() const
{
    return header->visualIndex(currentColumn());
}

int TupExposureTable::currentFrame() const
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupExposureTable::currentFrame()]";
    #endif

    QTableWidgetItem *frame = currentItem();
    if (frame) {
        if (frame->data(TupExposureTable::IsEmpty).toInt() != Unset)
            return currentRow();
    }

    return 0;
}

void TupExposureTable::insertLayer(int index, const QString & name)
{
    insertColumn(index);
    setColumnWidth(index, 70);
    header->insertSection(index, name);
}

void TupExposureTable::insertFrame(int layerIndex, int frameIndex, const QString &name, bool external)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupExposureTable::insertFrame()] - layerIndex, frameIndex -> "
                 << layerIndex << "," << frameIndex;
    #endif

    QTableWidgetItem *frame = new QTableWidgetItem;

    QColor color = Qt::transparent;
    if (themeName.compare("Dark") == 0)
        frame->setForeground(Qt::black);
    frame->setBackground(color);

    QFont font = this->font();
    font.setPointSize(7);
    frame->setFont(font);
    frame->setSizeHint(QSize(65, 10));
    frame->setText(name);
    frame->setData(IsEmpty, Empty);
    frame->setTextAlignment(Qt::AlignCenter);

    int logicalIndex = header->logicalIndex(layerIndex);
    header->setLastFrame(logicalIndex, header->lastFrame(logicalIndex) + 1);
    setItem(header->lastFrame(logicalIndex) - 1, logicalIndex, frame);

    int init = header->lastFrame(logicalIndex) - 1;
    for (int i=init; i>frameIndex; i--)
        exchangeFrame(layerIndex, i, layerIndex, i - 1, external);

    if (header->lastFrame(logicalIndex) == rowCount()) {
        setRowCount(header->lastFrame(logicalIndex) + 100);
        int last = header->lastFrame(logicalIndex);
        for (int i=last; i < last + 100; i++)
             setRowHeight(i, 20);
    }
}

/*
void TupExposureTable::setLockLayer(int layerIndex, bool locked)
{
    header->setLockFlag(layerIndex, locked);
}
*/

void TupExposureTable::setLayerVisibility(int visualIndex, bool visibility)
{
    header->setSectionVisibility(header->logicalIndex(visualIndex), visibility);
}

/*
void TupExposureTable::setLayerOpacity(int layerIndex, double opacity)
{
    opacityControl[layerIndex] = opacity;
}

void TupExposureTable::storeUndoLayerOpacity(int layerIndex, double opacity)
{
    undoOpacities[layerIndex] = opacity;
}
*/

void TupExposureTable::removeLayer(int layerIndex)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupExposureTable::removeLayer()] - layerIndex -> " << layerIndex;
    #endif

    setUpdatesEnabled(false);

    removingLayer = true;
    header->removeSection(layerIndex);
    removeColumn(layerIndex);

    setUpdatesEnabled(true);
}

void TupExposureTable::removeFrame(int layerIndex, int frameIndex)
{
    QTableWidgetItem *item = takeItem(frameIndex, layerIndex);

    if (item) {
        header->setLastFrame(layerIndex, header->lastFrame(layerIndex)-1);
    } else {
        #ifdef TUP_DEBUG
            qDebug() << "[TupExposureTable::removeFrame()] - No item available at [" + QString::number(layerIndex) + "," + QString::number(frameIndex) + "]";
        #endif
    }
}

void TupExposureTable::exchangeFrame(int oldPosLayer, int oldPosFrame, int newPosLayer, int newPosFrame, bool external)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupExposureTable::exchangeFrame()]";
        qDebug() << "*** frameIndex 1, frameIndex 2 -> " << oldPosFrame << "," << newPosFrame;
    #endif

    QTableWidgetItem *frame1 = item(oldPosFrame, oldPosLayer);
    QTableWidgetItem *frame2 = item(newPosFrame, newPosLayer);

    if (frame1 && frame2) { 
        TupExposureTable::FrameType type1 = TupExposureTable::FrameType(frame1->data(TupExposureTable::IsEmpty).toInt());
        QString name1 = frame1->text();

        TupExposureTable::FrameType type2 = TupExposureTable::FrameType(frame2->data(TupExposureTable::IsEmpty).toInt());
        QString name2 = frame2->text();

        frame1->setText(name2);
        frame1->setData(IsEmpty, type2);

        frame2->setText(name1);
        frame2->setData(IsEmpty, type1);

        if (!external)
            setCurrentItem(frame2);
    } else {
        #ifdef TUP_DEBUG
            qDebug() << "[TupExposureTable::exchangeFrame()] - Fatal Error: Some of the frame cells are NULL -> indexes: ["
                        + QString::number(oldPosFrame) +  "," + QString::number(newPosFrame) + "]";
        #endif
    }
}

void TupExposureTable::moveLayer(int oldPosLayer, int newPosLayer)
{
    header->moveHeaderSection(oldPosLayer, newPosLayer, isLocalRequest);
    if (isLocalRequest)
        isLocalRequest = false;

    for (int frameIndex = 0; frameIndex < header->lastFrame(oldPosLayer); frameIndex++)
         exchangeFrame(oldPosLayer, frameIndex, newPosLayer, frameIndex, true);

    blockSignals(true);
    selectFrame(newPosLayer, currentRow());
    blockSignals(false);
}

void TupExposureTable::markUsedFrames(int frameIndex, int layerIndex)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupExposureTable::markUsedFrames()]";
    #endif

    int lastFrame = header->lastFrame(layerIndex);

    if (frameIndex >= lastFrame) {
        int used = usedFrames(layerIndex);
        if (lastFrame >= used) {
            for (int frame=used; frame <= frameIndex; frame++)
                emit frameUsed(layerIndex, frame);
        }
        emit frameSelected(layerIndex, frameIndex);
    } 
}

void TupExposureTable::markNextFrame(int frameIndex, int layerIndex)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupExposureTable::markNextFrame()]";
    #endif

    int lastFrame = header->lastFrame(layerIndex);

    if (frameIndex >= lastFrame) {
        int used = usedFrames(layerIndex);
        if (frameIndex == used) {
            emit frameUsed(layerIndex, frameIndex);
            emit frameSelected(layerIndex, frameIndex);
        }
    }
}

int TupExposureTable::usedFrames(int column) const
{
    return header->lastFrame(column);
}

bool TupExposureTable::edit(const QModelIndex & index, EditTrigger trigger, QEvent *event)
{
    QTableWidgetItem *item = itemFromIndex(index);

    if (item) {
        if (item->data(TupExposureTable::IsEmpty).toInt() != Unset)
            QTableWidget::edit(index, trigger, event);
        else
            return false;
    }

    return false;
}

void TupExposureTable::mousePressEvent(QMouseEvent *event)
{
    int frameIndex = rowAt(event->y());
    int layerIndex = currentLayer();
    if (event->button() == Qt::RightButton) {
        if (frameIndex <= header->lastFrame(layerIndex)) {
            int frames = selectedItems().count();
            if (frames == 1) {
                if (singleMenu)
                    singleMenu->exec(event->globalPos());
            }
            /* SQA: PopUp menu for multiple selection
            } else if (frames > 1) {
                if (multipleMenu) {
                    // clearFocus();
                    multipleMenu->exec(event->globalPos());
                }
            }
            */
        } else {
            return;
        }
    }

    QTableWidget::mousePressEvent(event);
}

void TupExposureTable::mouseMoveEvent(QMouseEvent *event)
{
    int layerIndex = currentLayer();
    int frameIndex = rowAt(event->y()); 
    QList<int> layers = currentSelection();

    if (!layers.isEmpty()) {
        for (int j=layers.at(0); j<=layerIndex; j++) {
            int top = header->lastFrame(j);
            if (frameIndex >= top) {
                for (int i=top; i<=frameIndex; i++)
                    emit frameUsed(j, i);
            }
        }
    }

    QTableWidget::mouseMoveEvent(event);
}

void TupExposureTable::mouseDoubleClickEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
        isEditing = true;

    QTableWidget::mouseDoubleClickEvent(event);
}

void TupExposureTable::commitData(QWidget *editor)
{
    QLineEdit *lineEdit = qobject_cast<QLineEdit *>(editor);
    QTableWidget::commitData(nullptr); // Don't rename

    if (lineEdit)
        emit frameRenamed(currentLayer(), currentFrame(), lineEdit->text());
}

int TupExposureTable::layersCount()
{
    return header->columnsTotal();
}

int TupExposureTable::framesCount()
{
    return header->lastFrame(0);
}

int TupExposureTable::framesCountAtCurrentLayer()
{
    return header->lastFrame(currentLayer());
}

int TupExposureTable::framesCountAtLayer(int layer)
{
    return header->lastFrame(layer);
}

void TupExposureTable::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Return) {
        if (!isEditing && !header->layerNameEdited()) {
            emit newPerspective(4);
        } else {
            isEditing = false;
            if (header->layerNameEdited())
                header->updateLayerNameFlag(false);
        }

        return;
    }

    if (event->key() == Qt::Key_Backspace || event->key() == Qt::Key_Delete) {
        emit selectionRemoved();
        return;
    }

    if ((event->key() == Qt::Key_A) && (event->modifiers() == Qt::ControlModifier)) {
        clearSelection();
        for (int i = 0; i < header->columnsTotal(); i++) {
            int frames = header->lastFrame(i);
            for (int j = 0; j < frames; j++)
                selectFrame(i, j);
        }
        emit selectionCopied();
        return;
    }

    if (event->key() == Qt::Key_Up || event->key() == Qt::Key_PageUp) {
        int row = currentRow() - 1;
        if (row > -1) {
            if (event->modifiers() == Qt::ControlModifier)
                emit selectionRemoved();
            else
                setCurrentCell(row, currentColumn());
        } else {
            if (row == -1 && event->modifiers() == Qt::ControlModifier)
                emit selectionRemoved();
        }
        return;
    }

    if (event->key() == Qt::Key_Down || event->key() == Qt::Key_PageDown) {
        int framesCount = header->lastFrame(currentLayer());
        int next = currentRow() + 1;
        if (event->modifiers() == Qt::ControlModifier) {
            emit frameExtended(currentLayer(), currentFrame());
        } else {
            if (next >= framesCount)
                markNextFrame(next, currentColumn());
            else
                setCurrentCell(next, currentColumn());
        }
        return;
    }

    // Clone frame
    if (event->key() == Qt::Key_8) {
        emit frameExtended(currentLayer(), currentFrame());
        return;
    }

    if (event->key() == Qt::Key_C) {
        if (event->modifiers() == Qt::ControlModifier) {
            emit selectionCopied();
        } else {
            if (event->modifiers() == Qt::AltModifier) {
                clearSelection();
                int layer = currentLayer();
                int frames = header->lastFrame(layer);
                for (int j = 0; j < frames; j++)
                    selectionModel()->select(model()->index(j, layer), QItemSelectionModel::Select);
                emit selectionCopied();
            }
        }
        return;
    }

    if (event->key() == Qt::Key_R) {
        if (event->modifiers() == Qt::AltModifier) {
            clearSelection();
            int columns = header->columnsTotal();
            int frame = currentFrame();
            for (int j = 0; j < columns; j++)
                selectionModel()->select(model()->index(frame, j), QItemSelectionModel::Select);
            emit selectionCopied();
        }
        return;
    }

    if (event->key() == Qt::Key_V) {
        if (event->modifiers() == Qt::ControlModifier)
            emit selectionPasted();
        return;
    }

    if (event->key() == Qt::Key_X) {
        if (event->modifiers() == Qt::ControlModifier) {
            emit selectionCopied();
            emit selectionRemoved();
        }
        return;
    }

    if (event->key() == Qt::Key_Right) {
        int limit = columnCount() - 1;
        int column = currentColumn() + 1;
        if (column <= limit) {
            int framesCount = header->lastFrame(column);
            int frameIndex = currentRow();
            if (frameIndex >= framesCount)
                markUsedFrames(frameIndex, column);
            else
                setCurrentCell(frameIndex, column);
        }
        return;
    }

    if (event->key() == Qt::Key_Left) {
        int column = currentColumn() - 1;
        if (column > -1) {
            int framesCount = header->lastFrame(column);
            int frameIndex = currentRow();
            if (frameIndex >= framesCount)
                markUsedFrames(frameIndex, column);
            else
                setCurrentCell(frameIndex, column);
        }
        return;
    }
}

// SQA : Verify if this method is required
void TupExposureTable::notifyCellClicked(int frame, int layer) 
{
    emit cellClicked(frame, layer);
}

void TupExposureTable::enterEvent(QEvent *event)
{
    if (!hasFocus())
        setFocus();

    QTableWidget::enterEvent(event);
}

void TupExposureTable::leaveEvent(QEvent *event)
{
    if (hasFocus())
        clearFocus();

    QTableWidget::leaveEvent(event);
}

void TupExposureTable::reset()
{
    int cols = columnCount();
    if (cols > 1) {
        for (int i=1; i < cols; i++)
             removeLayer(i);
    }

    int rows = rowCount();
    for (int i=1; i < rows;  i++)
         takeItem(i, 0);

    header->setLastFrame(0, 1);
}

QList<int> TupExposureTable::currentSelection()
{
    QList<int> coords;
    QList<int> layers;
    QList<int> frames;
    QModelIndexList selection = selectedIndexes();

    if (!selection.isEmpty()) {
        foreach (QModelIndex cell, selection) { 
            int layer = cell.column();
            int frame = cell.row();

            if (!layers.contains(layer))
                layers << layer;
            if (!frames.contains(frame))
                frames << frame;
        }
        coords << layers.first() << layers.last() << frames.first() << frames.last();
    }

    return coords;
}

void TupExposureTable::updateSceneView(int layerIndex, int frameIndex)
{
    scrollToItem(item(frameIndex, layerIndex));
}

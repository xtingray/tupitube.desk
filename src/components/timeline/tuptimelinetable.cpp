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

#include "tuptimelinetable.h"
#include "tconfig.h"

////////// TupTimeLineTableItemDelegate ///////////

class TupTimeLineTableItemDelegate : public QItemDelegate
{
    public:
        TupTimeLineTableItemDelegate(QObject *parent = nullptr);
        ~TupTimeLineTableItemDelegate();

        virtual void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
        // virtual QSize sizeHint(const QStyleOptionViewItem & option, const QModelIndex &index) const;

    private:
        QString themeName;
};

TupTimeLineTableItemDelegate::TupTimeLineTableItemDelegate(QObject *parent) : QItemDelegate(parent)
{
    TCONFIG->beginGroup("General");
    themeName = TCONFIG->value("Theme", "Light").toString();
}

TupTimeLineTableItemDelegate::~TupTimeLineTableItemDelegate()
{
}

/*
QSize TupTimeLineTableItemDelegate::sizeHint(const QStyleOptionViewItem & option, const QModelIndex &index) const
{
    return QSize(5, 10);
}
*/

void TupTimeLineTableItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_ASSERT(index.isValid());

    QItemDelegate::paint(painter, option, index);
    TupTimeLineTable *table = qobject_cast<TupTimeLineTable *>(index.model()->parent());
    TupTimeLineTableItem *item = dynamic_cast<TupTimeLineTableItem *>(table->itemFromIndex(index));
    
    // draw the background color
    QVariant value = index.data(Qt::BackgroundColorRole);
    
    if (value.isValid()) {
        painter->save();
        
        bool sound = table->isSoundLayer(index.row());
        if (!sound)
            painter->fillRect(option.rect, value.value<QColor>());
        
        painter->restore();
    } else {
        painter->save();
        
        bool sound = table->isSoundLayer(index.row());
        if (!sound) {
            int counter = index.column() + 1;
            if (counter == 1 || counter % 5 == 0) 
                painter->fillRect(option.rect, QColor(230, 230, 230));
            else 
                painter->fillRect(option.rect, Qt::white);
        } 
        
        painter->restore();
    }
    
    // Selection!
    if (option.showDecorationSelected && (option.state & QStyle::State_Selected)) {
        painter->save();

        // Painting a selected cell
        QColor color(0, 136, 0, 180);
        if (themeName.compare("Dark") == 0)
            color = QColor(80, 80, 80, 180);

        painter->fillRect(option.rect, color);
        painter->restore();
    }
    
    // Draw attributes
    if (item && index.isValid()) {
        int offset = option.rect.width() / 3;
        // int offset = 2;
        if (item->isUsed()) {
            painter->save();
            QColor gray(80, 80, 80);
            painter->setPen(QPen(gray, 1, Qt::SolidLine));
            painter->setBrush(gray);
            painter->setRenderHint(QPainter::Antialiasing, true);
            
            if (!item->isSound()) {
                if (item->isLocked()) {
                    painter->setPen(QPen(Qt::red, 1, Qt::SolidLine));
                    painter->setBrush(Qt::red);
                }
                painter->drawEllipse(option.rect.x() + ((option.rect.width() - offset)/2), 
                                     option.rect.y() + ((option.rect.height() - offset)/2),
                                     offset, offset);
            } else {
                painter->setBrush(QColor(0, 136, 0));
                painter->drawRect(option.rect.x() + ((option.rect.width() - offset)/2), 
                                  option.rect.y() + ((option.rect.height() + offset)/2), 
                                  offset, offset);
            }
            
            painter->restore();
        }
    }
}

////////// TupTimeLineTableItem ////////

TupTimeLineTableItem::TupTimeLineTableItem()
{
}

TupTimeLineTableItem::~TupTimeLineTableItem()
{
}

bool TupTimeLineTableItem::isUsed()
{
    return data(IsUsed).toBool();
}

bool TupTimeLineTableItem::isLocked()
{
    return data(IsLocked).toBool();
}

bool TupTimeLineTableItem::isSound()
{
    QVariant data = this->data(IsSound);
    
    if (data.canConvert<bool>())
        return data.toBool();

    return false;
}

//// TupTimeLineTable

TupTimeLineTable::TupTimeLineTable(int index, QWidget *parent) : QTableWidget(0, 200, parent)
{
    setItemSize(5, 5);

    isLocalRequest = false;
    sceneIndex = index;
    frameIndex = 0;
    layerIndex = 0;

    ruler = new TupTimeLineRuler;
    connect(ruler, SIGNAL(headerSelectionChanged(int)), this, SLOT(frameSelectionFromRuler(int)));

    removingLayer = false;
    removingFrame = false;

    layersColumn = new TupTimeLineHeader;
    connect(layersColumn, SIGNAL(nameChanged(int, const QString &)), this, SIGNAL(layerNameChanged(int, const QString &)));
    connect(layersColumn, SIGNAL(headerSelectionChanged(int)), this, SLOT(frameSelectionFromLayerHeader(int)));
    connect(layersColumn, SIGNAL(visibilityChanged(int, bool)), this, SIGNAL(visibilityChanged(int, bool)));
    connect(layersColumn, SIGNAL(sectionMoved(int, int, int)), this, SLOT(requestLayerMove(int, int, int)));

    setup();
}

TupTimeLineTable::~TupTimeLineTable()
{
}

void TupTimeLineTable::setup()
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupTimeLineTable::setup()]";
    #endif

    setItemDelegate(new TupTimeLineTableItemDelegate(this));
    setSelectionBehavior(QAbstractItemView::SelectItems);
    setSelectionMode(QAbstractItemView::ExtendedSelection);
    setEditTriggers(QAbstractItemView::NoEditTriggers);

    connect(this, SIGNAL(currentCellChanged(int, int, int, int)), this, SLOT(requestFrameSelection(int, int, int, int)));

    setHorizontalHeader(ruler);
    setVerticalHeader(layersColumn);
    
    ruler->setSectionResizeMode(QHeaderView::Custom);
    layersColumn->setSectionResizeMode(QHeaderView::Custom);
}

void TupTimeLineTable::frameSelectionFromRuler(int frameIndex)
{
    emit frameSelected(0, frameIndex);
}

void TupTimeLineTable::frameSelectionFromLayerHeader(int layerIndex)
{
    if (layersColumn->currentSectionIndex() != layerIndex)
        layersColumn->updateSelection(layerIndex);

    emit frameSelected(layerIndex, currentColumn());
}

void TupTimeLineTable::requestLayerMove(int logicalIndex, int oldLayerIndex, int newLayerIndex)
{
    Q_UNUSED(logicalIndex);

    if (!layersColumn->sectionIsMoving()) {
        int distance = newLayerIndex - oldLayerIndex;
        if (distance > 0) {
            if (distance > 1)
                newLayerIndex = oldLayerIndex + 1;
        } else {
            distance = abs(distance);
            if (distance > 1)
                newLayerIndex = oldLayerIndex - 1;
        }

        isLocalRequest = true;
        emit layerMoved(oldLayerIndex, newLayerIndex);
    }
}

void TupTimeLineTable::setItemSize(int w, int h)
{
    rectHeight = h;
    rectWidth = w;
    
    fixSize();
}

bool TupTimeLineTable::isSoundLayer(int layerIndex)
{
    if (layerIndex < 0 && layerIndex >= rowCount())
        return false;
    
    return layersColumn->isSound(layerIndex);
}

void TupTimeLineTable::insertLayer(int layerIndex, const QString &name)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupTimeLineTable::insertLayer()] - layerIndex -> " << layerIndex;
    #endif

    insertRow(layerIndex);
    layersColumn->insertSection(layerIndex, name);
    fixSize();
}

void TupTimeLineTable::insertSoundLayer(int layerIndex, const QString &name)
{
    insertRow(layerIndex);
    layersColumn->insertSection(layerIndex, name);
    fixSize();
}

void TupTimeLineTable::removeLayer(int layerIndex)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupTimeLineTable::removeLayer()] - layerIndex -> " << layerIndex;
    #endif

    removingLayer = true;
    removeRow(layerIndex);
    layersColumn->removeSection(layerIndex);
}

void TupTimeLineTable::moveLayer(int oldLayerIndex, int newLayerIndex)
{
    if (oldLayerIndex < 0 || oldLayerIndex >= rowCount() || newLayerIndex < 0 || newLayerIndex >= rowCount()) 
        return;

    layersColumn->moveHeaderSection(oldLayerIndex, newLayerIndex, isLocalRequest);
    if (isLocalRequest)
        isLocalRequest = false;

    int framesTotal = layersColumn->lastFrame(oldLayerIndex);
    for (int frameIndex = 0; frameIndex <= framesTotal; frameIndex++)
         exchangeFrame(frameIndex, oldLayerIndex, frameIndex, newLayerIndex);

    blockSignals(true);
    selectFrame(newLayerIndex, currentColumn());
    blockSignals(false);
}

void TupTimeLineTable::exchangeFrame(int currentFrameIndex, int currentLayerIndex, int newFrameIndex, int newLayerIndex)
{
    QTableWidgetItem *oldItem = takeItem(currentLayerIndex, currentFrameIndex);
    QTableWidgetItem *newItem = takeItem(newLayerIndex, newFrameIndex);

    setItem(newLayerIndex, newFrameIndex, oldItem);
    setItem(currentLayerIndex, currentFrameIndex, newItem);
}

void TupTimeLineTable::setLayerVisibility(int layerIndex, bool isVisible)
{
    if (layersColumn) {
        clearSelection();
        layersColumn->setSectionVisibility(layerIndex, isVisible);
    }
}

void TupTimeLineTable::setLayerName(int layerIndex, const QString &name)
{
    if (layersColumn)
        layersColumn->setSectionTitle(layerIndex, name);
}

int TupTimeLineTable::currentLayer()
{
    return currentRow();
}

int TupTimeLineTable::currentFrame()
{
    return currentColumn();
}

int TupTimeLineTable::layersCount()
{
    return rowCount();
}

int TupTimeLineTable::lastFrameByLayer(int layerIndex)
{
    if (layerIndex < 0 || layerIndex >= rowCount())
        return -1;

    if (layersColumn)
        return layersColumn->lastFrame(layerIndex);

    return -1;
}

// FRAMES

void TupTimeLineTable::insertFrame(int layerIndex)
{
    if (layerIndex < 0 || layerIndex >= rowCount())
        return;

    layersColumn->updateLastFrame(layerIndex, true);
  
    int lastFrame = layersColumn->lastFrame(layerIndex);

    setAttribute(layerIndex, lastFrame, TupTimeLineTableItem::IsUsed, true);
    setAttribute(layerIndex, lastFrame, TupTimeLineTableItem::IsSound, false);
}

void TupTimeLineTable::restoreFrameSelection(int layerIndex, int frameIndex, const QString &selection)
{
    if (layerIndex < 0 || layerIndex >= rowCount())
        return;

    QStringList blocks = selection.split(":");
    QStringList params = blocks.at(0).split(",");
    int layers = params.at(0).toInt();
    int frames = params.at(1).toInt();
    QStringList flags = blocks.at(1).split(",");

    for (int i=layerIndex,index=0; i<layers; i++,index++) {
        bool remove = flags.at(index).toInt();
        if (remove)
            removeFrame(i, 0);
    }
    generateFrames(layerIndex, layers, frames);

    blockSignals(true);
    setCurrentItem(item(layerIndex, frameIndex));
    blockSignals(false);

    viewport()->update();
}

void TupTimeLineTable::pasteFrameSelection(int layerIndex, int frameIndex, int layers, int frames)
{
    if (layerIndex < 0 || layerIndex >= rowCount())
        return;

    generateFrames(layerIndex, layers, frames);

    clearSelection();
    blockSignals(true);
    setCurrentItem(item(layerIndex, frameIndex + (frames - 1)));
    blockSignals(false);

    viewport()->update();
}

void TupTimeLineTable::generateFrames(int layerIndex, int layers, int frames)
{
    int layersTotal = layerIndex + layers;
    for (int i=layerIndex; i<layersTotal; i++) {
         if (i < layersCount()) {
             int initFrame = layersColumn->lastFrame(i) + 1;
             int framesTotal = initFrame + frames;
             for (int j=initFrame; j<framesTotal; j++) {
                 setAttribute(i, j, TupTimeLineTableItem::IsUsed, true);
                 layersColumn->updateLastFrame(i, true);
             }
         }
    }
}

void TupTimeLineTable::updateLayerHeader(int layerIndex)
{
    layersColumn->updateSelection(layerIndex);
}

void TupTimeLineTable::selectFrame(int frameIndex)
{
    setCurrentItem(item(currentRow(), frameIndex));
}

void TupTimeLineTable::removeFrame(int layerIndex, int frameIndex)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupTimeLineTable::removeFrame()]";
    #endif

    Q_UNUSED(frameIndex)

    if (layerIndex < 0 || layerIndex >= rowCount())
        return;

    removingFrame = true;
    setAttribute(layerIndex, layersColumn->lastFrame(layerIndex), TupTimeLineTableItem::IsUsed, false);
    layersColumn->updateLastFrame(layerIndex, false);

    viewport()->update();
}

void TupTimeLineTable::removeFrameSelection(int layerIndex, int frameIndex, int layers, int frames, bool doSelection)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupTimeLineTable::removeFrameSelection()]";
    #endif

    if (layerIndex < 0 || layerIndex >= rowCount())
        return;

    int layersTotal = layerIndex + layers;
    for (int i=layerIndex; i<layersTotal; i++) {
         int frameTarget = frameIndex;  
         int framesTotal = frameIndex + frames;
         if (frames == (layersColumn->lastFrame(i) + 1))
             frameTarget = 1;

         for (int j=frameTarget; j<framesTotal; j++) {
              setAttribute(i, layersColumn->lastFrame(i), TupTimeLineTableItem::IsUsed, false);
              layersColumn->updateLastFrame(i, false);
         }
    }

    int lastIndex = layersColumn->lastFrame(layerIndex);
    if (lastIndex < frameIndex)
        frameIndex = lastIndex;

    if (doSelection) {
        blockSignals(true);
        setCurrentItem(item(layerIndex, frameIndex));
        blockSignals(false);
    }

    viewport()->update();
}

/*
void TupTimeLineTable::lockFrame(int layerIndex, int frameIndex, bool lock)
{
    if (layerIndex < 0 || layerIndex >= rowCount())
        return;
    
    setAttribute(layerIndex, frameIndex, TupTimeLineTableItem::IsLocked, lock);
    viewport()->update();
}

bool TupTimeLineTable::frameIsLocked(int layerIndex, int frameIndex)
{
    TupTimeLineTableItem *frame = dynamic_cast<TupTimeLineTableItem *>(item(layerIndex, frameIndex));

    if (frame) {
        return frame->isLocked(); 
    } else {
        #ifdef TUP_DEBUG
            qDebug() << "TupTimeLineTable::frameIsLocked() - Layer: " + QString::number(layerIndex) + QString(", Frame: ") + QString::number(frameIndex) + QString(" doesn't exist");
        #endif
    }

    return false;
}
*/

void TupTimeLineTable::setAttribute(int layerIndex, int frameIndex, TupTimeLineTableItem::Attributes att, bool value)
{
    QTableWidgetItem *item = this->item(layerIndex, frameIndex);
    if (!item) {
        item = new TupTimeLineTableItem;
        setItem(layerIndex, frameIndex, item);
    }
    
    item->setData(att, value);
}

void TupTimeLineTable::fixSize()
{
    int framesTotal = columnCount();
    for (int column = 0; column < framesTotal; column++)
         horizontalHeader()->resizeSection(column, rectWidth);

    int layerTotal = rowCount();
    for (int row = 0; row < layerTotal; row++)
         layersColumn->resizeSection(row, rectHeight);
}

void TupTimeLineTable::requestFrameSelection(int currentLayerIndex, int currentFrameIndex, int previousLayerIndex, int previousFrameIndex)
{
    if (!removingLayer) {
        if (removingFrame) {
            removingFrame = false;
            emit frameSelected(currentRow(), currentColumn());
            return;
        } else {
            if (previousFrameIndex != currentFrameIndex || previousLayerIndex != currentLayerIndex) 
                emit frameSelected(currentRow(), currentColumn());
        }
    } else { // A layer is being removed
        removingLayer = false;

        if (previousLayerIndex != 0) {
            if (previousLayerIndex != rowCount() - 1) {
                blockSignals(true);
                setCurrentItem(item(previousLayerIndex - 1, currentColumn())); 
                layersColumn->updateSelection(previousLayerIndex - 1);
                blockSignals(false);
            }
        }
    }
}

void TupTimeLineTable::mousePressEvent(QMouseEvent *event)
{
    int frameIndex = columnAt(event->x());

    int total = columnCount();
    if ((frameIndex >= total - 11) && (frameIndex <= total - 1)) {
        int newTotal = total + 100;
        for (int i=total; i < newTotal; i++)
             insertColumn(i);
        fixSize();
    }

    QTableWidget::mousePressEvent(event);
}

void TupTimeLineTable::mouseMoveEvent(QMouseEvent *event)
{
    // int frameIndex = columnAt(event->y());
    QList<int> selection = currentSelection();

    for (int j=selection.at(0); j<=selection.at(1); j++) {
        int top = layersColumn->lastFrame(j);
        if (selection.at(3) >= top) {
            for (int i=top; i<=selection.at(3); i++)
                emit frameSelected(j, i);
        }
    }

    QTableWidget::mouseMoveEvent(event);
}

void TupTimeLineTable::keyPressEvent(QKeyEvent *event)
{
    /*
      qDebug() << "TupTimeLineTable::keyPressEvent() - event->key() -> " << event->key();
      qDebug() << "TupTimeLineTable::keyPressEvent() - event->modifiers() -> " << event->modifiers();
    */

    // SQA: Check if this piece of code is obsolete 
    // Fn + Left/Right arrow
    if (event->key() == 16777232 || event->key() == 16777233)
        return;

    if (event->key() == Qt::Key_C) {
        if (event->modifiers() == Qt::ControlModifier) {
            emit selectionCopied();
        } else {
            if (event->modifiers() == Qt::AltModifier) {
                clearSelection();
                int layer = currentLayer();
                int frames = layersColumn->lastFrame(layer);
                for (int j = 0; j <= frames; j++)
                    selectionModel()->select(model()->index(layer, j), QItemSelectionModel::Select);
                emit selectionCopied();
            }
        }

        return;
    }

    if (event->key() == Qt::Key_R) {
        if (event->modifiers() == Qt::AltModifier) {
            clearSelection();
            int columns = layersColumn->columnsTotal();
            int frame = currentFrame();
            for (int j = 0; j < columns; j++)
                selectionModel()->select(model()->index(j, frame), QItemSelectionModel::Select);
            emit selectionCopied();
        }
        return;
    }

    if ((event->key() == Qt::Key_A) && (event->modifiers() == Qt::ControlModifier)) {
        clearSelection();
        for (int i = 0; i < layersColumn->columnsTotal(); i++) {
            int frames = layersColumn->lastFrame(i);
            for (int j = 0; j <= frames; j++)
                selectFrame(i, j);
        }
        emit selectionCopied();
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

    if (event->key() == Qt::Key_Backspace || event->key() == Qt::Key_Delete) {
        emit selectionRemoved();
        return;
    }

    if (event->key() == Qt::Key_Return) {
        emit newPerspective(4);
        return;
    }

    if (event->key() == Qt::Key_Right || event->key() == Qt::Key_PageDown) {
        int limit = columnCount() - 1;
        int next = currentColumn() + 1;
        if (next <= limit) { 
            if (event->modifiers() == Qt::ControlModifier)
                emit frameExtended(currentRow(), currentColumn());
            else
                setCurrentCell(currentRow(), next);
        }
        return;
    }    

    if (event->key() == Qt::Key_Left || event->key() == Qt::Key_PageUp) {
        int next = currentColumn() - 1;
        if (next >= 0) { 
            if (event->modifiers() == Qt::ControlModifier)
                emit selectionRemoved();
            else
                setCurrentCell(currentRow(), next);
        } else {
            if (next == -1 && event->modifiers() == Qt::ControlModifier)
                emit selectionRemoved();
        }
        return;
    }

    if (event->key() == Qt::Key_Up) {
        int next = currentRow() - 1;
        if (next >= 0) 
            setCurrentCell(next, currentColumn());
        return;
    }

    if (event->key() == Qt::Key_Down) {
        int limit = rowCount() - 1;
        int next = currentRow() + 1;
        if (next <= limit)
            setCurrentCell(next, currentColumn());
        return;
    }

    QTableWidget::keyPressEvent(event);
}

void TupTimeLineTable::enterEvent(QEvent *event)
{
    if (!hasFocus())
        setFocus();

    QTableWidget::enterEvent(event);
}

void TupTimeLineTable::leaveEvent(QEvent *event)
{
    if (hasFocus())
        clearFocus();

    QTableWidget::leaveEvent(event);
}

void TupTimeLineTable::selectFrame(int layerIndex, int frameIndex)
{
    blockSignals(true);
    setCurrentCell(layerIndex, frameIndex);
    updateLayerHeader(layerIndex);
    blockSignals(false);
}

void TupTimeLineTable::selectFrame(int layerIndex, int frameIndex, const QString &selection)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupTimeLineTable::selectFrame()] - layerIndex -> " << layerIndex
                 << " - frameIndex -> " << frameIndex << " - selection -> " << selection;
    #endif

    blockSignals(true);
    setCurrentCell(layerIndex, frameIndex);
    updateLayerHeader(layerIndex);
    blockSignals(false);

    QStringList coords = selection.split(","); 
    if (coords.count() == 4) {
        int initLayer = coords.at(0).toInt();
        int lastLayer = coords.at(1).toInt(); 
        int initFrame = coords.at(2).toInt();
        int lastFrame = coords.at(3).toInt();

        selectionModel()->clearSelection();

        QModelIndexList indexes;
        for (int i=initLayer; i<=lastLayer; i++) {
            for (int j=initFrame; j<=lastFrame; j++)
                selectionModel()->select(model()->index(i, j), QItemSelectionModel::Select);
        }
    } else {
        #ifdef TUP_DEBUG
            qDebug() << "[TupTimeLineTable::selectFrame()] - Error: Selection coords are invalid!";
        #endif
    }
}

QList<int> TupTimeLineTable::currentSelection()
{
    QModelIndexList selection = selectedIndexes();
    QList<int> coords;
    QList<int> layers;
    QList<int> frames;

    if (!selection.isEmpty()) {
        foreach (QModelIndex cell, selection) {
            int layer = cell.row();
            int frame = cell.column();

            if (!layers.contains(layer))
                layers << layer;
            if (!frames.contains(frame))
                frames << frame;
        }
        coords << layers.first() << layers.last() << frames.first() << frames.last();
    }

    return coords;
}

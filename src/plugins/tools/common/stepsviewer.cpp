/***************************************************************************
 *   Project TUPITUBE DESK                                                *
 *   Project Contact: info@maefloresta.com                                 *
 *   Project Website: http://www.maefloresta.com                           *
 *   Project Leader: Gustav Gonzalez <info@maefloresta.com>                *
 *                                                                         *
 *   Developers:                                                           *
 *                                                                         *
 *   2019:                                                                 *
 *    Alejandro Carrasco                                                   *
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

#include "stepsviewer.h"

#include <QLineEdit>

StepsViewer::StepsViewer(QWidget *parent) : QTableWidget(parent)
{
    verticalHeader()->hide();

    intervalsTotal = 0;
    setColumnCount(4);
    setColumnWidth(0, 70);
    setColumnWidth(1, 60);
    setColumnWidth(2, 20); 
    setColumnWidth(3, 20);

    setHorizontalHeaderLabels(QStringList() << tr("Interval") << tr("Frames") << tr("") << tr(""));

    setMinimumWidth(174);
    setMaximumHeight(800);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    plusButton = new QList<TPushButton*>();
    minusButton = new QList<TPushButton*>();
}

StepsViewer::~StepsViewer()
{
    delete plusButton;
    delete minusButton;
}

QSize StepsViewer::sizeHint() const
{
    return QSize(maximumWidth(), maximumHeight());
}

void StepsViewer::loadPath(const QGraphicsPathItem *pathItem, QList<int> intervals)
{
    #ifdef TUP_DEBUG
        qDebug() << "[StepsViewer::loadPath()] - intervals size ->" << intervals.size();
    #endif

    frameIntervals = intervals;
    intervalsTotal = frameIntervals.count();

    // Set of key points which define the path
    path = pathItem->path();
    pointsList = path.toFillPolygon();

    if (pointsList.isEmpty()) {
        #ifdef TUP_DEBUG
            qDebug() << "[StepsViewer::loadPath()] - Fatal Error: Path has NO points!";
        #endif
        return;
    }

    pointsList.removeLast();

    // This method calculates the list of (green) key points of the path
    calculateKeys();
    // This method calculates the blocks of points per segment
    calculateGroups();

    for (int row=0; row<intervalsTotal; row++) { // Processing every segment
         QList<QPointF> block = blocksList.at(row);
         int framesCount = frameIntervals.at(row);
         int blockSize = block.size();

         QList<QPointF> segmentPoints;

         if ((blockSize > 2) && (framesCount > 2)) {
             int delta = blockSize/(framesCount-1);
             int pos = delta;
             if (row==0) { // For the first segment, save the first point
                 framesCount--;
                 segmentPoints.append(block.at(0));
             } else {
                 delta = blockSize/framesCount;
             }

             for (int i=1; i < framesCount; i++) { // calculating points set for the segment j
                 segmentPoints << block.at(pos);
                 pos += delta;
             }

             /*
             if (framesCount > 2) {
                 for (int i=1; i < framesCount; i++) { // calculating points set for the segment j
                      segmentPoints << block.at(pos);
                      pos += delta;
                 }
             } else {
                 if (row > 0)
                     segmentPoints << block.at(pos);
             }
             */

             segmentPoints << keys.at(row);
         } else { // Only two points in the segment
             QPointF init = block.at(0);
             int range = framesCount;
             if (row == 0) {
                 range--;
                 segmentPoints << init;
             } else {
                 init = keys.at(row-1);
             }

             if (row == 0 && range == 1)
                 segmentPoints << keys.at(row);
             else
                 segmentPoints.append(calculateSegmentPoints(init, keys.at(row), range));
         } 

         segments << segmentPoints;
         addTableRow(row, segmentPoints.count());
    }

    loadTweenPoints();
}

void StepsViewer::setPath(const QGraphicsPathItem *pathItem)
{
    #ifdef TUP_DEBUG
        qDebug() << "[StepsViewer::setPath()]";
    #endif

    if (!pathItem) {
        #ifdef TUP_DEBUG
            qDebug() << "[StepsViewer::setPath()] - Fatal Error: pathItem is NULL!";
        #endif
        return;
    }

    // Set of key points which define the path 
    path = pathItem->path();
    pointsList = path.toFillPolygon();

    if (!pointsList.isEmpty())
        pointsList.removeLast();

    calculateKeys();

    if (intervalsTotal < keys.size()) { // A new table row must be added. Last segment must be calculated
        intervalsTotal = keys.size();
        int row = intervalsTotal - 1;

        QList<QPointF> segment;
        QPointF pInit;
        int framesCount = 4;
        int range = framesCount;
        if (row == 0) {
            framesCount++;
            pInit = pointsList.at(0);
            segment << pInit;
        } else {
            pInit = keys.at(row-1);
        }

        frameIntervals << framesCount;
        addTableRow(row, framesCount);

        segment.append(calculateSegmentPoints(pInit, keys.at(row), range));
        segments << segment;
    } else { // A node was edited, segments must be recalculated
        // Recalculating segments
        calculateGroups();

        int total = frameIntervals.count();
        for (int row=0; row < total; row++) { // Processing every segment
             QList<QPointF> block = blocksList.at(row);
             int framesCount = frameIntervals.at(row);
             int size = block.size();
             QList<QPointF> segment; 

             if (size > 2) {
                 if (size < framesCount) { // There are less available points than path points
                     int range = size;
                     QList<QPointF> input = block;
                     while (range < framesCount) {
                            QList<QPointF> newBlock;
                            for (int i=0; i<input.size()-1; i++) {
                                 QPointF step = input.at(i+1) - input.at(i);
                                 QPointF middle = input.at(i) + QPointF(step.x()/2, step.y()/2);
                                 newBlock << input.at(i) << middle;
                            }
                            newBlock << input.last();
                            range = newBlock.size();
                            input = newBlock;
                     }
                     size = input.size();
                     block = input;
                 } 

                 if (row==0) {
                     framesCount--;
                     segment.append(block.at(0));
                 }

                 int delta = size/framesCount;
                 int pos = delta;
                 if (framesCount > 2) {
                     int modDelta = 0;
                     int module = size % framesCount;
                     if (module > 0)
                         modDelta = framesCount/module;

                     int modPos = 1;
                     int modCounter = 1;

                     for (int i=1; i < framesCount; i++) { // calculating points set for the segment j
                          if (module > 0) {
                              if (i == modPos && modCounter < module) {
                                  pos++;
                                  modPos += modDelta;
                                  modCounter++;
                              }
                          }
                          segment << block.at(pos);
                          pos += delta;
                     }
                 } else {
                     if (row > 0)
                         segment << block.at(pos);
                     else // when frames == 3
                         segment << block.at(size/2);
                 }

                 segment << keys.at(row);
             } else {
                 QPointF init = block.at(0);
                 if (row == 0) {
                     framesCount--;
                     segment << init;
                 } else {
                     init = keys.at(row-1);
                 }

                 if (row == 0 && framesCount == 1)
                     segment << keys.at(row);
                 else
                     segment.append(calculateSegmentPoints(init, keys.at(row), framesCount));
             }  

             segments.replace(row, segment);
        }
    }

    loadTweenPoints();
}

// +/- frames slot and text/input slot 
void StepsViewer::updatePathSection(int column, int row)
{
    #ifdef TUP_DEBUG
        qDebug() << "[StepsViewer::updatePathSection()]";
        qDebug() << "*** column: " <<  column << " - row: " << row;
    #endif

    QTableWidgetItem *cell = item(row, 1);
    int framesCount = cell->text().toInt();

    calculateKeys();
    calculateGroups();

    QList<QPointF> block = blocksList.at(row);
    int range = block.size();

    if (column == 2) // Plus button clicked
        framesCount += 1;

    if (column == 3) // Minus button clicked 
        framesCount -= 1;

    if (row == 0) {
        if (framesCount < 2)
            framesCount = 2;
    } else {
        if (framesCount < 1)
            framesCount = 1;
    }

    frameIntervals.replace(row, framesCount);
    QList<QPointF> segment;

    if (range > 2) {
        if (range < framesCount) { // There are less available points than path points
            int size = range;
            QList<QPointF> input = block;
            while (size < framesCount) {
                   QList<QPointF> newBlock;
                   for (int i=0; i<input.size()-1; i++) {
                        QPointF step = input.at(i+1) - input.at(i);  
                        QPointF middle = input.at(i) + QPointF(step.x()/2, step.y()/2); 
                        newBlock << input.at(i) << middle;
                   }
                   newBlock << input.last();
                   size = newBlock.size(); 
                   input = newBlock;
            }

            range = input.size();
            block = input;
        }

        if (row==0) {
            framesCount--;
            segment.append(block.at(0));
        }
        int delta = range/framesCount;
        int pos = delta;

        if (framesCount > 2) {
            int module = range % framesCount;
            int modDelta = 0;  
            if (module > 0)
                modDelta = framesCount/module;
            int modPos = 1;
            int modCounter = 1;

            for (int i=1; i < framesCount; i++) { // calculating points set for the segment j
                 if (module > 0) {
                     if (i == modPos && modCounter < module) {
                         pos++;
                         modCounter++;
                         modPos += modDelta;
                     }
                 }
                 segment << block.at(pos);
                 pos += delta;
            }
        } else {
            if (row > 0)
                segment << block.at(pos);
            else  // when frames == 3
                segment << block.at(range/2);
        }

        segment << keys.at(row);
    } else {
        QPointF init = block.at(0);
        if (row == 0) {
            framesCount--;
            segment << init;
        } else {
            init = keys.at(row-1);
        }

        if (row == 0 && framesCount == 1)
            segment << keys.at(row);
        else 
            segment.append(calculateSegmentPoints(init, keys.at(row), framesCount));
    }

    cell->setText(QString::number(segment.count()));
    segments.replace(row, segment);

    loadTweenPoints();

    emit totalHasChanged(totalSteps());
}

QVector<TupTweenerStep *> StepsViewer::steps()
{
    QVector<TupTweenerStep *> stepsVector;
    int i = 0;
    foreach (QList<QPointF> segment, segments) {
        foreach (QPointF point, segment) {
            TupTweenerStep *step = new TupTweenerStep(i);
            step->setPosition(point);
            stepsVector << step;
            i++;
        }
    }

    return stepsVector;
}

int StepsViewer::totalSteps()
{
    #ifdef TUP_DEBUG
        qDebug() << "[StepsViewer::totalSteps()] - frames.count() ->" << frameIntervals.count();
    #endif

    int total = 0;
    for (int i=0; i < frameIntervals.count(); i++)
         total += frameIntervals.at(i);

    #ifdef TUP_DEBUG
        qDebug() << "[StepsViewer::totalSteps()] - total ->" << total;
    #endif

    return total;
}

void StepsViewer::loadTweenPoints()
{
    tweenPointsList.clear();
    foreach (QList<QPointF> segment, segments) {
        foreach (QPointF point, segment) 
             tweenPointsList << point;
    }
}

QList<QPointF> StepsViewer::tweenPoints()
{
    return tweenPointsList;
}

QString StepsViewer::intervals()
{
    QString output = ""; 
    foreach(int interval, frameIntervals)
        output += QString::number(interval) + ",";

    output.chop(1);
    return output;
}

void StepsViewer::clearInterface()
{
    #ifdef TUP_DEBUG
        qDebug() << "[StepsViewer::clearInterface()]";
    #endif

    intervalsTotal = 0;
    frameIntervals.clear();
    segments.clear();
    tweenPointsList.clear();

    int size = rowCount() - 1;
    for (int i=size; i >= 0; i--)
         removeRow(i);

    plusButton->clear();
    minusButton->clear();
}

QList<QPointF> StepsViewer::calculateSegmentPoints(QPointF begin, QPointF end, int total)
{
    #ifdef TUP_DEBUG
        qDebug() << "[StepsViewer::calculateSegmentPoints()] - begin point -> " << begin;
        qDebug() << "[StepsViewer::calculateSegmentPoints()] - end point -> " << end;
    #endif

    QList<QPointF> pathPoints;
    QPointF dot;
    qreal x;
    qreal y;
    qreal delta;

    if (end.x() != begin.x()) {
        qreal m = (end.y() - begin.y())/(end.x() - begin.x());
        qreal b = begin.y() - (m*begin.x());
        delta = (end.x() - begin.x())/total;
        x = begin.x();

        for (int i=0; i<(total-1); i++) {
             x += delta;
             y = m*x + b;
             dot.setX(x);
             dot.setY(y);
             pathPoints.append(dot);
        }
    } else {
        delta = (end.y() - begin.y())/total;
        x = begin.x();
        y = begin.y();
        for (int i=0; i<(total-1); i++) {
             y += delta;
             dot.setX(x);
             dot.setY(y);
             pathPoints.append(dot);
        }
    }

    pathPoints.append(end);

    return pathPoints;
}

void StepsViewer::addTableRow(int row, int frames)  
{
    #ifdef TUP_DEBUG
        qDebug() << "[StepsViewer::addTableRow()]";
        qWarning() << "row: " << row;
        qWarning() << "frames: " << frames;
    #endif

    setRowCount(rowCount() + 1);

    QTableWidgetItem *intervalItem = new QTableWidgetItem();
    intervalItem->setTextAlignment(Qt::AlignCenter);
    intervalItem->setText(QString::number(row + 1));
    intervalItem->setFlags(intervalItem->flags() & ~Qt::ItemIsEditable);

    QTableWidgetItem *framesItem = new QTableWidgetItem();
    framesItem->setTextAlignment(Qt::AlignCenter);
    framesItem->setText(QString::number(frames));

    plusButton->append(new TPushButton(this, "+", 2, row));
    connect(plusButton->at(row), SIGNAL(clicked(int, int)), this, SLOT(updatePathSection(int, int)));
    minusButton->append(new TPushButton(this, "-", 3, row));
    connect(minusButton->at(row), SIGNAL(clicked(int, int)), this, SLOT(updatePathSection(int, int)));

    setItem(row, 0, intervalItem);
    setItem(row, 1, framesItem);
    setCellWidget(row, 2, plusButton->at(row));
    setCellWidget(row, 3, minusButton->at(row));

    setRowHeight(row, 20);
}

// Store all the points of the current path
void StepsViewer::calculateKeys()
{
    #ifdef TUP_DEBUG
        qDebug() << "[StepsViewer::calculateKeys()]";
    #endif

    keys.clear();
    int total = path.elementCount();
    int count = 0;

    for (int i = 1; i < total; i++) {
         QPainterPath::Element e = path.elementAt(i);
         if (e.type != QPainterPath::CurveToElement) {
             if ((e.type == QPainterPath::CurveToDataElement) &&
                 (path.elementAt(i-1).type == QPainterPath::CurveToElement))
                 continue;

             QPointF point(e.x, e.y);
             keys.append(point);
             count++;
         }
    }
}

// Calculate blocks of points per segment 
void StepsViewer::calculateGroups()
{
    #ifdef TUP_DEBUG
        qDebug() << "[StepsViewer::calculateGroups()]";
    #endif

    blocksList.clear();

    int index = 0;
    int total = pointsList.size();
    QList<QPointF> segment;

    for (int i=0; i < total; i++) { // Counting points between keys and saving key indexes
         QPointF point = pointsList.at(i);
         if (point == keys.at(index)) {
             segment << point;
             blocksList << segment;
             index++;
             segment = QList<QPointF>();
        } else {
             segment << point;
        }
    }
}

void StepsViewer::commitData(QWidget *editor)
{
    #ifdef TUP_DEBUG
        qDebug() << "[StepsViewer::commitData()]";
    #endif

    QLineEdit *lineEdit = qobject_cast<QLineEdit *>(editor);

    if (lineEdit) {
        QString value = lineEdit->text();
        bool ok;
        int framesCount = value.toInt(&ok, 10);

        if (ok) {
            value = QString::number(framesCount);
            int row = currentRow();
            int column = currentColumn();
            QTableWidgetItem *cell = item(row, column);
            cell->setText(value);
            #ifdef TUP_DEBUG
                qDebug() << "[StepsViewer::commitData()] - Processing value -> " << value;
            #endif
            updatePathSection(column, row);
        } else {
            #ifdef TUP_DEBUG
                qWarning() << "[StepsViewer::commitData()] - Input value -> " << value;
            #endif
        }
    }
}

void StepsViewer::undoSegment(const QPainterPath painterPath)
{
    #ifdef TUP_DEBUG
        qDebug() << "[StepsViewer::undoSegment()]";
    #endif

    path = painterPath;

    pointsList = path.toFillPolygon();
    if (!pointsList.isEmpty())
        pointsList.removeLast();

    calculateKeys();
    calculateGroups();

    if (!frameIntervals.isEmpty()) {
        undoFrames << frameIntervals.last();
        frameIntervals.removeLast();
    }

    intervalsTotal--;

    if (!segments.isEmpty()) {
        undoSegments << segments.last();
        segments.removeLast();
    }

    updateSegments();

    if (!plusButton->isEmpty())
        plusButton->removeLast();

    if (!minusButton->isEmpty())
        minusButton->removeLast();

    removeRow(rowCount()-1);

    emit totalHasChanged(totalSteps());
}

void StepsViewer::redoSegment(const QPainterPath painterPath)
{
    #ifdef TUP_DEBUG
        qDebug() << "[StepsViewer::redoSegment()]";
    #endif

    path = painterPath;

    pointsList = path.toFillPolygon();
    pointsList.removeLast();

    calculateKeys();
    calculateGroups();

    frameIntervals << undoFrames.last();
    undoFrames.removeLast();
    intervalsTotal++;

    segments << undoSegments.last();
    undoSegments.removeLast();

    updateSegments();

    addTableRow(rowCount(), frameIntervals.last());
}

void StepsViewer::updateSegments(const QPainterPath painterPath)
{
    #ifdef TUP_DEBUG
        qDebug() << "[StepsViewer::undoSegment()]";
    #endif

    path = painterPath;

    pointsList = path.toFillPolygon();
    if (!pointsList.isEmpty())
        pointsList.removeLast();

    calculateKeys();
    calculateGroups();
    updateSegments();
}

void StepsViewer::updateSegments()
{
    #ifdef TUP_DEBUG
        qDebug() << "[StepsViewer::updateSegments()]";
    #endif

    int total = frameIntervals.count();
    for (int row=0; row < total; row++) { // Processing every segment
         QList<QPointF> block = blocksList.at(row);
         int framesCount = frameIntervals.at(row);
         int size = block.size();
         QList<QPointF> segment; 

         if (size > 2) {
             if (size < framesCount) { // There are less available points than path points
                 int range = size;
                 QList<QPointF> input = block;
                 while (range < framesCount) {
                        QList<QPointF> newBlock;
                        for (int i=0; i<input.size()-1; i++) {
                             QPointF step = input.at(i+1) - input.at(i);
                             QPointF middle = input.at(i) + QPointF(step.x()/2, step.y()/2);
                             newBlock << input.at(i) << middle;
                        }
                        newBlock << input.last();
                        range = newBlock.size();
                        input = newBlock;
                 }
                 size = input.size();
                 block = input;
             } 

             if (row==0) {
                 framesCount--;
                 segment.append(block.at(0));
             }

             int delta = size/framesCount;
             int pos = delta;
             if (framesCount > 2) {
                 int modDelta = 0;
                 int module = size % framesCount;
                 if (module > 0)
                     modDelta = framesCount/module;

                 int modPos = 1;
                 int modCounter = 1;

                 for (int i=1; i < framesCount; i++) { // calculating points set for the segment j
                      if (module > 0) {
                          if (i == modPos && modCounter < module) {
                              pos++;
                              modPos += modDelta;
                              modCounter++;
                          }
                      }
                      segment << block.at(pos);
                      pos += delta;
                 }
             } else {
                 if (row > 0)
                     segment << block.at(pos);
                 else // when frames == 3
                     segment << block.at(size/2);
             }

             segment << keys.at(row);
         } else {
             QPointF init = block.at(0);
             if (row == 0) {
                 framesCount--;
                 segment << init;
             } else {
                 init = keys.at(row-1);
             }

             if (row == 0 && framesCount == 1)
                 segment << keys.at(row);
             else
                 segment.append(calculateSegmentPoints(init, keys.at(row), framesCount));
         }  

         segments.replace(row, segment);
    }

    loadTweenPoints();
}

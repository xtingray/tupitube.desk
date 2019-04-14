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

#include "filltool.h"

#include "tconfig.h"
#include "tupsvgitem.h"
#include "tupserializer.h"
#include "tupitemconverter.h"
#include "tuprequestbuilder.h"
#include "tupscene.h"
#include "tupinputdeviceinformation.h"
#include "tupgraphicsscene.h"
#include "tupprojectrequest.h"
#include "tupbrushmanager.h"
#include "tupgraphiclibraryitem.h"
#include "tupitemgroup.h"
#include "tosd.h"

FillTool::FillTool()
{
    setupActions();
}

FillTool::~FillTool()
{
}

void FillTool::init(TupGraphicsScene *gScene)
{
    scene = gScene;

    TCONFIG->beginGroup("ColorPalette");
    int colorMode = TCONFIG->value("CurrentColorMode", 0).toInt();
    mode = TColorCell::FillType(colorMode);
}

QStringList FillTool::keys() const
{
    return QStringList() << tr("Fill Tool");
}

void FillTool::setupActions()
{
    insideCursor = QCursor(kAppProp->themeDir() + "cursors/internal_fill.png", 0, 11);
    contourCursor = QCursor(kAppProp->themeDir() + "cursors/line_fill.png", 0, 13);

    TAction *action1 = new TAction(QIcon(kAppProp->themeDir() + "icons/internal_fill.png"), tr("Fill Tool"), this);
    action1->setShortcut(QKeySequence(tr("F")));
    action1->setToolTip(tr("Fill Tool") + " - " + "F");
    action1->setCursor(insideCursor);
    fillActions.insert(tr("Fill Tool"), action1);
}

void FillTool::press(const TupInputDeviceInformation *input, TupBrushManager *brushManager, TupGraphicsScene *gScene)
{
    #ifdef TUP_DEBUG
        #ifdef Q_OS_WIN
            qDebug() << "[FillTool::press()]";
        #else
            T_FUNCINFOX("tools");
        #endif
    #endif

    if (input->buttons() == Qt::LeftButton) {
        // SQA: Enhance this plugin to support several items with one click 
        // QList<QGraphicsItem *> list = scene->items(input->pos(), Qt::IntersectsItemShape, Qt::DescendingOrder, QTransform());

        QGraphicsItem *item = gScene->itemAt(input->pos(), QTransform());
        if (item) {
            int itemIndex = -1;
            int currentLayer;
            int currentFrame;
            TupFrame *frame;

            if (gScene->getSpaceContext() == TupProject::FRAMES_EDITION) {
                frame = gScene->currentFrame();
                itemIndex = frame->indexOf(item);
                currentLayer = gScene->currentLayerIndex();
                currentFrame = gScene->currentFrameIndex();
            } else {
                currentLayer = -1;
                currentFrame = -1;
                TupBackground *bg = gScene->currentScene()->sceneBackground();
                if (gScene->getSpaceContext() == TupProject::STATIC_BACKGROUND_EDITION) {
                    frame = bg->staticFrame();
                    itemIndex = frame->indexOf(item);
                } else if (gScene->getSpaceContext() == TupProject::DYNAMIC_BACKGROUND_EDITION) {
                    frame = bg->dynamicFrame();
                    itemIndex = frame->indexOf(item);
                }
            }

            if (itemIndex >= 0) {
                if (TupGraphicLibraryItem *libraryItem = qgraphicsitem_cast<TupGraphicLibraryItem *>(item)) {
                    // This condition only applies for images
                    if (libraryItem->type() != TupLibraryObject::Item) {
                        TOsd::self()->display(tr("Error"), tr("Sorry, only native objects can be filled"), TOsd::Error);
                        #ifdef TUP_DEBUG
                            QString msg = "FillTool::press() - Warning: item is a RASTER object!";
                            #ifdef Q_OS_WIN
                                qWarning() << msg;
                            #else
                                tWarning() << msg;
                            #endif
                        #endif
                        return;
                    }
                }

                // Testing if object is a SVG file
                TupSvgItem *svg = qgraphicsitem_cast<TupSvgItem *>(item);
                if (svg) {
                    TOsd::self()->display(tr("Error"), tr("Sorry, only native objects can be filled"), TOsd::Error);
                    #ifdef TUP_DEBUG
                        QString msg = "FillTool::press() - Warning: item is a SVG object!";
                        #ifdef Q_OS_WIN
                            qWarning() << msg;
                        #else
                            tWarning() << msg;
                        #endif
                    #endif
                    return;
                }

                if (qgraphicsitem_cast<TupItemGroup *>(item)) {
                    TOsd::self()->display(tr("Error"), tr("Sorry, Groups can't be filled yet"), TOsd::Error);
                    return;
                }

                if (qgraphicsitem_cast<QAbstractGraphicsShapeItem *>(item)) {
                    QDomDocument doc;
                    TupProjectRequest::Action action = TupProjectRequest::Brush;
                    if (mode == TColorCell::Inner) {
                        frame->checkBrushStatus(itemIndex); 
                        QBrush brush = brushManager->brush();
                        doc.appendChild(TupSerializer::brush(&brush, doc));
                    } else if (mode == TColorCell::Contour) {
                        frame->checkPenStatus(itemIndex);
                        QPen pen = brushManager->pen();
                        action = TupProjectRequest::Pen;
                        doc.appendChild(TupSerializer::pen(&pen, doc));
                    }

                    TupProjectRequest event = TupRequestBuilder::createItemRequest(
                                              gScene->currentSceneIndex(), currentLayer,
                                              currentFrame, itemIndex, QPointF(),
                                              gScene->getSpaceContext(), TupLibraryObject::Item,
                                              action, doc.toString());

                    emit requested(&event);
                } else {
                    #ifdef TUP_DEBUG
                        QString msg = "FillTool::press() - Fatal Error: QAbstractGraphicsShapeItem cast has failed!";
                        #ifdef Q_OS_WIN
                            qDebug() << msg;
                        #else
                            tError() << msg;
                        #endif
                    #endif
                }
            }
        } else {
            #ifdef TUP_DEBUG
                QString msg = "FillTool::press() - No item found";
                #ifdef Q_OS_WIN
                    qDebug() << msg;
                #else
                    tError() << msg;
                #endif
            #endif
            return;
        }
    }
}

void FillTool::move(const TupInputDeviceInformation *, TupBrushManager *, TupGraphicsScene *)
{
}

void FillTool::release(const TupInputDeviceInformation *, TupBrushManager *, TupGraphicsScene *)
{
}

QMap<QString, TAction *> FillTool::actions() const
{
    return fillActions;
}

int FillTool::toolType() const
{
    return TupToolInterface::Fill;
}
        
QWidget *FillTool::configurator()
{
    return 0;
}

void FillTool::aboutToChangeScene(TupGraphicsScene *)
{
}

void FillTool::aboutToChangeTool() 
{
    foreach (QGraphicsItem *item, scene->items()) {
        item->setFlag(QGraphicsItem::ItemIsSelectable, false);
        item->setFlag(QGraphicsItem::ItemIsFocusable, false);
    }
}

QPainterPath FillTool::mapPath(const QPainterPath &path, const QPointF &pos)
{
    QMatrix transform;
    transform.translate(pos.x(), pos.y());
    
    QPainterPath painter = transform.map(path);
    painter.closeSubpath();
    
    return painter;
}

QPainterPath FillTool::mapPath(const QGraphicsPathItem *item)
{
    return mapPath(item->path(), item->pos());
}

void FillTool::saveConfig()
{
}

void FillTool::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_F11 || event->key() == Qt::Key_Escape) {
        emit closeHugeCanvas();
    } else {
        QPair<int, int> flags = TupToolPlugin::setKeyAction(event->key(), event->modifiers());
        if (flags.first != -1 && flags.second != -1)
            emit callForPlugin(flags.first, flags.second);
    }
}

QCursor FillTool::polyCursor() const
{
    if (mode == TColorCell::Inner) {
        return insideCursor;
    } else if (mode == TColorCell::Contour) {
        return contourCursor;
    }

    return QCursor(Qt::ArrowCursor);
}

void FillTool::setColorMode(TColorCell::FillType colorMode)
{
    mode = colorMode;
}

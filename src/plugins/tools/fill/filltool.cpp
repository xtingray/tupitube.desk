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
#include "tuptextitem.h"
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

/*
SQA: Pay attention to this tip
You can use the QGraphicsItem::shape () which returns a QPainterPath to retrieve the shape of an item. 
For taking the intersection path this can be used :

QPainterPath QPainterPath::intersected ( const QPainterPath & p ) const;

So you can get the intersection path of two items like:

QPainterPath intersectedPath = item1->shape()->intersected(item2->shape());

Now you can fill the intersected area by :

painter->setBrush(QColor(122, 163, 39));
painter->drawPath(intersectedPath);
*/

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

QList<TAction::ActionId> FillTool::keys() const
{
    return QList<TAction::ActionId>() << TAction::FillTool;
}

void FillTool::setupActions()
{
    insideCursor = QCursor(kAppProp->themeDir() + "cursors/internal_fill.png", 0, 11);
    contourCursor = QCursor(kAppProp->themeDir() + "cursors/line_fill.png", 0, 13);

    TAction *action1 = new TAction(QIcon(kAppProp->themeDir() + "icons/internal_fill.png"), tr("Fill Tool"), this);
    action1->setShortcut(QKeySequence(tr("F")));
    action1->setToolTip(tr("Fill Tool") + " - " + "F");
    action1->setCursor(insideCursor);
    fillActions.insert(TAction::FillTool, action1);
}

void FillTool::press(const TupInputDeviceInformation *input, TupBrushManager *brushManager, TupGraphicsScene *gScene)
{
    #ifdef TUP_DEBUG
        qDebug() << "[FillTool::press()]";
    #endif

    if (input->buttons() == Qt::LeftButton) {
        // SQA: Enhance this plugin to support several items with one click 
        QList<QGraphicsItem *> list = scene->items(input->pos(), Qt::IntersectsItemShape, Qt::DescendingOrder, QTransform());
        foreach(QGraphicsItem *item, list) {
            // QGraphicsItem *item = gScene->itemAt(input->pos(), QTransform());
            if (item) {
                int itemIndex = -1;
                int currentLayer;
                int currentFrame;
                TupFrame *frame = new TupFrame;

                if (gScene->getSpaceContext() == TupProject::FRAMES_MODE) {
                    frame = gScene->currentFrame();
                    itemIndex = frame->indexOf(item);
                    currentLayer = gScene->currentLayerIndex();
                    currentFrame = gScene->currentFrameIndex();
                } else {
                    currentLayer = -1;
                    currentFrame = -1;
                    TupBackground *bg = gScene->currentScene()->sceneBackground();
                    if (gScene->getSpaceContext() == TupProject::VECTOR_STATIC_BG_MODE) {
                        frame = bg->vectorStaticFrame();
                        itemIndex = frame->indexOf(item);
                    } else if (gScene->getSpaceContext() == TupProject::VECTOR_DYNAMIC_BG_MODE) {
                        frame = bg->vectorDynamicFrame();
                        itemIndex = frame->indexOf(item);
                    } else if (gScene->getSpaceContext() == TupProject::VECTOR_FG_MODE) {
                        frame = bg->vectorForegroundFrame();
                        itemIndex = frame->indexOf(item);
                    }
                }

                if (itemIndex >= 0) {
                    if (TupGraphicLibraryItem *libraryItem = qgraphicsitem_cast<TupGraphicLibraryItem *>(item)) {
                        // This condition only applies for images
                        if (libraryItem->type() != TupLibraryObject::Item) {
                            TOsd::self()->display(TOsd::Error, tr("Sorry, only native objects can be filled"));
                            #ifdef TUP_DEBUG
                                qWarning() << "[FillTool::press()] - Warning: item is a RASTER object!";
                            #endif
                            return;
                        }
                    }

                    // Testing if object is a SVG file
                    TupSvgItem *svg = qgraphicsitem_cast<TupSvgItem *>(item);
                    if (svg) {
                        TOsd::self()->display(TOsd::Error, tr("Sorry, only native objects can be filled"));
                        #ifdef TUP_DEBUG
                            qWarning() << "[FillTool::press()] - Warning: item is a SVG object!";
                        #endif
                        return;
                    }

                    if (qgraphicsitem_cast<TupItemGroup *>(item)) {
                        TOsd::self()->display(TOsd::Error, tr("Sorry, Groups can't be filled yet"));
                        return;
                    }

                    if (qgraphicsitem_cast<TupTextItem *>(item)) {
                        QColor textColor = "";
                        frame->checkTextColorStatus(itemIndex);
                        if (mode == TColorCell::Inner) {
                            textColor = brushManager->brush().color();
                        } else if (mode == TColorCell::Contour) {
                            textColor = brushManager->pen().color();
                        }

                        qDebug() << "";
                        qDebug() << "TEXTCOLOR -> " << textColor.name();

                        TupProjectRequest event = TupRequestBuilder::createItemRequest(
                                                  gScene->currentSceneIndex(), currentLayer,
                                                  currentFrame, itemIndex, QPointF(),
                                                  gScene->getSpaceContext(), TupLibraryObject::Item,
                                                  TupProjectRequest::TextColor,
                                                  textColor.name() + "|" + QString::number(textColor.alpha()));
                        emit requested(&event);
                        return;
                    } else if (qgraphicsitem_cast<QAbstractGraphicsShapeItem *>(item)) {
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
                        return;
                    } else {
                        #ifdef TUP_DEBUG
                            qDebug() << "[FillTool::press()] - Fatal Error: QAbstractGraphicsShapeItem cast has failed!";
                        #endif
                    }
                } else {
                    #ifdef TUP_DEBUG
                        qDebug() << "[FillTool::press()] - Error: item is not available at the current frame";
                    #endif
                }
            } else {
                #ifdef TUP_DEBUG
                    qDebug() << "[FillTool::press()] - No item found";
                #endif
                return;
            }
        }
    }
}

void FillTool::move(const TupInputDeviceInformation *, TupBrushManager *, TupGraphicsScene *)
{
}

void FillTool::release(const TupInputDeviceInformation *, TupBrushManager *, TupGraphicsScene *)
{
}

QMap<TAction::ActionId, TAction *> FillTool::actions() const
{
    return fillActions;
}

int FillTool::toolType() const
{
    return TupToolInterface::Fill;
}
        
QWidget *FillTool::configurator()
{
    return nullptr;
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
    QTransform transform;
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

QCursor FillTool::polyCursor() // const
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

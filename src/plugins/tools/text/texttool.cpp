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

#include "texttool.h"
#include "tupscene.h"
#include "tupinputdeviceinformation.h"
#include "tupgraphicsscene.h"
#include "tupprojectrequest.h"
#include "tupprojectresponse.h"
#include "tuplibraryobject.h"
#include "tuprequestbuilder.h"
#include "tosd.h"
#include "tupserializer.h"

TextTool::TextTool()
{
    config = new TextConfigurator;
    connect(config, SIGNAL(textAdded()), this, SLOT(insertText()));

    setupActions();
}

TextTool::~TextTool()
{
}

void TextTool::init(TupGraphicsScene *gScene)
{
    scene = gScene;
    clearSelection();
    scene->clearSelection();

    nodeZValue = ((BG_LAYERS + 1) * ZLAYER_LIMIT) + (scene->currentScene()->layersCount() * ZLAYER_LIMIT);
    // initItems(scene);
}

/*
void TextTool::initItems(TupGraphicsScene *gScene)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TextTool::initItems()]";
    #endif

    foreach (QGraphicsView *view, gScene->views())
        view->setDragMode(QGraphicsView::RubberBandDrag);
}
*/

QList<TAction::ActionId> TextTool::keys() const 
{
    return QList<TAction::ActionId>() << TAction::Text;
}

QCursor TextTool::cursor()
{
    return QCursor(Qt::ArrowCursor);
}

void TextTool::press(const TupInputDeviceInformation *input, TupBrushManager *brushManager, TupGraphicsScene *scene)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TextTool::press()]";
    #endif

    Q_UNUSED(input)
    Q_UNUSED(brushManager)

    activeSelection = false;

    // User clicked on background
    QList<QGraphicsItem *> list = scene->items(input->pos(), Qt::IntersectsItemShape, Qt::DescendingOrder, QTransform());
    if (list.isEmpty()) {
        if (manager) {
            manager->parentItem()->setSelected(false);
            manager->clear();
            // manager = nullptr;
            scene->drawCurrentPhotogram();
        }
        return;
    }

    QList<QGraphicsItem *> currentSelection;
    frame = getCurrentFrame();
    if (frame->indexOf(scene->mouseGrabberItem()) != -1) {
        currentSelection << scene->mouseGrabberItem();
    } else {
        if (scene->selectedItems().count() > 0)
            currentSelection = scene->selectedItems();
    }

    qDebug() << "COUNT - scene->selectedItems().count() -> " << scene->selectedItems().count();
    qDebug() << "COUNT - currentSelection.count() -> " << currentSelection.count();

    if (!currentSelection.isEmpty()) {
        QGraphicsItem *item = currentSelection.at(0);
        if (qgraphicsitem_cast<TupTextItem *>(item)) {
            qDebug() << "ADDING MANAGER!";
            manager = nullptr;
            manager = new NodeManager(item, scene, nodeZValue);
            manager->show();
            manager->resizeNodes(realFactor);
            activeSelection = true;
        } else {
            qDebug() << "SELECTION IS NOT A TEXT!";
        }
    } else {
        qDebug() << "SELECTION IS EMPTY!";
    }
}

void TextTool::move(const TupInputDeviceInformation *input, TupBrushManager *brushManager, TupGraphicsScene *scene)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TextTool::move()]";
    #endif

    Q_UNUSED(input)
    Q_UNUSED(scene)
    Q_UNUSED(brushManager)

    // if (input->buttons() == Qt::LeftButton && scene->selectedItems().count() > 0)
    // if (input->buttons() == Qt::LeftButton && manager)
    if (input->buttons() == Qt::LeftButton && activeSelection)
        QTimer::singleShot(0, this, SLOT(syncNodes()));
}

void TextTool::release(const TupInputDeviceInformation *input, TupBrushManager *brushManager, TupGraphicsScene *scene)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TextTool::release()]";
    #endif

    Q_UNUSED(input)
    Q_UNUSED(brushManager)
    Q_UNUSED(scene)

    if (manager) {
        activeSelection = true;
        if (manager->isModified())
            requestTransformation(manager->parentItem(), frame);
    }
}

void TextTool::doubleClick(const TupInputDeviceInformation *input, TupGraphicsScene *scene)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TextTool::doubleClick()]";
    #endif

    Q_UNUSED(input)
    Q_UNUSED(scene)

    /*
    QList<QGraphicsItem *> list = scene->items(input->pos(), Qt::IntersectsItemShape, Qt::DescendingOrder, QTransform());
    if (!list.isEmpty()) {
        TupTextItem *item = qgraphicsitem_cast<TupTextItem *>(list.at(0));
        item->setEditable(true);
    }
    */
}

TupFrame* TextTool::frameAt(int sceneIndex, int layerIndex, int frameIndex)
{
    TupFrame *frame = nullptr;
    TupProject *project = scene->currentScene()->project();
    TupScene *sceneData = project->sceneAt(sceneIndex);
    if (sceneData) {
        if (scene->getSpaceContext() == TupProject::FRAMES_MODE) {
            TupLayer *layer = sceneData->layerAt(layerIndex);
            if (layer) {
                frame = layer->frameAt(frameIndex);
            } else {
                #ifdef TUP_DEBUG
                    qDebug() << "[TextTool::frameAt()] - Fatal Error: Layer is NULL! -> " << layerIndex;
                #endif
            }
        } else {
            TupBackground *bg = sceneData->sceneBackground();
            if (scene->getSpaceContext() == TupProject::VECTOR_STATIC_BG_MODE) {
                frame = bg->vectorStaticFrame();
            } else if (scene->getSpaceContext() == TupProject::VECTOR_FG_MODE) {
                frame = bg->vectorForegroundFrame();
            } else if (scene->getSpaceContext() == TupProject::VECTOR_DYNAMIC_BG_MODE) {
                frame = bg->vectorDynamicFrame();
                bg->scheduleVectorRender(true);
            }
       }
    } else {
       #ifdef TUP_DEBUG
           qDebug() << "[TextTool::frameAt()] - Fatal Error: Scene is NULL! -> " << sceneIndex;
       #endif
    }

    return frame;
}

void TextTool::itemResponse(const TupItemResponse *response)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TextTool::itemResponse()] - action -> " << response->getAction();
        qDebug() << "[TextTool::itemResponse()] - item index -> " << response->getItemIndex();
    #endif

    QGraphicsItem *item = nullptr;
    TupFrame *frame = frameAt(response->getSceneIndex(), response->getLayerIndex(), response->getFrameIndex());
    if (frame) {
        if (response->getItemType() == TupLibraryObject::Svg && frame->svgItemsCount() > 0) {
            item = frame->svgAt(response->getItemIndex());
        } else if (frame->graphicsCount() > 0) {
            item = frame->item(response->getItemIndex());
        }
    } else {
        #ifdef TUP_DEBUG
            qDebug() << "[TextTool::itemResponse()] - Fatal Error: frame is NULL at index -> " << response->getFrameIndex();
        #endif
        return;
    }

    switch (response->getAction()) {
        case TupProjectRequest::Transform:
        {
            #ifdef TUP_DEBUG
                qDebug() << "[TextTool::itemResponse()] - TupProjectRequest::Transform";
            #endif

            if (item) {
                if (manager) {
                    manager->show();
                    manager->syncNodesFromParent();
                    manager->beginToEdit();
                }
            } else {
                #ifdef TUP_DEBUG
                    qDebug() << "[TextTool::itemResponse()] - No item found";
                #endif
            }
        }
        break;
        case TupProjectRequest::Move:
        {
            #ifdef TUP_DEBUG
                qDebug() << "[TextTool::itemResponse()] - TupProjectRequest::Move";
            #endif
            syncNodes();
        }
        break;
        default:
        {
            #ifdef TUP_DEBUG
                qDebug() << "[TextTool::itemResponse()] - Switch Default Entry";
            #endif
            syncNodes();
        }
        break;
    }
}

void TextTool::layerResponse(const TupLayerResponse *response)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TextTool::layerResponse()] - action -> " << response->getAction();
    #endif

    switch (response->getAction()) {
        case TupProjectRequest::Move:
        {
            init(scene);
        }
    }
}

void TextTool::sceneResponse(const TupSceneResponse *event)
{
    Q_UNUSED(event)

    // if (event->getAction() == TupProjectRequest::Select)
    //     initItems(scene);
}

QMap<TAction::ActionId, TAction *> TextTool::actions() const
{
    return textActions;
}

int TextTool::toolType() const
{
    return TupToolInterface::Selection;
}

QWidget *TextTool::configurator()
{
    return config;
}

void TextTool::aboutToChangeTool()
{
    init(scene);
}

void TextTool::aboutToChangeScene(TupGraphicsScene *scene)
{
    Q_UNUSED(scene)

    clearSelection();
}

void TextTool::setupActions()
{
    realFactor = 1;
    activeSelection = false;
    manager = nullptr;

    TAction *text = new TAction(QIcon(kAppProp->themeDir() + "icons/text.png"), tr("Text"), this);
    text->setShortcut(QKeySequence(tr("T")));
    text->setCursor(QCursor(Qt::ArrowCursor));
    text->setActionId(TAction::Text);

    textActions.insert(TAction::Text, text);
}

void TextTool::saveConfig()
{
}

void TextTool::insertText()
{
    QString text = config->text();
    if (!text.isEmpty()) {
        TupTextItem *textItem = new TupTextItem;
        textItem->setDefaultTextColor(Qt::black);
        textItem->setPlainText(config->text());
        textItem->setFont(config->textFont());

        int textW = static_cast<int>(textItem->boundingRect().width());
        int textH = static_cast<int>(textItem->boundingRect().height());

        QSize dimension = scene->getSceneDimension();
        qreal xPos = 0;
        qreal yPos = 0;
        if (dimension.width() > textW)
            xPos = (dimension.width() - textW) / 2;
        if (dimension.height() > textH)
            yPos = (dimension.height() - textH) / 2;

        QPointF pos = QPointF(xPos, yPos);
        qDebug() << "POS -> " << pos;
        qDebug() << "dimension -> " << dimension;
        qDebug() << "Text W -> " << textW;
        qDebug() << "Text H -> " << textH;

        textItem->setPos(pos);
        scene->includeObject(textItem);

        QDomDocument doc;
        doc.appendChild(textItem->toXml(doc));

        TupProjectRequest request = TupRequestBuilder::createItemRequest(scene->currentSceneIndex(), scene->currentLayerIndex(), scene->currentFrameIndex(),
                                                                         0, pos, scene->getSpaceContext(), TupLibraryObject::Item, TupProjectRequest::Add,
                                                                         doc.toString());
        emit requested(&request);
    } else {
        TOsd::self()->display(TOsd::Warning, tr("Sorry, text can't be empty!"));
    }
}

void TextTool::resizeNode(qreal scaleFactor)
{
    realFactor = scaleFactor;
    if (manager)
        manager->resizeNodes(scaleFactor);
}

void TextTool::updateZoomFactor(qreal scaleFactor)
{
    realFactor = scaleFactor;
}

void TextTool::syncNodes()
{
    #ifdef TUP_DEBUG
        qDebug() << "[TextTool::syncNodes()]";
    #endif

    if (manager) {
        manager->show();
        QGraphicsItem *item = manager->parentItem();
        if (item) {
            manager->syncNodesFromParent();
            if (!item->isSelected())
                item->setSelected(true);
        } else {
            qDebug() << "NO ITEM!";
        }
    } else {
        #ifdef TUP_DEBUG
            qDebug() << "[TextTool::syncNodes()] - Warning: No node managers!";
        #endif
    }
}

void TextTool::requestTransformation(QGraphicsItem *item, TupFrame *frame)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TextTool::requestTransformation(QGraphicsItem *, TupFrame *)]";
    #endif

    QDomDocument doc;
    doc.appendChild(TupSerializer::properties(item, doc));

    TupTextItem *textItem = qgraphicsitem_cast<TupTextItem *>(item);
    int position = -1;
    TupLibraryObject::Type type;
    if (textItem) {
        type = TupLibraryObject::Item;
        position = frame->indexOf(item);
    }

    if (position >= 0) {
        TupProjectRequest event = TupRequestBuilder::createItemRequest(
                          scene->currentSceneIndex(), currentLayer, currentFrame,
                          position, QPointF(), scene->getSpaceContext(), type,
                          TupProjectRequest::Transform, doc.toString());

        emit requested(&event);
    } else {
        #ifdef TUP_DEBUG
            qDebug() << "TextTool::requestTransformation() - Fatal Error: Invalid item position !!! [ "
                        + QString::number(position) + " ]";
        #endif
    }
}

TupFrame* TextTool::getCurrentFrame()
{
    TupFrame *frame = nullptr;
    if (scene->getSpaceContext() == TupProject::FRAMES_MODE) {
        frame = scene->currentFrame();
        currentLayer = scene->currentLayerIndex();
        currentFrame = scene->currentFrameIndex();
    } else {
        currentLayer = -1;
        currentFrame = -1;

        TupScene *tupScene = scene->currentScene();
        TupBackground *bg = tupScene->sceneBackground();
        if (scene->getSpaceContext() == TupProject::VECTOR_STATIC_BG_MODE) {
            frame = bg->vectorStaticFrame();
        } else if (scene->getSpaceContext() == TupProject::VECTOR_FG_MODE) {
            frame = bg->vectorForegroundFrame();
        } else if (scene->getSpaceContext() == TupProject::VECTOR_DYNAMIC_BG_MODE) {
            frame = bg->vectorDynamicFrame();
        }
    }

    return frame;
}

void TextTool::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_F11 || event->key() == Qt::Key_Escape) {
        emit closeHugeCanvas();
    } else {
        if (!activeSelection) {
            QPair<int, int> flags = TupToolPlugin::setKeyAction(event->key(), event->modifiers());
            if (flags.first != -1 && flags.second != -1)
                emit callForPlugin(flags.first, flags.second);
        } else {
            int delta = 5;

            if (event->modifiers() == Qt::ShiftModifier)
                delta = 1;

            if (event->modifiers() == Qt::ControlModifier)
                delta = 10;

            TupFrame *frame = getCurrentFrame();
            QGraphicsItem *item = manager->parentItem();

            if (event->key() == Qt::Key_Left)
                item->moveBy(-delta, 0);

            if (event->key() == Qt::Key_Up)
                item->moveBy(0, -delta);

            if (event->key() == Qt::Key_Right)
                item->moveBy(delta, 0);

            if (event->key() == Qt::Key_Down)
                item->moveBy(0, delta);

            QTimer::singleShot(0, this, SLOT(syncNodes()));
            requestTransformation(item, frame);
        }
    }
}

void TextTool::clearSelection()
{
    #ifdef TUP_DEBUG
        qDebug() << "[TextTool::clearSelection()]";
    #endif

    if (activeSelection) {
        if (manager) {
            manager->parentItem()->setSelected(false);
            manager->clear();
            // manager = nullptr;
        }

        activeSelection = false;
        scene->drawCurrentPhotogram();
    }
}

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
#include "tconfig.h"

#include <QDomDocument>

TextTool::TextTool()
{
    #ifdef TUP_DEBUG
        qDebug() << "[TextTool::TextTool()]";
    #endif

    configPanel = new TextConfigurator;
    configPanel->setTextColor(Qt::black);

    connect(configPanel, SIGNAL(textAdded()), this, SLOT(insertText()));
    connect(configPanel, SIGNAL(textUpdated()), this, SLOT(updateText()));

    connect(configPanel, SIGNAL(xPosChanged(int)), this, SLOT(updateXPositionInScene(int)));
    connect(configPanel, SIGNAL(yPosChanged(int)), this, SLOT(updateYPositionInScene(int)));
    connect(configPanel, SIGNAL(rotationChanged(int)), this, SLOT(updateRotationInScene(int)));
    connect(configPanel, SIGNAL(scaleChanged(double,double)), this, SLOT(updateScaleInScene(double,double)));
    connect(configPanel, SIGNAL(resetActionCalled()), this, SLOT(resetTextTransformations()));

    connect(configPanel, SIGNAL(scaleUpdated(double,double)), this, SLOT(setItemScale(double,double)));
    connect(configPanel, SIGNAL(activateProportion(bool)), this, SLOT(enableProportion(bool)));
    connect(configPanel, SIGNAL(textObjectReleased()), this, SLOT(removeManager()));

    /* These connections doesn't work on Windows
    connect(configPanel, &TextConfigurator::textAdded, this, &TextTool::insertText);
    connect(configPanel, &TextConfigurator::textUpdated, this, &TextTool::updateText);

    connect(configPanel, &TextConfigurator::xPosChanged, this, &TextTool::updateXPositionInScene);
    connect(configPanel, &TextConfigurator::yPosChanged, this, &TextTool::updateYPositionInScene);
    connect(configPanel, &TextConfigurator::rotationChanged, this, &TextTool::updateRotationInScene);
    connect(configPanel, &TextConfigurator::scaleChanged, this, &TextTool::updateScaleInScene);
    connect(configPanel, &TextConfigurator::resetActionCalled, this, &TextTool::resetTextTransformations);

    connect(configPanel, &TextConfigurator::scaleUpdated, this, &TextTool::setItemScale);
    connect(configPanel, &TextConfigurator::activateProportion, this, &TextTool::enableProportion);
    connect(configPanel, &TextConfigurator::textObjectReleased, this, &TextTool::removeManager);
    */

    setupActions();
}

TextTool::~TextTool()
{
}

void TextTool::init(TupGraphicsScene *gScene)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TextTool::init()]";
    #endif

    loadTextColor();
    configPanel->setTextColor(currentColor);
    configPanel->updateMode(TextConfigurator::Add);

    scene = gScene;
    clearSelection();
    scene->clearSelection();
    nodesManager = nullptr;

    nodeZValue = ((BG_LAYERS + 1) * ZLAYER_LIMIT) + (scene->currentScene()->layersCount() * ZLAYER_LIMIT);
    if (scene->getSpaceContext() == TupProject::VECTOR_FG_MODE)
        nodeZValue += ZLAYER_LIMIT;

    initItems(scene);
}

void TextTool::initItems(TupGraphicsScene *gScene)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TextTool::initItems()]";
    #endif

    foreach (QGraphicsView *view, gScene->views())
        view->setDragMode(QGraphicsView::RubberBandDrag);
}

void TextTool::loadTextColor()
{
    TCONFIG->beginGroup("ColorPalette");
    currentColor = QColor(TCONFIG->value("TextColor").toString());
}

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

    foreach (QGraphicsView *view, scene->views())
        view->setDragMode(QGraphicsView::RubberBandDrag);

    QList<QGraphicsItem *> currentSelection;
    frame = getCurrentFrame();
    // User clicked the item directly
    if (frame->indexOf(scene->mouseGrabberItem()) != -1) {
        currentSelection << scene->mouseGrabberItem();
    } else {
        if (scene->selectedItems().count() > 0)
            currentSelection = scene->selectedItems();
    }

    if (!currentSelection.isEmpty()) {
        for(int i=1; i<currentSelection.size(); i++)
            currentSelection.at(i)->setSelected(false);

        QGraphicsItem *item = currentSelection.at(0);
        if (TupTextItem *textItem = qgraphicsitem_cast<TupTextItem *>(item)) {
            if (!nodesManager) {
                item->setFlag(QGraphicsItem::ItemIsMovable, false); // To avoid undesired small movements
                if (!item->isSelected())
                    item->setSelected(true);
                nodesManager = new NodeManager(Node::Text, item, scene, nodeZValue);
                connect(nodesManager, SIGNAL(positionUpdated(const QPointF&)), this, SLOT(updatePositionRecord(const QPointF&)));
                connect(nodesManager, SIGNAL(rotationUpdated(int)), this, SLOT(updateRotationAngleRecord(int)));
                connect(nodesManager, SIGNAL(scaleUpdated(double,double)), this, SLOT(updateScaleFactorRecord(double,double)));

                /* SQA: These connections don't work on Windows
                connect(nodesManager, &NodeManager::positionUpdated, this, &TextTool::updatePositionRecord);
                connect(nodesManager, &NodeManager::rotationUpdated, this, &TextTool::updateRotationAngleRecord);
                connect(nodesManager, &NodeManager::scaleUpdated, this, &TextTool::updateScaleFactorRecord);
                */
            } else {
                QGraphicsItem *parent = nodesManager->parentItem();
                if (parent) {
                    if (item != parent) {
                        if (!item->isSelected())
                            item->setSelected(true);

                        parent->setSelected(false);
                        nodesManager->clear();
                        nodesManager = nullptr;
                        nodesManager = new NodeManager(Node::Text, item, scene, nodeZValue);
                        connect(nodesManager, SIGNAL(positionUpdated(const QPointF&)), this, SLOT(updatePositionRecord(const QPointF&)));
                        connect(nodesManager, SIGNAL(rotationUpdated(int)), this, SLOT(updateRotationAngleRecord(int)));
                        connect(nodesManager, SIGNAL(scaleUpdated(double,double)), this, SLOT(updateScaleFactorRecord(double,double)));

                        /* SQA: These connections don't work on Windows
                        connect(nodesManager, &NodeManager::positionUpdated, this, &TextTool::updatePositionRecord);
                        connect(nodesManager, &NodeManager::rotationUpdated, this, &TextTool::updateRotationAngleRecord);
                        connect(nodesManager, &NodeManager::scaleUpdated, this, &TextTool::updateScaleFactorRecord);
                        */
                    }
                }
            }            
            nodesManager->show();
            nodesManager->resizeNodes(realFactor);

            QString text = textItem->data(0).toString();
            if (text.isEmpty()) {
                text = textItem->toPlainText();
                textItem->setData(0, text);
            }

            activeSelection = true;
            configPanel->loadTextSettings(textItem->font(), text, textItem->defaultTextColor());

            QPointF pos =  textItem->pos();
            pos += QPointF(textItem->boundingRect().size().width()/2, textItem->boundingRect().size().height()/2);

            QDomDocument doc;
            configPanel->displayControls(true, pos, TupSerializer::properties(textItem, doc, textItem->toPlainText(), textItem->textWidth()));
        } else {
            #ifdef TUP_DEBUG
                qDebug() << "[TextTool::press()] - Warning: Object is not a text item!";
            #endif
        }
    } else {
        #ifdef TUP_DEBUG
            qDebug() << "[TextTool::press()] - Warning: Selection is empty!";
        #endif
        // User clicked on background
        QList<QGraphicsItem *> list = scene->items(input->pos(), Qt::IntersectsItemShape, Qt::DescendingOrder, QTransform());
        if (list.isEmpty()) {
            if (nodesManager) {
                nodesManager->parentItem()->setSelected(false);
                nodesManager->clear();
                nodesManager = nullptr;

                scene->drawCurrentPhotogram();
            }
            configPanel->updateMode(TextConfigurator::Add);
            configPanel->displayControls(false);

            #ifdef TUP_DEBUG
                qDebug() << "[TextTool::press()] - User clicked on background... exiting!";
            #endif
        }
    }
}

void TextTool::move(const TupInputDeviceInformation *input, TupBrushManager *brushManager, TupGraphicsScene *scene)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TextTool::move()]";
    #endif

    Q_UNUSED(scene)
    Q_UNUSED(brushManager)

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

    if (nodesManager) {
        activeSelection = true;
        if (nodesManager->isModified())
            requestTransformation(nodesManager->parentItem(), frame);
    } else {
        QList<QGraphicsItem *> currentSelection = scene->selectedItems();
        if (currentSelection.count() > 0) {
            for(int i=1; i<currentSelection.size(); i++)
                currentSelection.at(i)->setSelected(false);

            QGraphicsItem *item = currentSelection.at(0);
            if (TupTextItem *textItem = qgraphicsitem_cast<TupTextItem *>(item)) {
                if (!textItem->isSelected())
                    textItem->setSelected(true);

                nodesManager = new NodeManager(Node::Text, textItem, scene, nodeZValue);
                connect(nodesManager, SIGNAL(positionUpdated(const QPointF &)), this, SLOT(updatePositionRecord(const QPointF&)));
                connect(nodesManager, SIGNAL(rotationUpdated(int)), this, SLOT(updateRotationAngleRecord(int)));
                connect(nodesManager, SIGNAL(scaleUpdated(double,double)), this, SLOT(updateScaleFactorRecord(double,double)));

                /* SQA: These connections don't work on Windows
                connect(nodesManager, &NodeManager::positionUpdated, this, &TextTool::updatePositionRecord);
                connect(nodesManager, &NodeManager::rotationUpdated, this, &TextTool::updateRotationAngleRecord);
                connect(nodesManager, &NodeManager::scaleUpdated, this, &TextTool::updateScaleFactorRecord);
                */
                nodesManager->show();
                nodesManager->resizeNodes(realFactor);

                activeSelection = true;
                configPanel->loadTextSettings(textItem->font(), textItem->data(0).toString(), textItem->defaultTextColor());

                QPointF point = textItem->pos();
                point += QPointF(textItem->boundingRect().size().width()/2, textItem->boundingRect().size().height()/2);

                QDomDocument doc;
                configPanel->displayControls(true, point,
                                        TupSerializer::properties(textItem, doc, textItem->toPlainText(), textItem->textWidth()));
            } else {
                item->setSelected(false);
                configPanel->displayControls(false);
            }
        }
    }
}

/*
void TextTool::doubleClick(const TupInputDeviceInformation *input, TupGraphicsScene *scene)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TextTool::doubleClick()]";
    #endif

    Q_UNUSED(input)
    Q_UNUSED(scene)

    QList<QGraphicsItem *> list = scene->items(input->pos(), Qt::IntersectsItemShape, Qt::DescendingOrder, QTransform());
    if (!list.isEmpty()) {
        TupTextItem *item = qgraphicsitem_cast<TupTextItem *>(list.at(0));
        if (item) {
            if (manager) {
                if (manager->parentItem() == item) {
                    manager->clear();
                }
            }
            item->setEditable(true);
        }
    }
}
*/

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

    if (response->getAction() == TupProjectRequest::Remove) {
        removeManager();
        configPanel->resetText();

        return;
    }

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
                if (nodesManager) {
                    nodesManager->show();
                    nodesManager->syncNodesFromParent();
                    nodesManager->beginToEdit();
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

void TextTool::frameResponse(const TupFrameResponse *response)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TextTool::frameResponse()] - action -> " << response->getAction();
    #else
        Q_UNUSED(response)
    #endif   

    activeSelection = false;
    configPanel->updateMode(TextConfigurator::Add);
}

void TextTool::libraryResponse(const TupLibraryResponse *response)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TextTool::libraryResponse()] - action -> " << response->getAction();
    #else
        Q_UNUSED(response)
    #endif

    removeManager();
    configPanel->updateMode(TextConfigurator::Add);
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

void TextTool::sceneResponse(const TupSceneResponse *response)
{
    if (response->getAction() == TupProjectRequest::Select)
        initItems(scene);
}

void TextTool::removeManager()
{
    if (nodesManager) {
        disconnect(nodesManager, SIGNAL(positionUpdated(const QPointF&)), this, SLOT(updatePositionRecord(const QPointF&)));
        disconnect(nodesManager, SIGNAL(rotationUpdated(int)), this, SLOT(updateRotationAngleRecord(int)));
        disconnect(nodesManager, SIGNAL(scaleUpdated(double,double)), this, SLOT(updateScaleFactorRecord(double,double)));

        /* SQA: These connections don't work on Windows
        disconnect(nodesManager, &NodeManager::positionUpdated, this, &TextTool::updatePositionRecord);
        disconnect(nodesManager, &NodeManager::rotationUpdated, this, &TextTool::updateRotationAngleRecord);
        disconnect(nodesManager, &NodeManager::scaleUpdated, this, &TextTool::updateScaleFactorRecord);
        */

        if (nodesManager->parentItem())
            nodesManager->parentItem()->setSelected(false);
        nodesManager->clear();
        nodesManager = nullptr;
    }

    activeSelection = false;
}

QMap<TAction::ActionId, TAction *> TextTool::actions() const
{
    return textActions;
}

TAction * TextTool::getAction(TAction::ActionId toolId)
{
    return textActions[toolId];
}

int TextTool::toolType() const
{
    return TupToolInterface::Selection;
}

QWidget *TextTool::configurator()
{
    return configPanel;
}

void TextTool::aboutToChangeTool()
{
    #ifdef TUP_DEBUG
        qDebug() << "[TextTool::aboutToChangeTool()]";
    #endif

    init(scene);
    configPanel->clearText();

    QFont font = configPanel->textFont();
    TCONFIG->beginGroup("TextTool");
    TCONFIG->setValue("FontFamily", font.family());
    TCONFIG->setValue("FontSize", font.pointSize());
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
    nodesManager = nullptr;

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
    #ifdef TUP_DEBUG
        qDebug() << "[TextTool::insertText()]";
    #endif

    QString text = configPanel->text();
    if (!text.isEmpty()) {
        TupTextItem *textItem = new TupTextItem;
        QTextOption option = textItem->document()->defaultTextOption();
        option.setAlignment(configPanel->textAlignment());
        textItem->document()->setDefaultTextOption(option);

        loadTextColor();
        textItem->setDefaultTextColor(currentColor);

        QFont font = configPanel->textFont();
        textItem->setFont(font);

        textItem->setPlainText(text);
        textItem->setData(0, text);

        int textW = static_cast<int>(textItem->boundingRect().width());

        QFontMetrics fm(font);
        QStringList list = text.split("\n");
        int longerLine = 0;
        foreach (QString sentence, list) {
            int width = fm.horizontalAdvance(sentence);
            if (width > longerLine)
                longerLine = width;
        }

        textItem->setTextWidth(longerLine + 9);
        int textH = static_cast<int>(textItem->boundingRect().height());

        QSize dimension = scene->getSceneDimension();
        qreal xPos = 0;
        qreal yPos = 0;
        if (dimension.width() > textW)
            xPos = (dimension.width() - textW) / 2;
        if (dimension.height() > textH)
            yPos = (dimension.height() - textH) / 2;

        QPointF pos = QPointF(xPos, yPos);
        textItem->setPos(pos);
        textItem->setData(TupGraphicObject::Rotate, 0);
        textItem->setData(TupGraphicObject::ScaleX, 1);
        textItem->setData(TupGraphicObject::ScaleY, 1);

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

void TextTool::updateText()
{
    #ifdef TUP_DEBUG
        qDebug() << "[TextTool::updateText()]";
    #endif

    if (nodesManager) {
        QGraphicsItem *item = nodesManager->parentItem();
        if (TupTextItem *textItem = qgraphicsitem_cast<TupTextItem *>(item)) {
            QString text = configPanel->text();
            if (text.isEmpty()) { // Remove item
                int itemIndex = -1;
                int frameIndex = -1;
                int layerIndex = -1;

                TupProject::Mode spaceMode = scene->getSpaceContext();
                if (spaceMode == TupProject::FRAMES_MODE) {
                    frameIndex = scene->currentFrameIndex();
                    layerIndex = scene->currentLayerIndex();
                    itemIndex = scene->currentFrame()->indexOf(item);
                } else {
                    TupBackground *bg = scene->currentScene()->sceneBackground();
                    if (bg) {
                        TupFrame *frame;
                        if (spaceMode == TupProject::VECTOR_STATIC_BG_MODE)
                            frame = bg->vectorStaticFrame();
                        else if (spaceMode == TupProject::VECTOR_FG_MODE)
                            frame = bg->vectorForegroundFrame();
                        else
                            frame = bg->vectorDynamicFrame();

                        if (frame)
                            itemIndex = frame->indexOf(item);
                    } else {
                        #ifdef TUP_DEBUG
                            qDebug() << "[TextTool::updateText()] - Fatal Error: Background frame is NULL!";
                        #endif
                    }
                }

                TupProjectRequest event = TupRequestBuilder::createItemRequest(
                                          scene->currentSceneIndex(), layerIndex, frameIndex,
                                          itemIndex, QPointF(), scene->getSpaceContext(), TupLibraryObject::Item,
                                          TupProjectRequest::Remove);
                emit requested(&event);
            } else {
                QTextOption option = textItem->document()->defaultTextOption();
                option.setAlignment(configPanel->textAlignment());
                textItem->document()->setDefaultTextOption(option);

                QFont font = configPanel->textFont();
                textItem->setFont(font);
                textItem->setPlainText(text);
                textItem->setData(0, text);
                textItem->setDefaultTextColor(configPanel->getTextColor());

                QFontMetrics fm(font);
                QStringList list = text.split("\n");
                int longerLine = 0;
                foreach (QString sentence, list) {
                    int width = fm.horizontalAdvance(sentence);
                    if (width > longerLine)
                        longerLine = width;
                }

                textItem->setTextWidth(longerLine + 9);
                nodesManager->syncNodesFromParent();
            }
        }
    } else {
        #ifdef TUP_DEBUG
            qDebug() << "[TextTool::updateText()] - Warning: No item selected!";
        #endif
    }
}

void TextTool::resizeNode(qreal scaleFactor)
{
    realFactor = scaleFactor;
    if (nodesManager)
        nodesManager->resizeNodes(scaleFactor);
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

    if (nodesManager) {
        nodesManager->show();
        QGraphicsItem *item = nodesManager->parentItem();
        if (item) {
            nodesManager->syncNodesFromParent();
            if (!item->isSelected())
                item->setSelected(true);
        } else {
            #ifdef TUP_DEBUG
                qDebug() << "[TextTool::syncNodes()] - Fatal Error: Item is NULL!";
            #endif
        }
    } else {
        #ifdef TUP_DEBUG
            qDebug() << "[TextTool::syncNodes()] - Fatal Error: Node manager is NULL!";
        #endif
    }
}

void TextTool::requestTransformation(QGraphicsItem *item, TupFrame *frame)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TextTool::requestTransformation(QGraphicsItem *, TupFrame *)]";
    #endif

    TupTextItem *textItem = qgraphicsitem_cast<TupTextItem *>(item);
    QDomDocument doc;
    doc.appendChild(TupSerializer::properties(item, doc, textItem->toPlainText(), textItem->textWidth()));

    int position = -1;
    TupLibraryObject::ObjectType type;
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
                        << position << " ]";
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
    #ifdef TUP_DEBUG
        qDebug() << "[TextTool::keyPressEvent()] - key -> " << event->key();
    #endif

    key = "NONE";

    if (event->key() == Qt::Key_F11 || event->key() == Qt::Key_Escape) {
        emit closeHugeCanvas();
    } else if ((event->key() == Qt::Key_Left) || (event->key() == Qt::Key_Up)
              || (event->key() == Qt::Key_Right) || (event->key() == Qt::Key_Down)) {
        if (!activeSelection) {
            QPair<int, int> flags = TAction::setKeyAction(event->key(), event->modifiers());
            if (flags.first != -1 && flags.second != -1)
                emit callForPlugin(flags.first, flags.second);
        } else {
            int delta = 5;

            if (event->modifiers() == Qt::ShiftModifier)
                delta = 1;

            if (event->modifiers() == Qt::ControlModifier)
                delta = 10;

            TupFrame *frame = getCurrentFrame();
            QGraphicsItem *item = nodesManager->parentItem();

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

            updatePositionRecord(item->pos() + QPointF(item->boundingRect().size().width()/2, item->boundingRect().size().height()/2));
        }
    } else if (event->modifiers() == Qt::ControlModifier) {
        configPanel->setProportionState(true);
        key = "CONTROL";
        if (activeSelection)
            nodesManager->setProportion(true);
    }
}

void TextTool::keyReleaseEvent(QKeyEvent *event)
{
    Q_UNUSED(event)

    if (key.compare("CONTROL") == 0) {
        configPanel->setProportionState(false);
        key = "NONE";
        if (activeSelection)
            nodesManager->setProportion(false);
    }
}

void TextTool::clearSelection()
{
    #ifdef TUP_DEBUG
        qDebug() << "[TextTool::clearSelection()]";
    #endif

    if (activeSelection) {
        if (nodesManager) {
            nodesManager->parentItem()->setSelected(false);
            nodesManager->clear();
        }

        activeSelection = false;
        scene->drawCurrentPhotogram();
    }
}

void TextTool::updateTextColor(const QColor &color)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TextTool::updateTextColor()] - color -> " << color;
    #endif

    configPanel->setTextColor(color);
}

void TextTool::updatePositionRecord(const QPointF &point)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TextTool::updatePositionRecord()] - point -> " << point;
    #endif

    configPanel->updatePositionCoords(point.x(), point.y());
    if (nodesManager)
        requestTransformation(nodesManager->parentItem(), frame);
}

void TextTool::updateRotationAngleRecord(int angle)
{
    configPanel->updateRotationAngle(angle);
    if (nodesManager)
        requestTransformation(nodesManager->parentItem(), frame);
}

void TextTool::updateScaleFactorRecord(double x, double y)
{
    configPanel->updateScaleFactor(x, y);
    if (nodesManager)
        requestTransformation(nodesManager->parentItem(), frame);
}

void TextTool::updateXPositionInScene(int x)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TextTool::updateXPositionInScene()] - x -> " << x;
    #endif

    if (nodesManager) {
        QGraphicsItem *item = nodesManager->parentItem();
        item->setPos(x, item->pos().y());
        nodesManager->syncNodesFromParent();
        requestTransformation(nodesManager->parentItem(), frame);
    }
}

// Actions executed from the panel

void TextTool::updateYPositionInScene(int y)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TextTool::updateYPositionInScene()] - y -> " << y;
    #endif

    if (nodesManager) {
        QGraphicsItem *item = nodesManager->parentItem();
        item->setPos(item->pos().x(), y);
        nodesManager->syncNodesFromParent();
        requestTransformation(nodesManager->parentItem(), frame);
    }
}

void TextTool::updateRotationInScene(int angle)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TextTool::updateRotationInScene()] - angle -> " << angle;
    #endif

    if (nodesManager) {
        nodesManager->rotate(angle);
        nodesManager->syncNodesFromParent();
        requestTransformation(nodesManager->parentItem(), frame);
    }
}

void TextTool::updateScaleInScene(double xFactor, double yFactor)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TextTool::updateScaleInScene()] - scale factor -> " << QPointF(xFactor, yFactor);
    #endif

    if (nodesManager) {
        nodesManager->scale(xFactor, yFactor);
        nodesManager->syncNodesFromParent();
        requestTransformation(nodesManager->parentItem(), frame);
    }
}

void TextTool::resetTextTransformations()
{
    #ifdef TUP_DEBUG
        qDebug() << "[TextTool::resetTextTransformations()]";
    #endif

    if (nodesManager) {
        QSizeF projectSize = scene->currentScene()->getDimension();
        int projectX = projectSize.width() / 2;
        int projectY = projectSize.height() / 2;

        QGraphicsItem *item = nodesManager->parentItem();
        if (item) {
            QSizeF mouthSize = item->boundingRect().size();
            int mouthX = mouthSize.width() / 2;
            int mouthY = mouthSize.height() / 2;

            updateXPositionInScene(projectX - mouthX);
            updateYPositionInScene(projectY - mouthY);
            updateRotationInScene(0);
            updateScaleInScene(1, 1);
        }
    }
}

void TextTool::setItemScale(double xFactor, double yFactor)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TextTool::setItemScale(double, double)]";
    #endif

    if (nodesManager) {
        nodesManager->scale(xFactor, yFactor);
        if (nodesManager->isModified())
            requestTransformation(nodesManager->parentItem(), frame);
    }
}

void TextTool::enableProportion(bool flag)
{
    key = "NONE";
    if (flag)
        key = "CONTROL";

    if (nodesManager)
        nodesManager->setProportion(flag);
}

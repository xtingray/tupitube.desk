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

TextTool::TextTool()
{
    config = new TextConfigurator;
    setupActions();
}

TextTool::~TextTool()
{
}

QStringList TextTool::keys() const
{
    return QStringList() << tr("Text");
}

void TextTool::press(const TupInputDeviceInformation *input, TupBrushManager *brushManager, TupGraphicsScene *scene)
{
    Q_UNUSED(brushManager);

    QList<QGraphicsItem *> items = scene->items(input->pos());

    if (items.count() > 0) {
        QGraphicsItem *itemPress = items[0];
        if  (itemPressed(itemPress))
             return;
    }

    textItem = new TupTextItem;
    textItem->setPos(input->pos());
    textItem->setDefaultTextColor(brushManager->penColor());
}

void TextTool::doubleClick(const TupInputDeviceInformation *input, TupGraphicsScene *scene)
{
    Q_UNUSED(input);
    Q_UNUSED(scene);
}

bool TextTool::itemPressed(QGraphicsItem *item)
{
    if (TupTextItem *text = qgraphicsitem_cast<TupTextItem *>(item)) {
        text->setEditable(true);
        text->setFocus();
        return true;
    }

    return false;
}

void TextTool::move(const TupInputDeviceInformation *input, TupBrushManager *brushManager, TupGraphicsScene *scene)
{
    Q_UNUSED(input);
    Q_UNUSED(scene);
    Q_UNUSED(scene);
    Q_UNUSED(brushManager);
}

void TextTool::release(const TupInputDeviceInformation *input, TupBrushManager *brushManager, TupGraphicsScene *scene)
{
    Q_UNUSED(input);
    Q_UNUSED(brushManager);

    if (config->text().isEmpty()) {
        delete textItem;
        return;
    }

    if (config->isHtml())
        textItem->setHtml(config->text());
    else
        textItem->setPlainText(config->text());

    textItem->setFont(config->textFont());

    scene->includeObject(textItem);

    QDomDocument doc;
    doc.appendChild(textItem->toXml(doc));

    TupProjectRequest request = TupRequestBuilder::createItemRequest(scene->currentSceneIndex(), scene->currentLayerIndex(), scene->currentFrameIndex(), 
                                                                     0, QPointF(), scene->spaceContext(), TupLibraryObject::Item, TupProjectRequest::Add, 
                                                                     doc.toString());
    emit requested(&request);
}

QMap<QString, TAction *> TextTool::actions() const
{
    return textActions;
}

int TextTool::toolType() const
{
    return Brush;
}

QWidget *TextTool::configurator()
{
    return config;
}

void TextTool::aboutToChangeTool()
{
}

void TextTool::aboutToChangeScene(TupGraphicsScene *scene)
{
    Q_UNUSED(scene);
}

void TextTool::setupActions()
{
    TAction *text = new TAction(QIcon(kAppProp->themeDir() + "icons/text.png"), tr("Text"), this);
    text->setShortcut(QKeySequence(tr("T")));
    text->setCursor(QCursor(kAppProp->themeDir() + "cursors/text.png"));

    textActions.insert(tr("Text"), text);
}

void TextTool::saveConfig()
{
}

void TextTool::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_F11 || event->key() == Qt::Key_Escape)
        emit closeHugeCanvas();
}

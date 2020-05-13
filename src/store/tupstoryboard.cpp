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

#include "tupstoryboard.h"

TupStoryboard::TupStoryboard(const QString &auth)
{
    title   = "";
    author  = auth;
    topics  = "";
    summary = "";
}

TupStoryboard::~TupStoryboard()
{
}

void TupStoryboard::init(int start, int size)
{
    for (int i=start; i < size; i++) {
         scene << "";
         duration << "";
         description << "";
    }
}

void TupStoryboard::reset()
{
    title   = "";
    author  = "";
    topics  = "";
    summary = "";

    scene.clear();
    duration.clear();
    description.clear();
}

void TupStoryboard::insertScene(int index)
{
    if (index >= 0 && index < scene.size()) {
        scene.insert(index, "");
        duration.insert(index, "");
        description.insert(index, "");
    }
}

void TupStoryboard::appendScene()
{
    scene.append("");
    duration.append("");
    description.append("");
}

void TupStoryboard::moveScene(int oldIndex, int newIndex)
{
    if (oldIndex >= 0 && oldIndex < scene.size() && newIndex >= 0 && newIndex < scene.size()) {
        scene.swapItemsAt(oldIndex, newIndex);
        duration.swapItemsAt(oldIndex, newIndex);
        description.swapItemsAt(oldIndex, newIndex);
    }
}

void TupStoryboard::resetScene(int index)
{
    if (index >= 0 && index < scene.size()) {
        scene.replace(index, "");
        duration.replace(index, "");
        description.replace(index, "");
    }
}

void TupStoryboard::removeScene(int index)
{
    if (index >= 0 && index < scene.size()) {
        scene.removeAt(index);
        duration.removeAt(index);
        description.removeAt(index);
    }
}

void TupStoryboard::setStoryTitle(const QString &headLine)
{
    title = headLine;
}

void TupStoryboard::setStoryTopics(const QString &tags)
{
    topics = tags;
}

void TupStoryboard::setStoryAuthor(const QString &auth)
{
    author = auth;
}

void TupStoryboard::setStorySummary(const QString &text)
{
    summary = text;
}

QString TupStoryboard::storyTitle() const
{
    return title;
}

QString TupStoryboard::storyAuthor() const
{
    return author;
}

QString TupStoryboard::storyTopics() const
{
    return topics;
}

QString TupStoryboard::storySummary() const
{
    return summary;
}

void TupStoryboard::setSceneTitle(int index, const QString &headLine)
{
    if (index >= 0 && index < scene.count()) {
        scene.replace(index, headLine);
    } else {
        #ifdef TUP_DEBUG
            qDebug() << "TupStoryboard::setSceneTitle() - Invalid index -> " + QString::number(index);
        #endif
    }
}

void TupStoryboard::setSceneDuration(int index, const QString &time)
{
    if (index >= 0 && index < duration.count()) {
        duration.replace(index, time);
    } else {
        #ifdef TUP_DEBUG
            qDebug() << "TupStoryboard::setSceneDuration() - Invalid index: " + QString::number(index);
        #endif
    }
}

void TupStoryboard::setSceneDescription(int index, const QString &desc)
{
    if (index >= 0 && index < description.count()) {
        description.replace(index, desc);
    } else {
        #ifdef TUP_DEBUG
            qDebug() << "TupStoryboard::setSceneDuration() - Invalid index: " + QString::number(index);
        #endif
    }
}

QString TupStoryboard::sceneTitle(int index) const
{
    if (index < scene.count())
        return scene.at(index);

    return "";
}

QString TupStoryboard::sceneDuration(int index) const
{
    if (index < duration.count())
        return duration.at(index);

    return "";
}

QString TupStoryboard::sceneDescription(int index) const
{
    if (index < description.count())
        return description.at(index);

    return "";
}

void TupStoryboard::fromXml(const QString &xml)
{
    QDomDocument document;
    if (!document.setContent(xml))
        return;

    QDomElement root = document.documentElement();
    QDomNode n = root.firstChild();

    while (!n.isNull()) {
           QDomElement e = n.toElement();
           if (!e.isNull()) {
               if (e.tagName() == "title") {
                   title = cleanString(e.text());
               } else if (e.tagName() == "author") {
                          author = cleanString(e.text());
               } else if (e.tagName() == "topics") {
                          topics = cleanString(e.text());
               } else if (e.tagName() == "summary") {
                          summary = cleanString(e.text());
               } else if (e.tagName() == "scene") {
                          QDomNode n2 = e.firstChild();
                          while (!n2.isNull()) {
                                 QDomElement e2 = n2.toElement();
                                 if (e2.tagName() == "title") {
                                     scene << e2.text();
                                 } else if (e2.tagName() == "duration") {
                                            duration << e2.text();
                                 } else if (e2.tagName() == "description") {
                                            description << e2.text();
                                 }
                                 n2 = n2.nextSibling();
                          }
               }
           }
           n = n.nextSibling();
    }
}

QDomElement TupStoryboard::toXml(QDomDocument &doc) const
{
    QDomElement storyboard = doc.createElement("storyboard");

    QDomText titleDom   = doc.createTextNode(title);
    QDomText authorDom  = doc.createTextNode(author);
    QDomText topicsDom  = doc.createTextNode(topics);
    QDomText summaryDom = doc.createTextNode(summary);

    storyboard.appendChild(doc.createElement("title")).appendChild(titleDom);
    storyboard.appendChild(doc.createElement("author")).appendChild(authorDom);
    storyboard.appendChild(doc.createElement("topics")).appendChild(topicsDom);
    storyboard.appendChild(doc.createElement("summary")).appendChild(summaryDom);

    for (int i=0; i<scene.size(); i++) {
         QDomElement sceneDom = doc.createElement("scene");
         QDomText titleDom = doc.createTextNode(scene.at(i));
         QDomText timeDom  = doc.createTextNode(duration.at(i));
         QDomText descDom  = doc.createTextNode(description.at(i));
         sceneDom.appendChild(doc.createElement("title")).appendChild(titleDom);
         sceneDom.appendChild(doc.createElement("duration")).appendChild(timeDom);
         sceneDom.appendChild(doc.createElement("description")).appendChild(descDom);
         storyboard.appendChild(sceneDom);
    }

    return storyboard;
}

int TupStoryboard::size()
{
    return scene.count();
}

QString TupStoryboard::cleanString(QString input) const
{
    input.replace(",", "\\,");
    input.replace("'", "\"");

    return input;
}

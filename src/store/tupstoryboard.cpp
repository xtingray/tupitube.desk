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

TupStoryboard::TupStoryboard()
{
    title   = "";
    author  = "";
    topics  = "";
    summary = "";

    coverDuration = "1.0";
}

TupStoryboard::~TupStoryboard()
{
}

void TupStoryboard::init(int start, int size)
{
    for (int i=start; i < size; i++)
         duration << "1.0";
}

void TupStoryboard::reset()
{
    title   = "";
    author  = "";
    topics  = "";
    summary = "";

    duration.clear();
}

void TupStoryboard::insertScene(int index)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupStoryboard::insertScene()] - index -> " << index;
    #endif

    if (index >= 0) {
        if (index < duration.size()) {
            duration.insert(index, "1.0");
        } else if (index == duration.size()) {
            appendScene();
        } else {
            #ifdef TUP_DEBUG
                qDebug() << "[TupStoryboard::insertScene()] - Error: invalid index -> " << index;
                qDebug() << "[TupStoryboard::insertScene()] - duration.size() -> " << duration.size();
            #endif
        }
    } else {
        #ifdef TUP_DEBUG
            qDebug() << "[TupStoryboard::insertScene()] - Error: invalid index -> " << index;
        #endif
    }
}

void TupStoryboard::appendScene()
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupStoryboard::appendScene()]";
    #endif

    duration.append("1.0");
}

void TupStoryboard::moveScene(int oldIndex, int newIndex)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupStoryboard::moveScene()] - oldIndex -> " << oldIndex;
        qDebug() << "[TupStoryboard::moveScene()] - newIndex -> " << newIndex;
    #endif

    if (oldIndex >= 0 && oldIndex < duration.size() && newIndex >= 0 && newIndex < duration.size())
        duration.swapItemsAt(oldIndex, newIndex);
}

void TupStoryboard::resetScene(int index)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupStoryboard::resetScene()] - index -> " << index;
    #endif

    if (index >= 0 && index < duration.size())
        duration.replace(index, "1.0");
}

void TupStoryboard::removeScene(int index)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupStoryboard::removeScene()] - index -> " << index;
    #endif

    if (index >= 0 && index < duration.size())
        duration.removeAt(index);
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

void TupStoryboard::setCoverDuration(const QString &duration)
{
    coverDuration = duration;
}

QString TupStoryboard::getCoverDuration() const
{
    return coverDuration;
}

void TupStoryboard::setSceneDuration(int index, const QString &time)
{
    if (index >= 0 && index < duration.count()) {
        duration.replace(index, time);
    } else {
        #ifdef TUP_DEBUG
            qDebug() << "[TupStoryboard::setSceneDuration()] - duration list size -> " << duration.count();
            qDebug() << "[TupStoryboard::setSceneDuration()] - Invalid index -> " << index;
        #endif
    }
}

QString TupStoryboard::sceneDuration(int index) const
{
    if (index < duration.count())
        return duration.at(index);

    return "1.0";
}

void TupStoryboard::fromXml(const QString &xml)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupStoryboard::fromXml()]";
    #endif

    QDomDocument document;
    if (!document.setContent(xml))
        return;

    QDomElement root = document.documentElement();
    coverDuration = root.attribute("cover_duration", "1.0");
    QDomNode n = root.firstChild();
    int counter = 0;

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
                counter++;
                QDomNode n2 = e.firstChild();
                while (!n2.isNull()) {
                       QDomElement e2 = n2.toElement();
                       if (e2.tagName() == "duration")
                           duration << e2.text();
                       n2 = n2.nextSibling();
                }
            }
       }
       n = n.nextSibling();
    }

    #ifdef TUP_DEBUG
        qDebug() << "[TupStoryboard::fromXml()] - counter -> " << counter;
    #endif
}

QDomElement TupStoryboard::toXml(QDomDocument &doc) const
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupStoryboard::toXml()]";
    #endif

    QDomElement storyboard = doc.createElement("storyboard");
    storyboard.setAttribute("cover_duration", coverDuration);

    if (storyboardHasData()) {
        QDomText titleDom   = doc.createTextNode(title);
        QDomText authorDom  = doc.createTextNode(author);
        QDomText topicsDom  = doc.createTextNode(topics);
        QDomText summaryDom = doc.createTextNode(summary);

        storyboard.appendChild(doc.createElement("title")).appendChild(titleDom);
        storyboard.appendChild(doc.createElement("author")).appendChild(authorDom);
        storyboard.appendChild(doc.createElement("topics")).appendChild(topicsDom);
        storyboard.appendChild(doc.createElement("summary")).appendChild(summaryDom);

        #ifdef TUP_DEBUG
            qDebug() << "[TupStoryboard::toXml()] - duration.size() -> " << duration.size();
        #endif

        for (int i=0; i<duration.size(); i++) {
             QDomElement sceneDom = doc.createElement("scene");
             QDomText timeDom  = doc.createTextNode(duration.at(i));
             sceneDom.appendChild(doc.createElement("duration")).appendChild(timeDom);
             storyboard.appendChild(sceneDom);
        }
    } else {
        #ifdef TUP_DEBUG
            qDebug() << "[TupStoryboard::toXml()] - No storyboard data to save";
        #endif
    }

    return storyboard;
}

int TupStoryboard::size()
{
    return duration.count();
}

QString TupStoryboard::cleanString(QString input) const
{
    input.replace(",", "\\,");
    input.replace("'", "\"");

    return input;
}

bool TupStoryboard::storyboardHasData() const
{
    for (int i=0; i<duration.size(); i++) {
        if (duration.at(i).isEmpty())
            return false;
    }

    return true;
}

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

#ifndef TUPSTORYBOARD_H
#define TUPSTORYBOARD_H

#include "tglobal.h"
#include "tupabstractserializable.h"

#include <QObject>
#include <QDomDocument>
#include <QDomElement>

class TUPITUBE_EXPORT TupStoryboard : public QObject, public TupAbstractSerializable
{
    Q_OBJECT

    public:
        TupStoryboard();
        ~TupStoryboard();

        void init(int start, int size);
        void reset();
        void insertScene(int index);
        void appendScene();
        void moveScene(int oldIndex, int newIndex);
        void resetScene(int index);
        void removeScene(int index);

        void setStoryTitle(const QString &title);
        void setStoryTopics(const QString &topics);
        void setStoryAuthor(const QString &author);
        void setStorySummary(const QString &desc);

        QString storyTitle() const;
        QString storyAuthor() const;
        QString storyTopics() const;
        QString storySummary() const;

        void setSceneDuration(int index, const QString &duration);
        QString sceneDuration(int index) const;
        
        virtual void fromXml(const QString &xml);
        virtual QDomElement toXml(QDomDocument &doc) const;

        int size();
        
    private:
        QString cleanString(QString input) const;
        bool storyboardHasData() const;

        QString title;
        QString author;
        QString topics;
        QString summary;

        QList<QString> duration;
};

#endif

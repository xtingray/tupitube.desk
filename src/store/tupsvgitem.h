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

#ifndef TUPSVGITEM_H
#define TUPSVGITEM_H

#include "tglobal.h"
#include "tupabstractserializable.h"
#include "tupitemtweener.h"

#include <QGraphicsSvgItem>
#include <QGraphicsSceneHoverEvent>
#include <QSvgRenderer>
#include <QFileInfo>

class TupFrame;

class TUPITUBE_EXPORT TupSvgItem : public QGraphicsSvgItem, public TupAbstractSerializable
{
    public:
        TupSvgItem(QGraphicsItem * parent = NULL);
        TupSvgItem(const QString &file, TupFrame *frame = NULL);
        ~TupSvgItem();

        void setSymbolName(const QString &symbolName);
        QString symbolName() const;
        QString itemPath() const;
        TupFrame *frame() const;
        int frameIndex();
        bool layerIsVisible();

        TupItemTweener* tween(const QString &name) const;
        QList<TupItemTweener *> tweensList() const;
        void removeTween(int index);
        void removeAllTweens();
        void rendering();
        void addTween(TupItemTweener *itemTween);
        bool hasTweens();
        void setLastTweenPos(QPointF point);
        QPointF lastTweenPos();

        bool transformationIsNotEdited();
        void saveInitTransformation();
        void storeItemTransformation(const QString &properties);
        void undoTransformation();
        void redoTransformation();

        virtual void fromXml(const QString &xml);
        virtual QDomElement toXml(QDomDocument &doc) const;

    protected:
        virtual void hoverEnterEvent(QGraphicsSceneHoverEvent *event);
        virtual void hoverLeaveEvent(QGraphicsSceneHoverEvent *event);

    // signals:
    //     void openInfoWidget();

    private:
        QString name;
        QString path;
        QString data;
        TupFrame *tupFrame;

        QList<TupItemTweener*> tweens;
        QPointF lastTweenPosition;

        QStringList doList;
        QStringList undoList;
};

#endif

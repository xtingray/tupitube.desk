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

#ifndef TUPTWITTER_H
#define TUPTWITTER_H

#include "tglobal.h"
#include "tapplicationproperties.h"

#include <QWidget>
#include <QtNetwork>
#include <QByteArray>

class TUPITUBE_EXPORT TupTwitter : public QWidget
{
    Q_OBJECT

    public:
        TupTwitter(QWidget *parent = nullptr);
        ~TupTwitter();

        void start();

    private slots:
        void closeRequest(QNetworkReply *reply);
        void slotError(QNetworkReply::NetworkError error);

    signals:
        void pageReady();
        void newUpdate(bool flag);

    private:
        void requestFile(const QString &target);
        void checkSoftwareUpdates(QByteArray array);
        void formatStatus(QByteArray array);
        bool saveFile(const QString &answer, const QString &fileName);
        QString getImageCode(const QString &answer) const;

        static QString IS_HOST_UP_URL;
        static QString USER_TIMELINE_URL;
        static QString TUPITUBE_VERSION_URL;
        static QString TUPITUBE_WEB_AD;
        static QString TUPITUBE_VIDEOS;
        static QString TUPITUBE_IMAGES;
        // static QString BROWSER_FINGERPRINT;

        QNetworkAccessManager *manager;
        QNetworkRequest request;
        QNetworkReply *reply;

        QString version;
        QString revision;
        QString codeName;
        QString word;
        QString url;
        QString webMsg;
        bool update;
        bool showAds;
        QString themeName;
        QString locale;
};

#endif

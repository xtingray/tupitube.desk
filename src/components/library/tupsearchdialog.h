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

#ifndef TUPSEARCHDIALOG_H
#define TUPSEARCHDIALOG_H

#include "tglobal.h"

#include <QDialog>
#include <QComboBox>
#include <QNetworkAccessManager>
#include <QNetworkReply>

class TUPITUBE_EXPORT TupSearchDialog : public QDialog
{
    Q_OBJECT

    public:
        TupSearchDialog(const QSize &size, QWidget *parent = nullptr);
        ~TupSearchDialog();

    private slots:
        void startSearch();
        void processResult(QNetworkReply *reply);
        void processMiniature(QNetworkReply *reply);
        void slotError(QNetworkReply::NetworkError error);

    private:
        void requestResults();
        void loadAssets(const QString &input);
        void getMiniature(const QString &code);

        QString assetsPath;
        QComboBox *searchLine;
        QComboBox *assetCombo;
        QString pattern;
        QString dimension;
        QString type;
        int itemsCounter;

        QNetworkAccessManager *manager;

        struct AssetRecord {
            QString description;
            QString code;
            QString creator;
            QString creatorUrl;
            QString licenseTitle;
            QString licenseUrl;
        };

        QList<AssetRecord> assetList;
};

#endif

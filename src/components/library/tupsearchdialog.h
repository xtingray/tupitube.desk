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
#include "tcollapsiblewidget.h"
#include "timagebutton.h"
#include "tcombobox.h"

#include <QDialog>
#include <QComboBox>
#include <QNetworkReply>
#include <QListWidget>
#include <QLabel>
#include <QProgressBar>
#include <QScreen>

class TUPITUBE_EXPORT TupSearchDialog : public QDialog
{
    Q_OBJECT

    public:
        enum StackId { Result = 0, Progressbar, NoResult, Error };
        enum AssetType { Object = 0, Character, StaticBg, DynamicBg, Sound };
        enum AssetExtension { JPG = 0, PNG, SVG, TOBJ };
        TupSearchDialog(const QSize &size, QWidget *parent = nullptr);
        ~TupSearchDialog();

    signals:
        void assetStored(const QString &name, AssetType assetTypeCode, const QString &extension,
                         int extensionId, QByteArray &data);

    private slots:
        void setSupportTab();
        void startSearch();
        void startSearchFromCombo();
        void processResult(QNetworkReply *reply);
        void processMiniature(QNetworkReply *reply);
        void slotError(QNetworkReply::NetworkError error);
        void updateAssetView(int index);
        void getAsset();
        void processAsset(QNetworkReply *reply);
        void openDonationLink();
        void openPatreonLink();
        void openCreditsLink();
        void updateProgress(qint64 bytesReceived, qint64 bytesTotal);

    private:
        QWidget * searchTab();
        QWidget * patreonTab();

        void requestResults();
        void loadAssets(const QString &input);
        void getMiniature(const QString &code, const QString &desc);
        void setLabelLink(QLabel *label, const QString &url);
        bool saveImage(const QString &path, const char *extension, const QByteArray &data);
        bool saveAssetFile(const QString path, const QByteArray &data);
        void resetProgress(StackId id);

        QTabWidget *tabWidget;
        TImageButton *searchButton;
        QListWidget *assetDescList;
        QWidget *resultPanel;
        QScreen *screen;
        QLabel *previewPic;
        QLabel *graphicType;
        QLabel *creator;
        QLabel *creatorUrl;
        QLabel *license;
        QLabel *licenseUrl;
        QLabel *progressLabel;
        QLabel *noResultLabel;
        QProgressBar *progressBar;

        TCollapsibleWidget *dynamicPanel;

        QString assetsPath;
        TComboBox *searchLine;
        QComboBox *assetCombo;
        QString pattern;
        QString dimension;
        int assetTypeCode;
        QString assetType;
        int itemsCounter;
        QString linkStyle;

        struct AssetRecord {
            QString description;
            QString code;
            QString ext;
            QString creator;
            QString creatorUrl;
            QString licenseTitle;
            QString licenseUrl;
        };

        QList<AssetRecord> assetList;
        QList<QString> extStrings;
};

#endif

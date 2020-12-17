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

#include "tupsearchdialog.h"
#include "tosd.h"
#include "tconfig.h"

#include <QVBoxLayout>
#include <QDialogButtonBox>
#include <QHBoxLayout>
#include <QIcon>

#include <QUrlQuery>
#include <QNetworkReply>
#include <QBuffer>

#include <QDomDocument>

TupSearchDialog::TupSearchDialog(const QSize &size, QWidget *parent) : QDialog(parent)
{
    setWindowTitle(tr("Assets Search Engine"));
    setWindowIcon(QIcon(QPixmap(THEME_DIR + "icons/search.png")));

    dimension = QString::number(size.width()) + ":" + QString::number(size.height());
    TCONFIG->beginGroup("General");
    assetsPath = TCONFIG->value("AssetsPath", CACHE_DIR + "assets").toString();

    QFile file(THEME_DIR + "config/ui.qss");
    if (file.exists()) {
        file.open(QFile::ReadOnly);
        QString styleSheet = QLatin1String(file.readAll());
        if (styleSheet.length() > 0)
            setStyleSheet(styleSheet);
        file.close();
    } else {
        #ifdef TUP_DEBUG
            qWarning() << "[TupSearchDialog::TupSearchDialog()] - theme file doesn't exist -> " << QString(THEME_DIR + "config/ui.qss");
        #endif
    }

    QVBoxLayout *layout = new QVBoxLayout(this);

    tabWidget = new QTabWidget;
    tabWidget->addTab(searchTab(), tr("Search"));
    tabWidget->addTab(patreonTab(), tr("Patreon"));

    QPushButton *closeButton = new QPushButton(tr("Close"));
    layout->addWidget(closeButton);
    connect(closeButton, SIGNAL(clicked()), this, SLOT(close()));

    QHBoxLayout *buttonLayout = new QHBoxLayout;
    buttonLayout->addWidget(closeButton, 1, Qt::AlignRight);

    layout->addWidget(tabWidget, Qt::AlignLeft);
    layout->addLayout(buttonLayout);
    layout->addStretch();

    setFixedWidth(800);
}

TupSearchDialog::~TupSearchDialog()
{
}

QWidget * TupSearchDialog::searchTab()
{
    QWidget *searchWidget = new QWidget;
    QVBoxLayout *layout = new QVBoxLayout(searchWidget);

    QWidget *searchPanel = new QWidget;
    QVBoxLayout *searchLayout = new QVBoxLayout(searchPanel);
    searchLayout->addWidget(new QWidget);

    QWidget *comboPanel = new QWidget;
    QHBoxLayout *comboLayout = new QHBoxLayout(comboPanel);
    comboLayout->setMargin(0);
    comboLayout->setSpacing(0);

    searchLine = new QComboBox;
    searchLine->setEditable(true);
    comboLayout->addWidget(searchLine);
    comboLayout->addSpacing(10);

    searchButton = new TImageButton(QPixmap(THEME_DIR + "icons/search.png"), 22, this);
    searchButton->setToolTip(tr("Search"));
    connect(searchButton, SIGNAL(clicked()), this, SLOT(startSearch()));
    comboLayout->addWidget(searchButton);

    searchLayout->addWidget(comboPanel, Qt::AlignHCenter);

    assetCombo = new QComboBox;
    assetCombo->setIconSize(QSize(15, 15));
    assetCombo->addItem(QIcon(THEME_DIR + "icons/bitmap.png"), tr("Object"));
    assetCombo->addItem(QIcon(THEME_DIR + "icons/bitmap.png"), tr("Character"));
    assetCombo->addItem(QIcon(THEME_DIR + "icons/bitmap.png"), tr("Static Background"));
    assetCombo->addItem(QIcon(THEME_DIR + "icons/bitmap.png"), tr("Dynamic Background"));
    // assetCombo->addItem(QIcon(THEME_DIR + "icons/bitmap.png"), tr("Puppet"));

    searchLayout->addWidget(assetCombo, Qt::AlignHCenter);
    searchLayout->addWidget(new QWidget);
    searchLayout->addStretch();

    resultPanel = new QWidget;
    QHBoxLayout *resultLayout = new QHBoxLayout(resultPanel);
    assetDescList = new QListWidget;
    assetDescList->setFixedWidth(150);
    connect(assetDescList, SIGNAL(currentRowChanged(int)),
            this, SLOT(updateAssetView(int)));
    resultLayout->addWidget(assetDescList);

    QWidget *picPanel = new QWidget;
    QVBoxLayout *picLayout = new QVBoxLayout(picPanel);
    previewPic = new QLabel;
    picLayout->addWidget(previewPic);

    // picLayout->addLayout(importLayout);

    QWidget *detailsPanel = new QWidget;
    detailsPanel->setStyleSheet("background-color:#c8c8c8;");
    QVBoxLayout *detailsLayout = new QVBoxLayout(detailsPanel);
    graphicType = new QLabel;
    creator = new QLabel;
    creatorUrl = new QLabel;
    license = new QLabel;
    licenseUrl = new QLabel;

    detailsLayout->addWidget(new QWidget);
    detailsLayout->addWidget(graphicType);
    detailsLayout->addSpacing(10);
    detailsLayout->addWidget(creator);
    detailsLayout->addWidget(creatorUrl);
    detailsLayout->addSpacing(10);
    detailsLayout->addWidget(license);
    detailsLayout->addWidget(licenseUrl);
    detailsLayout->addWidget(new QWidget);

    QHBoxLayout *importLayout = new QHBoxLayout;
    QPushButton *importButton = new QPushButton(tr("Import Asset"));
    connect(importButton, SIGNAL(clicked()), this, SLOT(importAsset()));
    importLayout->addWidget(new QWidget);
    importLayout->addWidget(importButton, 1, Qt::AlignHCenter);
    importLayout->addWidget(new QWidget);
    importLayout->addStretch();

    detailsLayout->addLayout(importLayout);

    QHBoxLayout *infoLayout = new QHBoxLayout;
    infoLayout->addWidget(picPanel);
    infoLayout->addWidget(detailsPanel);

    QVBoxLayout *dataLayout = new QVBoxLayout;
    dataLayout->addLayout(infoLayout, Qt::AlignHCenter);
    // dataLayout->addLayout(importLayout, Qt::AlignHCenter);

    resultLayout->addLayout(dataLayout);
    resultLayout->addStretch();

    QWidget *controlPanel = new QWidget;
    QHBoxLayout *controlLayout = new QHBoxLayout(controlPanel);
    controlLayout->addWidget(new QWidget);
    controlLayout->addWidget(searchPanel);
    controlLayout->addWidget(new QWidget);

    dynamicPanel = new TCollapsibleWidget;
    dynamicPanel->setWidget(resultPanel);

    layout->addWidget(controlPanel);
    layout->addWidget(dynamicPanel, Qt::AlignHCenter);
    layout->addStretch();

    // resultPanel->setVisible(false);

    return searchWidget;
}

QWidget * TupSearchDialog::patreonTab()
{
    QWidget *patreonWidget = new QWidget;
    QVBoxLayout *layout = new QVBoxLayout(patreonWidget);
    layout->addStretch();

    return patreonWidget;
}

void TupSearchDialog::startSearch()
{
    pattern = searchLine->currentText();
    if (pattern.length() > 0) {
        searchButton->setEnabled(false);
        if (dynamicPanel->isExpanded())
            dynamicPanel->setExpanded(false);
        assetList.clear();
        assetDescList->clear();
        QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
        qDebug() << "";
        qDebug() << "Search pattern -> " << pattern;

        manager = new QNetworkAccessManager(this);
        connect(manager, &QNetworkAccessManager::finished, this, &TupSearchDialog::processResult);
        connect(manager, &QNetworkAccessManager::finished, manager, &QNetworkAccessManager::deleteLater);

        QString apiEntry = TUPITUBE_URL + QString("/api/library/");
        #ifdef TUP_DEBUG
            qDebug() << "[TupSearchDialog::startSearch()] - URL -> " << apiEntry;
        #endif

        QUrl url(apiEntry);
        QNetworkRequest request = QNetworkRequest();
        request.setRawHeader("User-Agent", BROWSER_FINGERPRINT);
        request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
        request.setSslConfiguration(QSslConfiguration::defaultConfiguration());

        request.setUrl(QUrl(url));

        QUrlQuery params = QUrlQuery();
        params.addQueryItem("pattern", pattern);
        params.addQueryItem("type", QString::number(assetCombo->currentIndex()));
        params.addQueryItem("dimension", dimension);

        QByteArray postData = params.query(QUrl::FullyEncoded).toUtf8();
        QNetworkReply *reply = manager->post(request, postData);
        connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(slotError(QNetworkReply::NetworkError)));
        connect(reply, &QNetworkReply::finished, reply, &QNetworkReply::deleteLater);
        reply->setParent(manager);
    } else {
        TOsd::self()->display(TOsd::Warning, tr("Invalid search: Empty pattern!"));
    }
}

void TupSearchDialog::processResult(QNetworkReply *reply)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupSearchDialog::processResult()]";
    #endif

    QByteArray data = reply->readAll();
    QString answer(data);
    if (answer.length() > 0) {
        #ifdef TUP_DEBUG
            qDebug() << "[TupSearchDialog::processResult()] - answer -> " << answer;
        #endif
        itemsCounter = 0;
        loadAssets(answer);
    } else {
        #ifdef TUP_DEBUG
            qDebug() << "[TupSearchDialog::processResult()] - Error: No answer from server!";
        #endif
        TOsd::self()->display(TOsd::Error, tr("Network Error 609. Please, contact us!"));
    }
}

void TupSearchDialog::slotError(QNetworkReply::NetworkError error)
{
    TOsd::self()->display(TOsd::Error, tr("Network Fatal Error. Please, contact us!"));

    switch (error) {
        case QNetworkReply::HostNotFoundError:
             {
             #ifdef TUP_DEBUG
                 qDebug() << "[TupSearchDialog::slotError()] - Network Error: Host not found";
             #endif
             }
        break;
        case QNetworkReply::TimeoutError:
             {
             #ifdef TUP_DEBUG
                 qDebug() << "[TupSearchDialog::slotError()] - Network Error: Time out!";
             #endif
             }
        break;
        case QNetworkReply::ConnectionRefusedError:
             {
             #ifdef TUP_DEBUG
                 qDebug() << "[TupSearchDialog::slotError()] - Network Error: Connection Refused!";
             #endif
             }
        break;
        case QNetworkReply::ContentNotFoundError:
             {
             #ifdef TUP_DEBUG
                 qDebug() << "[TupSearchDialog::slotError()] - Network Error: Content not found!";
             #endif
             }
        break;
        case QNetworkReply::UnknownNetworkError:
        default:
             {
             #ifdef TUP_DEBUG
                 qDebug() << "[TupSearchDialog::slotError()] - Network Error: Unknown Network error!";
             #endif
             }
        break;
    }
}

void TupSearchDialog::loadAssets(const QString &input)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupSearchDialog::loadAssets()] - input -> " << input;
    #endif

    QDomDocument doc;
    if (doc.setContent(input)) {
        QDomElement root = doc.documentElement();
        int total = root.attribute("size", "0").toInt();
        qDebug() << "TOTAL -> " << total;
        if (total == 0) {
            qDebug() << "No recourds found!";
            searchButton->setEnabled(true);
            QApplication::restoreOverrideCursor();
            return;
        }

        QDomNode n = root.firstChild();
        while (!n.isNull()) {
            QDomElement e = n.toElement();
            if (!e.isNull()) {
                qDebug() << "TAG: " << e.tagName();
                if (e.tagName() == "item") {
                    AssetRecord asset;
                    QDomNode item = e.firstChild();
                    while (!item.isNull()) {
                        QDomElement record = item.toElement();
                        qDebug() << "RECORD -> " << record.tagName();
                        qDebug() << "VALUE -> " << record.text();
                        if (record.tagName() == "desc") {
                            asset.description = record.text();
                            new QListWidgetItem(record.text(), assetDescList);
                        } else if (record.tagName() == "code") {
                            asset.code = record.text();
                        } else if (record.tagName() == "type") {
                            asset.type = record.text();
                        } else if (record.tagName() == "creator") {
                            asset.creator = record.text();
                        } else if (record.tagName() == "creator_url") {
                            asset.creatorUrl = record.text();
                        } else if (record.tagName() == "license") {
                            asset.licenseTitle = record.text();
                        } else if (record.tagName() == "license_url") {
                            asset.licenseUrl = record.text();
                        }
                        item = item.nextSibling();
                    }
                    assetList << asset;
                }
            }
            n = n.nextSibling();
        }

        for (int i=0; i<assetList.count(); i++) {
            AssetRecord asset = assetList.at(i);
            QString path = assetsPath + asset.code;
            qDebug() << "Item path -> " << path;
            QDir assetDir(path);
            if (!assetDir.exists()) {
                if (assetDir.mkpath(path)) {
                    getMiniature(asset.code);
                } else {
                    #ifdef TUP_DEBUG
                        qDebug() << "[TupSearchDialog::loadAssets()] - Fatal Error: Can't create asset dir -> " << path;
                    #endif
                }
            } else {
                QFileInfo file(path + "/miniature.png");
                if (file.exists()) {
                    itemsCounter++;
                    if (itemsCounter == assetList.count()) {
                        assetDescList->setCurrentRow(0);
                        // resultPanel->setVisible(true);
                        dynamicPanel->setExpanded(true);
                        searchButton->setEnabled(true);
                        QApplication::restoreOverrideCursor();
                    }
                } else {
                    getMiniature(asset.code);
                }
            }
        }
    }
}

void TupSearchDialog::getMiniature(const QString &code)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupSearchDialog::getMiniature()] - code -> " << code;
    #endif

    manager = new QNetworkAccessManager(this);
    connect(manager, &QNetworkAccessManager::finished, this, &TupSearchDialog::processMiniature);
    connect(manager, &QNetworkAccessManager::finished, manager, &QNetworkAccessManager::deleteLater);

    QString apiEntry = TUPITUBE_URL + QString("/api/library/miniature/");
    #ifdef TUP_DEBUG
        qDebug() << "[TupSearchDialog::getMiniature()] - URL -> " << apiEntry;
    #endif

    QUrl url(apiEntry);
    QNetworkRequest request = QNetworkRequest();
    request.setRawHeader("User-Agent", BROWSER_FINGERPRINT);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    request.setSslConfiguration(QSslConfiguration::defaultConfiguration());
    request.setUrl(QUrl(url));

    QUrlQuery params = QUrlQuery();
    params.addQueryItem("code", code);

    QByteArray postData = params.query(QUrl::FullyEncoded).toUtf8();
    QNetworkReply *reply = manager->post(request, postData);
    connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(slotError(QNetworkReply::NetworkError)));
    connect(reply, &QNetworkReply::finished, reply, &QNetworkReply::deleteLater);
    reply->setParent(manager);
}

void TupSearchDialog::processMiniature(QNetworkReply *reply)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupSearchDialog::processMiniature()]";
    #endif

    QByteArray data = reply->readAll();
    if (data.size() > 0) {
        QString code = reply->rawHeader("Code");
        #ifdef TUP_DEBUG
            qDebug() << "[TupSearchDialog::processMiniature()] - Saving miniature...";
            qDebug() << "[TupSearchDialog::processMiniature()] - Code -> " << code;
            qDebug() << "[TupSearchDialog::processMiniature()] - Image size -> " << data.size();
        #endif

        QImage miniature;
        if (miniature.loadFromData(data, "PNG")) {
            QString miniaturePath = assetsPath + code + "/miniature.png";
            if (miniature.save(miniaturePath, "PNG")) {
                #ifdef TUP_DEBUG
                    qDebug() << "[TupSearchDialog::processMiniature()] - Miniature saved successfully! -> " << miniaturePath;
                #endif
            } else {
                #ifdef TUP_DEBUG
                    qDebug() << "[TupSearchDialog::processMiniature()] - Can't save miniature!";
                #endif
                TOsd::self()->display(TOsd::Error, tr("Can't load result images!"));
            }
        } else {
            #ifdef TUP_DEBUG
                qDebug() << "[TupSearchDialog::processMiniature()] - Fatal Error: Can't load image bytes!";
            #endif
            TOsd::self()->display(TOsd::Error, tr("Can't load result images!"));
        }

        itemsCounter++;
        if (itemsCounter == assetList.count()) {
            assetDescList->setCurrentRow(0);
            // resultPanel->setVisible(true);
            dynamicPanel->setExpanded(true);
            searchButton->setEnabled(true);
            QApplication::restoreOverrideCursor();
        }
    } else {
        #ifdef TUP_DEBUG
            qDebug() << "[TupSearchDialog::processMiniature()] - Fatal Error: No answer from server!";
        #endif
        TOsd::self()->display(TOsd::Error, tr("Network Error 709. Please, contact us!"));
    }
}

void TupSearchDialog::updateAssetView(int index)
{
    qDebug() << "";
    qDebug() << "Current Index -> " << index;
    qDebug() << "assetList.count() -> " << assetList.count();

    if (!assetList.isEmpty()) {
        AssetRecord item = assetList.at(index);
        QString path = assetsPath + item.code + "/miniature.png";
        previewPic->setPixmap(QPixmap(path));

        graphicType->setText("<b>" + tr("Type:") + "</b> " + item.type);

        creator->setText("<b>" + tr("Creator:") + "</b> " + item.creator);
        setLabelLink(creatorUrl, item.creatorUrl);

        license->setText("<b>" + tr("License:") + "</b> " + item.licenseTitle);
        setLabelLink(licenseUrl, item.licenseUrl);
    } else {
        #ifdef TUP_DEBUG
            qDebug() << "[TupSearchDialog::updateAssetView()] - Fatal Error: Assets list is empty!";
        #endif
    }
}

void TupSearchDialog::setLabelLink(QLabel *label, const QString &url)
{
    label->setText("<a href=\"" + url + "\">" + url + "</a>");
    label->setTextFormat(Qt::RichText);
    label->setTextInteractionFlags(Qt::TextBrowserInteraction);
    label->setOpenExternalLinks(true);
}

void TupSearchDialog::importAsset()
{
    qDebug() << "";
    qDebug() << "TupSearchDialog::importAsset() - Tracing...";
}

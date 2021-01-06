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
#include "tlabel.h"

#include <QVBoxLayout>
#include <QDialogButtonBox>
#include <QHBoxLayout>
#include <QIcon>
#include <QTextEdit>
#include <QDesktopServices>

#include <QNetworkAccessManager>
#include <QUrlQuery>
#include <QNetworkReply>
#include <QBuffer>
#include <QDomDocument>

TupSearchDialog::TupSearchDialog(const QSize &size, QWidget *parent) : QDialog(parent)
{
    setModal(true);
    setWindowTitle(tr("Assets Search Engine"));
    setWindowIcon(QIcon(QPixmap(THEME_DIR + "icons/search.png")));

    dimension = QString::number(size.width()) + ":" + QString::number(size.height());
    TCONFIG->beginGroup("General");
    assetsPath = TCONFIG->value("AssetsPath", CACHE_DIR + "assets").toString();
    getNews = TCONFIG->value("GetNews", true).toBool();

    extStrings << "jpg" << "png" << "svg" << "tobj";

    QFile file(THEME_DIR + "config/ui.qss");
    if (file.exists()) {
        file.open(QFile::ReadOnly);
        QString styleSheet = QLatin1String(file.readAll());
        if (styleSheet.length() > 0)
            setStyleSheet(styleSheet);
        file.close();
    } else {
        #ifdef TUP_DEBUG
            qWarning() << "[TupSearchDialog::TupSearchDialog()] - theme file doesn't exist -> "
                       << QString(THEME_DIR + "config/ui.qss");
        #endif
    }

    QVBoxLayout *layout = new QVBoxLayout(this);

    tabWidget = new QTabWidget;
    tabWidget->addTab(searchTab(), tr("Search"));
    if (getNews)
        tabWidget->addTab(patreonTab(), tr("Support Us"));

    QPushButton *closeButton = new QPushButton(tr("Close"));
    layout->addWidget(closeButton);
    connect(closeButton, SIGNAL(clicked()), this, SLOT(accept()));

    QHBoxLayout *buttonLayout = new QHBoxLayout;
    buttonLayout->addWidget(closeButton, 1, Qt::AlignRight);

    layout->addWidget(tabWidget, Qt::AlignLeft);
    layout->addLayout(buttonLayout);
    layout->addStretch();

    screen = QGuiApplication::screens().at(0);

    setFixedWidth(800);
}

TupSearchDialog::~TupSearchDialog()
{
}

QWidget * TupSearchDialog::searchTab()
{
    linkStyle = "style=\"color:#0064be;\"";
    QWidget *searchWidget = new QWidget;
    QVBoxLayout *layout = new QVBoxLayout(searchWidget);

    QWidget *searchPanel = new QWidget;
    QVBoxLayout *searchLayout = new QVBoxLayout(searchPanel);
    searchLayout->addWidget(new QWidget);

    QWidget *comboPanel = new QWidget;
    QHBoxLayout *comboLayout = new QHBoxLayout(comboPanel);
    comboLayout->setMargin(0);
    comboLayout->setSpacing(0);

    searchLine = new TComboBox;
    connect(searchLine, SIGNAL(enterPressed()), this, SLOT(startSearch()));
    searchLine->setEditable(true);
    comboLayout->addWidget(searchLine);
    comboLayout->addSpacing(10);

    searchButton = new TImageButton(QPixmap(THEME_DIR + "icons/search.png"), 22);
    searchButton->setToolTip(tr("Search"));
    connect(searchButton, SIGNAL(clicked()), this, SLOT(startSearch()));
    comboLayout->addWidget(searchButton);

    searchLayout->addWidget(comboPanel, Qt::AlignHCenter);

    assetCombo = new QComboBox;
    assetCombo->setIconSize(QSize(15, 15));
    assetCombo->addItem(QIcon(THEME_DIR + "icons/tip.png"), tr("Object"));
    assetCombo->addItem(QIcon(THEME_DIR + "icons/animation_mode.png"), tr("Character"));
    assetCombo->addItem(QIcon(THEME_DIR + "icons/static_background_mode.png"), tr("Static Background"));
    assetCombo->addItem(QIcon(THEME_DIR + "icons/dynamic_background_mode.png"), tr("Dynamic Background"));
    assetCombo->addItem(QIcon(THEME_DIR + "icons/speaker.png"), tr("Sound"));
    // assetCombo->addItem(QIcon(THEME_DIR + "icons/bitmap.png"), tr("Puppet"));

    connect(assetCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(startSearchFromCombo()));

    assetCombo->setItemData(4, 0, Qt::UserRole - 1);
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

    QWidget *detailsPanel = new QWidget;
    detailsPanel->setStyleSheet("background-color:#c8c8c8; border-radius: 10px;");
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
    QPushButton  *importButton = new QPushButton(QPixmap(THEME_DIR + "icons/import_asset.png"), tr("Import Asset"));
    importButton->setStyleSheet("padding:8px;");
    connect(importButton, SIGNAL(clicked()), this, SLOT(getAsset()));

    importLayout->addStretch();
    importLayout->addWidget(new QWidget);
    importLayout->addWidget(importButton, 1, Qt::AlignHCenter);
    importLayout->addWidget(new QWidget);
    importLayout->addStretch();

    QVBoxLayout *rightLayout = new QVBoxLayout;
    rightLayout->addStretch();
    rightLayout->addWidget(detailsPanel);
    rightLayout->addSpacing(10);
    rightLayout->addLayout(importLayout);
    rightLayout->addStretch();

    QHBoxLayout *infoLayout = new QHBoxLayout;
    infoLayout->addWidget(picPanel);
    infoLayout->addLayout(rightLayout);

    QWidget *dataPanel = new QWidget;
    QVBoxLayout *dataLayout = new QVBoxLayout(dataPanel);
    dataLayout->addLayout(infoLayout, Qt::AlignHCenter);

    resultLayout->addWidget(dataPanel, Qt::AlignHCenter);
    resultLayout->addStretch();

    QWidget *controlPanel = new QWidget;
    QHBoxLayout *controlLayout = new QHBoxLayout(controlPanel);
    controlLayout->addWidget(new QWidget);
    controlLayout->addWidget(searchPanel);
    controlLayout->addWidget(new QWidget);

    QWidget *progressPanel = new QWidget;
    QVBoxLayout *progressLayout = new QVBoxLayout(progressPanel);

    TCONFIG->beginGroup("General");
    QString themeName = TCONFIG->value("Theme", "Light").toString();
    QString progressStyle = "QProgressBar { background-color: #DDDDDD; "
                    "text-align: center; color: #FFFFFF; border-radius: 2px; } ";
    QString color = "#009500";
    if (themeName.compare("Dark") == 0)
        color = "#444444";
    progressStyle += "QProgressBar::chunk { background-color: " + color + "; border-radius: 2px; }";

    progressLabel = new QLabel("<b>" + tr("Searching...") + "</b>");
    progressLabel->setAlignment(Qt::AlignHCenter);

    QHBoxLayout *barLayout = new QHBoxLayout;
    progressBar = new QProgressBar;
    progressBar->setTextVisible(true);
    progressBar->setStyleSheet(progressStyle);
    progressBar->setRange(1, 100);
    barLayout->addStretch();
    barLayout->addWidget(progressBar);
    barLayout->addStretch();

    QWidget *innerProgressPanel = new QWidget;
    innerProgressPanel->setStyleSheet("background-color:#c8c8c8; border-radius: 10px;");

    QVBoxLayout *innerProgressLayout = new QVBoxLayout(innerProgressPanel);
    innerProgressLayout->addStretch();
    innerProgressLayout->addWidget(progressLabel);
    innerProgressLayout->addSpacing(10);
    innerProgressLayout->addLayout(barLayout);
    innerProgressLayout->addStretch();

    progressLayout->addWidget(innerProgressPanel, Qt::AlignCenter);

    QWidget *noResultPanel = new QWidget;
    noResultPanel->setStyleSheet("background-color:#c8c8c8; border-radius: 10px;");
    QVBoxLayout *noResultLayout = new QVBoxLayout(noResultPanel);

    QLabel *noResultIcon = new QLabel;
    noResultIcon->setPixmap(QPixmap(THEME_DIR + "icons/warning_sign.png"));
    noResultIcon->setAlignment(Qt::AlignHCenter);

    noResultLabel = new QLabel;
    QFont font = noResultLabel->font();
    font.setPointSize(18);
    font.setBold(true);
    noResultLabel->setFont(font);
    noResultLabel->setAlignment(Qt::AlignHCenter);

    QLabel *descLabel = new QLabel(tr("But we are working on new assets..."));
    descLabel->setAlignment(Qt::AlignHCenter);
    font.setPointSize(14);
    font.setBold(false);
    descLabel->setFont(font);

    TLabel *supportLabel = new TLabel("<a href=\"https://tupitube.com\" " + linkStyle + ">" + tr("Want to support us?") + "</a>");
    supportLabel->setAlignment(Qt::AlignHCenter);
    supportLabel->setFont(font);
    supportLabel->setTextFormat(Qt::RichText);
    supportLabel->setTextInteractionFlags(Qt::TextBrowserInteraction);
    connect(supportLabel, SIGNAL(clicked()), this, SLOT(setSupportTab()));

    noResultLayout->addStretch();
    noResultLayout->addWidget(noResultIcon);
    noResultLayout->addWidget(noResultLabel);
    noResultLayout->addWidget(descLabel);
    if (getNews)
        noResultLayout->addWidget(supportLabel);
    noResultLayout->addStretch();

    QWidget *errorPanel = new QWidget;
    errorPanel->setStyleSheet("background-color:#c8c8c8; border-radius: 10px;");
    QVBoxLayout *errorLayout = new QVBoxLayout(errorPanel);

    QLabel *errorIcon = new QLabel;
    errorIcon->setPixmap(QPixmap(THEME_DIR + "icons/warning_sign.png"));
    errorIcon->setAlignment(Qt::AlignHCenter);

    QLabel *errorLabel = new QLabel(tr("Error while processing request. Please, try again."));
    font.setPointSize(14);
    font.setBold(true);
    errorLabel->setFont(font);
    errorLabel->setAlignment(Qt::AlignHCenter);

    errorLayout->addStretch();
    errorLayout->addWidget(errorIcon);
    errorLayout->addWidget(errorLabel);
    errorLayout->addStretch();

    dynamicPanel = new TCollapsibleWidget;
    dynamicPanel->addWidget(resultPanel);
    dynamicPanel->addWidget(progressPanel);
    dynamicPanel->addWidget(noResultPanel);    
    dynamicPanel->addWidget(errorPanel);

    layout->addWidget(controlPanel);
    layout->addWidget(dynamicPanel, Qt::AlignCenter);
    layout->addStretch();

    return searchWidget;
}

QWidget * TupSearchDialog::patreonTab()
{
    QWidget *patreonWidget = new QWidget;
    QVBoxLayout *layout = new QVBoxLayout(patreonWidget);

    QFont font = this->font();
    font.setPointSize(11);

    QTextEdit *patreonText = new QTextEdit;
    patreonText->setHtml(tr("From the <b>MaeFloresta</b> startup we are requesting the support from our users community "
                            "to keep creating new assets for our library repository. Thanks to your contributions "
                            "we can deliver a better product. You can start helping from <b>USD 1</b>."));
    patreonText->setSizeAdjustPolicy(QAbstractScrollArea::AdjustToContents);
    patreonText->setFont(font);
    layout->addWidget(patreonText);

    font.setPointSize(10);
    font.setBold(true);

    QHBoxLayout *buttonsLayout = new QHBoxLayout;
    QPushButton *oneTimeButton = new QPushButton(tr("One Time Donation"));
    oneTimeButton->setStyleSheet("padding:8px;");
    oneTimeButton->setFont(font);
    connect(oneTimeButton, SIGNAL(clicked()), this, SLOT(openDonationLink()));

    QPushButton *patreonButton = new QPushButton(tr("Join Our Patreon"));
    patreonButton->setStyleSheet("padding:8px;");
    patreonButton->setFont(font);
    connect(patreonButton, SIGNAL(clicked()), this, SLOT(openPatreonLink()));

    QPushButton *creditsButton = new QPushButton(tr("Sponsors"));
    creditsButton->setStyleSheet("padding:8px;");
    creditsButton->setFont(font);
    connect(creditsButton, SIGNAL(clicked()), this, SLOT(openCreditsLink()));

    buttonsLayout->addStretch();
    buttonsLayout->addWidget(new QWidget);
    buttonsLayout->addWidget(oneTimeButton);
    buttonsLayout->addSpacing(10);
    buttonsLayout->addWidget(patreonButton);
    buttonsLayout->addSpacing(10);
    buttonsLayout->addWidget(creditsButton);
    buttonsLayout->addWidget(new QWidget);
    buttonsLayout->addStretch();

    layout->addLayout(buttonsLayout);
    layout->addStretch();

    return patreonWidget;
}

void TupSearchDialog::setSupportTab()
{
    tabWidget->setCurrentIndex(1);
}

void TupSearchDialog::startSearchFromCombo()
{
    pattern = searchLine->currentText();
    if (pattern.length() > 0)
        startSearch();
}

void TupSearchDialog::startSearch()
{
    pattern = searchLine->currentText();
    if (pattern.length() > 0) {
        if (pattern.length() > 30)
            pattern = pattern.left(30);
        assetTypeCode = assetCombo->currentIndex();
        assetType = QString::number(assetTypeCode);

        #ifdef TUP_DEBUG
            qDebug() << "[TupSearchDialog::startSearch()] - pattern -> " << pattern;
        #endif

        assetList.clear();
        assetDescList->clear();
        searchButton->setEnabled(false);

        dynamicPanel->setCurrentIndex(Progressbar);
        if (!dynamicPanel->isExpanded())
            dynamicPanel->setExpanded(true);

        QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

        QNetworkAccessManager *manager = new QNetworkAccessManager(this);
        connect(manager, &QNetworkAccessManager::finished, this, &TupSearchDialog::processResult);
        connect(manager, &QNetworkAccessManager::finished, manager, &QNetworkAccessManager::deleteLater);

        QString apiEntry = LIBRARY_URL + QString("/api/search/");
        #ifdef TUP_DEBUG
            qDebug() << "[TupSearchDialog::startSearch()] - Getting URL -> " << apiEntry;
        #endif

        QUrl url(apiEntry);
        QNetworkRequest request = QNetworkRequest();
        request.setRawHeader("User-Agent", BROWSER_FINGERPRINT);
        request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
        request.setSslConfiguration(QSslConfiguration::defaultConfiguration());

        request.setUrl(QUrl(url));

        QUrlQuery params = QUrlQuery();
        params.addQueryItem("pattern", pattern);
        params.addQueryItem("type", assetType);
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
    resetProgress(NoResult);

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
        if (total == 0) {
            #ifdef TUP_DEBUG
                qDebug() << "[TupSearchDialog::loadAssets()] - No records found!";
            #endif
            noResultLabel->setText(tr("No Results for") + " \"" + pattern + "\" " + tr("YET!"));
            resetProgress(NoResult);
            return;
        }

        QDomNode n = root.firstChild();
        while (!n.isNull()) {
            QDomElement e = n.toElement();
            if (!e.isNull()) {
                if (e.tagName() == "item") {
                    AssetRecord asset;
                    QDomNode item = e.firstChild();
                    while (!item.isNull()) {
                        QDomElement record = item.toElement();
                        if (record.tagName() == "desc") {
                            asset.description = record.text();
                            new QListWidgetItem(record.text(), assetDescList);
                        } else if (record.tagName() == "code") {
                            asset.code = record.text();
                        } else if (record.tagName() == "ext") {
                            asset.ext = record.text();
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

        #ifdef TUP_DEBUG
            qDebug() << "---";
        #endif

        for (int i=0; i<assetList.count(); i++) {
            AssetRecord asset = assetList.at(i);
            QString path = assetsPath + asset.code;
            QDir assetDir(path);
            if (!assetDir.exists()) { // Record hasn't downloaded yet
                if (assetDir.mkpath(path)) {
                    #ifdef TUP_DEBUG
                        qDebug() << "[TupSearchDialog::loadAssets()] - Request No " << (i + 1);
                    #endif
                    getMiniature(asset.code, asset.description);
                } else {
                    #ifdef TUP_DEBUG
                        qDebug() << "[TupSearchDialog::loadAssets()] - Fatal Error: Can't create asset dir -> " << path;
                    #endif
                    resetProgress(NoResult);
                }
            } else { // Folder already exists
                QFileInfo file(path + "/miniature.png");
                if (file.exists()) { // File was downloaded previously
                    itemsCounter++;
                    if (itemsCounter == assetList.count()) {
                        assetDescList->setCurrentRow(0);
                        resetProgress(Result);
                    }
                } else { // Getting file for the first time
                    getMiniature(asset.code, asset.description);
                }
            }
        }
    }
}

void TupSearchDialog::getMiniature(const QString &code, const QString &desc)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupSearchDialog::getMiniature()] - code -> " << code;
    #endif

    progressLabel->setText("<b>" + tr("Getting item") + " " + desc + "</b>");
    progressBar->reset();

    QNetworkAccessManager *manager = new QNetworkAccessManager(this);
    connect(manager, &QNetworkAccessManager::finished, this, &TupSearchDialog::processMiniature);
    connect(manager, &QNetworkAccessManager::finished, manager, &QNetworkAccessManager::deleteLater);

    QString apiEntry = LIBRARY_URL + QString("/api/miniature/");
    #ifdef TUP_DEBUG
        qDebug() << "[TupSearchDialog::getMiniature()] - Calling URL -> " << apiEntry;
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
    connect(reply, &QNetworkReply::downloadProgress, this, &TupSearchDialog::updateProgress);
    connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(slotError(QNetworkReply::NetworkError)));
    connect(reply, &QNetworkReply::finished, reply, &QNetworkReply::deleteLater);
    reply->setParent(manager);
    reply = manager->post(request, postData);

    #ifdef TUP_DEBUG
        qDebug() << "---";
    #endif
}

void TupSearchDialog::updateProgress(qint64 bytesReceived, qint64 bytesTotal)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupSearchDialog::updateProgress()] - bytesSent -> " << bytesReceived;
    #endif

    if (bytesTotal > 0) {
        double percent = (bytesReceived * 100) / bytesTotal;
        #ifdef TUP_DEBUG
            qDebug() << "[TupSearchDialog::updateProgress()] - percent -> " << percent;
        #endif
        progressBar->setValue(percent);
    }
}

void TupSearchDialog::resetProgress(StackId id)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupSearchDialog::resetProgress()]";
    #endif

    progressBar->reset();
    dynamicPanel->setCurrentIndex(id);
    searchButton->setEnabled(true);
    QApplication::restoreOverrideCursor();
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
                TOsd::self()->display(TOsd::Error, tr("Can't load the result images!"));
            }
        } else {
            #ifdef TUP_DEBUG
                qDebug() << "[TupSearchDialog::processMiniature()] - Fatal Error: Can't load image bytes!";
            #endif
            TOsd::self()->display(TOsd::Error, tr("Can't load the result images!"));
        }

        itemsCounter++;
        if (itemsCounter == assetList.count()) {
            assetDescList->setCurrentRow(0);
            resetProgress(Result);
        }
    } else {
        #ifdef TUP_DEBUG
            qDebug() << "[TupSearchDialog::processMiniature()] - Fatal Error: No answer from server!";
        #endif
        resetProgress(Error);
    }

    progressBar->reset();
}

void TupSearchDialog::updateAssetView(int index)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupSearchDialog::updateAssetView()] - index -> " << index;
    #endif

    if (!assetList.isEmpty()) {
        AssetRecord item = assetList.at(index);
        QString path = assetsPath + item.code + "/miniature.png";
        previewPic->setPixmap(QPixmap(path));

        graphicType->setText("<b>" + tr("Asset Extension:") + "</b> " + extStrings[item.ext.toInt()].toUpper());

        creator->setText("<b>" + tr("Creator:") + "</b> " + item.creator);
        setLabelLink(creatorUrl, item.creatorUrl);

        license->setText("<b>" + tr("License:") + "</b> " + item.licenseTitle);
        setLabelLink(licenseUrl, item.licenseUrl);

        move(static_cast<int>((screen->geometry().width() - width())/2),
             static_cast<int>((screen->geometry().height() - height())/2));
    } else {
        #ifdef TUP_DEBUG
            qDebug() << "[TupSearchDialog::updateAssetView()] - Fatal Error: Assets list is empty!";
        #endif
    }
}

void TupSearchDialog::setLabelLink(QLabel *label, const QString &url)
{
    label->setText("<a href=\"" + url + "\" " + linkStyle + ">" + url + "</a>");
    label->setTextFormat(Qt::RichText);
    label->setTextInteractionFlags(Qt::TextBrowserInteraction);
    label->setOpenExternalLinks(true);
}

void TupSearchDialog::getAsset()
{
    int index = assetDescList->currentRow();
    AssetRecord item = assetList.at(index);

    #ifdef TUP_DEBUG
        qDebug() << "[TupSearchDialog::getAsset()] - code -> " << item.code;
    #endif

    QString path = assetsPath + item.code + "/1.jpg";
    QFile assetFile(path);

    if (assetFile.exists(path)) {
        if (assetFile.open(QIODevice::ReadOnly)) {
            QByteArray data = assetFile.readAll();
            assetFile.close();
            int extId = item.ext.toInt();
            emit assetStored(item.code, static_cast<AssetType>(assetTypeCode), extStrings[extId], extId, data);
        } else {
            #ifdef TUP_DEBUG
                qWarning() << "[TupSearchDialog::getAsset()] - Fatal Error: can't open asset -> " << path;
            #endif
        }
    } else {
        QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

        QNetworkAccessManager *manager = new QNetworkAccessManager(this);
        connect(manager, &QNetworkAccessManager::finished, this, &TupSearchDialog::processAsset);
        connect(manager, &QNetworkAccessManager::finished, manager, &QNetworkAccessManager::deleteLater);

        QString apiEntry = LIBRARY_URL + QString("/api/item/");
        #ifdef TUP_DEBUG
            qDebug() << "[TupSearchDialog::getAsset()] - Getting URL -> " << apiEntry;
        #endif

        QUrl url(apiEntry);
        QNetworkRequest request = QNetworkRequest();
        request.setRawHeader("User-Agent", BROWSER_FINGERPRINT);
        request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
        request.setSslConfiguration(QSslConfiguration::defaultConfiguration());

        request.setUrl(QUrl(url));

        QUrlQuery params = QUrlQuery();
        params.addQueryItem("code", item.code);
        int type = assetType.toInt();
        if (type == 2 || type == 3)
            params.addQueryItem("dimension", dimension);

        QByteArray postData = params.query(QUrl::FullyEncoded).toUtf8();
        QNetworkReply *reply = manager->post(request, postData);
        connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(slotError(QNetworkReply::NetworkError)));
        connect(reply, &QNetworkReply::finished, reply, &QNetworkReply::deleteLater);
        reply->setParent(manager);
    }
}

void TupSearchDialog::processAsset(QNetworkReply *reply)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupSearchDialog::processAsset()]";
    #endif

    QByteArray data = reply->readAll();
    if (data.size() > 0) {
        QString code = reply->rawHeader("Code");
        QString name = reply->rawHeader("Name");
        int extId = reply->rawHeader("ExtId").toInt();
        #ifdef TUP_DEBUG
            qDebug() << "[TupSearchDialog::processAsset()] - Saving asset...";
            qDebug() << "[TupSearchDialog::processAsset()] - Code -> " << code;
            qDebug() << "[TupSearchDialog::processAsset()] - Extension -> " << extId;
            qDebug() << "[TupSearchDialog::processAsset()] - Object size -> " << data.size();
        #endif

        QString ext = extStrings[extId];
        QString path = assetsPath + code + "/1." + ext;
        switch(extId) {
          case JPG:
          case PNG:
            {
                if (saveImage(path, ext.toUpper().toUtf8(), data)) {
                    emit assetStored(name, static_cast<AssetType>(assetTypeCode), ext, extId, data);
                } else {
                    resetProgress(Error);
                    #ifdef TUP_DEBUG
                        qDebug() << "[TupSearchDialog::processAsset()] - Fatal Error: can't save asset! -> " << name;
                    #endif
                }
            }
          break;
          case SVG:
          case TOBJ:
            {
                if (saveAssetFile(path, data)) {
                    emit assetStored(name, static_cast<AssetType>(assetTypeCode), ext, extId, data);
                } else {
                    resetProgress(Error);
                    #ifdef TUP_DEBUG
                        qDebug() << "[TupSearchDialog::processAsset()] - Fatal Error: can't save asset! -> " << name;
                    #endif
                }
            }
          break;
        }
    } else {
        #ifdef TUP_DEBUG
            qDebug() << "[TupSearchDialog::processAsset()()] - Fatal Error: No answer from server!";
        #endif
        TOsd::self()->display(TOsd::Error, tr("Network Error 809. Please, contact us!"));
    }

    QApplication::restoreOverrideCursor();
}

bool TupSearchDialog::saveImage(const QString &path, const char *extension, const QByteArray &data)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupSearchDialog::saveImage()] - extension -> " << extension;
    #endif

    QImage image;
    if (image.loadFromData(data, extension)) {
        if (image.save(path, extension)) {
            #ifdef TUP_DEBUG
                qDebug() << "[TupSearchDialog::saveImage()] - Asset saved successfully! -> " << path;
            #endif
        } else {
            #ifdef TUP_DEBUG
                qDebug() << "[TupSearchDialog::saveImage()] - Can't save asset! -> " << path;
            #endif
            TOsd::self()->display(TOsd::Error, tr("Can't save the asset!"));
            return false;
        }
    } else {
        #ifdef TUP_DEBUG
            qDebug() << "[TupSearchDialog::saveImage()] - Fatal Error: Can't load image bytes!";
        #endif
        TOsd::self()->display(TOsd::Error, tr("Can't load the asset!"));
        return false;
    }

    return true;
}

bool TupSearchDialog::saveAssetFile(const QString path, const QByteArray &data)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupSearchDialog::saveAssetFile()] - path -> " << path;
    #endif

    QFile svg(path);
    if (svg.open(QIODevice::WriteOnly)) {
        svg.write(data);
        svg.close();

        #ifdef TUP_DEBUG
            qDebug() << "[TupSearchDialog::saveAssetFile()] - Asset saved successfully!";
        #endif
    } else {
        #ifdef TUP_DEBUG
            qDebug() << "[TupSearchDialog::saveAssetFile()] - Can't save the asset! -> " << path;
        #endif
        TOsd::self()->display(TOsd::Error, tr("Can't load the asset file!"));
    }

    return true;
}

void TupSearchDialog::openDonationLink()
{
    QUrl url("https://paypal.me/maefloresta");
    QDesktopServices::openUrl(url);
}

void TupSearchDialog::openPatreonLink()
{
    QUrl url("https://www.patreon.com/maefloresta");
    QDesktopServices::openUrl(url);
}

void TupSearchDialog::openCreditsLink()
{
    QUrl url("https://www.maefloresta.com/credits");
    QDesktopServices::openUrl(url);
}

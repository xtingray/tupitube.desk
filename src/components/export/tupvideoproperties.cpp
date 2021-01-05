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

#include "tupvideoproperties.h"
#include "tconfig.h"
#include "talgorithm.h"
#include "tosd.h"

#include <QNetworkAccessManager>
#include <QUrlQuery>
#include <QHttpPart>
#include <QDomDocument>

TupVideoProperties::TupVideoProperties(Mode m) : TupExportWizardPage(tr("Animation Properties"))
{
    setTag("PROPERTIES");
    mode = m;
    aborted = false;
    setWindowParams();
    stackedWidget = new QStackedWidget;

    setForm();
    setProgressBar();

    stackedWidget->setCurrentIndex(0);
    setWidget(stackedWidget);
}

TupVideoProperties::~TupVideoProperties()
{
}

void TupVideoProperties::setForm()
{
    formWidget = new QWidget;
    QVBoxLayout *formLayout = new QVBoxLayout(formWidget);
    QLocale utf(QLocale::AnyLanguage, QLocale::AnyCountry);

    QLabel *titleLabel = new QLabel(tr("Title"));
    QString title = tr("My Animation");
    if (mode == Image)
        title = tr("My Image");

    titleEdit = new QLineEdit(title);
    titleEdit->setLocale(utf);
    connect(titleEdit, SIGNAL(textChanged(const QString &)), this, SLOT(resetTitleColor(const QString &)));
    titleLabel->setBuddy(titleEdit);

    QLabel *tagsLabel = new QLabel(tr("Hashtags"));
    QString tags = tr("#tupitube #animation #fun");
    if (mode == Image)
        tags = tr("#tupitube #image #fun");

    tagsEdit = new QLineEdit(tags);
    tagsEdit->setLocale(utf);
    connect(tagsEdit, SIGNAL(textChanged(const QString &)), this, SLOT(resetTagsColor(const QString &)));
    tagsLabel->setBuddy(tagsEdit);

    QLabel *descLabel = new QLabel(tr("Description"));

    defaultDesc = tr("Create and share animations easily with TupiTube");

    descText = new QTextEdit;
    descText->setLocale(utf);
    descText->setAcceptRichText(false);
    descText->setFixedHeight(80);
    descText->setText(defaultDesc);

    formLayout->addWidget(titleLabel);
    formLayout->addWidget(titleEdit);
    formLayout->addWidget(tagsLabel);
    formLayout->addWidget(tagsEdit);
    formLayout->addWidget(descLabel);
    formLayout->addWidget(descText);

    stackedWidget->addWidget(formWidget);
}

void TupVideoProperties::setProgressBar()
{
    // Progress Widget
    progressWidget = new QWidget;
    QHBoxLayout *progressLayout = new QHBoxLayout(progressWidget);

    TCONFIG->beginGroup("General");
    QString themeName = TCONFIG->value("Theme", "Light").toString();
    QString style = "QProgressBar { background-color: #DDDDDD; "
                    "text-align: center; color: #FFFFFF; border-radius: 2px; } ";
    QString color = "#009500";
    if (themeName.compare("Dark") == 0)
        color = "#444444";
    style += "QProgressBar::chunk { background-color: " + color + "; border-radius: 2px; }";

    progressBar = new QProgressBar;
    progressBar->setTextVisible(true);
    progressBar->setStyleSheet(style);
    progressBar->setRange(1, 100);

    progressLayout->addSpacing(50);
    progressLayout->addWidget(progressBar);
    progressLayout->addSpacing(50);

    // Cancel Button Widget
    QWidget *cancelWidget = new QWidget;
    QHBoxLayout *cancelLayout = new QHBoxLayout(cancelWidget);

    QPushButton *cancelButton = new QPushButton(tr("Cancel"));
    connect(cancelButton, SIGNAL(clicked()), this, SLOT(cancelPost()));

    cancelLayout->addStretch();
    cancelLayout->addWidget(cancelButton);
    cancelLayout->addStretch();

    // Main Widget
    QWidget *mainWidget = new QWidget;
    QVBoxLayout *mainLayout = new QVBoxLayout(mainWidget);
    mainLayout->addStretch();
    mainLayout->addWidget(progressWidget);
    mainLayout->addWidget(cancelWidget);
    mainLayout->addStretch();

    mainWidget->setVisible(false);

    stackedWidget->addWidget(mainWidget);
}

bool TupVideoProperties::isComplete() const
{
    return true;
}

void TupVideoProperties::reset()
{
}

QString TupVideoProperties::title() const
{
     QString title = QString::fromUtf8(titleEdit->text().toUtf8());
     return title;
}

QString TupVideoProperties::hashtags() const
{
     QString topics = QString::fromUtf8(tagsEdit->text().toUtf8());
     QStringList tags = topics.split(" ");
     topics = "";
     foreach(QString item, tags) {
         if (!item.startsWith("#"))
             item = "#" + item;
         topics += item + " ";
     }
     topics = topics.simplified();

     return topics;
}

QString TupVideoProperties::description() const
{
     QString description = QString::fromUtf8(descText->toPlainText().toUtf8());
     return description;
}

QList<int> TupVideoProperties::scenesList() const
{
     return scenes;
}

void TupVideoProperties::setProjectParams(const QString &login, const QString &secret, const QString &path)
{
     username = login;
     password = secret;
     filePath = path;
}

void TupVideoProperties::postIt()
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupVideoProperties::postIt()]";
    #endif

    QString title = titleEdit->text();
    QString tags = tagsEdit->text();
    QString desc = descText->toPlainText();
    if (username.compare("tupitube") == 0)
        flag = "tupitube";

    QString titleMessage = tr("Set a title for the post here!");
    if (title.length() == 0 || (title.compare(titleMessage) == 0)) {
        titleEdit->setText(titleMessage);
        titleEdit->selectAll();
        TOsd::self()->display(TOsd::Error, tr("Title is missing!"));
        return;
    }

    QString tagsMessage = tr("Set some topic tags for the post here!");
    if (tags.length() == 0 || (tags.compare(tagsMessage) == 0)) {
        tagsEdit->setText(tagsMessage);
        tagsEdit->selectAll();
        TOsd::self()->display(TOsd::Error, tr("Tags are missing!"));
        return;
    } else {
        tags = hashtags();
    }

    if (desc.length() > 0) {
        if (desc.contains("<") || desc.contains(">") || desc.contains("http"))
            desc = "";

        if (desc.length() > 500) {
            desc = desc.left(500);
            descText->setPlainText(desc);
        }

        if (desc.compare(defaultDesc) == 0)
            desc = formatPromoComment();
    } else {
        desc = formatPromoComment();
    }

    #ifdef TUP_DEBUG
        qDebug() << "[TupVideoProperties::postIt()] - Tags -> " << tags;
        qDebug() << "[TupVideoProperties::postIt()] - Comment -> " << desc;
    #endif

    stackedWidget->setCurrentIndex(1);
    emit postHasStarted();

    QNetworkAccessManager *manager = new QNetworkAccessManager(this);
    connect(manager, &QNetworkAccessManager::finished, this, &TupVideoProperties::serverAuthAnswer);
    connect(manager, &QNetworkAccessManager::finished, manager, &QNetworkAccessManager::deleteLater);

    QString apiEntry = TUPITUBE_URL + QString("/api/desk/add/video/");
    if (mode == Image)
        apiEntry = TUPITUBE_URL + QString("/api/desk/add/image/");

    #ifdef TUP_DEBUG
        qDebug() << "[TupVideoProperties::postIt()] - URL -> " << apiEntry;
    #endif

    QUrl url(apiEntry);
    QNetworkRequest request = QNetworkRequest();
    request.setRawHeader("User-Agent", BROWSER_FINGERPRINT);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    request.setSslConfiguration(QSslConfiguration::defaultConfiguration());

    /*
    QSslConfiguration conf = request.sslConfiguration();
    conf.setPeerVerifyMode(QSslSocket::VerifyNone);
    request.setSslConfiguration(conf);
    */

    request.setUrl(QUrl(url));

    params = QUrlQuery();
    params.addQueryItem("username", username);
    params.addQueryItem("password", password);
    params.addQueryItem("title", title);
    params.addQueryItem("tags", tags);
    params.addQueryItem("desc", desc);
    params.addQueryItem("content", flag);

    QByteArray postData = params.query(QUrl::FullyEncoded).toUtf8();
    QNetworkReply *reply = manager->post(request, postData);
    connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(slotError(QNetworkReply::NetworkError)));
    connect(this, &TupVideoProperties::postAborted, reply, &QNetworkReply::abort);
    reply->setParent(manager);
}

QString TupVideoProperties::formatPromoComment() const
{
    QString desc = "promo.en";

    QStringList langSupport;
    langSupport << "es" << "pt";
    QString locale = QString(QLocale::system().name()).left(2);

    if (locale.compare("en") != 0 && langSupport.contains(locale))
        desc = "promo." + locale;

    return desc;
}

void TupVideoProperties::serverAuthAnswer(QNetworkReply *reply)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupVideoProperties::serverAuthAnswer()]";
    #endif

    /*
      <auth>
        <code></code>
      </auth>
    */

    QByteArray bArray = reply->readAll();
    QString answer(bArray);
    if (answer.length() > 0) {        
        #ifdef TUP_DEBUG
            qDebug() << "[TupVideoProperties::serverAuthAnswer()] - answer -> " << answer;
        #endif

        QDomDocument doc;
        if (doc.setContent(answer)) {
            QDomNode root = doc.documentElement();
            QDomElement element = root.firstChildElement("code");
            QString projectCode = element.text();

            if (projectCode.length() > 0) {
                QNetworkAccessManager *manager = new QNetworkAccessManager(this);
                connect(manager, &QNetworkAccessManager::finished, this, &TupVideoProperties::closeRequest);
                connect(manager, &QNetworkAccessManager::finished, manager, &QNetworkAccessManager::deleteLater);

                QString apiEntry = TUPITUBE_URL + QString("/api/desk/upload/video/");
                if (mode == Image)
                    apiEntry = TUPITUBE_URL + QString("/api/desk/upload/image/");

                #ifdef TUP_DEBUG
                    qDebug() << "[TupVideoProperties::serverAuthAnswer()] - URL -> " << apiEntry;
                #endif

                QUrl url(apiEntry);
                QNetworkRequest request = QNetworkRequest();
                request.setRawHeader(QByteArray("User-Agent"), QByteArray(BROWSER_FINGERPRINT));
                request.setSslConfiguration(QSslConfiguration::defaultConfiguration());

                /*
                QSslConfiguration conf = request.sslConfiguration();
                conf.setPeerVerifyMode(QSslSocket::VerifyNone);
                request.setSslConfiguration(conf);
                */

                request.setUrl(QUrl(url));

                QHttpMultiPart *multiPart = new QHttpMultiPart(QHttpMultiPart::FormDataType);

                QHttpPart loginPart;
                loginPart.setHeader(QNetworkRequest::ContentTypeHeader, QVariant("text/plain"));
                loginPart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"username\""));
                loginPart.setBody(username.toUtf8());

                QHttpPart passwdPart;
                passwdPart.setHeader(QNetworkRequest::ContentTypeHeader, QVariant("text/plain"));
                passwdPart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"password\""));
                passwdPart.setBody(password.toUtf8());

                QHttpPart codePart;
                codePart.setHeader(QNetworkRequest::ContentTypeHeader, QVariant("text/plain"));
                codePart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"project\""));
                codePart.setBody(projectCode.toUtf8());

                QHttpPart contentPart;
                contentPart.setHeader(QNetworkRequest::ContentTypeHeader, QVariant("text/plain"));
                contentPart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"content\""));
                contentPart.setBody(flag.toUtf8());

                QHttpPart filePart;
                filePart.setHeader(QNetworkRequest::ContentDispositionHeader,
                                   QVariant("form-data; name=\"file\"; filename=\"" + projectCode + ".tup\""));
                filePart.setHeader(QNetworkRequest::ContentTypeHeader, QVariant("application/octet-stream"));

                QFile *projectFile = new QFile(filePath);
                projectFile->open(QIODevice::ReadOnly);

                filePart.setBodyDevice(projectFile);
                projectFile->setParent(multiPart);

                multiPart->append(loginPart);
                multiPart->append(passwdPart);
                multiPart->append(codePart);
                if (mode == Video) {
                    QString scenesStr = "";
                    int total = scenes.count();
                    if (total == 1) {
                        scenesStr += QString::number(scenes.at(0));
                    } else {
                        for (int i=0; i < total; i++) {
                            scenesStr += QString::number(scenes.at(i));
                            scenesStr += ",";
                        }
                        scenesStr.chop(1);
                    }

                    QHttpPart scenesPart;
                    scenesPart.setHeader(QNetworkRequest::ContentTypeHeader, QVariant("text/plain"));
                    scenesPart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"scenes\""));
                    scenesPart.setBody(scenesStr.toUtf8());

                    multiPart->append(scenesPart);
                }
                multiPart->append(contentPart);
                multiPart->append(filePart);

                QNetworkReply *projectReply = manager->post(request, multiPart);
                connect(projectReply, &QNetworkReply::uploadProgress, this, &TupVideoProperties::tracingPostProgress);
                connect(this, &TupVideoProperties::postAborted, projectReply, &QNetworkReply::abort);
                multiPart->setParent(projectReply);
                projectReply->setParent(manager);
            } else {
                element = root.firstChildElement("error");
                QString error = element.text();
                if (error.length() > 0) {
                    int errorCode = error.toInt();
                    switch (errorCode) {
                        case 401:
                        {
                            // Invalid password
                            #ifdef TUP_DEBUG
                                qDebug() << "[TupVideoProperties::serverAuthAnswer()] - Error: Invalid credentials!";
                            #endif
                            cancelPost();
                            TAlgorithm::resetCacheID();
                            TOsd::self()->display(TOsd::Error, tr("Access denied. Invalid password!"));
                        }
                        break;
                        case 500:
                        {
                            // Server side error. No root path
                            #ifdef TUP_DEBUG
                                qDebug() << "[TupVideoProperties::serverAuthAnswer()] - Error: No root path on server!";
                            #endif
                            cancelPost();
                            TOsd::self()->display(TOsd::Error, tr("Network Error 500. Please, contact us!"));
                        }
                        break;
                        case 501:
                        {
                            // Can't create db record
                            #ifdef TUP_DEBUG
                                qDebug() << "[TupVideoProperties::serverAuthAnswer()] - Error: Can't create db record!";
                            #endif
                            cancelPost();
                            TOsd::self()->display(TOsd::Error, tr("Network Error 501. Please, contact us!"));
                        }
                        break;
                        case 502:
                        {
                            // Can't get uid
                            #ifdef TUP_DEBUG
                                qDebug() << "[TupVideoProperties::serverAuthAnswer()] - Error: Can't get uid!";
                            #endif
                            cancelPost();
                            TOsd::self()->display(TOsd::Error, tr("Network Error 502. Please, contact us!"));
                        }
                        break;
                        case 503:
                        {
                            // Can't get filename string
                            #ifdef TUP_DEBUG
                                qDebug() << "[TupVideoProperties::serverAuthAnswer()] - Error: Can't generate filename string!";
                            #endif
                            cancelPost();
                            TOsd::self()->display(TOsd::Error, tr("Network Error 503. Please, contact us!"));
                        }
                        break;
                        case 504:
                        {
                            // Incomplete request
                            #ifdef TUP_DEBUG
                                qDebug() << "[TupVideoProperties::serverAuthAnswer()] - Error: Incomplete request!";
                            #endif
                            cancelPost();
                            TOsd::self()->display(TOsd::Error, tr("Network Error 504. Please, contact us!"));
                        }
                        break;
                        case 505:
                        {
                            // Invalid client
                            #ifdef TUP_DEBUG
                                qDebug() << "[TupVideoProperties::serverAuthAnswer()] - Error: Invalid client!";
                            #endif
                            cancelPost();
                            TOsd::self()->display(TOsd::Error, tr("Network Error 505. Please, contact us!"));
                        }
                        break;
                        default:
                        {
                            // Unknown code error
                            #ifdef TUP_DEBUG
                                qDebug() << "[TupVideoProperties::serverAuthAnswer()] - Unknown error! :/";
                            #endif
                            cancelPost();
                            TOsd::self()->display(TOsd::Error, tr("Network Error 506. Please, contact us!"));
                        }
                    }
                } else {
                    // Unknown answer - no error
                    #ifdef TUP_DEBUG
                        qDebug() << "[TupVideoProperties::serverAuthAnswer()] - Error: Answer package is corrupt!";
                    #endif
                    cancelPost();
                    TOsd::self()->display(TOsd::Error, tr("Network Error 507. Please, contact us!"));
                }
            }
        } else {
            // Invalid answer - no xml
            #ifdef TUP_DEBUG
                qDebug() << "[TupVideoProperties::serverAuthAnswer()] - Error: Invalid answer format!";
            #endif
            cancelPost();
            TOsd::self()->display(TOsd::Error, tr("Network Error 508. Please, contact us!"));
        }
    } else {
        #ifdef TUP_DEBUG
            qDebug() << "[TupVideoProperties::serverAuthAnswer()] - Error: No answer from server!";
        #endif
        cancelPost();
        TOsd::self()->display(TOsd::Error, tr("Network Error 509. Please, contact us!"));
    }
}

void TupVideoProperties::tracingPostProgress(qint64 bytesSent, qint64 bytesTotal)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupVideoProperties::tracingPostProgress()]";
    #endif

    if (aborted)
        return;

    if (bytesTotal > 0) {
        double percent = (bytesSent * 100) / bytesTotal;
        #ifdef TUP_DEBUG
            qDebug() << "[TupVideoProperties::tracingPostProgress()] - percent -> " << percent;
        #endif
        progressBar->setValue(percent);
    }
}

void TupVideoProperties::closeRequest(QNetworkReply *reply)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupVideoProperties::closeRequest()]";
    #endif

    /*
      <result>
        <code></code>
      </result>
    */

    if (aborted) {
        #ifdef TUP_DEBUG
            qDebug() << "[TupVideoProperties::closeRequest()] - Aborted by user!";
        #endif
        TOsd::self()->display(TOsd::Info, tr("Post action cancelled!"));
        return;
    }

    QByteArray bArray = reply->readAll();
    QString answer(bArray);
    if (answer.length() > 0) {
        #ifdef TUP_DEBUG
            qDebug() << "[TupVideoProperties::closeRequest()] - answer -> " << answer;
        #endif

        QDomDocument doc;
        if (doc.setContent(answer)) {
            QDomNode root = doc.documentElement();
            QDomElement element = root.firstChildElement("code");
            QString code = element.text();
            if (code.length() > 0) {
                int key = code.toInt();
                switch (key) {
                    case 200:
                    {
                        // Succeed operation
                        #ifdef TUP_DEBUG
                            qDebug() << "[TupVideoProperties::closeRequest()] - Project posted successfully!";
                        #endif

                        if (mode == Image) {
                            QFile file(filePath);
                            if (!file.remove()) {
                                #ifdef TUP_DEBUG
                                    qDebug() << "[TupVideoProperties::closeRequest()] - Removing temp file -> " << filePath;
                                    qDebug() << "[TupVideoProperties::closeRequest()] - Error: Can't remove project file -> " << filePath;
                                #endif
                            }
                        }

                        TOsd::self()->display(TOsd::Info, tr("Project was uploaded successfully!"));
                    }
                    break;
                    case 400:
                    {
                        // No root path to store files
                        #ifdef TUP_DEBUG
                            qDebug() << "[TupVideoProperties::closeRequest()] - Error: No root path to store files!";
                        #endif
                        TOsd::self()->display(TOsd::Error, tr("Network Error 400. Please, contact us!"));
                    }
                    break;
                    case 401:
                    {
                        // Invalid password
                        #ifdef TUP_DEBUG
                            qDebug() << "[TupVideoProperties::closeRequest()] - Error: Invalid credentials!";
                        #endif
                        TOsd::self()->display(TOsd::Error, tr("Access denied. Invalid password!"));
                    }
                    break;
                    case 402:
                    {
                        // Can't create scenes file
                        #ifdef TUP_DEBUG
                            qDebug() << "[TupVideoProperties::closeRequest()] - Error: Can't create scenes file!";
                        #endif
                        TOsd::self()->display(TOsd::Error, tr("Network Error 402. Please, contact us!"));
                    }
                    break;
                    case 403:
                    {
                        // Can't update database
                        #ifdef TUP_DEBUG
                            qDebug() << "[TupVideoProperties::closeRequest()] - Error: Can't update database!";
                        #endif
                        TOsd::self()->display(TOsd::Error, tr("Network Error 403. Please, contact us!"));
                    }
                    break;
                    case 404:
                    {
                        // Can't store file on server
                        #ifdef TUP_DEBUG
                            qDebug() << "[TupVideoProperties::closeRequest()] - Error: Can't store file!";
                        #endif
                        TOsd::self()->display(TOsd::Error, tr("Network Error 404. Please, contact us!"));
                    }
                    break;
                    case 405:
                    {
                        // Incomplete form request
                        #ifdef TUP_DEBUG
                            qDebug() << "[TupVideoProperties::closeRequest()] - Error: Incomplete form request!";
                        #endif
                        TOsd::self()->display(TOsd::Error, tr("Network Error 405. Please, contact us!"));
                    }
                    break;
                    case 406:
                    {
                        // Invalid browser
                        #ifdef TUP_DEBUG
                            qDebug() << "[TupVideoProperties::closeRequest()] - Error: Invalid browser!";
                        #endif
                        TOsd::self()->display(TOsd::Error, tr("Network Error 406. Please, contact us!"));
                    }
                    break;
                }
            } else {
                // No code - Invalid answer - 407
                #ifdef TUP_DEBUG
                    qDebug() << "[TupVideoProperties::closeRequest()] - Error: Invalid answer. No answer code!";
                #endif
                TOsd::self()->display(TOsd::Error, tr("Network Error 407. Please, contact us!"));
            }
        } else {
            // Invalid answer (no xml) - 408
            #ifdef TUP_DEBUG
                qDebug() << "[TupVideoProperties::closeRequest()] - Error: Invalid answer. No XML format!";
            #endif
            TOsd::self()->display(TOsd::Error, tr("Network Error 408. Please, contact us!"));
        }
    } else {
        // Empty answer - 409
        #ifdef TUP_DEBUG
            qDebug() << "[TupVideoProperties::closeRequest()] - Error: No answer from server!";
        #endif
        TOsd::self()->display(TOsd::Error, tr("Network Error 409. Please, contact us!"));
    }

    emit isDone();
}

void TupVideoProperties::cancelPost()
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupVideoProperties::cancelPost()]";
    #endif

    aborted = true;
    emit postAborted();
    emit isDone();
}

void TupVideoProperties::slotError(QNetworkReply::NetworkError error)
{
    TOsd::self()->display(TOsd::Error, tr("Network Fatal Error. Please, contact us!"));

    switch (error) {
        case QNetworkReply::HostNotFoundError:
             {
             #ifdef TUP_DEBUG
                 qDebug() << "[TupVideoProperties::slotError()] - Network Error: Host not found";
             #endif
             }
        break;
        case QNetworkReply::TimeoutError:
             {
             #ifdef TUP_DEBUG
                 qDebug() << "[TupVideoProperties::slotError()] - Network Error: Time out!";
             #endif
             }
        break;
        case QNetworkReply::ConnectionRefusedError:
             {
             #ifdef TUP_DEBUG
                 qDebug() << "[TupVideoProperties::slotError()] - Network Error: Connection Refused!";
             #endif
             }
        break;
        case QNetworkReply::ContentNotFoundError:
             {
             #ifdef TUP_DEBUG
                 qDebug() << "[TupVideoProperties::slotError()] - Network Error: Content not found!";
             #endif
             }
        break;
        case QNetworkReply::UnknownNetworkError:
        default:
             {
             #ifdef TUP_DEBUG
                 qDebug() << "[TupVideoProperties::slotError()] - Network Error: Unknown Network error!";
             #endif
             }
        break;
    }

    emit isDone();
}

void TupVideoProperties::setWindowParams()
{
    flag = TAlgorithm::windowCacheID();
}

void TupVideoProperties::resetTitleColor(const QString &)
{
    QPalette palette = titleEdit->palette();
    if (titleEdit->text().length() > 0
        && titleEdit->text().compare(tr("Set a title for the picture here!")) != 0)
        palette.setBrush(QPalette::Base, Qt::white);
    else
        palette.setBrush(QPalette::Base, QColor(255, 140, 138));

    titleEdit->setPalette(palette);
}

void TupVideoProperties::resetTagsColor(const QString &)
{
    QPalette palette = tagsEdit->palette();
    if (tagsEdit->text().length() > 0 &&
        tagsEdit->text().compare(tr("Set some topic tags for the picture here!")) != 0)
        palette.setBrush(QPalette::Base, Qt::white);
    else
        palette.setBrush(QPalette::Base, QColor(255, 140, 138));

    tagsEdit->setPalette(palette);
}

void TupVideoProperties::setScenesIndexes(const QList<int> &indexes)
{
    scenes = indexes;
}

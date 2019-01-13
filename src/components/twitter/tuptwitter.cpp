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

#include "tuptwitter.h"
#include "tconfig.h"
#include "talgorithm.h"

#include <QDomDocument>
#include <QSslConfiguration>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QNetworkAccessManager>
#include <QFile>

QString TupTwitter::IS_HOST_UP_URL = QString("updates/test.xml");
QString TupTwitter::USER_TIMELINE_URL = QString("updates/tweets.php");
QString TupTwitter::TUPITUBE_VERSION_URL = QString("updates/current_version.xml");
QString TupTwitter::TUPITUBE_WEB_MSG = QString("updates/web_msg.");
QString TupTwitter::TUPITUBE_VIDEOS = QString("updates/videos.xml");
QString TupTwitter::TUPITUBE_IMAGES = QString("updates/images/");
QString TupTwitter::BROWSER_FINGERPRINT = QString("Tupi_Browser 2.0");

struct TupTwitter::Private
{
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

TupTwitter::TupTwitter(QWidget *parent) : QWidget(parent), k(new Private)
{
    /*
    #ifdef TUP_DEBUG
        qDebug() << "TupTwitter() - SSL version use for build: " << QSslSocket::sslLibraryBuildVersionString();
        qDebug() << "TupTwitter() - SSL version use for run-time: " << QSslSocket::sslLibraryVersionNumber();
        qDebug() << "TupTwitter() - Library Paths: " << QCoreApplication::libraryPaths();
    #endif
    */

    k->update = false;
    TCONFIG->beginGroup("General");
    k->themeName = TCONFIG->value("Theme", "Light").toString();
    k->showAds = TCONFIG->value("ShowAds", true).toBool();

    k->locale = QString(QLocale::system().name()).left(2);
    if (k->locale.length() < 2) {
        k->locale = "en";
    } else {
        QList<QString> localeSupport;
        localeSupport << "en" << "es" << "pt";
        if (!localeSupport.contains(k->locale))
            k->locale = "en";
    }
}

void TupTwitter::start()
{
    QString url = MAEFLORESTA_URL + IS_HOST_UP_URL;

    #ifdef TUP_DEBUG
        QString msg = "TupTwitter::start() - Getting news updates...";
        #ifdef Q_OS_WIN
            qWarning() << msg;
        #else
            tWarning() << msg;
        #endif
    #endif

    k->manager = new QNetworkAccessManager(this);
    connect(k->manager, SIGNAL(finished(QNetworkReply*)), this, SLOT(closeRequest(QNetworkReply*)));

    k->request.setSslConfiguration(QSslConfiguration::defaultConfiguration());
    k->request.setUrl(QUrl(url));
    k->request.setRawHeader("User-Agent", BROWSER_FINGERPRINT.toLatin1());

    k->reply = k->manager->get(k->request);
    connect(k->reply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(slotError(QNetworkReply::NetworkError)));
}

TupTwitter::~TupTwitter()
{
    #ifdef TUP_DEBUG
        #ifdef Q_OS_WIN
            qDebug() << "[~TupTwitter()]";
        #else
            TEND;
        #endif
    #endif

    delete k->manager;
    k->manager = NULL;
    delete k->reply;
    k->reply = NULL;
    delete k;
}

void TupTwitter::requestFile(const QString &target)
{
    #ifdef TUP_DEBUG
        QString msg = "TupTwitter::requestFile() - Requesting url -> " + target;
        #ifdef Q_OS_WIN
            qWarning() << msg;
        #else
            tWarning() << msg;
        #endif
    #endif

    k->request.setUrl(QUrl(target));
    k->request.setRawHeader("User-Agent", BROWSER_FINGERPRINT.toLatin1());
    k->reply = k->manager->get(k->request);
}

void TupTwitter::closeRequest(QNetworkReply *reply)
{
    #ifdef TUP_DEBUG
        QString msg = "TupTwitter::closeRequest() - Getting answer from request...";
        #ifdef Q_OS_WIN
            qWarning() << msg;
        #else
            tWarning() << msg;
        #endif
    #endif

    QByteArray array = reply->readAll();

    QString imageName = reply->url().fileName();
    if (imageName.endsWith(".png", Qt::CaseInsensitive)) {
        QString imgPath = QDir::homePath() + "/." + QCoreApplication::applicationName() + "/images/";
        QDir dir(imgPath);
        if (!dir.exists())
            dir.mkpath(imgPath);

        QString image = imgPath + imageName;
        QFile file(image);
        if (file.open(QIODevice::WriteOnly)) {
            file.write(array);
            file.close();
        }

        requestFile(MAEFLORESTA_URL + TUPITUBE_VIDEOS);
        return;
    }

    QString answer(array);
    answer.chop(1);

    if (answer.length() > 0) {
        if (answer.compare("<ok>true</ok>") == 0) { // The webserver data is available! 
            requestFile(MAEFLORESTA_URL + TUPITUBE_VERSION_URL);
        } else {
            if (answer.startsWith("<version>")) { // Processing TupiTube versioning data
                checkSoftwareUpdates(array);

                TCONFIG->beginGroup("General");
                QString id = TCONFIG->value("ClientID", "0").toString();
                if (id.compare("0") == 0 || !TAlgorithm::isKeyRandomic(id)) {
                    id = TAlgorithm::randomString(20); 
                    TCONFIG->setValue("ClientID", id);
                }

                QString os = "unknown" ;
                #ifdef Q_OS_LINUX
                    os = "linux";
                #elif defined(Q_OS_MAC)
                    os = "osx";
                #elif defined(Q_OS_WIN)
                    os = "win";
                #endif

                requestFile(MAEFLORESTA_URL + USER_TIMELINE_URL + "?id=" + id + "&os=" + os + "&v=" + kAppProp->version()
                            + "." + kAppProp->revision());
            } else {
                if (answer.startsWith("<div")) { // Getting Twitter records 
                    if (!array.isNull()) {
                        formatStatus(array);
                    } else {
                        #ifdef TUP_DEBUG
                            QString msg = "TupTwitter::closeRequest() - Network Error: Twitter output is NULL!";
                            #ifdef Q_OS_WIN
                                qDebug() << msg;
                            #else
                                tError() << msg;
                            #endif
                        #endif
                    }
                    requestFile(MAEFLORESTA_URL + TUPITUBE_WEB_MSG + k->locale + ".html");
                } else {
                    if (answer.startsWith("<webmsg>")) { // Getting web msg
                        if (k->showAds) {
                            saveFile(answer, "webmsg.html");
                            if (answer.contains("<image>")) {
                                QString code = getImageCode(answer) + ".png";
                                if (!code.isEmpty()) {
                                    QString imgPath = QDir::homePath() + "/." + QCoreApplication::applicationName() + "/images/" + code;
                                    if (!QFile::exists(imgPath)) {
                                        requestFile(MAEFLORESTA_URL + TUPITUBE_IMAGES + code);
                                        return;
                                    }
                                }
                            }
                        }
                        requestFile(MAEFLORESTA_URL + TUPITUBE_VIDEOS);
                    } else {
                        if (answer.startsWith("<youtube>")) { // Getting video list
                            saveFile(answer, "videos.xml");
                            k->reply->deleteLater();
                            k->manager->deleteLater();
                        } else {
                            #ifdef TUP_DEBUG
                                QString msg = "TupTwitter::closeRequest() - Network Error: Invalid data!";
                                #ifdef Q_OS_WIN
                                    qDebug() << msg;
                                #else
                                    tError() << msg;
                                #endif
                            #endif
                        }
                    }
                }
            }
        }
    } else {
        #ifdef TUP_DEBUG
            QString msg = "TupTwitter::closeRequest() - Network Error: Gosh! No Internet? :S";
            #ifdef Q_OS_WIN
                qDebug() << msg;
            #else
                tError() << msg;
            #endif
        #endif
    } 
}

void TupTwitter::slotError(QNetworkReply::NetworkError error)
{
    switch (error) {
            case QNetworkReply::HostNotFoundError:
                 { 
                 #ifdef TUP_DEBUG
                     QString msg = "TupTwitter::slotError() - Network Error: Host not found";
                     #ifdef Q_OS_WIN
                            qDebug() << msg;
                     #else
                            tError() << msg;
                     #endif
                 #endif
                 }
            break;
            case QNetworkReply::TimeoutError:
                 {
                 #ifdef TUP_DEBUG
                     QString msg = "TupTwitter::slotError() - Network Error: Time out!";
                     #ifdef Q_OS_WIN
                            qDebug() << msg;
                     #else
                            tError() << msg;
                     #endif
                 #endif
                 }
            break;
            case QNetworkReply::ConnectionRefusedError:
                 {
                 #ifdef TUP_DEBUG
                     QString msg = "TupTwitter::slotError() - Network Error: Connection Refused!";
                     #ifdef Q_OS_WIN
                            qDebug() << msg;
                     #else
                            tError() << msg;
                     #endif
                 #endif
                 }
            break;
            case QNetworkReply::ContentNotFoundError:
                 {
                 #ifdef TUP_DEBUG
                     QString msg = "TupTwitter::slotError() - Network Error: Content not found!";
                     #ifdef Q_OS_WIN
                            qDebug() << msg;
                     #else
                            tError() << msg;
                     #endif
                 #endif
                 }
            break;
            case QNetworkReply::UnknownNetworkError:
            default:
                 {
                 #ifdef TUP_DEBUG
                     QString msg = "TupTwitter::slotError() - Network Error: Unknown Network error!";
                     #ifdef Q_OS_WIN
                            qDebug() << msg;
                     #else
                            tError() << msg;
                     #endif
                 #endif
                 }
            break;
    }
}

void TupTwitter::checkSoftwareUpdates(QByteArray array)
{
    #ifdef TUP_DEBUG
        QString msg = "TupTwitter::checkSoftwareUpdates() - Processing updates file...";
        #ifdef Q_OS_WIN
            qWarning() << msg;
        #else
            tWarning() << msg;
        #endif
    #endif

    QDomDocument doc;

    if (doc.setContent(array)) {
        QDomElement root = doc.documentElement();
        QDomNode n = root.firstChild();

        while (!n.isNull()) {
            QDomElement e = n.toElement();
            if (!e.isNull()) {
                if (e.tagName() == "branch") {
                    k->version = e.text();
                    if (k->version.compare(kAppProp->version())!=0)
                        k->update = true;
                } else if (e.tagName() == "rev") {
                    k->revision = e.text();
                    if (k->revision.compare(kAppProp->revision())!=0)
                        k->update = true;
                } else if (e.tagName() == "codeName") {
                    k->codeName = e.text();
                }
            }
            n = n.nextSibling();
        }

        #ifdef TUP_DEBUG
            QString msg1 = "TupTwitter::checkSoftwareUpdates() - Update Flag: " + QString::number(k->update);
            QString msg2 = "Server Version: " + k->version + " - Revision: " + k->revision + " - Code Name: " + k->codeName;
            QString msg3 = "Local Version: " + kAppProp->version() + " - Revision: " + kAppProp->revision();
            #ifdef Q_OS_WIN
                qWarning() << msg1;
                qWarning() << msg2;
                qWarning() << msg3;
            #else
                tWarning() << msg1;
                tWarning() << msg2;
                tWarning() << msg3;
            #endif
        #endif

        emit newUpdate(k->update);
    }
}

void TupTwitter::formatStatus(QByteArray array)
{
    #ifdef TUP_DEBUG
        QString msg = "TupTwitter::formatStatus() - Formatting news file...";
        #ifdef Q_OS_WIN
            qWarning() << msg;
        #else
            tWarning() << msg;
        #endif
    #endif

    QString tweets = QString(array);
    QString html = "";

    html += "<html>\n";
    html += "<head>\n";
    html += "<meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\">\n";
    html += "<link rel=\"stylesheet\" type=\"text/css\" href=\"file:tupitube.css\">\n";
    html += "</head>\n";

    if (k->themeName.compare("Dark") == 0) {
        html += "<body class=\"twitter_gray\">\n";
        html += "<div class=\"tupi_background5\">";
    } else {
        html += "<body class=\"twitter_white\">\n";
        html += "<div class=\"tupi_background1\">";
    }

    html += "<center><img src=\"file:maefloresta.png\" alt=\"maefloresta\"/></center>\n";
    html += "<div class=\"twitter_headline\"><center>&nbsp;&nbsp;@tupitube</center></div></div>\n";
    QString css = "twitter_tupi_version";  
    if (k->update)
        css = "twitter_tupi_update"; 

    html += "<div class=\"" + css + "\"><center>\n";
    html += tr("Latest Version") + ": <b>" + k->version + "</b> &nbsp;&nbsp;&nbsp;"; 
    html += tr("Revision") + ": <b>" + k->revision + "</b> &nbsp;&nbsp;&nbsp;";
    html += tr("Code Name") + ": <b>" + k->codeName + "</b>";

    if (k->update)
        html += "&nbsp;&nbsp;&nbsp;<b>[</b> <a href=\"https://www.maefloresta.com\">" + tr("It's time to upgrade! Click here!") + "</a>  <b>]</b>"; 

    html += "</center></div>\n";
    html += "<div class=\"twitter_tupi_donation\"><center>\n";
    html += "<a href=\"https://www.patreon.com/maefloresta\">" + tr("Want to help us to make a better project? Click here!") + "</a>";
    html += "</center></div>\n";
    html += tweets;
    html += "</body>\n";
    html += "</html>";

    QString twitterPath = QDir::homePath() + "/." + QCoreApplication::applicationName() + "/twitter.html";
    QFile file(twitterPath);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&file);
        out << html;
        file.close();
    }

    #ifdef TUP_DEBUG
        msg = "TupTwitter::formatStatus() - Saving file -> " + twitterPath;
        #ifdef Q_OS_WIN
            qWarning() << msg;
        #else
            tWarning() << msg;
        #endif
    #endif

    emit pageReady();
}

bool TupTwitter::saveFile(const QString &answer, const QString &fileName)
{
    QString msgPath = QDir::homePath() + "/." + QCoreApplication::applicationName() + "/" + fileName;
    QFile file(msgPath);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&file);
        out << answer;
        file.close();
        return true;
    }

    return false;
}

QString TupTwitter::getImageCode(const QString &answer) const
{
    QDomDocument doc;
    if (doc.setContent(answer)) {
        QDomNode root = doc.namedItem("webmsg");
        QDomElement element = root.firstChildElement("image");
        QString image = element.text();
        return image;
    }

    return "";
}

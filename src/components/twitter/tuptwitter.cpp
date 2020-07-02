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
// QString TupTwitter::BROWSER_FINGERPRINT = QString("Tupi_Browser 2.0");

TupTwitter::TupTwitter(QWidget *parent): QWidget(parent)
{
    #ifdef TUP_DEBUG
        qDebug() << "TupTwitter() - SSL version use for build: " << QSslSocket::sslLibraryBuildVersionString();
        qDebug() << "TupTwitter() - SSL version use for run-time: " << QSslSocket::sslLibraryVersionNumber();
        qDebug() << "TupTwitter() - Library Paths: " << QCoreApplication::libraryPaths();
    #endif

    update = false;
    TCONFIG->beginGroup("General");
    themeName = TCONFIG->value("Theme", "Light").toString();
    showAds = TCONFIG->value("ShowAds", true).toBool();

    // locale = QString(QLocale::system().name()).left(2);
    locale = TCONFIG->value("Language", "en").toString();
    if (locale.length() < 2) {
        locale = "en";
    } else {
        QList<QString> localeSupport;
        localeSupport << "en" << "es" << "pt";
        if (!localeSupport.contains(locale))
            locale = "en";
    }
}

void TupTwitter::start()
{
    QString url = MAEFLORESTA_URL + IS_HOST_UP_URL;

    #ifdef TUP_DEBUG
        qWarning() << "TupTwitter::start() - Getting news updates...";
    #endif

    manager = new QNetworkAccessManager(this);
    connect(manager, SIGNAL(finished(QNetworkReply*)), this, SLOT(closeRequest(QNetworkReply*)));

    request.setSslConfiguration(QSslConfiguration::defaultConfiguration());
    request.setUrl(QUrl(url));
    // request.setRawHeader("User-Agent", BROWSER_FINGERPRINT.toLatin1());
    request.setRawHeader("User-Agent", BROWSER_FINGERPRINT); 

    reply = manager->get(request);
    connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(slotError(QNetworkReply::NetworkError)));
}

TupTwitter::~TupTwitter()
{
    #ifdef TUP_DEBUG
        qDebug() << "~TupTwitter()";
    #endif

    delete manager;
    manager = nullptr;
    delete reply;
    reply = nullptr;
}

void TupTwitter::requestFile(const QString &target)
{
    #ifdef TUP_DEBUG
        qWarning() << "TupTwitter::requestFile() - Requesting url -> " + target;
    #endif

    request.setUrl(QUrl(target));
    // request.setRawHeader("User-Agent", BROWSER_FINGERPRINT.toLatin1());
    request.setRawHeader("User-Agent", BROWSER_FINGERPRINT);
    reply = manager->get(request);
}

void TupTwitter::closeRequest(QNetworkReply *reply)
{
    #ifdef TUP_DEBUG
        qWarning() << "TupTwitter::closeRequest() - Getting answer from request...";
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
                            qDebug() << "TupTwitter::closeRequest() - Network Error: Twitter output is NULL!";
                        #endif
                    }
                    requestFile(MAEFLORESTA_URL + TUPITUBE_WEB_MSG + locale + ".html");
                } else {
                    if (answer.startsWith("<webmsg>")) { // Getting web msg
                        if (showAds) {
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
                            reply->deleteLater();
                            manager->deleteLater();
                        } else {
                            #ifdef TUP_DEBUG
                                qDebug() << "TupTwitter::closeRequest() - Network Error: Invalid data!";
                            #endif
                        }
                    }
                }
            }
        }
    } else {
        #ifdef TUP_DEBUG
            qDebug() << "TupTwitter::closeRequest() - Network Error: Gosh! No Internet? :S";
        #endif
    } 
}

void TupTwitter::slotError(QNetworkReply::NetworkError error)
{
    switch (error) {
            case QNetworkReply::HostNotFoundError:
                 { 
                 #ifdef TUP_DEBUG
                     qDebug() << "TupTwitter::slotError() - Network Error: Host not found";
                 #endif
                 }
            break;
            case QNetworkReply::TimeoutError:
                 {
                 #ifdef TUP_DEBUG
                     qDebug() << "TupTwitter::slotError() - Network Error: Time out!";
                 #endif
                 }
            break;
            case QNetworkReply::ConnectionRefusedError:
                 {
                 #ifdef TUP_DEBUG
                     qDebug() << "TupTwitter::slotError() - Network Error: Connection Refused!";
                 #endif
                 }
            break;
            case QNetworkReply::ContentNotFoundError:
                 {
                 #ifdef TUP_DEBUG
                     qDebug() << "TupTwitter::slotError() - Network Error: Content not found!";
                 #endif
                 }
            break;
            case QNetworkReply::UnknownNetworkError:
            default:
                 {
                 #ifdef TUP_DEBUG
                     qDebug() << "TupTwitter::slotError() - Network Error: Unknown Network error!";
                 #endif
                 }
            break;
    }
}

void TupTwitter::checkSoftwareUpdates(QByteArray array)
{
    #ifdef TUP_DEBUG
        qWarning() << "TupTwitter::checkSoftwareUpdates() - Processing updates file...";
    #endif

    QDomDocument doc;

    if (doc.setContent(array)) {
        QDomElement root = doc.documentElement();
        QDomNode n = root.firstChild();

        while (!n.isNull()) {
            QDomElement e = n.toElement();
            if (!e.isNull()) {
                if (e.tagName() == "branch") {
                    version = e.text();
                    if (version.compare(kAppProp->version()) != 0)
                        update = true;
                } else if (e.tagName() == "rev") {
                    revision = e.text();
                    if (revision.compare(kAppProp->revision()) != 0)
                        update = true;
                } else if (e.tagName() == "codeName") {
                    codeName = e.text();
                }
            }
            n = n.nextSibling();
        }

        #ifdef TUP_DEBUG
            qWarning() << "TupTwitter::checkSoftwareUpdates() - Update Flag: " + QString::number(update);
            qWarning() << "Server Version: " + version + " - Revision: " + revision + " - Code Name: " + codeName;
            qWarning() << "Local Version: " + kAppProp->version() + " - Revision: " + kAppProp->revision();
        #endif

        emit newUpdate(update);
    }
}

void TupTwitter::formatStatus(QByteArray array)
{
    #ifdef TUP_DEBUG
        qWarning() << "TupTwitter::formatStatus() - Formatting news file...";
    #endif

    QString tweets = QString(array);
    QString html = "";

    html += "<html>\n";
    html += "<head>\n";
    html += "<meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\">\n";
    html += "<link rel=\"stylesheet\" type=\"text/css\" href=\"file:tupitube.css\">\n";
    html += "</head>\n";

    if (themeName.compare("Dark") == 0) {
        html += "<body class=\"twitter_gray\">\n";
        html += "<div class=\"tupi_background5\">";
    } else {
        html += "<body class=\"twitter_white\">\n";
        html += "<div class=\"tupi_background1\">";
    }

    html += "<center><img src=\"file:maefloresta.png\" alt=\"maefloresta\"/></center>\n";
    html += "<div class=\"twitter_headline\"><center>&nbsp;&nbsp;@tupitube</center></div></div>\n";
    QString css = "twitter_tupi_version";  
    if (update)
        css = "twitter_tupi_update"; 

    html += "<div class=\"" + css + "\"><center>\n";
    html += tr("Latest Version") + ": <b>" + version + "</b> &nbsp;&nbsp;&nbsp;";
    html += tr("Revision") + ": <b>" + revision + "</b> &nbsp;&nbsp;&nbsp;";
    html += tr("Description") + ": <b>" + codeName + "</b>";

    if (update)
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
        qWarning() << "TupTwitter::formatStatus() - Saving file -> " + twitterPath;
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

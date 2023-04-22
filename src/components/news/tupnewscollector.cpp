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

#include "tupnewscollector.h"
#include "tconfig.h"
#include "talgorithm.h"

#include <QDomDocument>
#include <QSslConfiguration>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QNetworkAccessManager>
#include <QFile>

QString TupNewsCollector::IS_HOST_UP_URL = QString("updates/test.xml");
QString TupNewsCollector::USER_TIMELINE_URL = QString("updates/news.php");
QString TupNewsCollector::TUPITUBE_VERSION_URL = QString("updates/current_version.xml");
QString TupNewsCollector::TUPITUBE_WEB_AD = QString("updates/web_ad.");
QString TupNewsCollector::TUPITUBE_IMAGES = QString("updates/images/");

TupNewsCollector::TupNewsCollector(QWidget *parent): QWidget(parent)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupNewsCollector()] - SSL enabled? -> " << QSslSocket::supportsSsl();
        qDebug() << "[TupNewsCollector()] - SSL version use for build -> " << QSslSocket::sslLibraryBuildVersionString();
        qDebug() << "[TupNewsCollector()] - SSL version use for run-time -> " << QSslSocket::sslLibraryVersionNumber();
        qDebug() << "[TupNewsCollector()] - Library Paths -> " << QCoreApplication::libraryPaths();
    #endif

    update = false;
    TCONFIG->beginGroup("General");
    themeName = TCONFIG->value("Theme", "Light").toString();
    showAds = TCONFIG->value("ShowAds", true).toBool();
    enableStatistics = TCONFIG->value("EnableStatistics", true).toBool();

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

void TupNewsCollector::start()
{
    QString url = MAEFLORESTA_URL + IS_HOST_UP_URL;

    #ifdef TUP_DEBUG
        qWarning() << "[TupNewsCollector::start()] - Getting news updates...";
    #endif

    manager = new QNetworkAccessManager(this);
    connect(manager, SIGNAL(finished(QNetworkReply*)), this, SLOT(closeRequest(QNetworkReply*)));

    request.setSslConfiguration(QSslConfiguration::defaultConfiguration());
    request.setUrl(QUrl(url));
    request.setRawHeader("User-Agent", BROWSER_FINGERPRINT);

    /*
    QSslConfiguration conf = request.sslConfiguration();
    conf.setPeerVerifyMode(QSslSocket::VerifyNone);
    request.setSslConfiguration(conf);
    */

    reply = manager->get(request);
    //  connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(slotError(QNetworkReply::NetworkError)));
    connect(reply, SIGNAL(errorOccurred(QNetworkReply::NetworkError)), this, SLOT(slotError(QNetworkReply::NetworkError)));
}

TupNewsCollector::~TupNewsCollector()
{
    #ifdef TUP_DEBUG
        qDebug() << "[~TupNewsCollector()]";
    #endif

    delete manager;
    manager = nullptr;
    delete reply;
    reply = nullptr;
}

void TupNewsCollector::requestFile(const QString &target)
{
    #ifdef TUP_DEBUG
        qWarning() << "[TupNewsCollector::requestFile()] - Requesting url -> " << target;
    #endif

    request.setUrl(QUrl(target));
    if (enableStatistics)
        request.setRawHeader("User-Agent", BROWSER_FINGERPRINT);
    else
        request.setRawHeader("User-Agent", MOZILLA_FINGERPRINT);

    reply = manager->get(request);
}

void TupNewsCollector::closeRequest(QNetworkReply *reply)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupNewsCollector::closeRequest()] - Getting answer from request...";
    #endif

    QByteArray array = reply->readAll();

    // Reading banner image from server
    QString imageName = reply->url().fileName();
    if (imageName.endsWith(".png", Qt::CaseInsensitive)) {
        QString imgPath = QDir::homePath() + "/." + QCoreApplication::applicationName() + "/images/";
        QDir dir(imgPath);
        if (!dir.exists()) {
            #ifdef TUP_DEBUG
                qWarning() << "[TupNewsCollector::closeRequest()] - Image path doesn't exist -> " << imgPath;
                qWarning() << "*** Creating it...";
            #endif
            if (!dir.mkpath(imgPath)) {
                #ifdef TUP_DEBUG
                    qWarning() << "[TupNewsCollector::closeRequest()] - Error while creating path -> " << imgPath;
                #endif
            }
        }

        QString image = imgPath + imageName;
        QFile file(image);
        #ifdef TUP_DEBUG
            qDebug() << "[TupNewsCollector::closeRequest()] - Saving image -> " << image;
        #endif
        if (file.open(QIODevice::WriteOnly)) {
            file.write(array);
            file.close();
        } else {
            #ifdef TUP_DEBUG
                qDebug() << "[TupNewsCollector::closeRequest()] - Can't create file -> " << image;
            #endif
        }

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

                QString target = MAEFLORESTA_URL + USER_TIMELINE_URL;
                if (enableStatistics)
                    target += "?id=" + id + "&os=" + os + "&v=" + kAppProp->version() + "." + kAppProp->revision();
                requestFile(target);
            } else {
                if (answer.startsWith("<div")) { // Getting Twitter records 
                    if (!array.isNull()) {
                        formatStatus(array);
                    } else {
                        #ifdef TUP_DEBUG
                            qDebug() << "[TupNewsCollector::closeRequest()] - Network Error: Twitter output is NULL!";
                        #endif
                    }
                    requestFile(MAEFLORESTA_URL + TUPITUBE_WEB_AD + locale + ".html");
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
                    }
                }
            }
        }
    } else {
        #ifdef TUP_DEBUG
            qDebug() << "[TupNewsCollector::closeRequest()] - Network Error: Gosh! No Internet? :S";
        #endif
    } 
}

void TupNewsCollector::slotError(QNetworkReply::NetworkError error)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupNewsCollector::slotError()] - Error -> " << error;
    #endif

    switch (error) {
        case QNetworkReply::HostNotFoundError:
             {
             #ifdef TUP_DEBUG
                 qDebug() << "[TupNewsCollector::slotError()] - Network Error: Host not found";
             #endif
             }
        break;
        case QNetworkReply::TimeoutError:
             {
             #ifdef TUP_DEBUG
                 qDebug() << "[TupNewsCollector::slotError()] - Network Error: Time out!";
             #endif
             }
        break;
        case QNetworkReply::ConnectionRefusedError:
             {
             #ifdef TUP_DEBUG
                 qDebug() << "[TupNewsCollector::slotError()] - Network Error: Connection Refused!";
             #endif
             }
        break;
        case QNetworkReply::ContentNotFoundError:
             {
             #ifdef TUP_DEBUG
                 qDebug() << "[TupNewsCollector::slotError()] - Network Error: Content not found!";
             #endif
             }
        break;
        case QNetworkReply::UnknownNetworkError:
        default:
             {
             #ifdef TUP_DEBUG
                 qDebug() << "[TupNewsCollector::slotError()] - Network Error: Unknown Network error!";
             #endif
             }
        break;
    }
}

void TupNewsCollector::checkSoftwareUpdates(QByteArray array)
{
    #ifdef TUP_DEBUG
        qWarning() << "[TupNewsCollector::checkSoftwareUpdates()] - Processing updates file...";
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
            qWarning() << "[TupNewsCollector::checkSoftwareUpdates()] - Update Flag -> " << update;
            qWarning() << "*** Server Version: " << version << " - Revision: " << revision << " - Code Name: " << codeName;
            qWarning() << "*** Local Version: " << kAppProp->version() << " - Revision: " << kAppProp->revision();
        #endif

        emit newUpdate(update);
    }
}

void TupNewsCollector::formatStatus(QByteArray array)
{
    #ifdef TUP_DEBUG
        qWarning() << "[TupNewsCollector::formatStatus()] - Formatting news file...";
    #endif

    QString tweets = QString(array);
    QString releaseHtml = "";

    QString htmlHeader = "";
    htmlHeader += "<html>\n";
    htmlHeader += "<head>\n";
    htmlHeader += "<meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\">\n";
    htmlHeader += "<link rel=\"stylesheet\" type=\"text/css\" href=\"file:tupitube.css\">\n";
    htmlHeader += "</head>\n";
    htmlHeader += "<body class=\"twitter_white\">\n";
    htmlHeader += "<div class=\"tupi_background5\">";

    QString htmlFooter = "</div></body>\n";
    htmlFooter += "</html>";

    releaseHtml = htmlHeader;
    releaseHtml += "<center><img src=\"file:maefloresta.png\" alt=\"maefloresta\"/></center>\n";
    releaseHtml += "<div class=\"twitter_headline\"><center>&nbsp;&nbsp;@tupitube</center></div></div>\n";
    QString css = "twitter_tupi_version";  
    if (update)
        css = "twitter_tupi_update";

    releaseHtml += "<div class=\"" + css + "\"><center>\n";
    releaseHtml += tr("Latest Version") + ": <b>" + version + "</b> &nbsp;&nbsp;&nbsp;";
    releaseHtml += tr("Revision") + ": <b>" + revision + "</b> &nbsp;&nbsp;&nbsp;";
    releaseHtml += tr("Description") + ": <b>" + codeName + "</b>";
    releaseHtml += "</center></div>";

    if (update)
        releaseHtml += "<div class=\"alert\"><center><b>[</b> <a class=\"alert\" href=\"https://www.tupitube.com\">"
                       + tr("It's time to upgrade! Click here!") + "</a>  <b>]</center></b></div>";

    releaseHtml += "<div class=\"twitter_separator\">&nbsp;</div>"
                "<div class=\"twitter_links\">"
                "<center>"
                "<a href=\"https://www.youtube.com/tupitube\">"
                "<img src=\"file:youtube.png\" alt=\"Youtube\"/></a>"
                "&nbsp;&nbsp;&nbsp;<a href=\"https://www.instagram.com/tupitube\"><img src=\"file:instagram.png\" alt=\"Instagram\"/></a>"
                "&nbsp;&nbsp;&nbsp;<a href=\"https://www.facebook.com/tupitube\"><img src=\"file:facebook.png\" alt=\"Facebook\"/></a>"
                "&nbsp;&nbsp;&nbsp;<a href=\"https://www.tiktok.com/@tupitube\"><img src=\"file:tiktok.png\" alt=\"TikTok\"/></a>"
                "&nbsp;&nbsp;&nbsp;<a href=\"https://twitter.com/tupitube\"><img src=\"file:twitter.png\" alt=\"Twitter\"/></a>"
                "</center>"
                "</div>";
    releaseHtml += "<div class=\"twitter_separator\">&nbsp;</div>";
    releaseHtml += "<div class=\"twitter_tupi_donation\"><center>\n";
    releaseHtml += "<a href=\"https://www.patreon.com/maefloresta\">" + tr("Want to help us to make a better project? Click here!") + "</a>";
    releaseHtml += "</center></div>\n";
    releaseHtml += "<p/><p/>"
                   "<div class=\"twitter_separator\">&nbsp;</div>";
    releaseHtml += htmlFooter;

    QString newsHtml = "";
    newsHtml += htmlHeader;
    newsHtml += tweets;
    newsHtml += htmlFooter;

    QString mainPath = QDir::homePath() + "/." + QCoreApplication::applicationName() + "/";
    QString releasePath = mainPath + "release.html";
    QFile releaseFile(releasePath);
    if (releaseFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&releaseFile);
        out << releaseHtml;
        releaseFile.close();
    }

    QString newsPath = mainPath + "news.html";
    QFile newsFile(newsPath);
    if (newsFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&newsFile);
        out << newsHtml;
        newsFile.close();
    }

    #ifdef TUP_DEBUG
        qWarning() << "[TupNewsCollector::formatStatus()] - Saving file -> " << releasePath;
        qWarning() << "[TupNewsCollector::formatStatus()] - Saving file -> " << newsPath;
    #endif

    emit pageReady();
}

bool TupNewsCollector::saveFile(const QString &answer, const QString &fileName)
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

QString TupNewsCollector::getImageCode(const QString &answer) const
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

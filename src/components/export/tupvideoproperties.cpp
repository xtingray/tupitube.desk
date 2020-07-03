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

#include <QNetworkAccessManager>
#include <QUrlQuery>

TupVideoProperties::TupVideoProperties() : TupExportWizardPage(tr("Animation Properties"))
{
    setTag("PROPERTIES");
    isOk = false;
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
    titleEdit = new QLineEdit(tr("My Animation"));
    titleEdit->setLocale(utf);
    connect(titleEdit, SIGNAL(textChanged(const QString &)), this, SLOT(resetTitleColor(const QString &)));
    titleLabel->setBuddy(titleEdit);

    QLabel *topicsLabel = new QLabel(tr("Topics"));
    topicsEdit = new QLineEdit(tr("#tupitube #animation #fun"));
    topicsEdit->setLocale(utf);
    connect(topicsEdit, SIGNAL(textChanged(const QString &)), this, SLOT(resetTopicsColor(const QString &)));
    topicsLabel->setBuddy(topicsEdit);

    QLabel *descLabel = new QLabel(tr("Description"));

    descText = new QTextEdit;
    descText->setLocale(utf);
    descText->setAcceptRichText(false);
    descText->setFixedHeight(80);
    descText->setText(tr("Create and share animations easily with TupiTube"));

    formLayout->addWidget(titleLabel);
    formLayout->addWidget(titleEdit);
    formLayout->addWidget(topicsLabel);
    formLayout->addWidget(topicsEdit);
    formLayout->addWidget(descLabel);
    formLayout->addWidget(descText);

    stackedWidget->addWidget(formWidget);
}

void TupVideoProperties::setProgressBar()
{
    progressWidget = new QWidget;
    QHBoxLayout *progressLayout = new QHBoxLayout(progressWidget);

    TCONFIG->beginGroup("General");
    QString themeName = TCONFIG->value("Theme", "Light").toString();
    QString style = "QProgressBar { background-color: #DDDDDD; text-align: center; color: #FFFFFF; border-radius: 2px; } ";
    QString color = "#009500";
    if (themeName.compare("Dark") == 0)
        color = "#444444";
    style += "QProgressBar::chunk { background-color: " + color + "; border-radius: 2px; }";

    QProgressBar *progressBar = new QProgressBar;
    progressBar->setTextVisible(true);
    progressBar->setStyleSheet(style);
    progressBar->setRange(1, 100);

    progressLayout->addSpacing(50);
    progressLayout->addWidget(progressBar);
    progressLayout->addSpacing(50);
    progressWidget->setVisible(false);

    stackedWidget->addWidget(progressWidget);
}

bool TupVideoProperties::isComplete() const
{
    return isOk;
}

void TupVideoProperties::reset()
{
}

QString TupVideoProperties::title() const
{
     QString title = QString::fromUtf8(titleEdit->text().toUtf8());
     return title;
}

QString TupVideoProperties::topics() const
{
     QString topics = QString::fromUtf8(topicsEdit->text().toUtf8());
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

void TupVideoProperties::setProjectParams(const QString &login, const QString &passwd, const QString &path)
{
     username = login;
     token = passwd;
     filePath = path;
}

void TupVideoProperties::postIt()
{
    QString title = titleEdit->text();
    QString tags = topicsEdit->text();
    QString desc= descText->toPlainText();

    if (title.length() == 0) {
        titleEdit->setText(tr("Set a title for the picture here!"));
        titleEdit->selectAll();
        isOk = false;
        return;
    }

    if (tags.length() == 0) {
        topicsEdit->setText(tr("Set some topic tags for the picture here!"));
        topicsEdit->selectAll();
        isOk = false;
        return;
    }

    stackedWidget->setCurrentIndex(1);
    isOk = true;

    emit postHasStarted();

    qDebug() << "Username: " << username;
    qDebug() << "Token: " << token;
    qDebug() << "Title: " << title;
    qDebug() << "Tags: " << tags;
    qDebug() << "Desc: " << desc;
    qDebug() << "Scenes: " << scenes;

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

    QNetworkAccessManager *manager = new QNetworkAccessManager(this);
    connect(manager, SIGNAL(finished(QNetworkReply *)), this, SLOT(serverAuthAnswer(QNetworkReply *)));

    QUrl url(TUPITUBE_URL + QString("/api/desk.php"));
    QNetworkRequest request(url);
    request.setRawHeader("User-Agent", BROWSER_FINGERPRINT);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    request.setSslConfiguration(QSslConfiguration::defaultConfiguration());
    request.setUrl(QUrl(url));

    QUrlQuery params;
    params.addQueryItem("username", username);
    params.addQueryItem("token", token);
    params.addQueryItem("title", title);
    params.addQueryItem("tags", tags);
    params.addQueryItem("desc", desc);
    params.addQueryItem("scenes", scenesStr);

    QByteArray postData = params.query(QUrl::FullyEncoded).toUtf8();
    QNetworkReply *reply = manager->post(request, postData);
    connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(slotError(QNetworkReply::NetworkError)));

    // Call this line when the implementation is done!
    // emit isDone();
}

void TupVideoProperties::serverAuthAnswer(QNetworkReply *reply)
{
    QByteArray array = reply->readAll();
    QString answer(array);
    answer.chop(1);
    if (answer.length() > 0) {
        qDebug() << "TupVideoProperties::serverAuthAnswer() - answer -> " << answer;
    }
}

void TupVideoProperties::slotError(QNetworkReply::NetworkError error)
{
    switch (error) {
        case QNetworkReply::HostNotFoundError:
             {
             #ifdef TUP_DEBUG
                 qDebug() << "TupVideoProperties::slotError() - Network Error: Host not found";
             #endif
             }
        break;
        case QNetworkReply::TimeoutError:
             {
             #ifdef TUP_DEBUG
                 qDebug() << "TupVideoProperties::slotError() - Network Error: Time out!";
             #endif
             }
        break;
        case QNetworkReply::ConnectionRefusedError:
             {
             #ifdef TUP_DEBUG
                 qDebug() << "TupVideoProperties::slotError() - Network Error: Connection Refused!";
             #endif
             }
        break;
        case QNetworkReply::ContentNotFoundError:
             {
             #ifdef TUP_DEBUG
                 qDebug() << "TupVideoProperties::slotError() - Network Error: Content not found!";
             #endif
             }
        break;
        case QNetworkReply::UnknownNetworkError:
        default:
             {
             #ifdef TUP_DEBUG
                 qDebug() << "TupVideoProperties::slotError() - Network Error: Unknown Network error!";
             #endif
             }
        break;
    }
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

void TupVideoProperties::resetTopicsColor(const QString &)
{
    QPalette palette = topicsEdit->palette();
    if (topicsEdit->text().length() > 0 &&
        topicsEdit->text().compare(tr("Set some topic tags for the picture here!")) != 0)
        palette.setBrush(QPalette::Base, Qt::white);
    else
        palette.setBrush(QPalette::Base, QColor(255, 140, 138));

    topicsEdit->setPalette(palette);
}

void TupVideoProperties::setScenesIndexes(const QList<int> &indexes)
{
    scenes = indexes;
}

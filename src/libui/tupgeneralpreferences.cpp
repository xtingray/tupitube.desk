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

#include "tupgeneralpreferences.h"
#include "tconfig.h"
#include "tformfactory.h"
#include "talgorithm.h"
#include "tosd.h"
#include "tseparator.h"
#include "tupsecurity.h"

#include <QPushButton>
#include <QToolButton>
#include <QFileDialog>
#include <QNetworkAccessManager>
#include <QDesktopServices>
#include <QRegularExpression>

TupGeneralPreferences::TupGeneralPreferences()
{
    QVBoxLayout *layout = new QVBoxLayout(this);

    tabWidget = new QTabWidget;
    tabWidget->addTab(generalTab(), tr("General"));
    tabWidget->addTab(cacheTab(), tr("Cache"));
    tabWidget->addTab(socialTab(), tr("Social Network"));

    layout->addWidget(tabWidget, Qt::AlignLeft);
    layout->addStretch(3);
}

TupGeneralPreferences::~TupGeneralPreferences()
{
}

QGridLayout * TupGeneralPreferences::createForm(const QString &groupName, Group group,
                                                QStringList keys, QStringList labels)
{
    langChanged = false;
    QGridLayout *form = new QGridLayout;
    int total = labels.count();

    TCONFIG->beginGroup(groupName);
    QList<QCheckBox *> list;
    for (int i=0; i<total; i++) {
         bool flag = TCONFIG->value(keys.at(i), true).toBool();
         QCheckBox *check = new QCheckBox(labels.at(i));
         check->setChecked(flag);
         list << check;
         form->addWidget(check, i, 0, Qt::AlignLeft);
    }

    if (group == Startup)
        interfaceList = list;
    else if (group == Confirm)
        confirmList = list;
    else if (group == Player)
        playerList = list;

    return form;
}

QWidget * TupGeneralPreferences::generalTab()
{
    newLang = "";
    interfaceOptions << "OpenLastProject" << "EnableStatistics";

    QStringList labels;
    labels << tr("Always open last project")
           << tr("Allow TupiTube to collect use statistics (No private/personal info)");

    QGridLayout *interfaceForm = createForm("General", Startup, interfaceOptions, labels);

    confirmation << "ConfirmRemoveFrame" << "ConfirmRemoveLayer"
                 << "ConfirmRemoveScene" << "ConfirmRemoveObject";

    labels.clear();
    labels << tr("Confirm \"Remove frame\" action") << tr("Confirm \"Remove layer\" action")
           << tr("Confirm \"Remove scene\" action") << tr("Confirm \"Remove object\" action from library");

    QGridLayout *confirmForm = createForm("General", Confirm, confirmation, labels);

    player << "AutoPlay";

    labels.clear();
    labels << tr("Render and play project automatically");

    QGridLayout *playerForm = createForm("AnimationParameters", Player, player, labels);

    QLabel *generalLabel = new QLabel(tr("General Preferences"));
    QFont labelFont = font();
    labelFont.setBold(true);
    labelFont.setPointSize(labelFont.pointSize() + 3);
    generalLabel->setFont(labelFont);

    QLabel *interfaceLabel = new QLabel(tr("Interface"));
    labelFont = font();
    labelFont.setBold(true);
    interfaceLabel->setFont(labelFont);

    saveCheck = new QCheckBox(tr("Enable autosave feature every"));
    saveCheck->setChecked(getAutoSaveFlag());
    connect(saveCheck, SIGNAL(stateChanged(int)), this, SLOT(updateTimeFlag(int)));

    saveCombo = new QComboBox();
    saveTimeList = TCONFIG->timeRanges();
    saveCombo->addItems(saveTimeList);
    if (!saveCheck->isChecked())
        saveCombo->setEnabled(false);
    saveCombo->setCurrentIndex(getAutoSaveTime());
    QLabel *minLabel = new QLabel(tr("minutes"));

    QHBoxLayout *saveLayout = new QHBoxLayout;
    saveLayout->addWidget(saveCheck);
    saveLayout->addWidget(saveCombo);
    saveLayout->addWidget(minLabel);
    saveLayout->addStretch();

    langSupport = TCONFIG->languages();
    // langSupport << "zh_CN" << "zh_TW" << "en" << "fr" << "pt" << "es";
    QLabel *langLabel = new QLabel(tr("Language:"));
    langCombo = new QComboBox();
    langCombo->addItem("简体中文"); // Simplified Chinese
    langCombo->addItem("繁體中文"); // Traditional Chinese
    langCombo->addItem("English");
    langCombo->addItem("Français");
    langCombo->addItem("Português");
    langCombo->addItem("Español");
    langCombo->addItem("русский");

    langCombo->setCurrentIndex(getLangIndex());
    connect(langCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(updateAppLang(int)));

    QHBoxLayout *langLayout = new QHBoxLayout;
    langLayout->addWidget(langLabel);
    langLayout->addWidget(langCombo);
    langLayout->addStretch();

    QLabel *confirmLabel = new QLabel(tr("Confirmation Dialogs"));
    confirmLabel->setFont(labelFont);

    QLabel *playerLabel = new QLabel(tr("Player"));
    playerLabel->setFont(labelFont);    

    QWidget *widget = new QWidget;
    QVBoxLayout *widgetLayout = new QVBoxLayout(widget);
    widgetLayout->addWidget(generalLabel);
    widgetLayout->addSpacing(15);
    widgetLayout->addWidget(interfaceLabel);
    widgetLayout->addLayout(langLayout);
    widgetLayout->addLayout(saveLayout);
    widgetLayout->addLayout(interfaceForm);
    widgetLayout->addSpacing(15);
    widgetLayout->addWidget(confirmLabel);
    widgetLayout->addLayout(confirmForm);
    widgetLayout->addSpacing(15);
    widgetLayout->addWidget(playerLabel);
    widgetLayout->addLayout(playerForm);

    return widget;
}

QWidget * TupGeneralPreferences::cacheTab()
{
    QWidget *widget = new QWidget;
    QVBoxLayout *layout = new QVBoxLayout(widget);

    QLabel *cacheLabel = new QLabel(tr("Cache Settings"));
    QFont font = this->font();
    font.setBold(true);
    font.setPointSize(font.pointSize() + 3);
    cacheLabel->setFont(font);

    QString msg = tr("The CACHE path is the folder where TupiTube creates temporary files while you work "
                     "on your animation projects.<br/>"
                     "<b>Note:</b> Don't change this parameter unless you know what you are doing.");
    QLabel *descLabel = new QLabel(msg);

    TCONFIG->beginGroup("General");
    cachePath = TCONFIG->value("Cache").toString();
    cacheLine = new QLineEdit(cachePath);

    QToolButton *openButton = new QToolButton;
    openButton->setIcon(QIcon(THEME_DIR + "icons/open.png"));
    openButton->setToolTip(tr("Choose another path"));
    connect(openButton, SIGNAL(clicked()), this, SLOT(chooseDirectory()));

    QHBoxLayout *filePathLayout = new QHBoxLayout;
    filePathLayout->addWidget(new QLabel(tr("CACHE Path: ")));
    filePathLayout->addWidget(cacheLine);
    filePathLayout->addWidget(openButton);

    QPushButton *restoreButton = new QPushButton(tr("Restore default value"));
    connect(restoreButton, SIGNAL(clicked()), this, SLOT(restoreCachePath()));
    QWidget *restoreWidget = new QWidget;
    QHBoxLayout *restoreLayout = new QHBoxLayout(restoreWidget);
    restoreLayout->addWidget(restoreButton);
    restoreLayout->addStretch();

    layout->addWidget(cacheLabel);
    layout->addSpacing(15);
    layout->addWidget(descLabel);
    layout->addLayout(filePathLayout);
    layout->addWidget(new TSeparator);
    layout->addWidget(restoreWidget);
    layout->addStretch();

    return widget;
}

QWidget * TupGeneralPreferences::socialTab()
{
    QWidget *widget = new QWidget;
    QVBoxLayout *layout = new QVBoxLayout(widget);

    TCONFIG->beginGroup("Network");
    username = TCONFIG->value("Username").toString();
    passwd = TCONFIG->value("Password").toString();
    bool anonymous = TCONFIG->value("Anonymous", false).toBool();

    QLabel *socialLabel = new QLabel(tr("TupiTube Credentials"));

    QFont font = this->font();
    font.setBold(true);
    font.setPointSize(font.pointSize() + 3);
    socialLabel->setFont(font);

    QLabel *usernameLabel = new QLabel(tr("Username / Email: "));
    usernameEdit = new QLineEdit();

    QLabel *passwdLabel = new QLabel(tr("Password: "));
    cacheString = new QLineEdit();
    cacheString->setEchoMode(QLineEdit::Password);

    QHBoxLayout *usernameLayout = new QHBoxLayout;
    usernameLayout->addWidget(usernameLabel);
    usernameLayout->addWidget(usernameEdit);
    usernameLayout->addStretch();

    QHBoxLayout *passwdLayout = new QHBoxLayout;
    passwdLayout->addWidget(passwdLabel);
    passwdLayout->addWidget(cacheString);
    passwdLayout->addStretch();

    usernameEdit->setText(username);
    anonymousBox = new QCheckBox(tr("Enable anonymous mode"));
    anonymousBox->setChecked(anonymous);

    font.setPointSize(font.pointSize() - 3);
    font.setBold(true);

    QLabel *registerLabel = new QLabel(tr("Don't have a TupiTube account?"));
    registerLabel->setFont(font);

    font.setBold(false);

    QLabel *emailLabel = new QLabel(tr("Email: "));
    emailLabel->setFont(font);
    emailEdit = new QLineEdit();
    connect(emailEdit, SIGNAL(returnPressed()), this, SLOT(formatEmail()));
    emailEdit->setFont(font);

    QHBoxLayout *emailLayout = new QHBoxLayout;
    emailLayout->addWidget(emailLabel);
    emailLayout->addWidget(emailEdit);

    registerButton = new QPushButton(tr("Register"));
    connect(registerButton, SIGNAL(clicked()), this, SLOT(sendRegisterRequest()));

    /* SQA: This connection doesn't work on Windows
    connect(registerButton, &QPushButton::clicked, this, &TupGeneralPreferences::sendRegisterRequest);
    */

    QWidget *registerWidget = new QWidget;
    QHBoxLayout *registerLayout = new QHBoxLayout(registerWidget);
    registerLayout->addWidget(registerButton);
    registerLayout->addStretch();

    layout->addWidget(socialLabel);
    layout->addSpacing(15);
    layout->addLayout(usernameLayout);
    layout->addLayout(passwdLayout);
    layout->addWidget(anonymousBox);
    layout->addSpacing(10);
    layout->addWidget(new TSeparator);
    layout->addWidget(registerLabel);
    layout->addLayout(emailLayout);
    layout->addWidget(registerWidget);
    layout->addStretch();

    return widget;
}

void TupGeneralPreferences::formatEmail()
{
    QString input = emailEdit->text();
    emailEdit->setText(input.toLower());
}

void TupGeneralPreferences::sendRegisterRequest()
{
    #ifdef TUP_DEBUG
        qDebug() << "TupGeneralPreferences::sendRequest() - Tracing...";
    #endif

    QString email = emailEdit->text().toLower();
    if (!email.isEmpty()) {
        // QRegExp mailREX("\\b[A-Z0-9._%+-]+@[A-Z0-9.-]+\\.[A-Z]{2,4}\\b");
        // mailREX.setCaseSensitivity(Qt::CaseInsensitive);
        // mailREX.setPatternSyntax(QRegExp::RegExp);
        // if (mailREX.exactMatch(email)) {

        QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
        QRegularExpression mailREX("\\b[A-Z0-9._%+-]+@[A-Z0-9.-]+\\.[A-Z]{2,4}\\b", QRegularExpression::CaseInsensitiveOption);
        QRegularExpressionMatch match = mailREX.match(email);
        if (match.hasMatch()) {
            registerButton->setEnabled(false);
            emailEdit->setText(email);
            QString url = TUPITUBE_URL + QString("/api/?a=register&e=" + email);
            manager = new QNetworkAccessManager(this);
            connect(manager, SIGNAL(finished(QNetworkReply*)), this, SLOT(registerAnswer(QNetworkReply*)));
            #ifdef TUP_DEBUG
                qDebug() << "GET request -> " << url;
            #endif
            QNetworkRequest request;
            request.setSslConfiguration(QSslConfiguration::defaultConfiguration());
            request.setUrl(QUrl(url));
            request.setRawHeader("User-Agent", BROWSER_FINGERPRINT);

            QNetworkReply *reply = manager->get(request);
            connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(slotError(QNetworkReply::NetworkError)));
        } else {
            #ifdef TUP_DEBUG
                qDebug() << "TupGeneralPreferences::sendRequest() - Error: Invalid email syntax! -> " << email;
            #endif
            emailEdit->setText(" " + tr("Email is invalid. Please, fix it!"));
            QTimer::singleShot(2000, this, SLOT(cleanMessage()));
        }
    } else {
        #ifdef TUP_DEBUG
            qDebug() << "TupGeneralPreferences::sendRequest() - Invalid email: field is empty!";
        #endif
        emailEdit->setText(" " + tr("Email field is empty. Type one!"));
        QTimer::singleShot(2000, this, SLOT(cleanMessage()));
    }
}

void TupGeneralPreferences::registerAnswer(QNetworkReply *reply)
{
    #ifdef TUP_DEBUG
       qDebug() << "TupGeneralPreferences::registerAnswer() - Tracing...";
    #endif

    QByteArray array = reply->readAll();
    QString answer(array);
    if (!answer.isEmpty()) {
        if (answer.compare("FALSE") == 0) {
            #ifdef TUP_DEBUG
                qDebug() << "TupGeneralPreferences::registerAnswer() - Error: e-mail already registered! :(";
            #endif
            emailEdit->setText(" " + tr("Error: Email already registered!"));
            QTimer::singleShot(2000, this, SLOT(cleanMessage()));
        } else {
            #ifdef TUP_DEBUG
                qDebug() << "TupGeneralPreferences::registerAnswer() - URL: " << answer;
            #endif
            if (answer.startsWith("http")) {
                QDesktopServices::openUrl(answer);
            } else {
                #ifdef TUP_DEBUG
                    qDebug() << "TupGeneralPreferences::registerAnswer() - Error: Invalid register URL! :(";
                #endif
                emailEdit->setText(" " + tr("Please contact us at info@maefloresta.com"));
                QTimer::singleShot(3000, this, SLOT(cleanMessage()));
            }
        }
    } else {
        #ifdef TUP_DEBUG
            qDebug() << "TupGeneralPreferences::registerAnswer() - Error: No data from server! :(";
        #endif
        emailEdit->setText(" " + tr("Please contact us at info@maefloresta.com"));
        QTimer::singleShot(3000, this, SLOT(cleanMessage()));
    }

    manager->deleteLater();
    registerButton->setEnabled(true);
    QApplication::restoreOverrideCursor();
}

void TupGeneralPreferences::slotError(QNetworkReply::NetworkError error)
{
    switch (error) {
        case QNetworkReply::HostNotFoundError:
             {
             #ifdef TUP_DEBUG
                 qDebug() << "TupGeneralPreferences::slotError() - Network Error: Host not found";
             #endif
             }
        break;
        case QNetworkReply::TimeoutError:
             {
             #ifdef TUP_DEBUG
                 qDebug() << "TupGeneralPreferences::slotError() - Network Error: Time out!";
             #endif
             }
        break;
        case QNetworkReply::ConnectionRefusedError:
             {
             #ifdef TUP_DEBUG
                 qDebug() << "TupGeneralPreferences::slotError() - Network Error: Connection Refused!";
             #endif
             }
        break;
        case QNetworkReply::ContentNotFoundError:
             {
             #ifdef TUP_DEBUG
                 qDebug() << "TupGeneralPreferences::slotError() - Network Error: Content not found!";
             #endif
             }
        break;
        case QNetworkReply::UnknownNetworkError:
        default:
             {
             #ifdef TUP_DEBUG
                 qDebug() << "TupGeneralPreferences::slotError() - Network Error: Unknown Network error!";
             #endif
             }
        break;
    }
}

void TupGeneralPreferences::chooseDirectory()
{
    cachePath = QFileDialog::getExistingDirectory(this, tr("Choose a directory..."), QDir::homePath(),
                                                 QFileDialog::ShowDirsOnly
                                                 | QFileDialog::DontResolveSymlinks);
    if (!cachePath.isEmpty())
        cacheLine->setText(cachePath);
}

void TupGeneralPreferences::restoreCachePath()
{
    cachePath = QDir::tempPath();
    cacheLine->setText(cachePath);

    TCONFIG->beginGroup("General");
    TCONFIG->setValue("Cache", cachePath);
    TCONFIG->sync();
}

bool TupGeneralPreferences::saveValues()
{
    TCONFIG->beginGroup("General");

    // General Preferences

    int total = interfaceOptions.count();
    for (int i=0; i<total; i++)
         TCONFIG->setValue(interfaceOptions.at(i), interfaceList.at(i)->isChecked());

    total = confirmation.count();
    for (int i=0; i<total; i++)
         TCONFIG->setValue(confirmation.at(i), confirmList.at(i)->isChecked());

    if (newLang.length() > 0)
        TCONFIG->setValue("Language", newLang);

    TCONFIG->setValue("AutoSave", saveCheck->isChecked());
    TCONFIG->setValue("AutoSaveTime", saveCombo->currentText());

    bool changed = false;
    QString data = cacheString->text();
    if (!data.isEmpty()) {
        if (TAlgorithm::cacheIDChanged(data)) {
            changed = true;
            TAlgorithm::storeData(data);
        }
    }

    cachePath = cacheLine->text();
    if (cachePath.isEmpty()) {
        tabWidget->setCurrentIndex(Cache);
        cacheLine->setFocus();
        TOsd::self()->display(TOsd::Error, tr("Cache path is empty. Set a value!"));
        return false;
    } else {
        QDir dir(cachePath);
        if (!dir.exists()) {
            tabWidget->setCurrentIndex(Cache);
            cacheLine->setFocus();
            TOsd::self()->display(TOsd::Error, tr("Cache path doesn't exist. Create it!"));
            return false;
        } else {
            TCONFIG->setValue("Cache", cachePath);
        }
    }

    // Social Network Credentials
    TCONFIG->beginGroup("Network");
    QString login = usernameEdit->text();
    if (!login.isEmpty()) {
        if (login.compare(username) != 0)
            TCONFIG->setValue("Username", login);
    }

    if (changed) {
        if (passwd.isEmpty())
            TCONFIG->setValue("Password", TupSecurity::encryptPassword(SECRET_KEY));
        TCONFIG->setValue("StorePassword", true);
    }

    bool anonymous = false;
    if (anonymousBox->isChecked())
        anonymous = true;
    TCONFIG->setValue("Anonymous", anonymous);

    TCONFIG->beginGroup("AnimationParameters");
    total = player.count();
    for (int i=0; i<total; i++)
         TCONFIG->setValue(player.at(i), playerList.at(i)->isChecked());

    TCONFIG->sync();

    return true;
}

int TupGeneralPreferences::getLangIndex()
{
    TCONFIG->beginGroup("General");
    QString locale = TCONFIG->value("Language", "en").toString();
    int index = langSupport.indexOf(locale);
    if (index == -1)
        index = langSupport.indexOf("en");

    return index;
}

bool TupGeneralPreferences::getAutoSaveFlag()
{
    TCONFIG->beginGroup("General");
    return TCONFIG->value("AutoSave", "true").toBool();
}

int TupGeneralPreferences::getAutoSaveTime()
{
    TCONFIG->beginGroup("General");
    QString time = TCONFIG->value("AutoSaveTime", "5").toString();
    int index = saveTimeList.indexOf(time);
    if (index == -1)
        index = 5;

    return index;
}

void TupGeneralPreferences::updateAppLang(int index)
{
    langChanged = true;
    newLang = langSupport.at(index);
}

bool TupGeneralPreferences::showWarning()
{
    return langChanged;
}

void TupGeneralPreferences::updateTimeFlag(int status)
{
    bool flag = false;
    if (status == Qt::Checked)
        flag = true;

    saveCombo->setEnabled(flag);
}

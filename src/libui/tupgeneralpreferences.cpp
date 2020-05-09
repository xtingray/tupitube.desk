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
#include "tosd.h"
#include "tseparator.h"

#include <QPushButton>
#include <QToolButton>
#include <QFileDialog>

TupGeneralPreferences::TupGeneralPreferences()
{
    QVBoxLayout *layout = new QVBoxLayout(this);

    tabWidget = new QTabWidget;
    tabWidget->addTab(generalTab(), tr("General"));
    tabWidget->addTab(cacheTab(), tr("Cache"));

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
         bool flag = TCONFIG->value(keys.at(i)).toBool();
         QCheckBox *check = new QCheckBox(labels.at(i));
         check->setChecked(flag);
         list << check;
         form->addWidget(check, i, 0, Qt::AlignLeft);
    }

    if (group == Startup)
        startupList = list;
    else if (group == Confirm)
        confirmList = list;
    else if (group == Player)
        playerList = list;

    return form;
}

QWidget * TupGeneralPreferences::generalTab()
{
    newLang = "";
    startup << "OpenLastProject" << "ShowTipOfDay";

    QStringList labels;
    labels << tr("Always open last project") << tr("Show tip of the day");

    QGridLayout *startupForm = createForm("General", Startup, startup, labels);

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

    QWidget *widget = new QWidget;
    QVBoxLayout *widgetLayout = new QVBoxLayout;

    QLabel *generalLabel = new QLabel(tr("General Preferences"));
    QFont labelFont = font();
    labelFont.setBold(true);
    labelFont.setPointSize(labelFont.pointSize() + 3);
    generalLabel->setFont(labelFont);
    widgetLayout->addWidget(generalLabel);
    widgetLayout->addSpacing(15);

    QLabel *startupLabel = new QLabel(tr("On Startup"));
    labelFont = font();
    labelFont.setBold(true);
    startupLabel->setFont(labelFont);
    widgetLayout->addWidget(startupLabel);

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

    langCombo->setCurrentIndex(getLangIndex());
    connect(langCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(updateAppLang(int)));

    QHBoxLayout *langLayout = new QHBoxLayout;
    langLayout->addWidget(langLabel);
    langLayout->addWidget(langCombo);
    langLayout->addStretch();

    widgetLayout->addLayout(langLayout);
    widgetLayout->addLayout(startupForm);

    widgetLayout->addSpacing(15);

    QLabel *confirmLabel = new QLabel(tr("Confirmation Dialogs"));
    confirmLabel->setFont(labelFont);
    widgetLayout->addWidget(confirmLabel);
    widgetLayout->addLayout(confirmForm);

    widgetLayout->addSpacing(15);

    QLabel *playerLabel = new QLabel(tr("On Player"));
    playerLabel->setFont(labelFont);
    widgetLayout->addWidget(playerLabel);
    widgetLayout->addLayout(playerForm);

    widget->setLayout(widgetLayout);

    return widget;
}

QWidget * TupGeneralPreferences::cacheTab()
{
    QWidget *widget = new QWidget;
    QVBoxLayout *layout = new QVBoxLayout(widget);

    QString msg = tr("The CACHE path is the folder where TupiTube creates temporary files while you work on your animation projects.<br/>"
                     "<b>Note:</b> Don't change this parameter unless you know what you are doing.");
    QLabel *label = new QLabel(msg);

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

    layout->addWidget(label);
    layout->addLayout(filePathLayout);
    layout->addWidget(new TSeparator);
    layout->addWidget(restoreWidget);
    layout->addStretch();

    return widget;
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

    int total = startup.count();
    for (int i=0; i<total; i++)
         TCONFIG->setValue(startup.at(i), startupList.at(i)->isChecked());

    total = confirmation.count();
    for (int i=0; i<total; i++)
         TCONFIG->setValue(confirmation.at(i), confirmList.at(i)->isChecked());

    if (newLang.length() > 0)
        TCONFIG->setValue("Language", newLang);

    cachePath = cacheLine->text();
    if (cachePath.isEmpty()) {
        tabWidget->setCurrentIndex(Cache);
        cacheLine->setFocus();
        TOsd::self()->display(tr("Error"), tr("Cache path is empty. Set a value!"), TOsd::Error);
        return false;
    } else {
        QDir dir(cachePath);
        if (!dir.exists()) {
            tabWidget->setCurrentIndex(Cache);
            cacheLine->setFocus();
            TOsd::self()->display(tr("Error"), tr("Cache path doesn't exist. Create it!"), TOsd::Error);
            return false;
        } else {
            TCONFIG->setValue("Cache", cachePath);
        }
    }

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

void TupGeneralPreferences::updateAppLang(int index)
{
    langChanged = true;
    newLang = langSupport.at(index);
}

bool TupGeneralPreferences::showWarning()
{
    return langChanged;
}

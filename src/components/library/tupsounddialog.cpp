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

#include "tupsounddialog.h"
#include "tconfig.h"
#include "tseparator.h"
#include "tosd.h"
#include "tupmicmanager.h"

#include <QDialogButtonBox>
#include <QBoxLayout>
#include <QLabel>
#include <QToolButton>
#include <QIcon>
#include <QFileDialog>
#include <QPushButton>

TupSoundDialog::TupSoundDialog(QWidget *parent) : QDialog(parent)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupSoundDialog()]";
    #endif

    QFile file(THEME_DIR + "config/ui.qss");
    if (file.exists()) {
        file.open(QFile::ReadOnly);
        QString uiStyleSheet = QLatin1String(file.readAll());
        if (uiStyleSheet.length() > 0)
            setStyleSheet(uiStyleSheet);
        file.close();
    } else {
        #ifdef TUP_DEBUG
            qWarning() << "[TupSoundDialog()] - theme file doesn't exist -> " << QString(THEME_DIR + "config/ui.qss");
        #endif
    }

    setWindowTitle(tr("Import Sound Asset"));
    setWindowIcon(QIcon(QPixmap(THEME_DIR + "icons/sound_object.png")));
    setMinimumWidth(400);

    tabWidget = new QTabWidget;
    tabWidget->addTab(soundFileTab(), tr("Sound File"));
    tabWidget->addTab(soundRecordTab(), tr("Record Audio"));

    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(tabWidget);
    setLayout(layout);
}

TupSoundDialog::~TupSoundDialog()
{
}

QWidget* TupSoundDialog::soundFileTab()
{
    QWidget *widget = new QWidget;
    QVBoxLayout *layout = new QVBoxLayout;
    QHBoxLayout *fileLayout = new QHBoxLayout;
    QLabel *fileLabel = new QLabel(tr("Sound File:"));
    filePathInput = new QLineEdit;
    QToolButton *fileButton = new QToolButton;
    fileButton->setIcon(QIcon(THEME_DIR + "icons/open.png"));
    fileButton->setToolTip(tr("Load sound file"));
    connect(fileButton, SIGNAL(clicked()), this, SLOT(loadSoundFile()));

    QHBoxLayout *buttonsLayout = new QHBoxLayout;
    importFileButton = new QPushButton("");
    importFileButton->setIcon(QIcon(THEME_DIR + "icons/apply.png"));
    importFileButton->setToolTip(tr("Import sound file"));
    importFileButton->setEnabled(false);
    connect(importFileButton, SIGNAL(clicked()), this, SLOT(importSoundAsset()));
    QPushButton *cancelButton = new QPushButton("");
    cancelButton->setIcon(QIcon(THEME_DIR + "icons/close.png"));
    connect(cancelButton, SIGNAL(clicked()), this, SLOT(reject()));
    buttonsLayout->addWidget(importFileButton, Qt::AlignRight);
    buttonsLayout->addWidget(cancelButton, Qt::AlignRight);

    QHBoxLayout *bottomLayout = new QHBoxLayout;
    bottomLayout->addStretch(1);
    bottomLayout->addLayout(buttonsLayout);

    fileLayout->addWidget(fileLabel);
    fileLayout->addWidget(filePathInput);
    fileLayout->addWidget(fileButton);

    layout->addLayout(fileLayout);
    layout->addStretch(1);
    layout->addWidget(new TSeparator());
    layout->addLayout(bottomLayout);
    widget->setLayout(layout);

    return widget;
}

QWidget* TupSoundDialog::soundRecordTab()
{
    QWidget *widget = new QWidget;
    QVBoxLayout *layout = new QVBoxLayout;

    micManager = new TupMicManager;
    connect(micManager, &TupMicManager::soundReady, this, &TupSoundDialog::enableDialogButtons);

    QHBoxLayout *buttonsLayout = new QHBoxLayout;
    importRecordButton = new QPushButton("");
    importRecordButton->setIcon(QIcon(THEME_DIR + "icons/apply.png"));
    importRecordButton->setToolTip(tr("Import recorded sound"));
    importRecordButton->setEnabled(false);
    connect(importRecordButton, SIGNAL(clicked()), this, SLOT(importRecordingAsset()));

    lipsyncButton = new QPushButton("");
    lipsyncButton->setIcon(QIcon(THEME_DIR + "icons/papagayo.png"));
    lipsyncButton->setToolTip(tr("Open lip-sync module"));
    lipsyncButton->setEnabled(false);
    connect(lipsyncButton, SIGNAL(clicked()), this, SLOT(launchLipsyncModule()));

    QPushButton *cancelButton = new QPushButton("");
    cancelButton->setIcon(QIcon(THEME_DIR + "icons/close.png"));
    connect(cancelButton, SIGNAL(clicked()), this, SLOT(reject()));

    buttonsLayout->addWidget(importRecordButton, Qt::AlignRight);
    buttonsLayout->addWidget(lipsyncButton, Qt::AlignRight);
    buttonsLayout->addWidget(cancelButton, Qt::AlignRight);

    QHBoxLayout *bottomLayout = new QHBoxLayout;
    bottomLayout->addStretch(1);
    bottomLayout->addLayout(buttonsLayout);

    layout->addWidget(micManager);
    layout->addSpacing(5);
    layout->addWidget(new TSeparator());
    layout->addLayout(bottomLayout);
    widget->setLayout(layout);

    return widget;
}

void TupSoundDialog::loadSoundFile()
{
    TCONFIG->beginGroup("General");
    soundFilePath = TCONFIG->value("DefaultPath", QDir::homePath()).toString();

    QFileDialog dialog(this, tr("Import audio file..."), soundFilePath);
    dialog.setNameFilter(tr("Sound file") + " (*.ogg *.wav *.mp3)");
    dialog.setFileMode(QFileDialog::ExistingFile);

    if (dialog.exec() == QDialog::Accepted) {
        QStringList files = dialog.selectedFiles();
        filePathInput->setText(files.at(0));
        importFileButton->setEnabled(true);
    }
}

void TupSoundDialog::importSoundAsset()
{
    QString path = filePathInput->text();
    if (!path.isEmpty()) {
        if (QFile::exists(path)) {
            emit soundFilePicked(path);
            close();
        } else {
            filePathInput->setFocus();
            TOsd::self()->display(TOsd::Error, tr("Sound file doesn't exist!"));
        }
    } else {
        TOsd::self()->display(TOsd::Error, tr("Please, choose a sound file!"));
    }
}

void TupSoundDialog::importRecordingAsset()
{
    QString path = micManager->getRecordPath();
    if (!path.isEmpty()) {
        emit soundFilePicked(path);
        close();
    } else {
        #ifdef TUP_DEBUG
            qDebug() << "[TupSoundDialog::importRecordingAsset()] - Recording file path is empty!";
        #endif
    }
}

void TupSoundDialog::enableDialogButtons(bool enabled)
{
    importRecordButton->setEnabled(enabled);
    lipsyncButton->setEnabled(enabled);
}

void TupSoundDialog::launchLipsyncModule()
{   
    QString path = micManager->getRecordPath();
    if (!path.isEmpty()) {
        qDebug() << "[TupSoundDialog::launchLipsyncModule()] - emiting signal!!!";
        emit lipsyncModuleCalled(path);
        close();
    } else {
        #ifdef TUP_DEBUG
            qDebug() << "[TupSoundDialog::launchLipsyncModule()] - Recording file path is empty!";
        #endif
    }
}

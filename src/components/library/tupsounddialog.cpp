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
#include "tapptheme.h"
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
#include <QMessageBox>
#include <QScreen>

#ifdef __cplusplus
extern "C" {
#include <libavformat/avformat.h>
#include "libavfilter/avfilter.h"
#include "libavfilter/buffersink.h"
#include "libavfilter/buffersrc.h"
#include "libavutil/opt.h"
}
#endif

TupSoundDialog::TupSoundDialog(QWidget *parent) : QDialog(parent)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupSoundDialog()]";
    #endif

    setModal(true);
    setStyleSheet(TAppTheme::themeSettings());

    setWindowTitle(tr("Import Audio Asset"));
    setWindowIcon(QIcon(QPixmap(THEME_DIR + "icons/sound_object.png")));
    setMinimumWidth(400);

    tabWidget = new QTabWidget;
    tabWidget->addTab(soundFileTab(), tr("Audio File"));
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
    QLabel *fileLabel = new QLabel(tr("Audio Path:"));
    filePathInput = new QLabel;
    filePathInput->setMinimumWidth(260);
    filePathInput->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
    filePathInput->setStyleSheet("background-color:#dddddd; padding-left:3px;");

    fileButton = new QToolButton;
    fileButton->setIcon(QIcon(THEME_DIR + "icons/open.png"));
    fileButton->setMinimumWidth(60);
    fileButton->setToolTip(tr("Load audio file"));
    connect(fileButton, SIGNAL(clicked()), this, SLOT(loadSoundFile()));

    QHBoxLayout *buttonsLayout = new QHBoxLayout;
    importFileButton = new QPushButton("");
    importFileButton->setIcon(QIcon(THEME_DIR + "icons/apply.png"));
    importFileButton->setMinimumWidth(60);
    importFileButton->setToolTip(tr("Import audio file"));
    importFileButton->setEnabled(false);
    connect(importFileButton, SIGNAL(clicked()), this, SLOT(importSoundAsset()));

    lipsyncButton02 = new QPushButton("");
    lipsyncButton02->setIcon(QIcon(THEME_DIR + "icons/papagayo.png"));
    lipsyncButton02->setMinimumWidth(60);
    lipsyncButton02->setToolTip(tr("Open lip-sync module"));
    lipsyncButton02->setEnabled(false);
    connect(lipsyncButton02, SIGNAL(clicked()), this, SLOT(launchLipsyncModule()));

    QPushButton *cancelButton = new QPushButton("");
    cancelButton->setIcon(QIcon(THEME_DIR + "icons/close.png"));
    cancelButton->setMinimumWidth(60);
    cancelButton->setToolTip(tr("Cancel"));
    connect(cancelButton, SIGNAL(clicked()), this, SLOT(reject()));
    buttonsLayout->addWidget(importFileButton, Qt::AlignRight);
    buttonsLayout->addWidget(lipsyncButton02, Qt::AlignRight);
    buttonsLayout->addWidget(cancelButton, Qt::AlignRight);

    QHBoxLayout *bottomLayout = new QHBoxLayout;
    bottomLayout->addStretch(1);
    bottomLayout->addLayout(buttonsLayout);

    fileLayout->addStretch();
    fileLayout->addWidget(fileLabel);
    fileLayout->addWidget(filePathInput);
    fileLayout->addWidget(fileButton);
    fileLayout->addStretch();

    layout->addSpacing(20);
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
    connect(micManager, SIGNAL(soundReady(bool)), this, SLOT(enableDialogButtons(bool)));

    /* SQA: This connect doesn't work on Windows
    connect(micManager, &TupMicManager::soundReady, this, &TupSoundDialog::enableDialogButtons);
    */

    QHBoxLayout *buttonsLayout = new QHBoxLayout;
    importRecordButton = new QPushButton("");
    importRecordButton->setIcon(QIcon(THEME_DIR + "icons/apply.png"));
    importRecordButton->setMinimumWidth(60);
    importRecordButton->setToolTip(tr("Import recorded audio"));
    importRecordButton->setEnabled(false);
    connect(importRecordButton, SIGNAL(clicked()), this, SLOT(importRecordingAsset()));

    lipsyncButton01 = new QPushButton("");
    lipsyncButton01->setIcon(QIcon(THEME_DIR + "icons/papagayo.png"));
    lipsyncButton01->setMinimumWidth(60);
    lipsyncButton01->setToolTip(tr("Open lip-sync module"));
    lipsyncButton01->setEnabled(false);
    connect(lipsyncButton01, SIGNAL(clicked()), this, SLOT(launchLipsyncModule()));

    QPushButton *cancelButton = new QPushButton("");
    cancelButton->setIcon(QIcon(THEME_DIR + "icons/close.png"));
    cancelButton->setMinimumWidth(60);
    connect(cancelButton, SIGNAL(clicked()), this, SLOT(closeDialog()));

    buttonsLayout->addWidget(importRecordButton, Qt::AlignRight);
    buttonsLayout->addWidget(lipsyncButton01, Qt::AlignRight);
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
    /*
    #ifdef Q_OS_WIN
        QString filter = tr("Audio file") + " (*.wav)";
    #else
        QString filter = tr("Audio file") + " (*.ogg *.wav *.mp3)";
    #endif
    */

    QString filter = tr("Audio file") + " (*.ogg *.wav *.mp3)";
    dialog.setNameFilter(filter);
    dialog.setFileMode(QFileDialog::ExistingFile);

    if (dialog.exec() == QDialog::Accepted) {
        QStringList files = dialog.selectedFiles();
        filePathInput->setText(files.at(0));
        importFileButton->setEnabled(true);
        lipsyncButton02->setEnabled(true);
    }
}

void TupSoundDialog::importSoundAsset()
{
    QString path = filePathInput->text();
    if (!path.isEmpty()) {
        if (QFile::exists(path)) {
            if (!isAudioInStereo(path)) {
                #ifdef TUP_DEBUG
                    qDebug() << "[TupSoundDialog::importSoundAsset()] - "
                                "Fatal Error: Audio file is NOT in stereo! ->" << path;
                #endif

                QScreen *screen = QGuiApplication::screens().at(0);
                QMessageBox msgBox;
                msgBox.setStyleSheet(TAppTheme::themeSettings());
                msgBox.setWindowTitle(tr("File:") + " " + path);
                msgBox.setIcon(QMessageBox::Warning);
                msgBox.setText(tr("Audio file only has one channel (Mono). The file must have two channels (Stereo)."));
                msgBox.setInformativeText(tr("Tip: Use an online service to convert your audio file from Mono to Stereo."));
                msgBox.setStandardButtons(QMessageBox::Ok);
                msgBox.show();

                msgBox.move(static_cast<int> ((screen->geometry().width() - msgBox.width()) / 2),
                            static_cast<int> ((screen->geometry().height() - msgBox.height()) / 2));

                msgBox.exec();

                return;
            }

            emit soundFilePicked(path);
            close();
        } else {
            fileButton->setFocus();
            TOsd::self()->display(TOsd::Error, tr("Audio file doesn't exist!"));
        }
    } else {
        TOsd::self()->display(TOsd::Error, tr("Please, choose an audio file!"));
    }
}

bool TupSoundDialog::isAudioInStereo(const QString &soundPath)
{
    #ifdef TUP_DEBUG
        qDebug() << "---";
        qDebug() << "[TupSoundDialog::isAudioInStereo()] - Checking audio file ->" << soundPath;
    #endif

    QByteArray array = soundPath.toLocal8Bit();
    char *filename = array.data();

    AVStream *in_stream;
    AVCodecParameters *in_codecpar;
    int error;

    // Open the input file to read from it.
    AVFormatContext *inputFormatContext = avformat_alloc_context();
    if ((error = avformat_open_input(&inputFormatContext, filename, nullptr, nullptr)) < 0) {
        #ifdef TUP_DEBUG
            qCritical() << "[TupSoundDialog::isAudioInStereo()] - Fatal Error: Could not open input file ->" << soundPath;
            qCritical() << "ERROR CODE ->" << error;
        #endif
        avformat_close_input(&inputFormatContext);

        return false;
    }

    // Get information on the input file (number of streams etc.).
    if ((error = avformat_find_stream_info(inputFormatContext, nullptr)) < 0) {
        #ifdef TUP_DEBUG
            qCritical() << "[TupSoundDialog::isAudioInStereo()] - Fatal Error: Could not open find stream ->" << soundPath;
            qCritical() << "ERROR CODE ->" << error;
        #endif
        avformat_close_input(&inputFormatContext);

        return false;
    }

    int streamsTotal = inputFormatContext->nb_streams;
    for(int i=0; i<streamsTotal; i++) {
        in_stream = inputFormatContext->streams[i];
        in_codecpar = in_stream->codecpar;
        if (in_codecpar->codec_type == AVMEDIA_TYPE_AUDIO) {
            #ifdef TUP_DEBUG
                qDebug() << "[TupSoundDialog::isAudioInStereo()] - Found audio stream!";
                qDebug() << "[TupSoundDialog::isAudioInStereo()] - Audio channels total ->" << in_codecpar->channels;
            #endif

            if (in_codecpar->channels == 2) {
                avformat_close_input(&inputFormatContext);

                return true;
            }
        }
    }
    avformat_close_input(&inputFormatContext);

    return false;
}

void TupSoundDialog::importRecordingAsset()
{
    QString path = micManager->getRecordPath();
    if (!path.isEmpty()) {
        emit soundFilePicked(path);
        close();
    } else {
        #ifdef TUP_DEBUG
            qDebug() << "[TupSoundDialog::importRecordingAsset()] - Fatal Error: Recording file path is empty!";
        #endif
    }
}

void TupSoundDialog::enableDialogButtons(bool enabled)
{
    importRecordButton->setEnabled(enabled);
    lipsyncButton01->setEnabled(enabled);
}

void TupSoundDialog::launchLipsyncModule()
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupSoundDialog::launchLipsyncModule()]";
    #endif

    QString path = "";
    PapagayoAppMode mode = Insert;
    if (tabWidget->currentIndex() == 0) { // Audio file comes from filesystem
        path = filePathInput->text();
    } else { // Audio file was recorded
        mode = VoiceRecorded;
        path = micManager->getRecordPath();
    }

    if (!path.isEmpty()) {
        emit lipsyncModuleCalled(mode, path);
        close();
    } else {
        #ifdef TUP_DEBUG
            qWarning() << "[TupSoundDialog::launchLipsyncModule()] - "
                          "Fatal Error: Audio file path is empty!";
        #endif
    }
}

void TupSoundDialog::closeDialog()
{
    if (micManager->isRecording())
        micManager->cancelRecording();

    reject();
}

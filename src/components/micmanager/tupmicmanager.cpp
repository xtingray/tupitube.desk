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

#include "tupmicmanager.h"

#include <QDir>
#include <QGridLayout>
#include <QMediaRecorder>
#include <QGroupBox>
#include <QTimer>
#include <QMessageBox>

static qreal getMaxValue(const QAudioFormat &format);
static QVector<qreal> getBufferLevels(const QAudioBuffer &buffer);

template <class T> static QVector<qreal> getBufferLevels(const T *buffer, int frames, int channels);

TupMicManager::TupMicManager()
{
    initRecorder();
    setupUI();
    setConnections();
}

TupMicManager::~TupMicManager()
{
    delete player;
    delete micRecorder;
    delete micProbe;
}

void TupMicManager::initRecorder()
{
    micRecorder = new QAudioRecorder(this);
    micProbe = new QAudioProbe(this);
    connect(micProbe, &QAudioProbe::audioBufferProbed, this, &TupMicManager::handleBuffer);
    micProbe->setSource(micRecorder);

    recording = false;
    secCounter = 0;
    player = new QMediaPlayer;
}

void TupMicManager::setupUI()
{
    centralWidget = new QWidget(this);
    QGridLayout *mainLayout = new QGridLayout(centralWidget);

    QGridLayout *formLayout = new QGridLayout();

    QLabel *nameLabel = new QLabel(centralWidget);
    nameLabel->setText(tr("Record Name:"));

    formLayout->addWidget(nameLabel, 0, 0, 1, 1);

    nameInput = new TInputField(tr("Audio 01"));
    connect(nameInput, SIGNAL(inputFilled(bool)), this, SLOT(enableRecordButton(bool)));

    formLayout->addWidget(nameInput, 0, 1, 1, 1);

    QLabel *deviceLabel = new QLabel(centralWidget);
    deviceLabel->setText(tr("Input Device:"));

    formLayout->addWidget(deviceLabel, 1, 0, 1, 1);

    audioDevDropList = new QComboBox(centralWidget);
    audioDevDropList->addItem(tr("Default"), QVariant(QString()));
    for (auto &device: micRecorder->audioInputs()) {
        audioDevDropList->addItem(device, QVariant(device));
    }

    formLayout->addWidget(audioDevDropList, 1, 1, 1, 1);
    mainLayout->addLayout(formLayout, 0, 0, 1, 3);

    controlsWidget = new QWidget;
    QHBoxLayout *controlsLayout = new QHBoxLayout;

    controlsLayout->addStretch();

    recordButton = new QPushButton(QIcon(QPixmap(THEME_DIR + "icons/record.png")), "", controlsWidget);
    recordButton->setToolTip(tr("Record"));
    recordButton->setMinimumWidth(60);
    connect(recordButton, SIGNAL(clicked()), this, SLOT(toggleRecord()));

    controlsLayout->addWidget(recordButton);

    pauseButton = new QPushButton(QIcon(QPixmap(THEME_DIR + "icons/pause.png")), "", controlsWidget);
    pauseButton->setToolTip(tr("Pause"));
    pauseButton->setMinimumWidth(60);
    pauseButton->setEnabled(false);
    connect(pauseButton, SIGNAL(clicked()), this, SLOT(togglePause()));

    controlsLayout->addWidget(pauseButton);
    controlsLayout->addStretch();

    controlsWidget->setLayout(controlsLayout);

    playerWidget = new QWidget;
    QHBoxLayout *playerLayout = new QHBoxLayout;

    playButton = new QPushButton(QIcon(QPixmap(THEME_DIR + "icons/play.png")), "", playerWidget);
    playButton->setToolTip(tr("Play"));
    connect(playButton, SIGNAL(clicked()), this, SLOT(playRecording()));

    playerLayout->addWidget(playButton);

    discardButton = new QPushButton(QIcon(QPixmap(THEME_DIR + "icons/delete.png")), "", playerWidget);
    discardButton->setToolTip(tr("Discard"));
    connect(discardButton, SIGNAL(clicked()), this, SLOT(discardRecording()));

    playerLayout->addWidget(discardButton);
    playerWidget->setLayout(playerLayout);
    playerWidget->setVisible(false);

    bottomWidget = new QWidget;
    QVBoxLayout *bottomLayout = new QVBoxLayout;

    QLabel *levelLabel = new QLabel(bottomWidget);
    levelLabel->setText(tr("Audio Level:"));

    bottomLayout->addWidget(levelLabel);

    levelsScreenLayout = new QVBoxLayout();

    initLevel = new TupMicLevel(bottomWidget);
    levelsScreenLayout->addWidget(initLevel);
    initLevelIncluded = true;

    bottomLayout->addLayout(levelsScreenLayout);

    statusLabel = new QLabel(bottomWidget);
    statusLabel->setText(" ");

    bottomLayout->addWidget(statusLabel);
    bottomLayout->addStretch(1);
    bottomWidget->setLayout(bottomLayout);

    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(centralWidget);
    layout->addWidget(controlsWidget);
    layout->addWidget(playerWidget);
    layout->addWidget(bottomWidget);
    layout->addStretch(1);
    setLayout(layout);
}

void TupMicManager::setConnections()
{
    connect(micRecorder, &QAudioRecorder::durationChanged, this, &TupMicManager::updateProgress);
    connect(micRecorder, &QAudioRecorder::statusChanged, this, &TupMicManager::updateStatus);
    connect(micRecorder, &QAudioRecorder::stateChanged, this, &TupMicManager::onStateChanged);
    connect(micRecorder, QOverload<QMediaRecorder::Error>::of(&QAudioRecorder::error),
            this, &TupMicManager::showErrorMessage);
}

void TupMicManager::updateProgress(qint64 duration)
{
    if (micRecorder->error() != QMediaRecorder::NoError || duration < 2000)
        return;

    statusLabel->setText(tr("%1 sec").arg(duration / 1000));
}

void TupMicManager::updateStatus(QMediaRecorder::Status status)
{
    QString statusMessage;

    switch (status) {
        case QMediaRecorder::RecordingStatus:
            statusMessage = tr("Recording...");
            break;
        case QMediaRecorder::PausedStatus:
            clearMicLevels();
            statusMessage = tr("Paused");
            break;
        case QMediaRecorder::UnloadedStatus:
        case QMediaRecorder::LoadedStatus:
            clearMicLevels();
            statusMessage = tr("Stopped");
        default:
            break;
    }

    if (micRecorder->error() == QMediaRecorder::NoError)
        statusLabel->setText(statusMessage);
}

void TupMicManager::onStateChanged(QMediaRecorder::State state)
{
    switch (state) {
        case QMediaRecorder::RecordingState:
            recording = true;
            recordButton->setIcon(QIcon(QPixmap(THEME_DIR + "icons/stop.png")));
            recordButton->setToolTip(tr("Stop"));
            pauseButton->setIcon(QIcon(QPixmap(THEME_DIR + "icons/pause.png")));
            pauseButton->setToolTip(tr("Pause"));
            break;
        case QMediaRecorder::PausedState:
            recording = false;
            recordButton->setIcon(QIcon(QPixmap(THEME_DIR + "icons/stop.png")));
            recordButton->setToolTip(tr("Stop"));
            pauseButton->setIcon(QIcon(QPixmap(THEME_DIR + "icons/resume.png")));
            pauseButton->setToolTip(tr("Resume"));
            break;
        case QMediaRecorder::StoppedState:
            recording = false;
            QString filename = CACHE_DIR + nameInput->text() + ".mp3";
            if (QFile::exists(filename)) {
                player->setMedia(QUrl::fromLocalFile(filename));
            } else {
                #ifdef TUP_DEBUG
                    qDebug() << "[TupMicManager::onStateChanged()] - Fatal Error: Sound file doesn't exist -> " << filename;
                #endif
            }

            recordButton->setIcon(QIcon(QPixmap(THEME_DIR + "icons/record.png")));
            recordButton->setToolTip(tr("Record"));
            pauseButton->setIcon(QIcon(QPixmap(THEME_DIR + "icons/pause.png")));
            pauseButton->setToolTip(tr("Pause"));
            controlsWidget->setVisible(false);
            playerWidget->setVisible(true);
            emit soundReady(true);
            break;
    }

    pauseButton->setEnabled(micRecorder->state() != QMediaRecorder::StoppedState);
}

static QVariant boxValue(const QComboBox *box)
{
    int idx = box->currentIndex();
    if (idx == -1)
        return QVariant();

    return box->itemData(idx);
}

void TupMicManager::toggleRecord()
{
    if (micRecorder->state() == QMediaRecorder::StoppedState) {
        nameInput->setReadOnly(true);
        micRecorder->setAudioInput(boxValue(audioDevDropList).toString());

        QAudioEncoderSettings settings;
        settings.setCodec("audio/mpeg, mpegversion=(int)1, layer=(int)3");
        settings.setSampleRate(0);
        settings.setBitRate(128000);
        settings.setChannelCount(1);
        settings.setQuality(QMultimedia::VeryHighQuality);
        settings.setEncodingMode(QMultimedia::ConstantQualityEncoding);

        micRecorder->setEncodingSettings(settings, QVideoEncoderSettings(), QString("audio/mpeg, mpegversion=(int)1"));
        micRecorder->setOutputLocation(QUrl::fromLocalFile(CACHE_DIR + nameInput->text() + ".mp3"));
        micRecorder->record();
    } else {
        micRecorder->stop();
    }
}

void TupMicManager::togglePause()
{
    if (micRecorder->state() != QMediaRecorder::PausedState)
        micRecorder->pause();
    else
        micRecorder->record();
}

void TupMicManager::discardRecording()
{
    QMessageBox msgBox(this);
    msgBox.setWindowTitle(tr("Question"));
    msgBox.setIcon(QMessageBox::Question);
    msgBox.setText(tr("Confirm Action."));
    msgBox.setInformativeText(tr("Do you want to delete this sound recording?"));

    msgBox.addButton(QString(tr("Ok")), QMessageBox::AcceptRole);
    msgBox.addButton(QString(tr("Cancel")), QMessageBox::RejectRole);
    msgBox.show();

    int ret = msgBox.exec();
    if (ret == QMessageBox::RejectRole)
        return;

    nameInput->setReadOnly(false);
    nameInput->setText(tr("Sound 01"));
    nameInput->setFocus();

    playerWidget->setVisible(false);
    controlsWidget->setVisible(true);
    QString filename = CACHE_DIR + nameInput->text() + ".mp3";
    if (QFile::exists(filename)) {
        if (!QFile::remove(filename)) {
            #ifdef TUP_DEBUG
                qDebug() << "[TupMicManager::discardRecording()] - Fatal Error: Can't remove file -> " << filename;
            #endif
        }
    }

    emit soundReady(false);
}

void TupMicManager::showErrorMessage()
{
    statusLabel->setText(micRecorder->errorString());
}

void TupMicManager::clearMicLevels()
{
    for (int i = 0; i < micLevels.size(); ++i)
        micLevels.at(i)->setLevel(0);
}

qreal getMaxValue(const QAudioFormat& format)
{
    if (!format.isValid())
        return qreal(0);

    if (format.codec() != "audio/pcm")
        return qreal(0);

    switch (format.sampleType()) {
        case QAudioFormat::Unknown:
            break;
        case QAudioFormat::Float:
            if (format.sampleSize() != 32)
                return qreal(0);
            return qreal(1.00003);
        case QAudioFormat::SignedInt:
            if (format.sampleSize() == 32)
                return qreal(INT_MAX);
            if (format.sampleSize() == 16)
                return qreal(SHRT_MAX);
            if (format.sampleSize() == 8)
                return qreal(CHAR_MAX);
            break;
        case QAudioFormat::UnSignedInt:
            if (format.sampleSize() == 32)
                return qreal(UINT_MAX);
            if (format.sampleSize() == 16)
                return qreal(USHRT_MAX);
            if (format.sampleSize() == 8)
                return qreal(UCHAR_MAX);
            break;
    }

    return qreal(0);
}

QVector<qreal> getBufferLevels(const QAudioBuffer& buffer)
{
    QVector<qreal> levels;

    if (!buffer.format().isValid() || buffer.format().byteOrder() != QAudioFormat::LittleEndian)
        return levels;

    if (buffer.format().codec() != "audio/pcm")
        return levels;

    int channelCount = buffer.format().channelCount();
    levels.fill(0, channelCount);
    qreal maxValue = getMaxValue(buffer.format());
    if (qFuzzyCompare(maxValue, qreal(0)))
        return levels;

    switch (buffer.format().sampleType()) {
        case QAudioFormat::Unknown:
        case QAudioFormat::UnSignedInt:
            if (buffer.format().sampleSize() == 32)
                levels = getBufferLevels(buffer.constData<quint32>(), buffer.frameCount(), channelCount);
            if (buffer.format().sampleSize() == 16)
                levels = getBufferLevels(buffer.constData<quint16>(), buffer.frameCount(), channelCount);
            if (buffer.format().sampleSize() == 8)
                levels = getBufferLevels(buffer.constData<quint8>(), buffer.frameCount(), channelCount);
            for (int i = 0; i < levels.size(); ++i)
                levels[i] = qAbs(levels.at(i) - maxValue / 2) / (maxValue / 2);
        break;
        case QAudioFormat::Float:
            if (buffer.format().sampleSize() == 32) {
                levels = getBufferLevels(buffer.constData<float>(), buffer.frameCount(), channelCount);
                for (int i = 0; i < levels.size(); ++i)
                    levels[i] /= maxValue;
            }
        break;
        case QAudioFormat::SignedInt:
            if (buffer.format().sampleSize() == 32)
                levels = getBufferLevels(buffer.constData<qint32>(), buffer.frameCount(), channelCount);
            if (buffer.format().sampleSize() == 16)
                levels = getBufferLevels(buffer.constData<qint16>(), buffer.frameCount(), channelCount);
            if (buffer.format().sampleSize() == 8)
                levels = getBufferLevels(buffer.constData<qint8>(), buffer.frameCount(), channelCount);
            for (int i = 0; i < levels.size(); ++i)
                levels[i] /= maxValue;
            break;
    }

    return levels;
}

template <class T> QVector<qreal> getBufferLevels(const T *buffer, int frames, int channels)
{
    QVector<qreal> max_values;
    max_values.fill(0, channels);

    for (int i = 0; i < frames; ++i) {
        for (int j = 0; j < channels; ++j) {
            qreal value = qAbs(qreal(buffer[i * channels + j]));
            if (value > max_values.at(j))
                max_values.replace(j, value);
        }
    }

    return max_values;
}

void TupMicManager::handleBuffer(const QAudioBuffer& buffer)
{
    /*
    #ifdef TUP_DEBUG
        qDebug() << "[TupMicManager::handleBuffer()]";
    #endif
    */

    if (initLevelIncluded) {
        levelsScreenLayout->removeWidget(initLevel);
        initLevelIncluded = false;
    }

    if (micLevels.count() != buffer.format().channelCount()) {
        qDeleteAll(micLevels);
        micLevels.clear();
        for (int i = 0; i < buffer.format().channelCount(); ++i) {
            TupMicLevel *level = new TupMicLevel(centralWidget);
            micLevels.append(level);
            levelsScreenLayout->addWidget(level);
        }
    }

    QVector<qreal> levels = getBufferLevels(buffer);
    for (int i = 0; i < levels.count(); ++i)
        micLevels.at(i)->setLevel(levels.at(i));
}

void TupMicManager::enableRecordButton(bool enabled)
{
    recordButton->setEnabled(enabled);
}

void TupMicManager::playRecording()
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupMicManager::playRecording()] - player->state() -> " << player->state();
    #endif

    if (player->state() == QMediaPlayer::StoppedState) {
        timer = new QTimer(this);
        connect(timer, &QTimer::timeout, this, QOverload<>::of(&TupMicManager::trackPlayerStatus));
        timer->start(500);

        playButton->setIcon(QIcon(QPixmap(THEME_DIR + "icons/stop.png")));
        discardButton->setEnabled(false);
        player->play();
        statusLabel->setText(tr("Playing..."));
    } else if (player->state() == QMediaPlayer::PlayingState) {
        player->stop();
        statusLabel->setText(tr(""));
        playButton->setIcon(QIcon(QPixmap(THEME_DIR + "icons/play.png")));
        discardButton->setEnabled(true);
    }
}

void TupMicManager::trackPlayerStatus()
{
    secCounter += 500.0;

    /*
    #ifdef TUP_DEBUG
        qDebug() << "[TupMicManager::checkPlayerStatus()] - status -> " << player->state();
        qDebug() << "milsec counter -> " << secCounter;
    #endif
    */

    if (secCounter >= player->duration()) {
        secCounter = 0;
        timer->stop();
        player->stop();
        statusLabel->setText(tr(""));
        playButton->setIcon(QIcon(QPixmap(THEME_DIR + "icons/play.png")));
        discardButton->setEnabled(true);
    }
}

QString TupMicManager::getRecordPath() const
{
    QString filename = nameInput->text();
    QString path = CACHE_DIR + filename + ".mp3";
    if (QFile::exists(path))
        return filename;

    return "";
}

bool TupMicManager::isRecording()
{
    return recording;
}

void TupMicManager::cancelRecording()
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupMicManager::cancelRecording()]";
    #endif

    if (micRecorder->state() == QMediaRecorder::RecordingState) {
        disconnect(micRecorder, &QAudioRecorder::durationChanged, this, &TupMicManager::updateProgress);
        disconnect(micRecorder, &QAudioRecorder::statusChanged, this, &TupMicManager::updateStatus);
        disconnect(micRecorder, &QAudioRecorder::stateChanged, this, &TupMicManager::onStateChanged);
        micRecorder->stop();
    } else if (player->state() == QMediaPlayer::PlayingState) {
        player->stop();
    }

    QString filename = nameInput->text();
    QString path = CACHE_DIR + filename + ".mp3";
    if (QFile::exists(path)) {
        if (!QFile::remove(path)) {
            #ifdef TUP_DEBUG
                qWarning() << "[TupMicManager::cancelRecording()] - Fatal Error: Can't remove temporary file -> " << path;
            #endif
        }
    }
}

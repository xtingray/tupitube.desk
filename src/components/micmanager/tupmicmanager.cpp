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
#include <QFileDialog>
#include <QMediaRecorder>
#include <QStandardPaths>
#include <QGroupBox>

static qreal getPeakValue(const QAudioFormat &format);
static QVector<qreal> getBufferLevels(const QAudioBuffer &buffer);

template <class T> static QVector<qreal> getBufferLevels(const T *buffer, int frames, int channels);

TupMicManager::TupMicManager()
{
    outputPosFlag = false;

    initRecorder();
    setupUI();
    setConnections();
}

TupMicManager::~TupMicManager()
{
}

void TupMicManager::setupUI()
{
    setObjectName(QString::fromUtf8("MicManager"));

    centralWidget = new QWidget(this);
    centralWidget->setObjectName(QString::fromUtf8("centralwidget"));
    QGridLayout *mainLayout = new QGridLayout(centralWidget);
    mainLayout->setObjectName(QString::fromUtf8("mainLayout"));

    QGridLayout *formLayout = new QGridLayout();
    formLayout->setObjectName(QString::fromUtf8("formLayout"));

    QLabel *nameLabel = new QLabel(centralWidget);
    nameLabel->setObjectName(QString::fromUtf8("deviceLabel"));
    nameLabel->setText(tr("Record Name:"));

    formLayout->addWidget(nameLabel, 0, 0, 1, 1);

    nameInput = new QLineEdit(tr("Sound 01"));
    nameInput->setObjectName(QString::fromUtf8("nameInput"));

    formLayout->addWidget(nameInput, 0, 1, 1, 1);

    QLabel *deviceLabel = new QLabel(centralWidget);
    deviceLabel->setObjectName(QString::fromUtf8("deviceLabel"));
    deviceLabel->setText(tr("Input Device:"));

    formLayout->addWidget(deviceLabel, 1, 0, 1, 1);

    audioDevDropList = new QComboBox(centralWidget);
    audioDevDropList->setObjectName(QString::fromUtf8("audioDeviceBox"));
    audioDevDropList->addItem(tr("Default"), QVariant(QString()));
    for (auto &device: micRecorder->audioInputs()) {
        audioDevDropList->addItem(device, QVariant(device));
    }

    formLayout->addWidget(audioDevDropList, 1, 1, 1, 1);
    mainLayout->addLayout(formLayout, 0, 0, 1, 3);

    recordButton = new QPushButton(centralWidget);
    recordButton->setObjectName(QString::fromUtf8("recordButton"));
    recordButton->setText(tr("Record"));
    connect(recordButton, SIGNAL(clicked()), this, SLOT(toggleRecord()));

    mainLayout->addWidget(recordButton, 2, 0, 1, 1);

    pauseButton = new QPushButton(centralWidget);
    pauseButton->setObjectName(QString::fromUtf8("pauseButton"));
    pauseButton->setText(tr("Pause"));
    pauseButton->setEnabled(false);
    connect(pauseButton, SIGNAL(clicked()), this, SLOT(togglePause()));

    mainLayout->addWidget(pauseButton, 2, 1, 1, 1);

    playButton = new QPushButton(centralWidget);
    playButton->setObjectName(QString::fromUtf8("playButton"));
    playButton->setText(tr("Play"));

    mainLayout->addWidget(playButton, 2, 2, 1, 1);

    discardButton = new QPushButton(centralWidget);
    discardButton->setObjectName(QString::fromUtf8("discardButton"));
    discardButton->setText(tr("Discard"));
    connect(discardButton, SIGNAL(clicked()), this, SLOT(setOutputLocation()));

    mainLayout->addWidget(discardButton, 2, 3, 1, 1);

    bottomWidget = new QWidget;
    QVBoxLayout *bottomLayout = new QVBoxLayout;

    QLabel *levelLabel = new QLabel(bottomWidget);
    levelLabel->setObjectName(QString::fromUtf8("levelLabel"));
    levelLabel->setText(tr("Audio Level:"));

    bottomLayout->addWidget(levelLabel);

    levelsScreenLayout = new QVBoxLayout();
    levelsScreenLayout->setObjectName(QString::fromUtf8("levelsLayout"));

    initLevel = new TupMicLevel(bottomWidget);
    levelsScreenLayout->addWidget(initLevel);
    initLevelIncluded = true;

    bottomLayout->addLayout(levelsScreenLayout);

    statusLabel = new QLabel(bottomWidget);
    statusLabel->setObjectName(QString::fromUtf8("statusLabel"));
    statusLabel->setText(" ");

    bottomLayout->addWidget(statusLabel);
    bottomLayout->addStretch(1);
    bottomWidget->setLayout(bottomLayout);

    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(centralWidget);
    layout->addWidget(bottomWidget);
    layout->addStretch(1);
    setLayout(layout);
}

void TupMicManager::initRecorder()
{
    micRecorder = new QAudioRecorder(this);
    micProbe = new QAudioProbe(this);
    connect(micProbe, &QAudioProbe::audioBufferProbed, this, &TupMicManager::processBuffer);
    micProbe->setSource(micRecorder);
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

    statusLabel->setText(tr("Recorded %1 sec").arg(duration / 1000));
}

void TupMicManager::updateStatus(QMediaRecorder::Status status)
{
    QString statusMessage;

    switch (status) {
        case QMediaRecorder::RecordingStatus:
            statusMessage = tr("Recording to %1").arg(micRecorder->actualLocation().toString());
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
            recordButton->setText(tr("Stop"));
            pauseButton->setText(tr("Pause"));
            break;
        case QMediaRecorder::PausedState:
            recordButton->setText(tr("Stop"));
            pauseButton->setText(tr("Resume"));
            break;
        case QMediaRecorder::StoppedState:
            recordButton->setText(tr("Record"));
            pauseButton->setText(tr("Pause"));
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
        // bottomWidget->setVisible(true);
        micRecorder->setAudioInput(boxValue(audioDevDropList).toString());

        QAudioEncoderSettings settings;
        settings.setCodec("audio/mpeg, mpegversion=(int)1, layer=(int)3");
        settings.setSampleRate(0);
        settings.setBitRate(128000);
        settings.setChannelCount(1);
        settings.setQuality(QMultimedia::VeryHighQuality);
        settings.setEncodingMode(QMultimedia::ConstantQualityEncoding);

        micRecorder->setEncodingSettings(settings, QVideoEncoderSettings(), QString("audio/mpeg, mpegversion=(int)1"));
        micRecorder->record();
    }
    else {
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

void TupMicManager::setOutputLocation()
{
    QString fileName = QFileDialog::getSaveFileName();
    micRecorder->setOutputLocation(QUrl::fromLocalFile(fileName));
    outputPosFlag = true;
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

qreal getPeakValue(const QAudioFormat& format)
{
    if (!format.isValid())
        return qreal(0);

    if (format.codec() != "audio/pcm")
        return qreal(0);

    switch (format.sampleType()) {
        case QAudioFormat::Unknown:
            break;
        case QAudioFormat::Float:
            if (format.sampleSize() != 32) // other sample formats are not supported
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
    qreal peak_value = getPeakValue(buffer.format());
    if (qFuzzyCompare(peak_value, qreal(0)))
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
                levels[i] = qAbs(levels.at(i) - peak_value / 2) / (peak_value / 2);
        break;
        case QAudioFormat::Float:
            if (buffer.format().sampleSize() == 32) {
                levels = getBufferLevels(buffer.constData<float>(), buffer.frameCount(), channelCount);
                for (int i = 0; i < levels.size(); ++i)
                    levels[i] /= peak_value;
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
                levels[i] /= peak_value;
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

void TupMicManager::processBuffer(const QAudioBuffer& buffer)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupMicManager::processBuffer()]";
    #endif

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

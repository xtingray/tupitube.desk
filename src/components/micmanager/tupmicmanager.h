#ifndef TUPMICMANAGER_H
#define TUPMICMANAGER_H

#include "tglobal.h"
#include "tapplicationproperties.h"
#include "tupmiclevel.h"
#include "tinputfield.h"

#include <QWidget>
#include <QComboBox>
#include <QPushButton>
#include <QRadioButton>
#include <QVBoxLayout>
#include <QLineEdit>
#include <QLabel>
#include <QTimer>

#include <QMediaCaptureSession>
#include <QAudioInput>
#include <QMediaRecorder>
#include <QAudioBuffer>
#include <QMediaPlayer>
#include <QAudioOutput>

class TUPITUBE_EXPORT TupMicManager : public QWidget
{
    Q_OBJECT

    public:
        TupMicManager();
        ~TupMicManager();

        QString getRecordPath();
        bool isRecording();
        void cancelRecording();

    signals:
        void soundReady(bool enabled);

    public slots:
        void handleBuffer(const QAudioBuffer &buffer);

    private slots:
        void discardRecording();
        void togglePause();
        void toggleRecord();

        void onStateChanged(QMediaRecorder::RecorderState state);
        void updateProgress(qint64 pos);
        void showErrorMessage();
        void enableRecordButton(bool enabled);

        void playRecording();
        void trackPlayerStatus();
        void enablePlayButton();

        // void updateStatus(QMediaRecorder::Status status);

    private:
        void initRecorder();
        void setupUI();
        void setConnections();
        void clearMicLevels();
        void resetMediaPlayer();

        QMediaCaptureSession captureSession;
        QMediaRecorder *micRecorder;
        QList<TupMicLevel*> micLevels;

        QWidget *centralWidget;
        QWidget *controlsWidget;
        QWidget *playerWidget;
        QWidget *bottomWidget;
        QVBoxLayout *levelsScreenLayout;
        TupMicLevel *audioLevel1;
        TupMicLevel *audioLevel2;
        bool audioLevelsIncluded;

        TInputField *nameInput;
        TInputField *durationInput;
        QComboBox *audioDevCombo;
        QLabel *statusLabel;

        QPushButton *recordButton;
        QPushButton *pauseButton;

        QPushButton *playButton;
        QPushButton *discardButton;

        QList<QMediaPlayer *> player;
        QAudioOutput *audioOutput;
        QTimer *timer;
        qreal secCounter;
        qreal audioDuration;
        bool recording;
        QString audioname;
        QString extension;
        QString recordTime;
};

#endif // TUPMICMANAGER_H

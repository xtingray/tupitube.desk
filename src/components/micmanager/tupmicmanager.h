#ifndef TUPMICMANAGER_H
#define TUPMICMANAGER_H

#include "tglobal.h"
#include "tapplicationproperties.h"
#include "tupmiclevel.h"
#include "tinputfield.h"

#include <QWidget>
#include <QMediaRecorder>
#include <QAudioRecorder>
#include <QAudioBuffer>
#include <QAudioProbe>
#include <QComboBox>
#include <QPushButton>
#include <QRadioButton>
#include <QVBoxLayout>
#include <QLineEdit>
#include <QLabel>
#include <QMediaPlayer>
#include <QTimer>

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

        void updateStatus(QMediaRecorder::Status status);
        void onStateChanged(QMediaRecorder::State state);
        void updateProgress(qint64 pos);
        void showErrorMessage();
        void enableRecordButton(bool enabled);

        void playRecording();
        void trackPlayerStatus();

    private:
        void initRecorder();
        void setupUI();
        void setConnections();
        void clearMicLevels();
        void resetMediaPlayer();

        QAudioRecorder *micRecorder;
        QAudioProbe *micProbe;
        QList<TupMicLevel*> micLevels;

        QWidget *centralWidget;
        QWidget *controlsWidget;
        QWidget *playerWidget;
        QWidget *bottomWidget;
        QVBoxLayout *levelsScreenLayout;
        TupMicLevel *micLevel;
        bool micLevelIncluded;

        TInputField *nameInput;
        TInputField *durationInput;
        QComboBox *audioDevDropList;
        QLabel *statusLabel;

        QPushButton *recordButton;
        QPushButton *pauseButton;

        QPushButton *playButton;
        QPushButton *discardButton;

        QList<QMediaPlayer *> player;
        QAudioProbe *playerProbe;
        QTimer *timer;
        qreal secCounter;
        qreal audioDuration;
        bool recording;
        QString extension;
        QString recordTime;
};

#endif // TUPMICMANAGER_H

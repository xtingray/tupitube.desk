#ifndef TUPMICMANAGER_H
#define TUPMICMANAGER_H

#include "tglobal.h"
#include "tapplicationproperties.h"

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

#include "tupmiclevel.h"

class TUPITUBE_EXPORT TupMicManager : public QWidget
{
    Q_OBJECT

    public:
        TupMicManager();
        ~TupMicManager();

    public slots:
        void handleBuffer(const QAudioBuffer&);

    private slots:
        void setOutputLocation();
        void togglePause();
        void toggleRecord();

        void updateStatus(QMediaRecorder::Status);
        void onStateChanged(QMediaRecorder::State);
        void updateProgress(qint64 pos);
        void showErrorMessage();

    private:
        void initRecorder();
        void setupUI();
        void setConnections();
        void clearMicLevels();

        QAudioRecorder *micRecorder;
        QAudioProbe *micProbe;
        QList<TupMicLevel*> micLevels;
        bool outputPosFlag;

        QWidget *centralWidget;
        QWidget *bottomWidget;
        QVBoxLayout *levelsScreenLayout;
        TupMicLevel *initLevel;
        bool initLevelIncluded;

        QLineEdit *nameInput;
        QComboBox *audioDevDropList;
        QLabel *statusLabel;

        QPushButton *playButton;
        QPushButton *discardButton;
        QPushButton *recordButton;
        QPushButton *pauseButton;
};

#endif // TUPMICMANAGER_H

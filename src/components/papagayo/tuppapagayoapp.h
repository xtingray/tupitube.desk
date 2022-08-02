/***************************************************************************
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

#ifndef TUPPAPAGAYOAPP_H
#define TUPPAPAGAYOAPP_H

#include "tglobal.h"
#include "tapplicationproperties.h"
#include "tuplipsyncdoc.h"
#include "tupwaveformview.h"
#include "tupmouthview.h"
#include "tupcustomizedmouthview.h"

#include "tupproject.h"
#include "tupprojectrequest.h"
#include "tuplipsync.h"

#include <QMainWindow>
#include <QLineEdit>
#include <QListWidgetItem>
#include <QPlainTextEdit>
#include <QComboBox>
#include <QPushButton>
#include <QStackedWidget>
#include <QSpinBox>

class TUPITUBE_EXPORT TupPapagayoApp : public QMainWindow
{
	Q_OBJECT

    public:
        TupPapagayoApp(PapagayoAppMode mode, TupProject *project, const QString &soundFile = QString(),
                       QList<int> indexes = QList<int>(), QWidget *parent = nullptr);

        TupPapagayoApp(PapagayoAppMode mode, TupProject *project, TupLipSync *lipsync, QList<int> indexes,
                       QWidget *parent = nullptr);

        ~TupPapagayoApp();

    public:
        void openFile(QString filePath);

    protected:
        void closeEvent(QCloseEvent *event);
        void dragEnterEvent(QDragEnterEvent *event);
        void dropEvent(QDropEvent *event);
        void keyPressEvent(QKeyEvent *event);

    signals:
        void requestTriggered(const TupProjectRequest *request);
        void soundRemoved(ModuleSource source, const QString &soundKey);

    public slots:
        void updateActions();

    private slots:
        void openFile();
        void playVoice();
        void pauseVoice();
        void stopVoice();
        void onVoiceNameChanged();
        void onVoiceTextChanged();
        void updateLanguage(int index);
        void runBreakdownAction();
        void runManualBreakdownAction();
        void updateMouthView(int index);
        void openImagesDialog();
        void updateFrame(int frame);
        void updatePauseButton();
        void createLipsyncRecord();
        void callUpdateProcedure();
        void closeWindow();
        bool validateLipsyncForm();

    private:
        void setUICore(const QString &filePath);
        void setBaseUI();
        void setupActions();
        void setupUI();
        void setupMenus();
        void saveDefaultPath(const QString &dir);
        void buildOtherLanguagePhonemes();
        void loadWordsFromDocument();
        void loadDocumentFromScratch(QStringList phonemes);
        int32 calculateDuration();
        bool confirmCloseDocument();
        bool saveLipsyncRecord();
        bool updateLipsyncRecord();
        void openBreakdownDialog(int wordIndex);

        TupWaveFormView *waveformView;
        TupMouthView *mouthView;
        TupCustomizedMouthView *customView;
        TupLipsyncDoc *document;
        bool enableAutoBreakdown;
        bool rebuildingList;
        int defaultFps;
        bool playerStopped;
        PapagayoAppMode mode;

        QAction *actionClose;
        QAction *actionOpen;
        QAction *actionPlay;
        QIcon playIcon;
        QIcon pauseIcon;

        QAction *actionStop;
        QAction *actionZoomIn;
        QAction *actionZoomOut;
        QAction *actionAutoZoom;

        QStackedWidget *mouthFrame;
        QLineEdit *voiceName;
        QPlainTextEdit *voiceText;
        QComboBox *mouthsCombo;
        QComboBox *languageChoice;
        QPushButton *breakdownButton;
        QLineEdit *mouthsPath;
        PapagayoAppLanguage currentLanguage;
        QString currentMouthPath;

        PapagayoAppViewType mouthType;
        int currentMouthIndex;

        QPushButton *okButton;
        QStringList wordsList;
        QStringList phonemesList;
        bool saveButtonPressed;
        QString pgoFolderPath;
        QString pgoFilePath;
        QString soundFilePath;
        QString soundKey;
        QString oldLipsyncName;

        QString tempSoundFile;
        QString tempMouthPath;

        TupProject *tupProject;
        int sceneIndex;
        int layerIndex;
        int frameIndex;

        TupLipsyncDictionary *dictionary;
        QString locale;
        QSpinBox *initFrameBox;
};

#endif // TUPPAPAGAYOAPP_H

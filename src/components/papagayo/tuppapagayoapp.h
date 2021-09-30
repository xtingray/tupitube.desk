#ifndef TUPPAPAGAYOAPP_H
#define TUPPAPAGAYOAPP_H

#include <QMainWindow>
#include <QLineEdit>
#include <QListWidgetItem>
#include <QPlainTextEdit>
#include <QComboBox>
#include <QPushButton>

#include "tglobal.h"
#include "tuplipsyncdoc.h"
#include "tupwaveformview.h"
#include "tupmouthview.h"

class Q_DECL_EXPORT TupPapagayoApp : public QMainWindow
{
	Q_OBJECT

    public:
        explicit TupPapagayoApp(QWidget *parent = nullptr);
        ~TupPapagayoApp();

    public:
        void openFile(QString filePath);
        bool isOKToCloseDocument();
        void restoreSettings();
        void saveSettings();

    protected:
        void closeEvent(QCloseEvent *event);
        void dragEnterEvent(QDragEnterEvent *event);
        void dropEvent(QDropEvent *event);
        void keyPressEvent(QKeyEvent *event);

    public slots:
        void updateActions();

    private slots:
        void onFileOpen();
        void onFileSave();
        void onFileSaveAs();
        void onPlay();
        void onStop();
        void onFpsChange(QString text);
        void onNewVoice();
        void onDeleteVoice();
        void onVoiceSelected(QListWidgetItem *item);
        void onVoiceItemChanged(QListWidgetItem *item);
        void onVoiceNameChanged();
        void onVoiceTextChanged();
        void onBreakdown();
        void onExport();

    private:
        void setupActions();
        void setupUI();
        void setupConnections();
        void setupMenus();
        void rebuildVoiceList();

        TupWaveFormView *waveformView;
        TupMouthView *mouthView;
        TupLipsyncDoc *document;
        bool enableAutoBreakdown;
        bool rebuildingList;
        int defaultFps;
        bool playerStopped;

        QAction *actionClose;
        QAction *actionOpen;
        QAction *actionSave;
        QAction *actionSaveAs;
        QAction *actionPlay;
        QAction *actionStop;
        QAction *actionZoomIn;
        QAction *actionZoomOut;
        QAction *actionAutoZoom;
        QAction *actionUndo;
        QAction *actionCut;
        QAction *actionCopy;
        QAction *actionPaste;

        QLineEdit *voiceName;
        QPlainTextEdit *voiceText;
        QLineEdit *fpsEdit;
        QComboBox *mouthsCombo;
        QComboBox *languageChoice;
        QPushButton *breakdownButton;
        QComboBox *exportChoice;
        QPushButton *exportButton;
        QListWidget *voiceList;
        QPushButton *newVoiceButton;
        QPushButton *deleteVoiceButton;
};

#endif // TUPPAPAGAYODIALOG_H

#ifndef TUPPAPAGAYODIALOG_H
#define TUPPAPAGAYODIALOG_H

#include <QMainWindow>

#include "tuplipsyncdoc.h"

class QListWidgetItem;

namespace Ui {
    class TupPapagayoDialog;
}

class TupPapagayoDialog : public QMainWindow
{
	Q_OBJECT

    public:
        explicit TupPapagayoDialog(QWidget *parent = nullptr);
        ~TupPapagayoDialog();

    public:
        void openFile(QString filePath);
        bool isOKToCloseDocument();
        void restoreSettings();
        void saveSettings();

    protected:
        void closeEvent(QCloseEvent *event);
        void dragEnterEvent(QDragEnterEvent *event);
        void dropEvent(QDropEvent *event);

    public slots:
        void updateActions();

    private slots:
        void onHelpAboutPapagayo();
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
        void rebuildVoiceList();

        TupLipsyncDoc *document;
        bool enableAutoBreakdown;
        bool rebuildingList;
        int defaultFps;

        Ui::TupPapagayoDialog *ui;
};

#endif // TUPPAPAGAYODIALOG_H

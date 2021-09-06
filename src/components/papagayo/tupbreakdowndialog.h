#ifndef TUPBREAKDOWNDIALOG_H
#define TUPBREAKDOWNDIALOG_H

#include <QDialog>

namespace Ui {
    class TupBreakdownDialog;
}

class LipsyncWord;

class TupBreakdownDialog : public QDialog
{
	Q_OBJECT

    public:
        explicit TupBreakdownDialog(LipsyncWord *word, QWidget *parent = nullptr);
        ~TupBreakdownDialog();

        QString phonemeString();

    private slots:
        void on_aiBut();
        void on_oBut();
        void on_eBut();
        void on_uBut();
        void on_lBut();
        void on_wqBut();
        void on_mbpBut();
        void on_fvBut();
        void on_etcBut();
        void on_restBut();

    private:
        void addPhoneme(QString phoneme);

        Ui::TupBreakdownDialog *ui;
};

#endif // TUPBREAKDOWNDIALOG_H

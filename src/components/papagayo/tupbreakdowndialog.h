#ifndef TUPBREAKDOWNDIALOG_H
#define TUPBREAKDOWNDIALOG_H

#include "tglobal.h"
#include "tuplipsyncdoc.h"

#include <QDialog>
#include <QBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QDialogButtonBox>

/*
namespace Ui {
    class TupBreakdownDialog;
}
*/

// class LipsyncWord;

class Q_DECL_EXPORT TupBreakdownDialog : public QDialog
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
        void setupUI();
        void addPhoneme(QString phoneme);

        // Ui::TupBreakdownDialog *ui;

        QDialog *breakdownDialog;
        QVBoxLayout *verticalLayout;
        QLabel *wordLabel;
        QGridLayout *gridLayout;
        QPushButton *aiBut;
        QPushButton *oBut;
        QPushButton *eBut;
        QPushButton *uBut;
        QPushButton *lBut;
        QPushButton *wqBut;
        QPushButton *mbpBut;
        QPushButton *fvBut;
        QPushButton *etcBut;
        QPushButton *restBut;
        QLineEdit *breakdownEdit;
        QDialogButtonBox *buttonBox;

        QString title;
};

#endif // TUPBREAKDOWNDIALOG_H

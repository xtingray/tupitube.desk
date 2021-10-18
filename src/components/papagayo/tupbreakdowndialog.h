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

#ifndef TUPBREAKDOWNDIALOG_H
#define TUPBREAKDOWNDIALOG_H

#include "tglobal.h"
#include "tuplipsyncdoc.h"

#include <QDialog>
#include <QBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QDialogButtonBox>
#include <QPushButton>

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

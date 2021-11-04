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
#include <QStackedWidget>
#include <QLineEdit>

class TUPITUBE_PLUGIN TupBreakdownDialog: public QDialog
{
    Q_OBJECT

    public:
        TupBreakdownDialog(LipsyncWord *word, const QString &mouthsPath, QWidget *parent = nullptr);
        ~TupBreakdownDialog();

        QString phonemeString();

    private slots:
        void addPhoneme(const QString &phoneme);
        void clearPhonemes();

    private:
        QWidget * createMouthsCollection();
        QWidget * createMouthPanel(int row, int column);

        QLineEdit *breakdownEdit;
        QStackedWidget *stackedWidget;
        QStringList mouthLabels;
        QString folder;
        int currentCollectionIndex;
        QString extension;
};

#endif

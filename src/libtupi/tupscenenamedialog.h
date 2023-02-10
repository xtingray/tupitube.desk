/***************************************************************************
 *   Project TUPITUBE DESK                                                *
 *   Project Contact: info@maefloresta.com                                 *
 *   Project Website: http://www.maefloresta.com                           *
 *   Project Leader: Gustav Gonzalez <info@maefloresta.com>                *
 *                                                                         *
 *   Developers:                                                           *
 *   2010:                                                                 *
 *    Gustavo Gonzalez                                                     *
 *                                                                         *
 *   KTooN's versions:                                                     * 
 *                                                                         *
 *   2006:                                                                 *
 *    David Cuadrado                                                       *
 *    Jorge Cuadrado                                                       *
 *   2003:                                                                 *
 *    Fernado Roldan                                                       *
 *    Simena Dinas                                                         *
 *                                                                         *
 *   Copyright (C) 2010 Gustav Gonzalez - http://www.maefloresta.com       *
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

#ifndef TUPSCENENAMEDIALOG_H
#define TUPSCENENAMEDIALOG_H 

#include "tglobal.h"
#include "tapplicationproperties.h"

#include <QDialog>
#include <QVBoxLayout>
#include <QLineEdit>
#include <QPushButton>

class TUPITUBE_EXPORT TupSceneNameDialog : public QDialog 
{
    Q_OBJECT

    public:
        enum DialogType { Add = 0, Rename };

        TupSceneNameDialog(DialogType type, const QString &sceneName, QWidget *parent = nullptr);
        ~TupSceneNameDialog();

        QString getSceneName() const;

    private slots:
        void checkInput(const QString &input);

    private:
        void setUI(const QString &sceneName);

        QVBoxLayout *layout;
        QLineEdit *sceneInput;
        QPushButton *okButton;

        QString sceneName;
        QString oldName;
        DialogType actionType;
        QString actionDesc;
};

#endif

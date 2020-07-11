/***************************************************************************
 *   Project TUPITUBE DESK                                                *
 *   Project Contact: info@maefloresta.com                                 *
 *   Project Website: http://www.maefloresta.com                           *
 *   Project Leader: Gustav Gonzalez <info@maefloresta.com>                *
 *                                                                         *
 *   Developers:                                                           *
 *   2010:                                                                 *
 *    Gustavo Gonzalez / xtingray                                          *
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

#ifndef TUPGENERALPREFERENCES_H
#define TUPGENERALPREFERENCES_H

#include "tglobal.h"

#include <QLineEdit>
#include <QGridLayout>
#include <QComboBox>
#include <QCheckBox>
#include <QPushButton>
#include <QNetworkAccessManager>
#include <QNetworkReply>

class TUPITUBE_EXPORT TupGeneralPreferences : public QWidget
{
    Q_OBJECT

    public:
        enum Group { Startup = 0, Confirm, Player };
        enum GeneralTab { General = 0, Cache };

        TupGeneralPreferences();
        ~TupGeneralPreferences();

        bool saveValues();
        bool showWarning();

    private slots:
        void updateAppLang(int index);
        void chooseDirectory();
        void restoreCachePath();
        void formatEmail();
        void sendRegisterRequest();
        void registerAnswer(QNetworkReply *reply);
        void slotError(QNetworkReply::NetworkError);

    private:        
        QTabWidget *tabWidget;
        QWidget * generalTab();
        QWidget * cacheTab();
        QWidget * socialTab();
        QString cacheID;
        QLineEdit *cacheString;

        int getLangIndex();
        QGridLayout * createForm(const QString &group, Group groupTag,
                                 QStringList keys, QStringList labels);
        QStringList startup;
        QStringList confirmation;
        QStringList player;

        QComboBox *langCombo;
        QList<QString> langSupport;
        QString newLang;

        QList<QCheckBox *> startupList;
        QList<QCheckBox *> confirmList;
        QList<QCheckBox *> playerList;

        QString cachePath;
        QLineEdit *cacheLine;
        bool langChanged;

        QString username;
        QString passwd;
        QLineEdit *usernameEdit;
        QLineEdit *passwdEdit;
        QLineEdit *emailEdit;
        QPushButton *registerButton;

        QNetworkAccessManager *manager;
};

#endif

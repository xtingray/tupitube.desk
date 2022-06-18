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

#ifndef TUPSOUNDDIALOG_H
#define TUPSOUNDDIALOG_H

#include "tglobal.h"
#include "tapplicationproperties.h"
#include "tupmicmanager.h"

#include <QDialog>
#include <QTabWidget>

/**
 * @author Gustav Gonzalez
**/

class TUPITUBE_EXPORT TupSoundDialog : public QDialog
{
    Q_OBJECT

    public:
        TupSoundDialog(QWidget *parent = nullptr);
        ~TupSoundDialog();

    private slots:
        void loadSoundFile();
        void importSoundAsset();
        void importRecordingAsset();
        void enableDialogButtons(bool enabled);
        void launchLipsyncModule();
        void closeDialog();

    signals:
        void soundFilePicked(const QString &path);
        void lipsyncModuleCalled(PapagayoAppMode mode, const QString &path);

    private:
        QWidget *soundFileTab();
        QWidget *soundRecordTab();

        QTabWidget *tabWidget;
        QToolButton *fileButton;
        QLabel *filePathInput;
        QString soundFilePath;
        QPushButton *importFileButton;
        QPushButton *importRecordButton;
        QPushButton *lipsyncButton01;
        QPushButton *lipsyncButton02;

        TupMicManager *micManager;
};
#endif

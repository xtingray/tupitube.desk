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

#ifndef TUPVIDEOIMPORTERDIALOG_H
#define TUPVIDEOIMPORTERDIALOG_H 

#include "tglobal.h"
#include "tapplicationproperties.h"
#include "tupvideocutter.h"

#include <QDialog>
#include <QVBoxLayout>
#include <QLabel>
#include <QSpinBox>
#include <QProgressBar>

class TUPITUBE_EXPORT TupVideoImporterDialog : public QDialog 
{
    Q_OBJECT

    public:
        TupVideoImporterDialog(const QString &filename, const QSize &projectSize, QWidget *parent = nullptr);
        ~TupVideoImporterDialog();

        int getPhotogramsTotal();

    signals:
        void extractionDone(ImportAction action, const QString &imagesPath);

    public slots:
        void endProcedure();

    private slots:
        void startExtraction();
        void updateStatus(const QString &);
        void updateUI(int index);
        void closeDialog();

    private:
        void setUI();

        QVBoxLayout *layout;
        QSpinBox *imagesBox;
        QWidget *progressWidget;
        QLabel *progressLabel;
        QProgressBar *progressBar;

        QPushButton *okButton;

        TupVideoCutter *videoCutter;
        QString videoPath;
        QString imagesPath;
        int imagesTotal;
        bool extractionStarted;
        int advance;
        QSize videoSize;
};

#endif

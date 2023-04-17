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
#include <QGroupBox>
#include <QRadioButton>

class TUPITUBE_EXPORT TupVideoImporterDialog : public QDialog 
{
    Q_OBJECT

    public:
        TupVideoImporterDialog(const QString &filename, const QString &photogramsPath, const QSize &projectSize,
                               TupVideoCutter *cutter, QWidget *parent = nullptr);
        ~TupVideoImporterDialog();

        int getPhotogramsTotal();

    signals:
        void extractionDone(ImportAction action, const QString &imagesPath, bool sizeFlag);
        void projectSizeHasChanged(const QSize dimension);

    public slots:
        void endProcedure();

    private slots:
        void startExtraction();
        void updateStatus(const QString &);
        void updateUI(int index);
        void startImageImportation();

    private:
        void setUI(bool fixSize);

        QVBoxLayout *layout;
        QSpinBox *imagesBox;
        QWidget *progressWidget;
        QLabel *progressLabel;
        QProgressBar *progressBar;

        QWidget *buttonsWidget;

        TupVideoCutter *videoCutter;
        QString videoPath;
        QString imagesPath;
        int imagesTotal;
        int advance;
        QSize projectSize;
        QSize videoSize;

        bool fixSize;
        bool sizeFlag;
        QGroupBox *groupBox;
        QRadioButton *checkButton1;
        QRadioButton *checkButton2;
        QRadioButton *checkButton3;
};

#endif

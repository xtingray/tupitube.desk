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

#include "tupvideoimporterdialog.h"
#include "tconfig.h"
#include "tseparator.h"
#include "tapptheme.h"
#include "tupvideocutter.h"
#include "talgorithm.h"
#include "tosd.h"
#include "tupprojectrequest.h"

#include <QPushButton>

TupVideoImporterDialog::TupVideoImporterDialog(const QString &filename, QWidget *parent) : QDialog(parent)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupVideoImporterDialog::TupVideoImporterDialog()]";
    #endif

    setModal(true);
    videoPath = filename;

    imagesTotal = 1;
    extractionStarted = false;

    QFileInfo fileInfo(videoPath);
    setWindowTitle(tr("Photograms Extractor") + " (" + fileInfo.fileName() + ")");
    setWindowIcon(QIcon(QPixmap(THEME_DIR + "icons/scenes.png")));
    setStyleSheet(TAppTheme::themeSettings());

    layout = new QVBoxLayout(this);
    setUI();
}

TupVideoImporterDialog::~TupVideoImporterDialog()
{
}

void TupVideoImporterDialog::setUI()
{
    QLabel *importLabel = new QLabel(tr("Select the amount of photograms to import:"));
    imagesBox = new QSpinBox;
    imagesBox->setMinimum(1);
    imagesBox->setMaximum(100);
    imagesBox->setValue(1);

    QWidget *formWidget = new QWidget;
    QHBoxLayout *formLayout = new QHBoxLayout(formWidget);
    formLayout->addWidget(importLabel);
    formLayout->addWidget(imagesBox);

    layout->addWidget(formWidget);

    progressBar = new QProgressBar;
    progressBar->setTextVisible(true);
    progressBar->setRange(1, 50);

    progressWidget = new QWidget;
    progressLabel = new QLabel("");
    progressLabel->setAlignment(Qt::AlignHCenter);

    QVBoxLayout *progressLayout = new QVBoxLayout(progressWidget);
    progressLayout->addWidget(progressLabel);
    progressLayout->addWidget(progressBar);
    progressWidget->setVisible(false);

    layout->addWidget(progressWidget);

    okButton = new QPushButton(QIcon(QPixmap(THEME_DIR + "icons/apply.png")), "");

    connect(okButton, SIGNAL(clicked()), this, SLOT(startExtraction()));

    QPushButton *closeButton = new QPushButton(QIcon(QPixmap(THEME_DIR + "icons/close.png")), "");
    closeButton->setToolTip(tr("Close"));
    connect(closeButton, SIGNAL(clicked()), this, SLOT(closeDialog()));

    QWidget *buttonsWidget = new QWidget;
    QHBoxLayout *buttonLayout = new QHBoxLayout(buttonsWidget);
    buttonLayout->addWidget(okButton);
    buttonLayout->addWidget(closeButton);

    layout->addWidget(buttonsWidget, 1, Qt::AlignRight);
    layout->addStretch(1);
}

void TupVideoImporterDialog::startExtraction()
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupVideoImporterDialog::startExtraction()]";
    #endif

    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
    imagesBox->setEnabled(false);
    okButton->setVisible(false);

    progressWidget->setVisible(true);
    progressLabel->setText(tr("Starting procedure..."));
    extractionStarted = true;

    imagesTotal = imagesBox->value();
    advance = 100/imagesTotal;

    QString tempFolder = TAlgorithm::randomString(10);
    imagesPath = CACHE_DIR + tempFolder + "/";

    if (!QFile::exists(imagesPath)) {
        QDir dir;
        if (!dir.mkpath(imagesPath)) {
            #ifdef TUP_DEBUG
                qDebug() << "[TupLibraryWidget::importVideoFile()] - Fatal Error: Couldn't create directory -> " << imagesPath;
            #endif
            TOsd::self()->display(TOsd::Error, tr("Couldn't create temporary directory!"));

            return;
        }
    }

    TupVideoCutter *videoCutter = new TupVideoCutter();
    connect(videoCutter, SIGNAL(msgSent(const QString &)), this, SLOT(updateStatus(const QString &)));
    connect(videoCutter, SIGNAL(imageExtracted(int)), this, SLOT(updateUI(int)));

    // Here: Compare video dimension and project dimension and ask the user to change it if it's necessary

    videoCutter->processFile(videoPath, imagesPath, imagesTotal);
    extractionStarted = false;
}

void TupVideoImporterDialog::updateUI(int index)
{
    progressLabel->setText(tr("Extracting photogram #") + QString::number(index));
    progressBar->setValue(progressBar->value() + advance);

    if (index == imagesBox->value()) {
        #ifdef TUP_DEBUG
            qDebug() << "[TupLibraryWidget::updateUI()] - Extraction is complete!";
        #endif
        emit  extractionDone(VideoAction, imagesPath);
    }
}

void TupVideoImporterDialog::updateStatus(const QString &msg)
{
    progressLabel->setText(msg);
}

void TupVideoImporterDialog::closeDialog()
{
    if (extractionStarted)
        qDebug() << "[TupLibraryWidget::closeDialog()] - Do you want to cancel de project?";

    endProcedure();
}

void TupVideoImporterDialog::endProcedure()
{
    QDir imgDir(imagesPath);
    #ifdef TUP_DEBUG
        qDebug() << "[TupLibraryWidget::removeTempFolder()] - Removing temporary folder -> " << imagesPath;
    #endif
    if (imgDir.exists()) {
        if (!imgDir.removeRecursively()) {
            #ifdef TUP_DEBUG
                qWarning() << "[TupLibraryWidget::removeTempFolder()] - Error: Can't remove temporary folder -> " << imagesPath;
            #endif
        }
    }
    QApplication::restoreOverrideCursor();
    TOsd::self()->display(TOsd::Info, tr("Video imported successfully!"));
    close();
}

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
#include "talgorithm.h"
#include "tosd.h"
#include "tupprojectrequest.h"
#include "tseparator.h"

#include <QPushButton>

TupVideoImporterDialog::TupVideoImporterDialog(const QString &filename, const QString &photogramsPath, const QSize &canvasSize,
                                               TupVideoCutter *cutter, QWidget *parent) : QDialog(parent)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupVideoImporterDialog::TupVideoImporterDialog()]";
    #endif

    setModal(true);
    videoPath = filename;
    projectSize = canvasSize;

    imagesTotal = 1;
    sizeFlag = false;

    QFileInfo fileInfo(videoPath);
    setWindowTitle(tr("Photograms Extractor") + " (" + fileInfo.fileName() + ")");
    setWindowIcon(QIcon(QPixmap(THEME_DIR + "icons/scenes.png")));
    setStyleSheet(TAppTheme::themeSettings());

    videoCutter = cutter;
    connect(videoCutter, SIGNAL(msgSent(const QString &)), this, SLOT(updateStatus(const QString &)));
    connect(videoCutter, SIGNAL(imageExtracted(int)), this, SLOT(updateUI(int)));

    imagesPath = photogramsPath;
    videoSize = videoCutter->getVideoSize();

    layout = new QVBoxLayout(this);
    fixSize = projectSize != videoSize;
    setUI(fixSize);
}

TupVideoImporterDialog::~TupVideoImporterDialog()
{
    delete videoCutter;
}

void TupVideoImporterDialog::setUI(bool fixSize)
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

    if (fixSize) {
        QWidget *sizeWidget = new QWidget;
        QVBoxLayout *sizeLayout = new QVBoxLayout(sizeWidget);

        TSeparator *div = new TSeparator();

        QLabel *sizeLabel = new QLabel("<b>" + tr("Project size and video size are different:") + "</b>");
        sizeLabel->setAlignment(Qt::AlignHCenter);

        QLabel *canvasLabel = new QLabel(tr("Project Size:") + " <b>" + QString::number(projectSize.width())
                                         + "x" + QString::number(projectSize.height()) + "</b>");
        canvasLabel->setAlignment(Qt::AlignHCenter);
        QLabel *videoLabel = new QLabel(tr("Video Size:") + " <b>" + QString::number(videoSize.width())
                                         + "x" + QString::number(videoSize.height()) + "</b>");
        videoLabel->setAlignment(Qt::AlignHCenter);

        groupBox = new QGroupBox(tr("What do you want to do?"));
        checkButton1 = new QRadioButton(tr("Keep original project and video sizes"));
        checkButton2 = new QRadioButton(tr("Adjust video size to project size"));
        checkButton3 = new QRadioButton(tr("Adjust project size to video size"));
        checkButton1->setChecked(true);

        QVBoxLayout *optionsBox = new QVBoxLayout;
        optionsBox->addWidget(checkButton1);
        optionsBox->addWidget(checkButton2);
        optionsBox->addWidget(checkButton3);
        optionsBox->addStretch(1);

        groupBox->setLayout(optionsBox);

        sizeLayout->addWidget(div);
        sizeLayout->addWidget(sizeLabel);
        sizeLayout->addWidget(canvasLabel);
        sizeLayout->addWidget(videoLabel);
        sizeLayout->addWidget(groupBox);

        layout->addWidget(sizeWidget);
    }

    progressBar = new QProgressBar;
    progressBar->setTextVisible(true);
    progressBar->setRange(0, 100);

    progressWidget = new QWidget;
    progressLabel = new QLabel("");
    progressLabel->setAlignment(Qt::AlignHCenter);

    QVBoxLayout *progressLayout = new QVBoxLayout(progressWidget);
    progressLayout->addWidget(progressLabel);
    progressLayout->addWidget(progressBar);
    progressWidget->setVisible(false);

    layout->addWidget(progressWidget);

    QPushButton *okButton = new QPushButton(QIcon(QPixmap(THEME_DIR + "icons/apply.png")), "");
    connect(okButton, SIGNAL(clicked()), this, SLOT(startExtraction()));

    QPushButton *closeButton = new QPushButton(QIcon(QPixmap(THEME_DIR + "icons/close.png")), "");
    closeButton->setToolTip(tr("Close"));
    connect(closeButton, SIGNAL(clicked()), this, SLOT(close()));

    buttonsWidget = new QWidget;
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
    imagesTotal = imagesBox->value();
    advance = 100/imagesTotal;

    imagesBox->setEnabled(false);
    buttonsWidget->setVisible(false);

    if (fixSize) {
        if (checkButton2->isChecked()) {
            // Adjust photograms size
            #ifdef TUP_DEBUG
                qDebug() << "[TupVideoImporterDialog::startExtraction()] - Resizing photograms...";
            #endif
            sizeFlag = true;
        } if (checkButton3->isChecked()) {
            // Adjust project size
            #ifdef TUP_DEBUG
                qDebug() << "[TupVideoImporterDialog::startExtraction()] - Resizing project canvas...";
            #endif
            emit projectSizeHasChanged(videoSize);
        }
    }

    progressWidget->setVisible(true);
    progressLabel->setText(tr("Starting procedure..."));

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

    videoCutter->setPhotogramsTotal(imagesTotal);
    if (!videoCutter->startExtraction()) {
        TOsd::self()->display(TOsd::Error, tr("Can't extract photograms!"));
        videoCutter->releaseResources();

        return;
    }

    videoCutter->releaseResources();
}

void TupVideoImporterDialog::updateUI(int index)
{
    QString msg = tr("Extracting photogram %1 of %2").arg(index).arg(imagesTotal);
    progressLabel->setText(msg);
    progressBar->setValue(advance);
    advance += advance;

    if (index == imagesTotal) {
        #ifdef TUP_DEBUG
            qDebug() << "[TupLibraryWidget::updateUI()] - Extraction is complete!";
            qDebug() << "[TupLibraryWidget::updateUI()] - Starting image importation...";
        #endif
        progressLabel->setText(tr("Importing images..."));
        emit extractionDone(VideoAction, imagesPath, sizeFlag);
    }
}

void TupVideoImporterDialog::updateStatus(const QString &msg)
{
    progressLabel->setText(msg);
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

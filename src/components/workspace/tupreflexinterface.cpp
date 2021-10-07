/***************************************************************************
 *   Project TUPITUBE DESK                                                *
 *   Project Contact: info@maefloresta.com                                 *
 *   Project Website: http://www.maefloresta.com                           *
 *   Project Leader: Gustav Gonzalez <info@maefloresta.com>                *
 *                                                                         *
 *   Developers:                                                           *
 *   2010:                                                                 *
 *    Gustav Gonzalez / xtingray                                           *
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

#include "tupreflexinterface.h"
#include "tupapplication.h"
#include "tapplicationproperties.h"
#include "talgorithm.h"
#include "tosd.h"

#include <QBoxLayout>
#include <QIcon>
#include <QDir>
#include <QScreen>
#include <QPushButton>
#include <QSpinBox>
#include <QColorDialog>

TupReflexInterface::TupReflexInterface(const QString &cameraDesc, const QString &resolution, QCameraInfo cameraDevice, 
                                       const QSize cameraSize, int i, QWidget *parent) : QDialog(parent)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupReflexInterface()]";
    #endif

    setModal(false);
    setWindowFlags(windowFlags() | Qt::WindowMinimizeButtonHint);
    setWindowTitle(tr("TupiTube Camera Manager") + " | " + tr("Current resolution:") + " " + resolution);
    setWindowIcon(QIcon(QPixmap(THEME_DIR + "icons/camera.png")));

    counter = i;

    QSize displaySize = cameraSize;
    // QDesktopWidget desktop;
    QScreen *screen = QGuiApplication::screens().at(0);
    int desktopWidth = screen->geometry().width();

    if (cameraSize.width() > desktopWidth) {
        int width = desktopWidth / 2;
        int height = width * cameraSize.height() / cameraSize.width();
        displaySize = QSize(width, height);
    } else {
        int maxWidth = 640;
        if (desktopWidth > 800)
            maxWidth = 800;

        if (cameraSize.width() > maxWidth) {
            int height = maxWidth * cameraSize.height() / cameraSize.width();
            displaySize = QSize(maxWidth, height);
        }
    }

    randomPath();

    camera = new QCamera(cameraDevice);
    imageCapture = new QCameraImageCapture(camera);
    camera->setCaptureMode(QCamera::CaptureStillImage);

    connect(camera, SIGNAL(error(QCamera::Error)), this, SLOT(error(QCamera::Error)));
    connect(imageCapture, SIGNAL(imageSaved(int, const QString)), this, SLOT(imageSavedFromCamera(int, const QString)));

    cameraScreen = new TupReflexRenderArea(displaySize);

    QWidget *menuWidget = new QWidget;
    QBoxLayout *menuLayout = new QBoxLayout(QBoxLayout::TopToBottom, menuWidget);

    QLabel *devicesLabel = new QLabel;
    QString deviceString = tr("Camera");

    devicesLabel->setText(deviceString);
    devicesLabel->setAlignment(Qt::AlignHCenter);

    menuLayout->addWidget(devicesLabel);

    QLabel *deviceDesc = new QLabel;
    QFont font = deviceDesc->font();
    font.setBold(true);
    deviceDesc->setFont(font);
    deviceDesc->setText(cameraDesc);
    menuLayout->addWidget(deviceDesc);

    clickButton = new QPushButton(QIcon(QPixmap(THEME_DIR + "icons/photo.png")), "");
    clickButton->setIconSize(QSize(20, 20));
    clickButton->setToolTip(tr("Take picture"));
    connect(clickButton, SIGNAL(clicked()), this, SLOT(takePicture()));

    safeAreaButton = new QPushButton(QIcon(QPixmap(THEME_DIR + "icons/safe_area.png")), "");
    safeAreaButton->setIconSize(QSize(20, 20));
    safeAreaButton->setToolTip(tr("Show safe area"));
    safeAreaButton->setShortcut(QKeySequence(tr("+")));
    safeAreaButton->setCheckable(true);
    connect(safeAreaButton, SIGNAL(clicked()), this, SLOT(enableActionSafeArea()));

    gridButton = new QPushButton(QIcon(QPixmap(THEME_DIR + "icons/subgrid.png")), "");
    gridButton->setIconSize(QSize(20, 20));
    gridButton->setToolTip(tr("Show grid"));
    gridButton->setShortcut(QKeySequence(tr("#")));
    gridButton->setCheckable(true);
    connect(gridButton, SIGNAL(clicked()), this, SLOT(enableGrid()));

    gridWidget = new QWidget;
    QGridLayout *gridLayout = new QGridLayout(gridWidget);
    gridLayout->setHorizontalSpacing(2);

    QLabel *gridLabel = new QLabel;
    gridLabel->setPixmap(QPixmap(THEME_DIR + "icons/grid_spacing.png"));
    gridLabel->setToolTip(tr("Grid spacing"));
    gridLabel->setMargin(2);

    QSpinBox *gridSpacing = new QSpinBox;
    gridSpacing->setSingleStep(10);
    gridSpacing->setRange(10, 100);
    gridSpacing->setValue(10);
    connect(gridSpacing, SIGNAL(valueChanged(int)), this, SLOT(updateGridSpacing(int)));

    QLabel *colorLabel = new QLabel;
    colorLabel->setPixmap(QPixmap(THEME_DIR + "icons/color_palette.png"));
    colorLabel->setToolTip(tr("Grid color"));
    colorLabel->setMargin(2);

    gridColor = QColor(0, 0, 180, 50);
    colorCell = new TupColorWidget(gridColor);
    connect(colorCell, SIGNAL(clicked()), this, SLOT(updateColour()));

    gridLayout->addWidget(gridLabel, 0, 0, Qt::AlignHCenter);
    gridLayout->addWidget(gridSpacing, 0, 1, Qt::AlignHCenter);
    gridLayout->addWidget(colorLabel, 1, 0, Qt::AlignHCenter);
    gridLayout->addWidget(colorCell, 1, 1, Qt::AlignHCenter);

    gridWidget->setVisible(false);

    historyButton = new QPushButton(QIcon(QPixmap(THEME_DIR + "icons/bitmap_array.png")), "");
    historyButton->setIconSize(QSize(20, 20));
    historyButton->setToolTip(tr("Show previous images"));
    historyButton->setShortcut(QKeySequence(tr("P")));
    historyButton->setCheckable(true);
    connect(historyButton, SIGNAL(clicked()), this, SLOT(showHistory()));

    historyButton->setVisible(false);

    historyWidget = new QWidget;
    QGridLayout *historyLayout = new QGridLayout(historyWidget);
    historyLayout->setHorizontalSpacing(2);

    QLabel *opacityLabel = new QLabel;
    opacityLabel->setPixmap(QPixmap(THEME_DIR + "icons/onion.png"));
    opacityLabel->setToolTip(tr("Image opacity level"));
    QDoubleSpinBox *opacitySpin = new QDoubleSpinBox;
    opacitySpin->setSingleStep(0.1);
    opacitySpin->setValue(0.5);
    opacitySpin->setRange(0.0, 1.0);
    opacitySpin->setDecimals(2);
    connect(opacitySpin, SIGNAL(valueChanged(double)), this, SLOT(updateImagesOpacity(double)));

    QLabel *previousLabel = new QLabel;
    previousLabel->setPixmap(QPixmap(THEME_DIR + "icons/layer.png"));
    previousLabel->setToolTip(tr("Amount of images to show"));
    QSpinBox *previousSpin = new QSpinBox;
    previousSpin->setValue(1);
    previousSpin->setRange(0, 5);
    connect(previousSpin, SIGNAL(valueChanged(int)), this, SLOT(updateImagesDepth(int)));

    historyLayout->addWidget(opacityLabel, 0, 0, Qt::AlignHCenter);
    historyLayout->addWidget(opacitySpin, 0, 1, Qt::AlignHCenter);
    historyLayout->addWidget(previousLabel, 1, 0, Qt::AlignHCenter);
    historyLayout->addWidget(previousSpin, 1, 1, Qt::AlignHCenter);

    historyWidget->setVisible(false);

    QPushButton *exitButton = new QPushButton(QIcon(QPixmap(THEME_DIR + "icons/exit.png")), "");
    exitButton->setIconSize(QSize(20, 20));
    exitButton->setToolTip(tr("Close manager"));
    exitButton->setShortcut(Qt::Key_Escape);
    connect(exitButton, SIGNAL(clicked()), this, SLOT(close()));

    menuLayout->addWidget(new TSeparator(Qt::Horizontal));

    menuLayout->addWidget(clickButton);
    menuLayout->addWidget(safeAreaButton);
    menuLayout->addWidget(gridButton);
    menuLayout->addWidget(gridWidget);
    menuLayout->addWidget(historyButton);
    menuLayout->addWidget(historyWidget);
    menuLayout->addWidget(exitButton);
    menuLayout->addStretch();

    QBoxLayout *dialogLayout = new QBoxLayout(QBoxLayout::LeftToRight, this); 
    dialogLayout->addWidget(cameraScreen);
    dialogLayout->addWidget(menuWidget);
}

TupReflexInterface::~TupReflexInterface()
{
    #ifdef TUP_DEBUG
        qDebug() << "[~TupReflexInterface()]";
    #endif
}

void TupReflexInterface::closeEvent(QCloseEvent *event)
{
    Q_UNUSED(event)

    reset();
    emit closed();
}

void TupReflexInterface::randomPath()
{
    path = CACHE_DIR + TAlgorithm::randomString(8);

    QDir dir;
    if (!dir.mkdir(path)) {
        #ifdef TUP_DEBUG
            qDebug() << "[TupReflexInterface::randomPath()] - Fatal Error: Can't create pictures directory -> " << path;
        #endif

        path = "";
        TOsd::self()->display(TOsd::Error, tr("Can't create pictures directory"));
    }
}

void TupReflexInterface::takePicture()
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupReflexInterface::takePicture()]";
    #endif

    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

    QString prev = "pic";
    if (counter < 10)
        prev += "00";
    if (counter >= 10 && counter < 100)
        prev += "0";
    QString imagePath = path + "/" + prev + QString::number(counter) + ".jpg";

    camera->load();
    camera->start();

    // on half pressed shutter button
    camera->searchAndLock();
    // on shutter button pressed
    imageCapture->capture(imagePath);
    //on shutter button released
    camera->unlock();

    camera->stop();
    camera->unload();

    // Take picture here
    counter++;
}

void TupReflexInterface::reset()
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupReflexInterface::reset()]";
    #endif

    if (camera->state() == QCamera::ActiveState)
        camera->stop();

    QDir dir(path);
    foreach (QString file, dir.entryList(QStringList() << "*.jpg")) {
        QString absolute = dir.absolutePath() + "/" + file;
        QFile::remove(absolute);
    }

    if (!dir.rmdir(dir.absolutePath())) {
        #ifdef TUP_DEBUG
            qDebug() << "[TupReflexInterface::reset()] - Fatal Error: Can't remove pictures directory -> " << dir.absolutePath();
        #endif
    }
}

void TupReflexInterface::error(QCamera::Error error)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupReflexInterface::error()] - Fatal Error: Camera error code -> " << error;
    #endif

    switch (error) {
        case QCamera::NoError:
        {
            break;
        }
        case QCamera::CameraError:
        {
            QMessageBox::warning(this, "TupReflexInterface", tr("General Camera error"));
            break;
        }
        case QCamera::InvalidRequestError:
        {
            QMessageBox::warning(this, "TupReflexInterface", tr("Camera invalid request error"));
            break;
        }
        case QCamera::ServiceMissingError:
        {
            QMessageBox::warning(this, "TupReflexInterface", tr("Camera service missing error"));
            break;
        }
        case QCamera::NotSupportedFeatureError :
        {
            QMessageBox::warning(this, "TupReflexInterface", tr("Camera not supported error"));
            break;
        }
    };
}

void TupReflexInterface::imageSavedFromCamera(int id, const QString folder)
{
    Q_UNUSED(id)

    #ifdef TUP_DEBUG
        qDebug() << "[TupCameraInterface::imageSavedFromCamera()] - Picture path -> " << folder;
    #endif

    if (folder.isEmpty())
        return;

    emit pictureHasBeenSelected(counter, folder);
    cameraScreen->addPixmap(folder);

    if (!historyButton->isVisible())
        historyButton->setVisible(true);
}

void TupReflexInterface::enableActionSafeArea()
{
    cameraScreen->enableSafeArea(safeAreaButton->isChecked());
}

void TupReflexInterface::enableGrid()
{
    bool flag = gridButton->isChecked();
    gridWidget->setVisible(flag);
    cameraScreen->enableGrid(flag);
}

void TupReflexInterface::updateColour()
{
    QColor color = QColorDialog::getColor(gridColor, this);
    if (color.isValid()) {
        cameraScreen->updateGridColor(color);
        colorCell->setBrush(QBrush(color));
    }
}

void TupReflexInterface::updateGridSpacing(int space)
{
    cameraScreen->updateGridSpacing(space);
}

void TupReflexInterface::showHistory()
{
    bool flag = historyButton->isChecked();
    historyWidget->setVisible(flag);
    cameraScreen->showHistory(flag);
}

void TupReflexInterface::updateImagesOpacity(double opacity)
{
    cameraScreen->updateImagesOpacity(opacity);
}

void TupReflexInterface::updateImagesDepth(int depth)
{
    cameraScreen->updateImagesDepth(depth);
}

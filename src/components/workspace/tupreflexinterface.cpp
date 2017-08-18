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
#include "tupreflexrenderarea.h"
#include "tupapplication.h"
#include "tapplicationproperties.h"
#include "talgorithm.h"
#include "tosd.h"
#include "tupcolorwidget.h"

#include <QCameraImageCapture>
#include <QBoxLayout>
#include <QIcon>
#include <QDir>
#include <QDesktopWidget>
#include <QPushButton>
#include <QSpinBox>
#include <QColorDialog>

struct TupReflexInterface::Private
{
    QCamera *camera;
    QCameraImageCapture *imageCapture;
    int counter;
    QString path;

    QPushButton *clickButton;
    QPushButton *safeAreaButton;
    QPushButton *gridButton;

    QWidget *gridWidget;
    QColor gridColor;
    TupColorWidget *colorCell;

    QPushButton *historyButton;
    QWidget *historyWidget;

    TupReflexRenderArea *screen;
};

TupReflexInterface::TupReflexInterface(const QString &cameraDesc, const QString &resolution, QByteArray cameraDevice, 
                                       const QSize cameraSize, int counter, QWidget *parent) : QDialog(parent), k(new Private)
{
    #ifdef TUP_DEBUG
        #ifdef Q_OS_WIN
            qDebug() << "[TupReflexInterface()]";
        #else
            TINIT;
        #endif
    #endif

    setModal(false);
    setWindowFlags(windowFlags() | Qt::WindowMinimizeButtonHint);
    setWindowTitle(tr("TupiTube Camera Manager") + " | " + tr("Current resolution:") + " " + resolution);
    setWindowIcon(QIcon(QPixmap(THEME_DIR + "icons/camera.png")));

    k->counter = counter;

    QSize displaySize = cameraSize;
    QDesktopWidget desktop;
    int desktopWidth = desktop.screenGeometry().width();

    if (cameraSize.width() > desktopWidth) {
        int width = desktopWidth/2;
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

    k->camera = new QCamera(cameraDevice);
    k->imageCapture = new QCameraImageCapture(k->camera);
    k->camera->setCaptureMode(QCamera::CaptureStillImage);

    connect(k->camera, SIGNAL(error(QCamera::Error)), this, SLOT(error(QCamera::Error)));
    connect(k->imageCapture, SIGNAL(imageSaved(int, const QString)), this, SLOT(imageSavedFromCamera(int, const QString)));

    k->screen = new TupReflexRenderArea(displaySize);

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

    k->clickButton = new QPushButton(QIcon(QPixmap(THEME_DIR + "icons/photo.png")), "");
    k->clickButton->setIconSize(QSize(20, 20));
    k->clickButton->setToolTip(tr("Take picture"));
    connect(k->clickButton, SIGNAL(clicked()), this, SLOT(takePicture()));

    k->safeAreaButton = new QPushButton(QIcon(QPixmap(THEME_DIR + "icons/safe_area.png")), "");
    k->safeAreaButton->setIconSize(QSize(20, 20));
    k->safeAreaButton->setToolTip(tr("Show safe area"));
    k->safeAreaButton->setShortcut(QKeySequence(tr("+")));
    k->safeAreaButton->setCheckable(true);
    connect(k->safeAreaButton, SIGNAL(clicked()), this, SLOT(enableActionSafeArea()));

    k->gridButton = new QPushButton(QIcon(QPixmap(THEME_DIR + "icons/subgrid.png")), "");
    k->gridButton->setIconSize(QSize(20, 20));
    k->gridButton->setToolTip(tr("Show grid"));
    k->gridButton->setShortcut(QKeySequence(tr("#")));
    k->gridButton->setCheckable(true);
    connect(k->gridButton, SIGNAL(clicked()), this, SLOT(enableGrid()));

    k->gridWidget = new QWidget;
    QGridLayout *gridLayout = new QGridLayout(k->gridWidget);
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

    k->gridColor = QColor(0, 0, 180, 50);
    k->colorCell = new TupColorWidget(k->gridColor);
    connect(k->colorCell, SIGNAL(clicked()), this, SLOT(updateColour()));

    gridLayout->addWidget(gridLabel, 0, 0, Qt::AlignHCenter);
    gridLayout->addWidget(gridSpacing, 0, 1, Qt::AlignHCenter);
    gridLayout->addWidget(colorLabel, 1, 0, Qt::AlignHCenter);
    gridLayout->addWidget(k->colorCell, 1, 1, Qt::AlignHCenter);

    k->gridWidget->setVisible(false);

    k->historyButton = new QPushButton(QIcon(QPixmap(THEME_DIR + "icons/bitmap_array.png")), "");
    k->historyButton->setIconSize(QSize(20, 20));
    k->historyButton->setToolTip(tr("Show previous images"));
    k->historyButton->setShortcut(QKeySequence(tr("P")));
    k->historyButton->setCheckable(true);
    connect(k->historyButton, SIGNAL(clicked()), this, SLOT(showHistory()));

    k->historyButton->setVisible(false); 

    k->historyWidget = new QWidget;
    QGridLayout *historyLayout = new QGridLayout(k->historyWidget);
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

    k->historyWidget->setVisible(false);

    QPushButton *exitButton = new QPushButton(QIcon(QPixmap(THEME_DIR + "icons/exit.png")), "");
    exitButton->setIconSize(QSize(20, 20));
    exitButton->setToolTip(tr("Close manager"));
    exitButton->setShortcut(Qt::Key_Escape);
    connect(exitButton, SIGNAL(clicked()), this, SLOT(close()));

    menuLayout->addWidget(new TSeparator(Qt::Horizontal));

    menuLayout->addWidget(k->clickButton);
    menuLayout->addWidget(k->safeAreaButton);
    menuLayout->addWidget(k->gridButton);
    menuLayout->addWidget(k->gridWidget);
    menuLayout->addWidget(k->historyButton);
    menuLayout->addWidget(k->historyWidget);
    menuLayout->addWidget(exitButton);
    menuLayout->addStretch();

    QBoxLayout *dialogLayout = new QBoxLayout(QBoxLayout::LeftToRight, this); 
    dialogLayout->addWidget(k->screen);
    dialogLayout->addWidget(menuWidget);
}

TupReflexInterface::~TupReflexInterface()
{
    #ifdef TUP_DEBUG
        #ifdef Q_OS_WIN
            qDebug() << "[~TupReflexInterface()]";
        #else
            TEND;
        #endif
    #endif
}

void TupReflexInterface::closeEvent(QCloseEvent *event)
{
    Q_UNUSED(event);

    reset();
    emit closed();
}

void TupReflexInterface::randomPath()
{
    k->path = CACHE_DIR + TAlgorithm::randomString(8);

    QDir dir;
    if (!dir.mkdir(k->path)) {
        #ifdef TUP_DEBUG
            QString msg = "TupReflexInterface::randomPath() - Fatal Error: Can't create pictures directory -> " + k->path;
            #ifdef Q_OS_WIN
                qDebug() << msg;
            #else
                tError() << msg;
            #endif
        #endif

        k->path = "";
        TOsd::self()->display(tr("Error"), tr("Can't create pictures directory"), TOsd::Error);
    }
}

void TupReflexInterface::takePicture()
{
    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

    QString prev = "pic";
    if (k->counter < 10)
        prev += "00";
    if (k->counter >= 10 && k->counter < 100)
        prev += "0";
    QString imagePath = k->path + "/" + prev + QString::number(k->counter) + ".jpg";

    k->camera->load();
    k->camera->start();

    // on half pressed shutter button
    k->camera->searchAndLock();
    // on shutter button pressed
    k->imageCapture->capture(imagePath);
    //on shutter button released
    k->camera->unlock();

    k->camera->stop();
    k->camera->unload(); 

    // Take picture here
    k->counter++;
}

void TupReflexInterface::reset()
{
    if (k->camera->state() == QCamera::ActiveState)
        k->camera->stop();

    QDir dir(k->path);
    foreach (QString file, dir.entryList(QStringList() << "*.jpg")) {
             QString absolute = dir.absolutePath() + "/" + file;
             QFile::remove(absolute);
    }

    if (!dir.rmdir(dir.absolutePath())) {
        #ifdef TUP_DEBUG
            QString msg = "TupReflexInterface::reset() - Fatal Error: Can't remove pictures directory -> " + dir.absolutePath();
            #ifdef Q_OS_WIN
                qDebug() << msg;
            #else
                tError() << msg;
            #endif
        #endif
    }
}

void TupReflexInterface::error(QCamera::Error error)
{
    #ifdef TUP_DEBUG
        QString msg = "TupReflexInterface::error() - Fatal Error: Camera error code -> ";
        #ifdef Q_OS_WIN
            qDebug() << msg << error;
        #else
            tError() << msg << error;
        #endif
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

void TupReflexInterface::imageSavedFromCamera(int id, const QString path)
{
    Q_UNUSED(id);

    #ifdef TUP_DEBUG
        QString msg = "TupCameraInterface::imageSavedFromCamera() - Picture path -> " + path;
        #ifdef Q_OS_WIN
            qDebug() << msg;
        #else
            tWarning() << msg;
        #endif
    #endif

    if (path.isEmpty())
        return;

    emit pictureHasBeenSelected(k->counter, path);
    k->screen->addPixmap(path);

    if (!k->historyButton->isVisible())
        k->historyButton->setVisible(true);
}

void TupReflexInterface::enableActionSafeArea()
{
    k->screen->enableSafeArea(k->safeAreaButton->isChecked());
}

void TupReflexInterface::enableGrid()
{
    bool flag = k->gridButton->isChecked();
    k->gridWidget->setVisible(flag);
    k->screen->enableGrid(flag);
}

void TupReflexInterface::updateColour()
{
    QColor color = QColorDialog::getColor(k->gridColor, this);
    if (color.isValid()) {
        k->screen->updateGridColor(color);
        k->colorCell->setBrush(QBrush(color));
    }
}

void TupReflexInterface::updateGridSpacing(int space)
{
    k->screen->updateGridSpacing(space);
}

void TupReflexInterface::showHistory()
{
    bool flag = k->historyButton->isChecked();
    k->historyWidget->setVisible(flag);
    k->screen->showHistory(flag);
}

void TupReflexInterface::updateImagesOpacity(double opacity)
{
    k->screen->updateImagesOpacity(opacity);
}

void TupReflexInterface::updateImagesDepth(int depth)
{
    k->screen->updateImagesDepth(depth);
}

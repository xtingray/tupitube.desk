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

#include <QCameraImageCapture>
#include <QBoxLayout>
#include <QIcon>
#include <QDir>
#include <QDesktopWidget>
#include <QPushButton>

struct TupReflexInterface::Private
{
    QCamera *camera;
    QCameraImageCapture *imageCapture;
    int counter;
    QString path;
    QPushButton *clickButton;
    QLabel *screen;
    QList<QPixmap> stack;
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

    QPixmap canvas(cameraSize);
    canvas.fill(Qt::gray);
    k->screen = new QLabel;
    k->screen->setPixmap(canvas);

    QWidget *menuWidget = new QWidget;
    QBoxLayout *menuLayout = new QBoxLayout(QBoxLayout::TopToBottom, menuWidget);

    QLabel *devicesLabel = new QLabel;
    QString deviceString = tr("Camera");

    devicesLabel->setText(deviceString);
    devicesLabel->setAlignment(Qt::AlignHCenter);

    k->clickButton = new QPushButton(QIcon(QPixmap(THEME_DIR + "icons/photo.png")), "");
    k->clickButton->setIconSize(QSize(20, 20));
    k->clickButton->setToolTip(tr("Take picture"));
    connect(k->clickButton, SIGNAL(clicked()), this, SLOT(takePicture()));

    menuLayout->addWidget(devicesLabel);

    QLabel *deviceDesc = new QLabel;
    QFont font = deviceDesc->font();
    font.setBold(true);
    deviceDesc->setFont(font);
    deviceDesc->setText(cameraDesc);
    menuLayout->addWidget(deviceDesc);

    QPushButton *exitButton = new QPushButton(QIcon(QPixmap(THEME_DIR + "icons/exit.png")), "");
    exitButton->setIconSize(QSize(20, 20));
    exitButton->setToolTip(tr("Close manager"));
    exitButton->setShortcut(Qt::Key_Escape);
    connect(exitButton, SIGNAL(clicked()), this, SLOT(close()));

    menuLayout->addWidget(new TSeparator(Qt::Horizontal));
    menuLayout->addWidget(k->clickButton);

    menuLayout->addWidget(exitButton);
    menuLayout->addStretch(2);

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

    tError() << "TupReflexInterface::takePicture() - 0 Camera state: " << k->camera->state();
    tError() << "TupReflexInterface::takePicture() - 0 Is camera available? -> " << k->camera->isAvailable();

    QString prev = "pic";
    if (k->counter < 10)
        prev += "00";
    if (k->counter >= 10 && k->counter < 100)
        prev += "0";
    QString imagePath = k->path + "/" + prev + QString::number(k->counter) + ".jpg";

    k->camera->load();
    k->camera->start();

    tError() << "TupReflexInterface::takePicture() - 1 Camera state: " << k->camera->state();
    tError() << "TupReflexInterface::takePicture() - 1 Is camera available? -> " << k->camera->isAvailable();

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

    tError() << "TupReflexInterface::takePicture() - 2 Camera state: " << k->camera->state();
    tError() << "TupReflexInterface::takePicture() - 2 Is camera available? -> " << k->camera->isAvailable();
    tError() << "";
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

    QPixmap picture;
    picture.load(path);
    k->stack << picture;

    int size = k->stack.count();
    double opacity = 0.5;
    if (size > 1) {
        QPixmap base = k->stack.at(0);
        QPainter painter(&base);
        QPixmap frame;
        for (int i=0; i < size; i++) {
            painter.setOpacity(opacity);
            // opacity -= 0.01;
            frame = k->stack.at(i);
            painter.drawPixmap(0, 0, frame);
        }
        k->screen->setPixmap(base);
    } else {
        k->screen->setPixmap(picture);
    }
}

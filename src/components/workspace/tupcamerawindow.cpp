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

#include "tupcamerawindow.h"

#include <QCameraExposure>
#include <QCameraFocus>
#include <QCameraInfo>
#include <QMessageBox>

TupCameraWindow::TupCameraWindow(QCamera *input, const QSize &camSize, const QSize &displaySize, QCameraImageCapture *capture,
                                 const QString &path, QWidget *parent) : QWidget(parent)
{
    setFixedSize(displaySize + QSize(1, 1));

    picturesPath = path;
    camera = input;
    imageCapture = capture;
    camera->setCaptureMode(QCamera::CaptureStillImage);

    /*
    QCameraExposure *exposure = camera->exposure();
    exposure->setExposureMode(QCameraExposure::ExposureManual);
    QCameraFocus *focus = camera->focus();
    focus->setFocusMode(QCameraFocus::ManualFocus);
    focus->setFocusPointMode(QCameraFocus::FocusPointCenter);
    */

    connect(camera, SIGNAL(error(QCamera::Error)), this, SLOT(error(QCamera::Error)));
    connect(imageCapture, SIGNAL(imageSaved(int, const QString)), this, SLOT(imageSavedFromCamera(int, const QString)));

    QMediaService *service = camera->service();

    // QVideoEncoderControl *encoderControl = service->requestControl<QVideoEncoderControl*>();
    // QVideoEncoderSettings settings = encoderControl->videoSettings();
    // settings.setResolution(camSize);
    // encoderControl->setVideoSettings(settings);

    QVideoRendererControl *rendererControl = service->requestControl<QVideoRendererControl*>();

    bool isScaled = false; 
    if (camSize != displaySize)
        isScaled = true; // Camera Size is bigger than Display Size

    QCameraInfo cameraInfo(*input); 

    #ifdef TUP_DEBUG
        qDebug() << "TupCameraWindow() - Camera Orientation: " + QString::number(cameraInfo.orientation());
    #endif

    videoSurface = new TupVideoSurface(this, this, displaySize, isScaled, cameraInfo.orientation(), this);
    rendererControl->setSurface(videoSurface);
}

TupCameraWindow::~TupCameraWindow()
{
}

void TupCameraWindow::startCamera()
{
    stopCamera();
    camera->start();
}

void TupCameraWindow::stopCamera()
{
    if (camera->state() == QCamera::ActiveState)
        camera->stop();
}

void TupCameraWindow::reset()
{
    if (videoSurface)
        videoSurface->stop();

    if (camera->state() == QCamera::ActiveState)
        camera->stop();

    QDir dir(picturesPath);
    foreach (QString file, dir.entryList(QStringList() << "*.jpg")) {
             QString absolute = dir.absolutePath() + "/" + file;
             QFile::remove(absolute);
    }

    if (! dir.rmdir(dir.absolutePath())) {
        #ifdef TUP_DEBUG
            qDebug() << "TupCameraWindow::reset() - Fatal Error: Can't remove pictures directory -> " + dir.absolutePath();
        #endif 
    }
}

void TupCameraWindow::error(QCamera::Error error)
{
    switch (error) {
        case QCamera::NoError:
        {
            break;
        }
        case QCamera::CameraError:
        {
            QMessageBox::warning(this, "TupCameraWindow", tr("General Camera error"));
            break;
        }
        case QCamera::InvalidRequestError:
        {
            QMessageBox::warning(this, "TupCameraWindow", tr("Camera invalid request error"));
            break;
        }
        case QCamera::ServiceMissingError:
        {
            QMessageBox::warning(this, "TupCameraWindow", tr("Camera service missing error"));
            break;
        }
        case QCamera::NotSupportedFeatureError :
        {
            QMessageBox::warning(this, "TupCameraWindow", tr("Camera not supported error"));
            break;
        }
    };
}

void TupCameraWindow::updateVideo()
{
    repaint();
}

void TupCameraWindow::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    QPainter painter(this);

    if (videoSurface && videoSurface->isActive())
        videoSurface->paint(&painter);
}

void TupCameraWindow::takePicture(int i)
{
    QString prev = "pic";
    if (i < 10)
        prev += "00";
    if (i >= 10 && i < 100)
        prev += "0";
    QString imagePath = picturesPath + "/" + prev + QString::number(i) + ".jpg";

    // on half pressed shutter button
    camera->searchAndLock();
    // on shutter button pressed
    imageCapture->capture(imagePath);
    // on shutter button released
    camera->unlock();

    counter = i;
}

void TupCameraWindow::imageSavedFromCamera(int id, const QString path)
{
    Q_UNUSED(id);

    #ifdef TUP_DEBUG
        qWarning() << "TupCameraInterface::imageSavedFromCamera() - Picture path -> " + path;
    #endif

    if (path.isEmpty())
        return;

    emit pictureHasBeenSelected(counter, path);
    videoSurface->setLastImage(QImage(path));
}

void TupCameraWindow::drawGrid(bool flag)
{
    videoSurface->drawGrid(flag);
}

void TupCameraWindow::drawActionSafeArea(bool flag)
{
    videoSurface->drawActionSafeArea(flag);
}

void TupCameraWindow::showHistory(bool flag)
{
    videoSurface->showHistory(flag);
}

void TupCameraWindow::updateImagesOpacity(double opacity)
{
    videoSurface->updateImagesOpacity(opacity);
}

void TupCameraWindow::updateImagesDepth(int depth)
{
    videoSurface->updateImagesDepth(depth);
}

void TupCameraWindow::updateGridSpacing(int space)
{
    videoSurface->updateGridSpacing(space);
}

void TupCameraWindow::updateGridColor(const QColor color)
{
    videoSurface->updateGridColor(color);
}

void TupCameraWindow::flipCamera()
{
    videoSurface->flipSurface();
}

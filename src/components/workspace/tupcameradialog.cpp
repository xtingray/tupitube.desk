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

#include "tupcameradialog.h"

#include <QCameraImageCapture>
#include <QCameraInfo>
#include <QLabel>
#include <QBoxLayout>
#include <QPushButton>

TupCameraDialog::TupCameraDialog(QComboBox *devices, const QSize dimension, QList<QSize> resList,
                                 QWidget *parent) : QDialog(parent)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupCameraDialog()]";
    #endif

    setModal(true);
    setWindowTitle(tr("Camera Settings"));
    setWindowIcon(QIcon(QPixmap(THEME_DIR + "icons/photo.png")));

    useBasicInterface = false;
    projectSize = dimension;
    resolutions = resList;
    resizeProject = false;
    devicesCombo = devices;

    QBoxLayout *layout = new QBoxLayout(QBoxLayout::TopToBottom, this);

    cameraReference = devicesCombo->itemText(0);
    if (devicesCombo->count() > 1) {
        deviceIndex = 0;
        setCamera(cameraReference);

        QLabel *cameraLabel = new QLabel(tr("Available Camera Devices:"));
        layout->addWidget(cameraLabel);
        layout->addWidget(devicesCombo);
        connect(devicesCombo, SIGNAL(currentIndexChanged(const QString &)),
                this, SLOT(changeCameraDevice(const QString &)));
    } else {
        QLabel *cameraLabel = new QLabel(tr("Camera Detected:"));
        QLabel *referenceLabel = new QLabel;
        QFont font = referenceLabel->font();
        font.setBold(true);
        referenceLabel->setFont(font);
        referenceLabel->setText(cameraReference);
        layout->addWidget(cameraLabel);
        layout->addWidget(referenceLabel);
    }

    deviceIndex = 0;

    QLabel *resolutionLabel = new QLabel(tr("Available Camera Resolutions:"));
    resolutionCombo = new QComboBox();
    int suggested = 0;
    for (int i = 0; i<resolutions.size(); i++) {
         QSize size = resolutions.at(i);
         QString label = QString::number(size.width()) + "x" + QString::number(size.height());
         resolutionCombo->addItem(label);
         if (size.width() > projectSize.width())
             suggested = i;
    }
    resolutionCombo->setCurrentIndex(suggested);
    cameraSize = resolutions.at(suggested);

    connect(resolutionCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(setCameraResolution(int)));

    QCheckBox *resizeCheck = new QCheckBox(tr("Resize my project to fit camera resolution"));
    connect(resizeCheck, SIGNAL(toggled(bool)), this, SLOT(projectSizeHasChanged(bool)));

    lowCheck = new QCheckBox(tr("Use the basic camera interface (low resources)"));
    connect(lowCheck, SIGNAL(toggled(bool)), this, SLOT(enableBasicCamera(bool)));

    QVBoxLayout *propertiesLayout = new QVBoxLayout;
    propertiesLayout->addWidget(resizeCheck);
    propertiesLayout->addWidget(lowCheck);
    
    QHBoxLayout *buttonLayout = new QHBoxLayout;
    buttonLayout->addStretch(1);

    QPushButton *cancelButton = new QPushButton;
    cancelButton->setIcon(QIcon(THEME_DIR + "icons/close.png"));
    cancelButton->setToolTip(tr("Cancel"));
    cancelButton->setMinimumWidth(60);
    connect(cancelButton, SIGNAL(clicked()), this, SLOT(reject()));
    buttonLayout->addWidget(cancelButton);

    QPushButton *okButton = new QPushButton;
    okButton->setIcon(QIcon(THEME_DIR + "icons/apply.png"));
    okButton->setToolTip(tr("Ok"));
    okButton->setMinimumWidth(60);
    connect(okButton, SIGNAL(clicked()), this, SLOT(accept()));
    buttonLayout->addWidget(okButton);
    okButton->setDefault(true);

    layout->addWidget(resolutionLabel);
    layout->addWidget(resolutionCombo);
    layout->addLayout(propertiesLayout);

    layout->addLayout(buttonLayout);

    updateCameraType();
}

TupCameraDialog::~TupCameraDialog()
{
}

void TupCameraDialog::changeCameraDevice(const QString &reference)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupCameraDialog::changeCameraDevice()] - Camera selected -> " << reference;
    #endif

    // disconnect(resolutionCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(setCameraResolution(int)));

    cameraReference = reference;
    deviceIndex = devicesCombo->currentIndex();
    setCamera(cameraReference);

    updateCameraType();

    // QCameraImageCapture *imageCapture = new QCameraImageCapture(camera);
    // resolutions = imageCapture->supportedResolutions();

    /*
    resolutions.clear();
    resolutions << QSize(1280, 1024);
    resolutions << QSize(1280, 960);
    resolutions << QSize(1224, 768);
    resolutions << QSize(800, 600);
    resolutions << QSize(640, 480);
    resolutions << QSize(352, 288);
    resolutions << QSize(320, 240);
    resolutions << QSize(176, 144);
    resolutions << QSize(160, 120);
    resolutionCombo->clear();
    int suggested = 0;
    for (int i=0; i<resolutions.size(); i++) {
         QSize size = resolutions.at(i);
         QString label = QString::number(size.width()) + "x" + QString::number(size.height());
         resolutionCombo->addItem(label);
         if (size.width() > projectSize.width())
             suggested = i;
    }
    resolutionCombo->setCurrentIndex(suggested);
    cameraSize = resolutions.at(suggested);

    connect(resolutionCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(setCameraResolution(int)));
    */
}

void TupCameraDialog::setCamera(const QString &reference)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupCameraDialog::setCamera()] - reference -> " << reference;
    #endif

    foreach(const QCameraInfo &deviceName, QCameraInfo::availableCameras()) {
        QString description = deviceName.description();
        if (description.compare(reference) == 0) {
            camera = new QCamera(deviceName);
            break;
        }
    }
}

void TupCameraDialog::setCameraResolution(int index)
{
    cameraSize = resolutions.at(index);
}

void TupCameraDialog::projectSizeHasChanged(bool flag)
{
    resizeProject = flag;
}

QSize TupCameraDialog::cameraResolution() const
{
    return cameraSize;
}

bool TupCameraDialog::changeProjectSize()
{
    return resizeProject;
}

int TupCameraDialog::cameraIndex()
{
    return deviceIndex;
}

void TupCameraDialog::enableBasicCamera(bool flag)
{
    useBasicInterface = flag;
}

bool TupCameraDialog::useBasicCamera()
{
    return useBasicInterface;
}

void TupCameraDialog::updateCameraType()
{
    if (cameraReference.contains("Nikon") || cameraReference.contains("Canon")) {
        lowCheck->setVisible(false);
        webcamFlag = false;
    } else {
        lowCheck->setVisible(true);
        webcamFlag = true;
    }
}

bool TupCameraDialog::isWebcam()
{
    return webcamFlag;
}

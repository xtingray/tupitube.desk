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

#include "tupcamerainterface.h"
#include "tupapplication.h"
#include "tapplicationproperties.h"
#include "talgorithm.h"
#include "tosd.h"

#include <QBoxLayout>
#include <QIcon>
#include <QDir>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QColorDialog>
#include <QCamera>
#include <QCameraImageCapture>

TupCameraInterface::TupCameraInterface(const QString &title, QList<QCameraInfo> cameraDevices, QComboBox *devicesCombo, int cameraIndex, 
                                       const QSize cameraSize, int i, QWidget *parent) : QFrame(parent)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupCameraInterface()]";
    #endif

    QString cameraModel = devicesCombo->itemText(0);

    setWindowTitle(cameraModel + " | " + tr("Resolution:") + " " + title);
    setWindowIcon(QIcon(QPixmap(THEME_DIR + "icons/camera.png")));

    counter = i;

    widgetStack = new QStackedWidget();

    QSize displaySize = cameraSize;
    QPair<int, int> dimension = TAlgorithm::screenDimension();
    int screenWidth = dimension.first;
    if (cameraSize.width() > screenWidth) { // If camera resolution is bigger than screen resolution
        int width = screenWidth / 2;
        int height = width * cameraSize.height() / cameraSize.width();
        displaySize = QSize(width, height);
    } else {
        int maxWidth = 640;
        if (screenWidth > 1000)
            maxWidth = 1000;

        if (cameraSize.width() > maxWidth) { // Limit display size to maxWidth
            int height = maxWidth * cameraSize.height() / cameraSize.width();
            displaySize = QSize(maxWidth, height);
        }
    }

    QString path = randomPath();

    if (cameraDevices.count() == 1) { // Only one camera was detected
        QCameraInfo device = cameraDevices.at(0);
        QCamera *camera = new QCamera(device);
        QCameraImageCapture *imageCapture = new QCameraImageCapture(camera);

        TupCameraWindow *cameraWindow = new TupCameraWindow(camera, cameraSize, displaySize, imageCapture, path);
        connect(cameraWindow, SIGNAL(pictureHasBeenSelected(int, const QString)), this, SIGNAL(pictureHasBeenSelected(int, const QString)));

        widgetStack->addWidget(cameraWindow);
    } else {
        for (int i=0; i < cameraDevices.size(); i++) {
             QCameraInfo device = cameraDevices.at(i);
             QCamera *camera = new QCamera(device); 
             QCameraImageCapture *imageCapture = new QCameraImageCapture(camera);

             // QSize camSize = setBestResolution(imageCapture->supportedResolutions(), cameraSize);

             QList<QSize> resolutions;
             resolutions << QSize(640, 480);
             QSize camSize = setBestResolution(resolutions, cameraSize);

             TupCameraWindow *cameraWindow = new TupCameraWindow(camera, camSize, displaySize, imageCapture, path);
             connect(cameraWindow, SIGNAL(pictureHasBeenSelected(int, const QString)), this, SIGNAL(pictureHasBeenSelected(int, const QString)));

             widgetStack->addWidget(cameraWindow);
        }
    } 

    widgetStack->setCurrentIndex(cameraIndex);
    currentCamera = (TupCameraWindow *) widgetStack->currentWidget();

    QWidget *menuWidget = new QWidget;
    QBoxLayout *menuLayout = new QBoxLayout(QBoxLayout::TopToBottom, menuWidget);

    QLabel *devicesLabel = new QLabel;
    QString deviceString = tr("Cameras");
    if (devicesCombo->count() == 1)
        deviceString = tr("Camera");
    devicesLabel->setText(deviceString);
    devicesLabel->setAlignment(Qt::AlignHCenter);

    QPushButton *clickButton = new QPushButton(QIcon(QPixmap(THEME_DIR + "icons/photo.png")), "");
    clickButton->setIconSize(QSize(20, 20));
    clickButton->setToolTip(tr("Take picture"));
    connect(clickButton, SIGNAL(clicked()), this, SLOT(takePicture()));

    QPushButton *flipButton = new QPushButton(QIcon(QPixmap(THEME_DIR + "icons/flip_camera.png")), "");
    flipButton->setIconSize(QSize(20, 20));
    flipButton->setToolTip(tr("Flip camera"));
    connect(flipButton, SIGNAL(clicked()), this, SLOT(flipCamera()));

    safeAreaButton = new QPushButton(QIcon(QPixmap(THEME_DIR + "icons/safe_area.png")), "");
    safeAreaButton->setIconSize(QSize(20, 20));
    safeAreaButton->setToolTip(tr("Show safe area"));
    safeAreaButton->setShortcut(QKeySequence(tr("+")));
    safeAreaButton->setCheckable(true);
    connect(safeAreaButton, SIGNAL(clicked()), this, SLOT(drawActionSafeArea()));

    gridButton = new QPushButton(QIcon(QPixmap(THEME_DIR + "icons/subgrid.png")), "");
    gridButton->setIconSize(QSize(20, 20));
    gridButton->setToolTip(tr("Show grid"));
    gridButton->setShortcut(QKeySequence(tr("#")));
    gridButton->setCheckable(true);
    connect(gridButton, SIGNAL(clicked()), this, SLOT(drawGrid()));

    // Grid properties

    gridWidget = new QWidget;
    QGridLayout *gridLayout = new QGridLayout(gridWidget);
    gridLayout->setHorizontalSpacing(2);

    // Spacing

    QLabel *gridLabel = new QLabel;
    gridLabel->setPixmap(QPixmap(THEME_DIR + "icons/grid_spacing.png"));
    gridLabel->setToolTip(tr("Grid spacing"));
    gridLabel->setMargin(2);

    TCONFIG->beginGroup("PaintArea");
    QString colorName = TCONFIG->value("GridColor", "#0000b4").toString();
    gridColor = QColor(colorName);
    gridColor.setAlpha(50);

    QSpinBox *gridSpacing = new QSpinBox;
    gridSpacing->setSingleStep(10);
    gridSpacing->setRange(10, 100);
    gridSpacing->setValue(TCONFIG->value("GridSeparation", "10").toInt());
    connect(gridSpacing, SIGNAL(valueChanged(int)), this, SLOT(updateGridSpacing(int)));

    // Thickness

    QLabel *gridThicknessLabel = new QLabel;
    gridThicknessLabel->setPixmap(QPixmap(THEME_DIR + "icons/grid_thickness.png"));
    gridThicknessLabel->setToolTip(tr("Grid line thickness"));
    gridThicknessLabel->setMargin(2);

    QSpinBox *gridThicknessBox = new QSpinBox;
    gridThicknessBox->setSingleStep(1);
    gridThicknessBox->setRange(1, 5);
    gridThicknessBox->setValue(TCONFIG->value("GridLineThickness", "1").toInt());
    connect(gridThicknessBox, SIGNAL(valueChanged(int)), this, SLOT(updateGridThickness(int)));

    // Color

    QLabel *colorLabel = new QLabel;
    colorLabel->setPixmap(QPixmap(THEME_DIR + "icons/color_palette.png"));
    colorLabel->setToolTip(tr("Grid color"));
    colorLabel->setMargin(2);

    colorCell = new TupColorWidget(gridColor);
    connect(colorCell, SIGNAL(clicked()), this, SLOT(updateColour()));

    gridLayout->addWidget(gridLabel, 0, 0, Qt::AlignHCenter);
    gridLayout->addWidget(gridSpacing, 0, 1, Qt::AlignHCenter);
    gridLayout->addWidget(gridThicknessLabel, 1, 0, Qt::AlignHCenter);
    gridLayout->addWidget(gridThicknessBox, 1, 1, Qt::AlignHCenter);
    gridLayout->addWidget(colorLabel, 2, 0, Qt::AlignHCenter);
    gridLayout->addWidget(colorCell, 2, 1, Qt::AlignHCenter);

    gridWidget->setVisible(false);

    // Onion Skin

    historyButton = new QPushButton(QIcon(QPixmap(THEME_DIR + "icons/bitmap_array.png")), "");
    historyButton->setIconSize(QSize(20, 20));
    historyButton->setToolTip(tr("Show previous images"));
    historyButton->setShortcut(QKeySequence(tr("P")));
    historyButton->setCheckable(true);
    connect(historyButton, SIGNAL(clicked()), this, SLOT(showHistory()));

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
    previousSpin->setValue(2);
    previousSpin->setRange(0, 5);
    connect(previousSpin, SIGNAL(valueChanged(int)), this, SLOT(updateImagesDepth(int)));

    historyLayout->addWidget(opacityLabel, 0, 0, Qt::AlignHCenter);
    historyLayout->addWidget(opacitySpin, 0, 1, Qt::AlignHCenter);
    historyLayout->addWidget(previousLabel, 1, 0, Qt::AlignHCenter);
    historyLayout->addWidget(previousSpin, 1, 1, Qt::AlignHCenter);

    historyWidget->setVisible(false);

    menuLayout->addWidget(devicesLabel);
    if (devicesCombo->count() == 1) {
        QLabel *deviceDesc = new QLabel;
        QFont font = deviceDesc->font();
        font.setBold(true);
        deviceDesc->setFont(font);
        deviceDesc->setText(devicesCombo->itemText(0));
        menuLayout->addWidget(deviceDesc);
    } else {
        menuLayout->addWidget(devicesCombo);
    } 

    QPushButton *exitButton = new QPushButton(QIcon(QPixmap(THEME_DIR + "icons/exit.png")), "");
    exitButton->setIconSize(QSize(20, 20));
    exitButton->setToolTip(tr("Close manager"));
    exitButton->setShortcut(Qt::Key_Escape);
    connect(exitButton, SIGNAL(clicked()), this, SLOT(close()));

    QLabel *takesLabel = new QLabel("Takes");
    takesLabel->setAlignment(Qt::AlignHCenter);

    QFont font = this->font();
    font.setPointSize(40);
    counterLabel = new QLabel("0");
    counterLabel->setAlignment(Qt::AlignHCenter);
    counterLabel->setFont(font);

    devicesCombo->setCurrentIndex(cameraIndex);
    menuLayout->addWidget(new TSeparator(Qt::Horizontal));
    menuLayout->addWidget(takesLabel);
    menuLayout->addWidget(counterLabel);
    menuLayout->addWidget(clickButton);
    menuLayout->addWidget(flipButton);
    menuLayout->addWidget(safeAreaButton);
    menuLayout->addWidget(gridButton);
    menuLayout->addWidget(gridWidget);
    menuLayout->addWidget(historyButton);
    menuLayout->addWidget(historyWidget);
    menuLayout->addWidget(exitButton);
    menuLayout->addStretch(2);

    connect(devicesCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(changeCameraDevice(int)));

    QBoxLayout *dialogLayout = new QBoxLayout(QBoxLayout::LeftToRight, this); 
    dialogLayout->addWidget(widgetStack);
    dialogLayout->addWidget(menuWidget);

    currentCamera->startCamera();
}

TupCameraInterface::~TupCameraInterface()
{
    #ifdef TUP_DEBUG
        qDebug() << "[~TupCameraInterface()]";
    #endif
}

void TupCameraInterface::closeEvent(QCloseEvent *event)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupCameraInterface::closeEvent()]";
    #endif

    Q_UNUSED(event)

    currentCamera->reset();
    emit closed();
}

QSize TupCameraInterface::setBestResolution(QList<QSize> resolutions, QSize cameraSize)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupCameraInterface::setBestResolution()] - cameraSize -> " << cameraSize;
    #endif

    QSize maxCameraSize = QSize(0, 0);
    for (int i=0; i < resolutions.size(); i++) {
         QSize resolution = resolutions.at(i);
         if (cameraSize.width() == resolution.width() && cameraSize.height() == resolution.height())
             return cameraSize;
         if (resolution.width() > maxCameraSize.width())
             maxCameraSize = resolution;
    }

    return maxCameraSize;
}

QString TupCameraInterface::randomPath()
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupCameraInterface::randomPath()]";
    #endif

    QString path = CACHE_DIR + TAlgorithm::randomString(8);
    QDir dir;
    if (!dir.mkdir(path)) {
        #ifdef TUP_DEBUG
            qDebug() << "TupCameraInterface::randomPath() - Fatal Error: Can't create pictures directory -> " + path;
        #endif

        path = "";
        TOsd::self()->display(TOsd::Error, tr("Can't create pictures directory"));
    }

    return path;
}

void TupCameraInterface::takePicture()
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupCameraInterface::takePicture()]";
    #endif
    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

    currentCamera->takePicture(counter);
    counterLabel->setText(QString::number(counter));
    counter++;
}

void TupCameraInterface::changeCameraDevice(int index)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupCameraInterface::changeCameraDevice()] - index -> " << index;
    #endif

    TupCameraWindow *item = (TupCameraWindow *) widgetStack->currentWidget();
    item->stopCamera(); 

    widgetStack->setCurrentIndex(index);
    currentCamera = (TupCameraWindow *) widgetStack->currentWidget();
    currentCamera->startCamera();

    drawGrid();
    drawActionSafeArea();
    showHistory();
}

void TupCameraInterface::drawGrid()
{
    bool flag = gridButton->isChecked();
    gridWidget->setVisible(flag);
    currentCamera->drawGrid(flag);
}

void TupCameraInterface::drawActionSafeArea()
{
    currentCamera->drawActionSafeArea(safeAreaButton->isChecked());
}

void TupCameraInterface::showHistory()
{
    bool flag = historyButton->isChecked();
    historyWidget->setVisible(flag);
    currentCamera->showHistory(flag);
}

void TupCameraInterface::updateImagesOpacity(double opacity)
{
    currentCamera->updateImagesOpacity(opacity);
}

void TupCameraInterface::updateImagesDepth(int depth)
{
    currentCamera->updateImagesDepth(depth);
}

void TupCameraInterface::updateGridThickness(int thickness)
{
    TCONFIG->beginGroup("PaintArea");
    TCONFIG->setValue("GridLineThickness", thickness);

    currentCamera->updateGridLineThickness(thickness);
}

void TupCameraInterface::updateGridSpacing(int space)
{
    TCONFIG->beginGroup("PaintArea");
    TCONFIG->setValue("GridSeparation", space);

    currentCamera->updateGridSpacing(space);
}

void TupCameraInterface::updateColour()
{
    QColor color = QColorDialog::getColor(gridColor, this);
    if (color.isValid()) {
        currentCamera->updateGridColor(color);
        colorCell->setBrush(QBrush(color));
        TCONFIG->beginGroup("PaintArea");
        TCONFIG->setValue("GridColor", color.name());
    }
}

void TupCameraInterface::flipCamera()
{
   currentCamera->flipCamera();
}

/***************************************************************************
 *   Project TUPITUBE DESK                                                *
 *   Project Contact: info@maefloresta.com                                 *
 *   Project Website: http://www.maefloresta.com                           *
 *   Project Leader: Gustav Gonzalez <info@maefloresta.com>                *
 *                                                                         *
 *   Developers:                                                           *
 *   2010:                                                                 *
 *    Gustavo Gonzalez / xtingray                                          *
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

#include "tuppaintareastatus.h"
#include "tcolorcell.h"
#include "tupbrushmanager.h"

#include <QHBoxLayout>
#include <QScreen>

TupPaintAreaStatus::TupPaintAreaStatus(StatusType type, QPen pen, QBrush brush, QWidget *parent) : QStatusBar(parent)
{
    setSizeGripEnabled(false);

    scaleFactor = 100;
    angle = 0;
    currentFrame = 1;
    colorContext = TColorCell::Contour;

    QScreen *screen = QGuiApplication::screens().at(0);
    int screenWidth = screen->geometry().width();
    QSize iconSize(STATUS_ICON_SIZE, STATUS_ICON_SIZE);
    int panelWidth = 700;
    // Big resolutions
    if (screenWidth > HD_WIDTH) {
        panelWidth = 1100;
        int iconWidth = screenWidth/100;
        iconSize = QSize(iconWidth, iconWidth);
    }

    QPushButton *clearAreaButton = new QPushButton(QIcon(QPixmap(THEME_DIR + "icons/clear_frame.png")), "");
    clearAreaButton->setIconSize(iconSize);
    clearAreaButton->setToolTip(tr("Clear Frame"));
    // clearAreaButton->setShortcut(QKeySequence(Qt::Key_3));
    connect(clearAreaButton, SIGNAL(clicked()), this, SIGNAL(clearFrameClicked()));
    addPermanentWidget(clearAreaButton);

    QPushButton *resetWSButton = new QPushButton(QIcon(QPixmap(THEME_DIR + "icons/reset_workspace.png")), "");
    resetWSButton->setIconSize(iconSize);
    resetWSButton->setToolTip(tr("Reset WorkSpace"));
    resetWSButton->setShortcut(QKeySequence(Qt::Key_3));
    connect(resetWSButton, SIGNAL(clicked()), this, SIGNAL(resetClicked()));
    addPermanentWidget(resetWSButton);

    QPushButton *actionSafeAreaButton = new QPushButton(QIcon(QPixmap(THEME_DIR + "icons/safe_area.png")), "");
    actionSafeAreaButton->setIconSize(iconSize);
    actionSafeAreaButton->setToolTip(tr("Action Safe Area"));
    // SQA: pending shortcut
    // actionSafeAreaButton->setShortcut(QKeySequence(tr(" ")));
    actionSafeAreaButton->setCheckable(true);
    connect(actionSafeAreaButton, SIGNAL(clicked()), this, SIGNAL(safeAreaClicked()));
    addPermanentWidget(actionSafeAreaButton);

    QPushButton *gridButton = new QPushButton(QIcon(QPixmap(THEME_DIR + "icons/subgrid.png")), "");
    gridButton->setIconSize(iconSize);
    gridButton->setToolTip(tr("Show Grid"));
    gridButton->setShortcut(QKeySequence(Qt::Key_G));
    gridButton->setCheckable(true);
    connect(gridButton, SIGNAL(clicked()), this, SIGNAL(gridClicked()));
    addPermanentWidget(gridButton);

    if (type == Vector) {
        fullScreenButton = new QPushButton(QIcon(QPixmap(THEME_DIR + "icons/full_screen.png")), "");
        fullScreenButton->setIconSize(iconSize);
        fullScreenButton->setToolTip(tr("Full Screen"));
        fullScreenButton->setShortcut(QKeySequence(tr("F11")));
        connect(fullScreenButton, SIGNAL(clicked()), this, SIGNAL(fullClicked()));
        addPermanentWidget(fullScreenButton);

        QWidget *frameContainer = new QWidget;
        frameContainer->setFixedWidth(70);
        QHBoxLayout *frameLayout = new QHBoxLayout(frameContainer);
        frameLayout->setSpacing(3);
        frameLayout->setMargin(1);
        QLabel *frameLabel = new QLabel("");
        frameLabel->setToolTip(tr("Current Frame"));
        QPixmap framePix(THEME_DIR + "icons/frame_number.png");
        frameLabel->setPixmap(framePix);

        frameField = new QLineEdit(frameContainer);
        frameField->setFixedWidth(40);
        frameField->setAlignment(Qt::AlignRight);
        frameField->setText(tr("1"));
        connect(frameField, SIGNAL(editingFinished()), this, SLOT(updateFramePointer()));

        frameLayout->addWidget(frameLabel);
        frameLayout->addWidget(frameField);
        addPermanentWidget(frameContainer);
    }

    QWidget *zoomContainer = new QWidget;
    QHBoxLayout *zoomLayout = new QHBoxLayout(zoomContainer);
    zoomLayout->setSpacing(3);
    zoomLayout->setMargin(1);

    QLabel *zoomTool = new QLabel("");
    zoomTool->setToolTip(tr("Zoom"));
    QPixmap pix(THEME_DIR + "icons/zoom_small.png");
    zoomTool->setPixmap(pix);
    zoomLayout->addWidget(zoomTool);

    zoomCombo = new QComboBox();
    zoomCombo->setDuplicatesEnabled(false);
    zoomCombo->setEditable(true);

    for (int i = 500; i >= 250; i-=50)
         zoomCombo->addItem(QString::number(i), i);
    for (int i = 200; i >= 25; i-=25)
         zoomCombo->addItem(QString::number(i), i);

    zoomCombo->setCurrentIndex(10);
    zoomCombo->setValidator(new QIntValidator(10, 200, this));
    zoomLayout->addWidget(zoomCombo);
    zoomLayout->addWidget(new QLabel(tr("%")));
    connect(zoomCombo, SIGNAL(activated(const QString &)), this, SLOT(applyZoom(const QString &)));
    addPermanentWidget(zoomContainer);

    QWidget *rotContainer = new QWidget;
    QHBoxLayout *rotLayout = new QHBoxLayout(rotContainer);
    rotLayout->setSpacing(3);
    rotLayout->setMargin(1);

    QLabel *rotateLabel = new QLabel("");
    rotateLabel->setToolTip(tr("Rotate Workspace"));
    QPixmap rotatePix(THEME_DIR + "icons/rotate_workspace.png");
    rotateLabel->setPixmap(rotatePix);

    rotLayout->addWidget(rotateLabel);

    rotationCombo = new QComboBox();
    rotationCombo->setDuplicatesEnabled(false);
    rotationCombo->setEditable(true);

    for (int i = 0; i < 360; i+=30)
         rotationCombo->addItem(QString::number(i), i);

    rotationCombo->setValidator(new QIntValidator(-360, 360, this));
    rotLayout->addWidget(rotationCombo);
    addPermanentWidget(rotContainer);
    connect(rotationCombo, SIGNAL(activated(const QString &)), this, SLOT(applyRotation(const QString &)));

    if (type == Vector) {
        contourStatus = new TupBrushStatus(tr("Contour Color"), TColorCell::Contour, QPixmap(THEME_DIR + "icons/contour_color.png"));
        contourStatus->setTooltip(tr("Contour Color"));
        addPermanentWidget(contourStatus);

        fillStatus = new TupBrushStatus(tr("Fill Color"), TColorCell::Inner, QPixmap(THEME_DIR + "icons/fill_color.png"));
        fillStatus->setTooltip(tr("Fill Color"));
        addPermanentWidget(fillStatus);

        contourStatus->setColor(pen);
        fillStatus->setColor(brush);

        toolStatus = new TupToolStatus;
        addPermanentWidget(toolStatus);
    }

    QWidget *empty2 = new QWidget();
    empty2->setFixedWidth(5);
    addPermanentWidget(empty2);

    setMinimumWidth(panelWidth);
}

TupPaintAreaStatus::~TupPaintAreaStatus()
{
}

void TupPaintAreaStatus::setPen(const QPen &pen)
{
    contourStatus->setColor(pen);
}

void TupPaintAreaStatus::setBrush(const QBrush &brush)
{
    fillStatus->setColor(brush);
}

void TupPaintAreaStatus::applyRotation(const QString &text)
{
    int angle = text.toInt();

    if (angle < 0)
        angle += 360;

    emit angleChanged(angle);
}

void TupPaintAreaStatus::applyZoom(const QString &text)
{
    bool ok;
    int input = text.toInt(&ok, 10);

    if (!ok) { // Conversion has failed
        QStringList list = text.split(".");
        if (list.size() > 1)
            input = list.at(0).toInt();
    }

    qreal factor = static_cast<qreal>(input) / static_cast<qreal>(scaleFactor);
    scaleFactor = input;

    emit zoomChanged(factor);
}

void TupPaintAreaStatus::setZoomPercent(const QString &percent)
{
    updateZoomField(percent);
    applyZoom(percent);
}

void TupPaintAreaStatus::setRotationAngle(const QString &degree)
{
    updateRotationField(degree);
    applyRotation(degree);
}

void TupPaintAreaStatus::updateZoomField(const QString &text)
{
    int index = zoomCombo->findText(text);
    if (index != -1)
        zoomCombo->setCurrentIndex(index);
    else
        zoomCombo->setEditText(text);
}

void TupPaintAreaStatus::updateRotationField(const QString &text)
{
    int index = rotationCombo->findText(text);
    if (index != -1)
        rotationCombo->setCurrentIndex(index);
    else
        rotationCombo->setEditText(text);
}

qreal TupPaintAreaStatus::currentZoomFactor()
{
    return scaleFactor;
}

void TupPaintAreaStatus::updateZoomFactor(double factor) 
{
    double newFactor = scaleFactor * factor;
    QString number = QString::number(newFactor);
    int index = number.indexOf(".");
    if (index != -1)
        number = number.left(index + 2);

    updateZoomField(number);

    scaleFactor = newFactor;
}

void TupPaintAreaStatus::updateTool(const QString &label, const QPixmap &pixmap)
{
    toolStatus->updateTooltip(label);
    toolStatus->updatePixmap(pixmap);
}

void TupPaintAreaStatus::updateFrameIndex(int index)
{
    index++;
    currentFrame = index;

    QString text = QString::number(index); 
    frameField->setText(text);
}

void TupPaintAreaStatus::updateFramePointer()
{
    QString text = frameField->text();

    if (text.length() == 0) {
        frameField->setText(QString::number(currentFrame));
        return;
    }

    bool ok = false;
    int index = text.toInt(&ok);   
    if (ok) {
        if (index < 1 || index > 999) {
            frameField->setText(QString::number(currentFrame));
            return;
        }

        if (currentFrame != index)
            emit newFramePointer(index);
    } else {
        frameField->setText(QString::number(currentFrame));
    }
}

void TupPaintAreaStatus::setFramePointer(int index)
{
    currentFrame = index;
    frameField->setText(QString::number(currentFrame));
}

void TupPaintAreaStatus::updateRotationAngle(int degrees)
{
    angle = degrees;
    QString text = QString::number(angle); 
    int index = rotationCombo->findText(text);

    rotationCombo->blockSignals(true);
    if (index != -1)
        rotationCombo->setCurrentIndex(index);
    else
        rotationCombo->setEditText(text);
    rotationCombo->blockSignals(false);
}

void TupPaintAreaStatus::enableFullScreenFeature(bool flag)
{
    fullScreenButton->setEnabled(flag);
}

void TupPaintAreaStatus::showPos(const QPointF &point)
{
    QPoint dot = point.toPoint();
    QString pos = "X: " +  QString::number(dot.x()) + " Y: " + QString::number(dot.y());
    showMessage(pos);
}

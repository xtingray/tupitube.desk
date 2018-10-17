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

struct TupPaintAreaStatus::Private
{
    TupDocumentView *documentView;
 
    QPushButton *fullScreenButton;
    QLineEdit *frameField; 
    QComboBox *zoom;
    QComboBox *rotation;
    QCheckBox *antialiasHint;

    TupBrushStatus *contourStatus;
    TupBrushStatus *fillStatus;

    TupToolStatus *toolStatus;
    TColorCell::FillType colorContext;
    qreal scaleFactor;
    int angle;
    int currentFrame;
};

TupPaintAreaStatus::TupPaintAreaStatus(QPen pen, QBrush brush, QWidget *parent) : QStatusBar(parent), k( new Private)
{
    setSizeGripEnabled(false);

    k->scaleFactor = 100;
    k->angle = 0;
    k->currentFrame = 1;
    k->colorContext = TColorCell::Contour;

    QPushButton *resetWSButton = new QPushButton(QIcon(QPixmap(THEME_DIR + "icons/reset_workspace.png")), "");
    resetWSButton->setIconSize(QSize(16, 16));
    resetWSButton->setToolTip(tr("Reset WorkSpace"));
    resetWSButton->setShortcut(QKeySequence(Qt::Key_3));
    connect(resetWSButton, SIGNAL(clicked()), this, SIGNAL(resetClicked()));
    addPermanentWidget(resetWSButton);

    QPushButton *actionSafeAreaButton = new QPushButton(QIcon(QPixmap(THEME_DIR + "icons/safe_area.png")), "");
    actionSafeAreaButton->setIconSize(QSize(16, 16));
    actionSafeAreaButton->setToolTip(tr("Action Safe Area"));
    // SQA: pending shortcut
    // actionSafeAreaButton->setShortcut(QKeySequence(tr(" ")));
    actionSafeAreaButton->setCheckable(true);
    connect(actionSafeAreaButton, SIGNAL(clicked()), this, SIGNAL(safeAreaClicked()));
    addPermanentWidget(actionSafeAreaButton);

    QPushButton *gridButton = new QPushButton(QIcon(QPixmap(THEME_DIR + "icons/subgrid.png")), "");
    gridButton->setIconSize(QSize(16, 16));
    gridButton->setToolTip(tr("Show grid"));
    gridButton->setShortcut(QKeySequence(Qt::Key_G));
    gridButton->setCheckable(true);
    connect(gridButton, SIGNAL(clicked()), this, SIGNAL(gridClicked()));
    addPermanentWidget(gridButton);

    k->fullScreenButton = new QPushButton(QIcon(QPixmap(THEME_DIR + "icons/full_screen.png")), "");
    k->fullScreenButton->setIconSize(QSize(16, 16));
    k->fullScreenButton->setToolTip(tr("Full screen"));
    k->fullScreenButton->setShortcut(QKeySequence(tr("F11")));
    connect(k->fullScreenButton, SIGNAL(clicked()), this, SIGNAL(fullClicked()));
    addPermanentWidget(k->fullScreenButton);

    QWidget *frameContainer = new QWidget;
    frameContainer->setFixedWidth(70);
    QHBoxLayout *frameLayout = new QHBoxLayout(frameContainer);
    frameLayout->setSpacing(3);
    frameLayout->setMargin(1);
    QLabel *frameLabel = new QLabel("");
    frameLabel->setToolTip(tr("Current Frame"));
    QPixmap framePix(THEME_DIR + "icons/frame_number.png");
    frameLabel->setPixmap(framePix);

    k->frameField = new QLineEdit(frameContainer);
    k->frameField->setFixedWidth(40);
    k->frameField->setAlignment(Qt::AlignRight);
    k->frameField->setText(tr("1"));
    connect(k->frameField, SIGNAL(editingFinished()), this, SLOT(updateFramePointer()));

    frameLayout->addWidget(frameLabel);
    frameLayout->addWidget(k->frameField);
    addPermanentWidget(frameContainer);

    QWidget *zoomContainer = new QWidget;
    QHBoxLayout *zoomLayout = new QHBoxLayout(zoomContainer);
    zoomLayout->setSpacing(3);
    zoomLayout->setMargin(1);

    QLabel *zoomTool = new QLabel("");
    zoomTool->setToolTip(tr("Zoom"));
    QPixmap pix(THEME_DIR + "icons/zoom_small.png");
    zoomTool->setPixmap(pix);
    zoomLayout->addWidget(zoomTool);

    k->zoom = new QComboBox();
    k->zoom->setDuplicatesEnabled(false);
    k->zoom->setEditable(true);

    for (int i = 500; i >= 250; i-=50)
         k->zoom->addItem(QString::number(i), i);
    for (int i = 200; i >= 25; i-=25)
         k->zoom->addItem(QString::number(i), i);

    k->zoom->setCurrentIndex(10);
    k->zoom->setValidator(new QIntValidator(10, 200, this));
    zoomLayout->addWidget(k->zoom);
    zoomLayout->addWidget(new QLabel(tr("%")));
    connect(k->zoom, SIGNAL(activated(const QString &)), this, SLOT(applyZoom(const QString &)));
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

    k->rotation = new QComboBox();
    k->rotation->setDuplicatesEnabled(false);
    k->rotation->setEditable(true);

    for (int i = 0; i < 360; i+=30)
         k->rotation->addItem(QString::number(i), i);

    k->rotation->setValidator(new QIntValidator(-360, 360, this));
    rotLayout->addWidget(k->rotation);
    addPermanentWidget(rotContainer);
    connect(k->rotation, SIGNAL(activated(const QString &)), this, SLOT(applyRotation(const QString &)));

    k->antialiasHint = new QCheckBox;
    k->antialiasHint->setIcon(QIcon(QPixmap(THEME_DIR + "icons/antialiasing.png")));
    k->antialiasHint->setToolTip(tr("Antialiasing"));
    k->antialiasHint->setCheckable(true);
    k->antialiasHint->setChecked(true);
    k->antialiasHint->setFixedWidth(36);
    k->antialiasHint->setFixedHeight(23);
    addPermanentWidget(k->antialiasHint);

    connect(k->antialiasHint, SIGNAL(clicked()), this, SLOT(selectAntialiasingHint()));

    k->contourStatus = new TupBrushStatus(tr("Contour Color"), TColorCell::Contour, QPixmap(THEME_DIR + "icons/contour_color.png"));
    k->contourStatus->setTooltip(tr("Contour Color"));
    addPermanentWidget(k->contourStatus);

    k->fillStatus = new TupBrushStatus(tr("Fill Color"), TColorCell::Inner, QPixmap(THEME_DIR + "icons/fill_color.png"));
    k->fillStatus->setTooltip(tr("Fill Color"));
    addPermanentWidget(k->fillStatus);

    k->contourStatus->setColor(pen);
    k->fillStatus->setColor(brush);

    k->toolStatus = new TupToolStatus;
    addPermanentWidget(k->toolStatus);
    QWidget *empty2 = new QWidget();
    empty2->setFixedWidth(5);
    addPermanentWidget(empty2);

    setMinimumWidth(700);
}

TupPaintAreaStatus::~TupPaintAreaStatus()
{
    delete k;
}

void TupPaintAreaStatus::selectAntialiasingHint()
{
    emit antialiasChanged(k->antialiasHint->isChecked());
}

void TupPaintAreaStatus::setPen(const QPen &pen)
{
    k->contourStatus->setColor(pen);
}

void TupPaintAreaStatus::setBrush(const QBrush &brush)
{
    k->fillStatus->setColor(brush);
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

    qreal factor = (qreal)input / (qreal)k->scaleFactor;
    k->scaleFactor = input;

    emit zoomChanged(factor);
}

void TupPaintAreaStatus::setZoomPercent(const QString &percent)
{
    updateZoomField(percent);
    applyZoom(percent);
}

void TupPaintAreaStatus::setRotationAngle(const QString &angle)
{
    updateRotationField(angle);
    applyRotation(angle);
}

void TupPaintAreaStatus::updateZoomField(const QString &text)
{
    int index = k->zoom->findText(text);
    if (index != -1)
        k->zoom->setCurrentIndex(index);
    else
        k->zoom->setEditText(text);
}

void TupPaintAreaStatus::updateRotationField(const QString &text)
{
    int index = k->rotation->findText(text);
    if (index != -1)
        k->rotation->setCurrentIndex(index);
    else
        k->rotation->setEditText(text);
}

qreal TupPaintAreaStatus::currentZoomFactor()
{
    return k->scaleFactor;
}

void TupPaintAreaStatus::updateZoomFactor(double factor) 
{
    double newFactor = k->scaleFactor * factor;
    QString number = QString::number(newFactor);
    int index = number.indexOf(".");
    if (index != -1)
        number = number.left(index + 2);

    updateZoomField(number);

    k->scaleFactor = newFactor;
}

void TupPaintAreaStatus::updateTool(const QString &label, const QPixmap &pixmap)
{
    k->toolStatus->updateTooltip(label);
    k->toolStatus->updatePixmap(pixmap);
}

void TupPaintAreaStatus::updateFrameIndex(int index)
{
    index++;
    k->currentFrame = index;

    tError() << "updateFrameIndex() -> " << index;

    QString text = QString::number(index); 
    k->frameField->setText(text);
}

void TupPaintAreaStatus::updateFramePointer()
{
    QString text = k->frameField->text();

    if (text.length() == 0) {
        k->frameField->setText(QString::number(k->currentFrame));
        return;
    }

    bool ok = false;
    int index = text.toInt(&ok);   
    if (ok) {
        if (index < 1 || index > 999) {
            k->frameField->setText(QString::number(k->currentFrame));
            return;
        }

        if (k->currentFrame != index)
            emit newFramePointer(index);
    } else {
        k->frameField->setText(QString::number(k->currentFrame));
    }
}

void TupPaintAreaStatus::setFramePointer(int index)
{
    k->currentFrame = index;
    k->frameField->setText(QString::number(k->currentFrame));
}

void TupPaintAreaStatus::updateRotationAngle(int angle)
{
    k->angle = angle;
    QString text = QString::number(angle); 
    int index = k->rotation->findText(text);

    k->rotation->blockSignals(true);
    if (index != -1)
        k->rotation->setCurrentIndex(index);
    else
        k->rotation->setEditText(text);
    k->rotation->blockSignals(false);
}

void TupPaintAreaStatus::enableFullScreenFeature(bool flag)
{
    k->fullScreenButton->setEnabled(flag);
}

void TupPaintAreaStatus::showPos(const QPointF &point)
{
    QPoint dot = point.toPoint();
    QString pos = "X: " +  QString::number(dot.x()) + " Y: " + QString::number(dot.y());
    showMessage(pos);
}

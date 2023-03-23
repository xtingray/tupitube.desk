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

#include "tupprojectsizedialog.h"
#include "tapptheme.h"

#include <QLabel>

TupProjectSizeDialog::TupProjectSizeDialog(const QSize &size, QWidget *parent) : QDialog(parent)
{
    setModal(true);
    setWindowTitle(tr("Project Canvas Size"));
    setWindowIcon(QIcon(QPixmap(THEME_DIR + "icons/size.png")));

    QString uiStyleSheet = TAppTheme::themeSettings();
    setStyleSheet(uiStyleSheet);

    projectSize = size;
    layout = new QVBoxLayout(this);
    setUI(projectSize);
}

TupProjectSizeDialog::~TupProjectSizeDialog()
{
}

void TupProjectSizeDialog::setUI(const QSize &size)
{
    okButton = new QPushButton(QIcon(QPixmap(THEME_DIR + "icons/apply.png")), "");
    okButton->setToolTip(tr("Apply"));
    okButton->setVisible(false);
    connect(okButton, SIGNAL(clicked()), this, SLOT(accept()));

    QLabel *instructions = new QLabel(tr("Please, set the project canvas size:"));
    layout->addWidget(instructions);

    presetsCombo = new QComboBox();
    presetsCombo->addItem(tr("Free Format"));
    presetsCombo->addItem(tr("520x380 - 24"));
    presetsCombo->addItem(tr("640x480 - 24"));
    presetsCombo->addItem(tr("480 (PAL DV/DVD) - 25"));
    presetsCombo->addItem(tr("576 (PAL DV/DVD) - 25"));
    presetsCombo->addItem(tr("720 (HD) - 24"));
    presetsCombo->addItem(tr("1080 (Mobile) - 24"));
    presetsCombo->addItem(tr("1080 (Full HD Vertical) - 24"));
    presetsCombo->addItem(tr("1080 (Full HD) - 24"));

    layout->addWidget(presetsCombo);

    QFrame *infoContainer = new QFrame();
    sizeSpin = new TXYSpinBox("", tr("Width:"), tr("Height:"), infoContainer);
    sizeSpin->setMinimum(50);
    sizeSpin->setMaximum(15000);

    setPresets(size);

    QHBoxLayout *sizeLayout = new QHBoxLayout;
    sizeLayout->addStretch();
    sizeLayout->addWidget(sizeSpin, Qt::AlignHCenter);
    sizeLayout->addStretch();

    layout->addLayout(sizeLayout, Qt::AlignHCenter);

    QPushButton *closeButton = new QPushButton(QIcon(QPixmap(THEME_DIR + "icons/close.png")), "");
    closeButton->setToolTip(tr("Cancel"));
    connect(closeButton, SIGNAL(clicked()), this, SLOT(close()));

    QWidget *buttonsWidget = new QWidget;
    QHBoxLayout *buttonLayout = new QHBoxLayout(buttonsWidget);
    buttonLayout->addWidget(closeButton);
    buttonLayout->addWidget(okButton);

    layout->addWidget(buttonsWidget, 1, Qt::AlignRight);
    layout->addStretch(1);

    connect(presetsCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(setPresets(int)));
    connect(sizeSpin, SIGNAL(valuesHaveChanged()), this, SLOT(updatePresetsCombo()));
}

QSize TupProjectSizeDialog::getSize()
{
    return QSize(sizeSpin->x(), sizeSpin->y());
}

void TupProjectSizeDialog::setPresets(const QSize &size)
{
    sizeSpin->blockSignals(true);
    sizeSpin->setX(size.width());
    sizeSpin->setY(size.height());
    sizeSpin->blockSignals(false);

    checkDimensions(size);
}

void TupProjectSizeDialog::checkDimensions(const QSize &size)
{
    presetsCombo->blockSignals(true);
    if (size == QSize(520, 380)) {
        presetsCombo->setCurrentIndex(1);
    } else if (size == QSize(640, 480)) {
        presetsCombo->setCurrentIndex(2);
    } else if (size == QSize(720, 480)) {
        presetsCombo->setCurrentIndex(3);
    } else if (size == QSize(720, 576)) {
        presetsCombo->setCurrentIndex(4);
    } else if (size == QSize(1280, 720)) {
        presetsCombo->setCurrentIndex(5);
    } else if (size == QSize(1080,1080)) {
        presetsCombo->setCurrentIndex(6);
    } else if (size == QSize(1080, 1920)) {
        presetsCombo->setCurrentIndex(7);
    } else if (size == QSize(1920, 1080)) {
        presetsCombo->setCurrentIndex(8);
    } else {
        presetsCombo->setCurrentIndex(0);
    }
    presetsCombo->blockSignals(false);

    enableOkButton();
}

void TupProjectSizeDialog::setPresets(int index)
{
    sizeSpin->blockSignals(true);

    switch(index) {
           case FREE:
           break;
           case FORMAT_520:
           {
               sizeSpin->setX(520);
               sizeSpin->setY(380);
           }
           break;
           case FORMAT_640:
           {
               sizeSpin->setX(640);
               sizeSpin->setY(480);
           }
           break;
           case FORMAT_480:
           {
               sizeSpin->setX(720);
               sizeSpin->setY(480);
           }
           break;
           case FORMAT_576:
           {
               sizeSpin->setX(720);
               sizeSpin->setY(576);
           }
           break;
           case FORMAT_720:
           {
               sizeSpin->setX(1280);
               sizeSpin->setY(720);
           }
           break;
           case FORMAT_MOBILE:
           {
               sizeSpin->setX(1080);
               sizeSpin->setY(1080);
           }
           break;
           case FORMAT_1080_VERTICAL:
           {
               sizeSpin->setX(1080);
               sizeSpin->setY(1920);
           }
           break;
           case FORMAT_1080:
           {
               sizeSpin->setX(1920);
               sizeSpin->setY(1080);
           }
           break;
    }

    sizeSpin->blockSignals(false);

    enableOkButton();
}

void TupProjectSizeDialog::updatePresetsCombo()
{
    checkDimensions(QSize(sizeSpin->x(), sizeSpin->y()));
}

void TupProjectSizeDialog::enableOkButton()
{
    if (projectSize != QSize(sizeSpin->x(), sizeSpin->y())) {
        if (okButton)
            okButton->setVisible(true);
    } else {
        if (okButton->isVisible())
            okButton->setVisible(false);
    }
}

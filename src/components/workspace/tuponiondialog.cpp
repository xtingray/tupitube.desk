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

#include "tuponiondialog.h"
#include "timagebutton.h"
#include "tseparator.h"

#include <QDialogButtonBox>
#include <cmath>

TupOnionDialog::TupOnionDialog(const QColor &brushColor, double opacity,
                               QWidget *parent) : QDialog(parent)
{
    setModal(true);
    setWindowTitle(tr("Onion Skin Factor"));
    setWindowIcon(QIcon(QPixmap(THEME_DIR + "icons/onion.png")));

    color = brushColor;
    currentOpacity = opacity;

    QBoxLayout *layout = new QHBoxLayout(this);
    layout->setContentsMargins(3, 3, 3, 3);
    layout->setSpacing(2);

    innerLayout = new QVBoxLayout;

    setOpacityCanvas();
    setButtonsPanel();

    TImageButton *closeButton = new TImageButton(QPixmap(THEME_DIR + "icons/close_big.png"), 60, this, true);
    closeButton->setToolTip(tr("Close"));
    closeButton->setDefault(true);
    connect(closeButton, SIGNAL(clicked()), this, SLOT(close()));

    QDialogButtonBox *buttonBox = new QDialogButtonBox(Qt::Horizontal, this);
    buttonBox->addButton(closeButton, QDialogButtonBox::ActionRole);

    innerLayout->addWidget(new TSeparator());
    innerLayout->addWidget(buttonBox);

    layout->addLayout(innerLayout);
}

TupOnionDialog::~TupOnionDialog()
{
}

QSize TupOnionDialog::sizeHint() const
{
    return QSize(500, 250);
}

void TupOnionDialog::setOpacityCanvas()
{
     opacityPreview = new TupPenThicknessWidget(this);
     opacityPreview->setColor(color);
     opacityPreview->setBrush(Qt::SolidPattern);
     opacityPreview->render(currentOpacity);
    
    innerLayout->addWidget( opacityPreview);
}

void TupOnionDialog::setButtonsPanel()
{
    TImageButton *minus5 = new TImageButton(QPixmap(THEME_DIR + "icons/minus_sign_big.png"), 40, this, true);
    minus5->setToolTip(tr("-0.05"));
    connect(minus5, SIGNAL(clicked()), this, SLOT(fivePointsLess()));

    TImageButton *minus = new TImageButton(QPixmap(THEME_DIR + "icons/minus_sign_medium.png"), 40, this, true);
    minus->setToolTip(tr("-0.01"));
    connect(minus, SIGNAL(clicked()), this, SLOT(onePointLess()));

    QString number = QString::number(currentOpacity);
    if (number.length() == 3)
        number = number + "0";

    sizeLabel = new QLabel(number);
    sizeLabel->setAlignment(Qt::AlignHCenter);
    QFont font = this->font();
    font.setPointSize(24);
    font.setBold(true);
    sizeLabel->setFont(font);
    sizeLabel->setFixedWidth(100);

    TImageButton *plus = new TImageButton(QPixmap(THEME_DIR + "icons/plus_sign_medium.png"), 40, this, true);
    plus->setToolTip(tr("+0.01"));
    connect(plus, SIGNAL(clicked()), this, SLOT(onePointMore()));

    TImageButton *plus5 = new TImageButton(QPixmap(THEME_DIR + "icons/plus_sign_big.png"), 40, this, true);
    plus5->setToolTip(tr("+0.05"));
    connect(plus5, SIGNAL(clicked()), this, SLOT(fivePointsMore()));

    QBoxLayout *layout = new QHBoxLayout;
    layout->addWidget(minus5);
    layout->addWidget(minus);
    layout->addWidget(sizeLabel);
    layout->addWidget(plus);
    layout->addWidget(plus5);

    innerLayout->addLayout(layout);
}

void TupOnionDialog::fivePointsLess()
{
    modifySize(-0.05);
}

void TupOnionDialog::onePointLess()
{
    modifySize(-0.01);
}

void TupOnionDialog::onePointMore()
{
    modifySize(0.01);
}

void TupOnionDialog::fivePointsMore()
{
    modifySize(0.05);
}

void TupOnionDialog::modifySize(double value)
{
    currentOpacity = (100 * currentOpacity)/100;
    currentOpacity += value;

    if (currentOpacity > 1)
        currentOpacity = 1;

    if (currentOpacity < 0)
        currentOpacity = 0;

    if (currentOpacity == 0) {
        sizeLabel->setText("0.00");
    } else if (currentOpacity == 1) {
        sizeLabel->setText("1.00");
    } else {
        QString number = QString::number(currentOpacity);
        if (number.length() == 3)
            number = number + "0";
        sizeLabel->setText(number);
    }

     opacityPreview->render(currentOpacity);

    emit updateOpacity(currentOpacity);
}

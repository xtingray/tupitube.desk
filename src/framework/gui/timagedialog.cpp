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

#include "timagedialog.h"
#include "tapplicationproperties.h"
#include "tseparator.h"
#include "talgorithm.h"
#include "tapptheme.h"

#include <QVBoxLayout>
#include <QPushButton>

TImageDialog::TImageDialog(const QString &target, const QString &image, QWidget *parent) : QDialog(parent)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TImageDialog::TImageDialog()] - url ->" << target;
        qDebug() << "[TImageDialog::TImageDialog()] - image ->" << image;
    #endif

    setModal(true);
    url = target;
    imageName = image;

    setupGUI();
}

TImageDialog::~TImageDialog()
{
}

void TImageDialog::setupGUI()
{
    setWindowTitle(tr("Breaking News!"));
    setWindowIcon(QPixmap(THEME_DIR + "icons/bubble.png"));
    setStyleSheet(TAppTheme::themeSettings());
    QVBoxLayout *layout = new QVBoxLayout(this);

    QString imgPath = QDir::homePath() + "/." + QCoreApplication::applicationName() + "/images/" + imageName + ".png";

    imgLabel = new QLabel();
    imgLabel->setText("<a href=\"" + url + "\"><img src=\"file:" + imgPath + "\"></a>");
    imgLabel->setTextFormat(Qt::RichText);
    imgLabel->setTextInteractionFlags(Qt::TextBrowserInteraction);
    imgLabel->setOpenExternalLinks(true);

    QPushButton *closeButton = new QPushButton(this);
    closeButton->setIcon(QIcon(THEME_DIR + "icons/close.png"));
    closeButton->setToolTip(tr("Close"));
    closeButton->setMinimumWidth(60);

    layout->addWidget(closeButton);
    connect(closeButton, SIGNAL(clicked()), this, SLOT(close()));

    QHBoxLayout *buttonLayout = new QHBoxLayout;
    buttonLayout->addWidget(closeButton, 1, Qt::AlignRight);

    layout->addWidget(imgLabel);
    layout->addWidget(new TSeparator);
    layout->addLayout(buttonLayout);

    setAttribute(Qt::WA_DeleteOnClose, true);
}

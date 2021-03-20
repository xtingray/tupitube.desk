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

#include "mouthsdialog.h"
#include <QComboBox>
#include <QVBoxLayout>
#include <QLabel>
#include <QDialogButtonBox>

MouthsDialog::MouthsDialog(QWidget *parent) : QDialog(parent)
{
    setWindowTitle(tr("LipSync Mouth Examples"));
    setWindowIcon(QIcon(QPixmap(THEME_DIR + "icons/papagayo.png")));

    QFile file(THEME_DIR + "config/ui.qss");
    if (file.exists()) {
        file.open(QFile::ReadOnly);
        QString styleSheet = QLatin1String(file.readAll());
        if (styleSheet.length() > 0)
            setStyleSheet(styleSheet);
        file.close();
    } else {
        #ifdef TUP_DEBUG
            qWarning() << "[MouthsDialog()] - theme file doesn't exist -> " << (THEME_DIR + "config/ui.qss");
        #endif
    }

    mouthLabels << "AI" << "E" << "etc" << "FV" << "L" << "MBP" << "O" << "rest" << "U" << "WQ";
    for (int i = 1; i < 6; i++)
        folder << SHARE_DIR + "data/mouths/" + QString::number(i);

    QVBoxLayout *layout = new QVBoxLayout(this);
    QComboBox *mouthCombo = new QComboBox();
    mouthCombo->addItem(QIcon(THEME_DIR + "icons/mouth.png"), tr("Mouth Sample Pack No 1"));
    mouthCombo->addItem(QIcon(THEME_DIR + "icons/mouth.png"), tr("Mouth Sample Pack No 2"));
    mouthCombo->addItem(QIcon(THEME_DIR + "icons/mouth.png"), tr("Mouth Sample Pack No 3"));
    mouthCombo->addItem(QIcon(THEME_DIR + "icons/mouth.png"), tr("Mouth Sample Pack No 4"));
    mouthCombo->addItem(QIcon(THEME_DIR + "icons/mouth.png"), tr("Mouth Sample Pack No 5"));
    connect(mouthCombo, SIGNAL(activated(int)), this, SLOT(updateMouthCollection(int)));

    stackedWidget = new QStackedWidget;
    for (int i=0; i<5; i++)
        stackedWidget->addWidget(createMouthsCollection(i));

    QHBoxLayout *comboLayout = new QHBoxLayout;
    comboLayout->addStretch();
    comboLayout->addWidget(mouthCombo);
    comboLayout->addStretch();

    layout->addLayout(comboLayout, Qt::AlignCenter);
    layout->addWidget(stackedWidget, Qt::AlignCenter);

    QDialogButtonBox *buttons = new QDialogButtonBox(QDialogButtonBox::Ok, Qt::Horizontal);
    connect(buttons, SIGNAL(accepted()), this, SLOT(accept()));

    layout->addWidget(buttons, 0, Qt::AlignRight);
}

MouthsDialog::~MouthsDialog()
{
}

QWidget * MouthsDialog::createMouthsCollection(int index)
{
    QWidget *collection = new QWidget;
    QGridLayout *mouthsLayout = new QGridLayout(collection);
    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < 5; j++)
            mouthsLayout->addWidget(createMouthPanel(index, i, j), i, j, Qt::AlignCenter);
    }

    return collection;
}

QWidget * MouthsDialog::createMouthPanel(int index, int row, int column)
{
    int labelIndex = column;
    if (row == 1)
        labelIndex = 5 + column;
    QString text = mouthLabels.at(labelIndex);

    QWidget *panel = new QWidget;
    QVBoxLayout *panelLayout = new QVBoxLayout(panel);
    QLabel *label = new QLabel("<b>" + text + "</b>");
    label->setAlignment(Qt::AlignHCenter);
    panelLayout->addWidget(label);

    QString imgPath = folder[index] + "/" + text + ".png";
    QLabel *mouthImage = new QLabel;
    mouthImage->setAlignment(Qt::AlignCenter);
    mouthImage->setPixmap(QPixmap(imgPath));
    mouthImage->setStyleSheet("QWidget { border: 1px solid #cccccc; border-radius: 3px; }");
    panelLayout->addWidget(mouthImage, Qt::AlignCenter);

    return panel;
}

void MouthsDialog::updateMouthCollection(int index)
{
    stackedWidget->setCurrentIndex(index);
}

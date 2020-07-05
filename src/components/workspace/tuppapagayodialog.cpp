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

#include "tuppapagayodialog.h"
#include "tconfig.h"
#include "tosd.h"

#include <QBoxLayout>
#include <QDialogButtonBox>
#include <QCheckBox>
#include <QPushButton>
#include <QFileDialog>

TupPapagayoDialog::TupPapagayoDialog() : QDialog()
{
    setWindowTitle(tr("Import Papagayo project"));
    setWindowIcon(QIcon(QPixmap(THEME_DIR + "icons/papagayo.png")));

    QVBoxLayout *layout = new QVBoxLayout(this);

    QHBoxLayout *blockLayout = new QHBoxLayout;
    QVBoxLayout *buttonsLayout = new QVBoxLayout;
    QVBoxLayout *textLayout = new QVBoxLayout;

    QPushButton *fileButton = new QPushButton(QIcon(QPixmap(THEME_DIR + "icons/papagayo.png")), " " + tr("&Load PGO File"), this); 
    connect(fileButton, SIGNAL(clicked()), this, SLOT(openFileDialog()));

    QPushButton *imagesButton = new QPushButton(QIcon(QPixmap(THEME_DIR + "icons/bitmap_array.png")), " " + tr("Load &Images"), this);
    connect(imagesButton, SIGNAL(clicked()), this, SLOT(openImagesDialog()));

    QPushButton *soundButton = new QPushButton(QIcon(QPixmap(THEME_DIR + "icons/bitmap_array.png")), " " + tr("Load &Sound"), this);
    connect(soundButton, SIGNAL(clicked()), this, SLOT(openSoundDialog()));

    buttonsLayout->addWidget(fileButton);
    buttonsLayout->addWidget(imagesButton);
    buttonsLayout->addWidget(soundButton);

    filePath = new QLineEdit();
    filePath->setReadOnly(true);
    imagesPath = new QLineEdit();
    imagesPath->setReadOnly(true);
    soundPath = new QLineEdit();
    soundPath->setReadOnly(true);

    textLayout->addWidget(filePath);
    textLayout->addWidget(imagesPath);
    textLayout->addWidget(soundPath);

    blockLayout->addLayout(buttonsLayout);
    blockLayout->addLayout(textLayout);

    layout->addLayout(blockLayout);

    QDialogButtonBox *buttons = new QDialogButtonBox(QDialogButtonBox::Ok 
                                | QDialogButtonBox::Cancel, Qt::Horizontal);
    connect(buttons, SIGNAL(accepted()), this, SLOT(checkRecords()));
    connect(buttons, SIGNAL(rejected()), this, SLOT(reject()));

    layout->addWidget(buttons, 0, Qt::AlignCenter);
}

TupPapagayoDialog::~TupPapagayoDialog()
{
}

void TupPapagayoDialog::openFileDialog()
{
    TCONFIG->beginGroup("General");
    QString path = TCONFIG->value("DefaultPath", QDir::homePath()).toString();
    QString file = QFileDialog::getOpenFileName(this, tr("Load Papagayo project"), path, tr("Papagayo Project (*.pgo)"));

    if (!file.isEmpty()) {
        filePath->setText(file);
        setDefaultPath(file);
    }
}

void TupPapagayoDialog::openImagesDialog()
{
    TCONFIG->beginGroup("General");
    QString path = TCONFIG->value("DefaultPath", QDir::homePath()).toString();
    QString dir = QFileDialog::getExistingDirectory(this, tr("Choose the images directory..."), path, 
                                                    QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    if (!dir.isEmpty()) {
        imagesPath->setText(dir);
        saveDefaultPath(dir);
    }
}

void TupPapagayoDialog::openSoundDialog()
{
    TCONFIG->beginGroup("General");
    QString path = TCONFIG->value("DefaultPath", QDir::homePath()).toString();
    QString file = QFileDialog::getOpenFileName(this, tr("Load sound file"), path, tr("Sound file (*.ogg *.wav *.mp3)"));

    if (!file.isEmpty()) {
        soundPath->setText(file);
        setDefaultPath(file);
    }
}

void TupPapagayoDialog::checkRecords()
{
    if (filePath->text().length() == 0) {
        TOsd::self()->display(TOsd::Error, tr("PGO path is unset!"));
        return;
    }

    if (imagesPath->text().length() == 0) {
        TOsd::self()->display(TOsd::Error, tr("Images directory is unset!"));
        return;
    }

    if (soundPath->text().length() == 0) {
        TOsd::self()->display(TOsd::Error, tr("Sound path is unset!"));
        return;
    }

    accept();
}

QString TupPapagayoDialog::getPGOFile() const
{
    return filePath->text();
}

QString TupPapagayoDialog::getImagesFile() const
{
    return imagesPath->text();
}

QString TupPapagayoDialog::getSoundFile() const
{
    return soundPath->text();
}

void TupPapagayoDialog::setDefaultPath(const QString &path)
{
    int last = path.lastIndexOf("/");
    QString dir = path.left(last);
    saveDefaultPath(dir);
}

void TupPapagayoDialog::saveDefaultPath(const QString &dir)
{
    TCONFIG->beginGroup("General");
    TCONFIG->setValue("DefaultPath", dir);
    TCONFIG->sync();
}

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

#include "tbackupdialog.h"
#include "tconfig.h"
#include "tapplicationproperties.h"
#include "tseparator.h"
#include "talgorithm.h"
#include "tosd.h"

#include <QVBoxLayout>
#include <QLabel>
#include <QToolButton>
#include <QPushButton>
#include <QFileDialog>

TBackupDialog::TBackupDialog(const QString &path, const QString &project, QWidget *parent) : QDialog(parent)
{
    setModal(true);

    sourcePath = path;
    projectName = project;
    setupGUI();
}

TBackupDialog::~TBackupDialog()
{
}

void TBackupDialog::setupGUI()
{
    setWindowTitle(tr("Recovery Mode"));
    setWindowIcon(QPixmap(THEME_DIR + "icons/alert.png"));

    QVBoxLayout *layout = new QVBoxLayout(this);

    QString msg = tr("There was an issue while saving your project.<br/>"
                     "Please, select a folder to try to recover it.");
    QLabel *label = new QLabel(msg);

    destPath = QDir::homePath();
    pathLine = new QLineEdit(destPath);

    QToolButton *openButton = new QToolButton;
    openButton->setIcon(QIcon(THEME_DIR + "icons/open.png"));
    openButton->setToolTip(tr("Choose another path"));
    connect(openButton, SIGNAL(clicked()), this, SLOT(chooseDirectory()));

    QHBoxLayout *filePathLayout = new QHBoxLayout;
    filePathLayout->addWidget(new QLabel(tr("Folder: ")));
    filePathLayout->addWidget(pathLine);
    filePathLayout->addWidget(openButton);

    QPushButton *backupButton = new QPushButton(tr("Make Backup"));
    layout->addWidget(backupButton);
    connect(backupButton, SIGNAL(clicked()), this, SLOT(makeBackup()));

    QPushButton *closeButton = new QPushButton(tr("Cancel"));
    layout->addWidget(closeButton);
    connect(closeButton, SIGNAL(clicked()), this, SLOT(close()));

    QHBoxLayout *buttonLayout = new QHBoxLayout;
    buttonLayout->addWidget(backupButton, 1, Qt::AlignHCenter);
    buttonLayout->addWidget(closeButton, 1, Qt::AlignHCenter);

    layout->addWidget(label);
    layout->addLayout(filePathLayout);
    layout->addWidget(new TSeparator);
    layout->addLayout(buttonLayout);

    setAttribute(Qt::WA_DeleteOnClose, true);
}

void TBackupDialog::chooseDirectory()
{
    destPath = QFileDialog::getExistingDirectory(this, tr("Choose a directory..."), QDir::homePath(),
                                                 QFileDialog::ShowDirsOnly
                                                 | QFileDialog::DontResolveSymlinks);
    if (!destPath.isEmpty())
        pathLine->setText(destPath);
}

void TBackupDialog::makeBackup()
{
    destPath = pathLine->text();
    QDir dir(destPath);

    if (!dir.exists(destPath)) {
        if (!dir.mkpath(destPath)) {
            #ifdef TUP_DEBUG
                qDebug() << "TBackupDialog::makeProjectBackup() - Fatal Error:  -> " + destPath;
            #endif
            TOsd::self()->display(tr("Error"), tr("Folder doesn't exist. Please, pick one!"), TOsd::Error);
            return;
        }
    }

    destPath += QString("/" + projectName + ".bck");
    TCONFIG->beginGroup("General");
    TCONFIG->setValue("RecoveryDir", destPath);
    TCONFIG->sync();

    if (makeProjectBackup(sourcePath, destPath)) {
        #ifdef TUP_DEBUG
            qDebug() << "TBackupDialog::makeProjectBackup() - Backup was made successfuly!";
        #endif
        QDialog::accept();
    } else {
        #ifdef TUP_DEBUG
            qDebug() << "TBackupDialog::makeProjectBackup() - Fatal Error: backup copy has failed!";
        #endif
        QDialog::rejected();
    }
}

bool TBackupDialog::makeProjectBackup(const QString &sourceFolder, const QString &destFolder)
{
    #ifdef TUP_DEBUG
        qDebug() << "TBackupDialog::makeProjectBackup() - source path: " + sourceFolder;
        qDebug() << "TBackupDialog::makeProjectBackup() - dest path: " + destFolder;
    #endif

    bool success = false;
    QDir sourceDir(sourceFolder);

    if (!sourceDir.exists()) {
        #ifdef TUP_DEBUG
            qDebug() << "TBackupDialog::makeProjectBackup() - Fatal Error: source folder doesn't exist -> " + sourceFolder;
        #endif
        return false;
    }

    QDir destDir(destFolder);
    if (!destDir.exists()) {
        if (!destDir.mkpath(destFolder)) {
            #ifdef TUP_DEBUG
                qDebug() << "TBackupDialog::makeProjectBackup() - Fatal Error:  -> " + destFolder;
            #endif
            return false;
        }
    }

    QStringList files = sourceDir.entryList(QDir::Files);
    for(int i = 0; i < files.count(); i++) {
        QString srcName = sourceFolder + QDir::separator() + files[i];
        QString destName = destFolder + QDir::separator() + files[i];
        #ifdef TUP_DEBUG
            qDebug() << "TBackupDialog::makeProjectBackup() - Copying item -> " << srcName << " at " << destName;
        #endif
        if (QFile::exists(destName))
            QFile::remove(destName);
        success = QFile::copy(srcName, destName);
        if (!success) {
            #ifdef TUP_DEBUG
                qDebug() << "TBackupDialog::makeProjectBackup() - Error: can't copy item -> " + srcName;
            #endif
            return false;
        }
    }

    files.clear();
    files = sourceDir.entryList(QDir::AllDirs | QDir::NoDotAndDotDot);
    for(int i = 0; i < files.count(); i++) {
        QString srcName = sourceFolder + QDir::separator() + files[i];
        QString destName = destFolder + QDir::separator() + files[i];
        success = makeProjectBackup(srcName, destName);
        if (!success) {
            #ifdef TUP_DEBUG
                qDebug() << "TBackupDialog::makeProjectBackup() - Error: can't copy item (recursive) -> " + srcName;
            #endif
            return false;
        }
    }

    #ifdef TUP_DEBUG
        qDebug() << "TBackupDialog::makeProjectBackup() - Project backup was made successfully!";
    #endif

    return true;
}

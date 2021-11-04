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

#include "toptionaldialog.h"
#include "tapplicationproperties.h"

TOptionalDialog::TOptionalDialog(const QString &text,const QString &title, bool showAgainBox,
                                 bool showDiscardButton, QWidget *parent) : QDialog(parent)
{
    setUIStyle();

    setWindowTitle(title);
    mainLayout = new QVBoxLayout;
    mainLayout->addStretch(10);
    QLabel *label = new QLabel(text, this);
    mainLayout->addWidget(label);
    mainLayout->addStretch(10);
    mainLayout->addWidget(new TSeparator);
    
    QHBoxLayout *buttonLayout = new QHBoxLayout;
    buttonLayout->addStretch(1);
    
    if (showAgainBox) {
        checkBox = new QCheckBox(tr("Don't show again"));
        buttonLayout->addWidget(checkBox);
    }
    
    QPushButton *cancelButton = new QPushButton(this);
    cancelButton->setToolTip(tr("Cancel"));
    cancelButton->setMinimumWidth(60);
    cancelButton->setIcon(QIcon(THEME_DIR + "icons/close.png"));
    connect(cancelButton, SIGNAL(clicked()), this, SLOT(callCancelAction()));
    buttonLayout->addWidget(cancelButton);

    if (showDiscardButton) {
        QPushButton *discardButton = new QPushButton(this);
        discardButton->setToolTip(tr("Discard"));
        discardButton->setMinimumWidth(60);
        discardButton->setIcon(QIcon(THEME_DIR + "icons/delete.png"));
        connect(discardButton, SIGNAL(clicked()), this, SLOT(callDiscardAction()));
        buttonLayout->addWidget(discardButton);
    }

    QPushButton *okButton = new QPushButton(this);
    okButton->setToolTip(tr("Accept"));
    okButton->setMinimumWidth(60);
    okButton->setIcon(QIcon(THEME_DIR + "icons/apply.png"));
    connect(okButton, SIGNAL(clicked()), this, SLOT(callAcceptAction()));
    buttonLayout->addWidget(okButton);
    
    mainLayout->addLayout(buttonLayout);
    setLayout(mainLayout);
}

TOptionalDialog::~TOptionalDialog()
{
}

void TOptionalDialog::setUIStyle()
{
    QFile file(THEME_DIR + "config/ui.qss");
    if (file.exists()) {
        file.open(QFile::ReadOnly);
        QString uiStyleSheet = QLatin1String(file.readAll());
        if (uiStyleSheet.length() > 0)
            setStyleSheet(uiStyleSheet);
        file.close();
    } else {
        #ifdef TUP_DEBUG
            qWarning() << "[TupPapagayoApp()] - Error: Theme file doesn't exist -> "
                       << QString(THEME_DIR + "config/ui.qss");
        #endif
    }
}

bool TOptionalDialog::shownAgain()
{
    return !checkBox->isChecked();
}

void TOptionalDialog::callAcceptAction()
{
    result = Accepted;
    accept();
}

void TOptionalDialog::callDiscardAction()
{
    result = Discarded;
    reject();
}

void TOptionalDialog::callCancelAction()
{
    result = Cancelled;
    reject();
}

TOptionalDialog::Result TOptionalDialog::getResult()
{
    return result;
}

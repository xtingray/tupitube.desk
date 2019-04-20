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

#include "tupconnectdialog.h"

TupConnectDialog::TupConnectDialog(QWidget *parent): QDialog(parent)
{
    setWindowTitle(tr("Connection Dialog"));
    loginLine = new QLineEdit;
    passwdLine = new QLineEdit;
    passwdLine->setEchoMode(QLineEdit::Password);
    
    serverLine = new QLineEdit;
    portBox = new QSpinBox;
    portBox->setMinimum(1);
    portBox->setMaximum(65000);
    
    QGridLayout *layout = TFormFactory::makeGrid(QStringList() << tr("Login") << tr("Password") << tr("Server") << tr("Port"), QWidgetList() << loginLine << passwdLine << serverLine << portBox);
    
    storePasswdBox = new QCheckBox(tr("Store password"));
    layout->addWidget(storePasswdBox, 5, 1);
    
    QDialogButtonBox *box = new QDialogButtonBox;
    
    QPushButton *ok = box->addButton(QDialogButtonBox::Ok);
    connect(ok, SIGNAL(clicked()), this, SLOT(accept()));
    QPushButton *cancel = box->addButton(QDialogButtonBox::Cancel);
    connect(cancel, SIGNAL(clicked()), this, SLOT(reject()));
    
    QVBoxLayout *mainLayout = new QVBoxLayout;
    
    mainLayout->addLayout(layout);
    mainLayout->addWidget(box);
    
    setLayout(mainLayout);
    
    loadSettings();
}

TupConnectDialog::~TupConnectDialog()
{
    saveSettings();
}

void TupConnectDialog::setServer(const QString &server)
{
    serverLine->setText(server);
}

void TupConnectDialog::setPort(int port)
{
    portBox->setValue(port);
}

QString TupConnectDialog::login() const
{
    return loginLine->text();
}

QString TupConnectDialog::password() const
{
    return passwdLine->text();
}

QString TupConnectDialog::server() const
{
    return serverLine->text();
}

int TupConnectDialog::port() const
{
    return portBox->value();
}

void TupConnectDialog::loadSettings()
{
    TCONFIG->beginGroup("Network");
    serverLine->setText(TCONFIG->value("Server", "tupitu.be").toString());
    portBox->setValue(TCONFIG->value("Port", 5000).toInt());
    loginLine->setText(TCONFIG->value("Login", QString::fromLocal8Bit(::getenv("USER"))).toString());
    passwdLine->setText(TCONFIG->value("Password", "").toString());
    
    storePasswdBox->setChecked(TCONFIG->value("StorePassword").toInt());
}

void TupConnectDialog::saveSettings()
{
    TCONFIG->beginGroup("Network");
    
    TCONFIG->setValue("Server", serverLine->text());
    TCONFIG->setValue("Port", portBox->value());
    TCONFIG->setValue("Login", loginLine->text());
    
    if (storePasswdBox->isChecked())
        TCONFIG->setValue("Password", passwdLine->text());
    else 
        TCONFIG->setValue("Password", "");
    
    TCONFIG->setValue("StorePassword", storePasswdBox->isChecked() ? 1 : 0);
    TCONFIG->sync();
}

void TupConnectDialog::accept()
{
    if (passwdLine->text().isEmpty()) {
        TOsd::self()->display(tr("Error"), tr("Please, fill in your password"), TOsd::Error);
        return;
    }

    QDialog::accept();    
}

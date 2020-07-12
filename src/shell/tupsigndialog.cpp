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

#include "tupsigndialog.h"
#include "tconfig.h"
#include "tformfactory.h"
#include "talgorithm.h"
#include "tapplication.h"
#include "tosd.h"
#include "tupsecurity.h"

#ifdef TUP_DEBUG
  #include <QDebug>
#endif

TupSignDialog::TupSignDialog(QWidget *parent) : QDialog(parent)
{
    setModal(true);
    setWindowIcon(QPixmap(THEME_DIR + "icons/social_network.png"));
    setWindowTitle(tr("Sign In"));

    setForm();
}

TupSignDialog::~TupSignDialog()
{
}

void TupSignDialog::setForm()
{
    layout = new QBoxLayout(QBoxLayout::TopToBottom, this);
    layout->setAlignment(Qt::AlignHCenter | Qt::AlignTop);

    username = new QLineEdit;
    username->setText(TCONFIG->value("Username", "").toString());

    metadata = new QLineEdit;
    metadata->setEchoMode(QLineEdit::Password);

    QWidget *form = new QWidget;
    QVBoxLayout *formLayout = new QVBoxLayout(form);
    formLayout->addLayout(TFormFactory::makeGrid(QStringList() << tr("Username") << tr("Password"),
                          QWidgetList() << username << metadata));

    storeMetadata = new QCheckBox(tr("Store password"));
    storeMetadata->setChecked(TCONFIG->value("StorePassword").toInt());
    formLayout->addWidget(storeMetadata);

    QHBoxLayout *buttonLayout = new QHBoxLayout;
    buttonLayout->addStretch(1);

    QPushButton *cancelButton = new QPushButton(tr("Cancel"));
    connect(cancelButton, SIGNAL(clicked()), this, SLOT(close()));
    buttonLayout->addWidget(cancelButton);

    QPushButton *applyButton = new QPushButton(tr("Accept"));
    connect(applyButton, SIGNAL(clicked()), this, SLOT(apply()));
    buttonLayout->addWidget(applyButton);

    layout->addWidget(form);
    layout->addLayout(buttonLayout);
}

void TupSignDialog::apply()
{
    if (username->text().isEmpty()) {
        TOsd::self()->display(TOsd::Error, tr("Please, fill in your username"));
        return;
    }
    if (metadata->text().isEmpty()) {
        TOsd::self()->display(TOsd::Error, tr("Please, fill in your password"));
        return;
    }

    // Saving credentials
    TCONFIG->beginGroup("Network");
    TCONFIG->setValue("Username", username->text());
    if (storeMetadata->isChecked()) {
        TCONFIG->setValue("Password", TupSecurity::encryptPassword(SECRET_KEY));
        TCONFIG->setValue("StorePassword", "1");
    } else {
        TCONFIG->setValue("Password", "");
        TCONFIG->setValue("StorePassword", "0");
    }

    // Storing cache settings
    QString data = metadata->text();
    TAlgorithm::storeData(data);

    accept();
}

QString TupSignDialog::getUsername() const
{
    return username->text();
}

QString TupSignDialog::getMetadata() const
{
    return metadata->text();
}

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

#include "tupscenenamedialog.h"
#include "tseparator.h"
#include "tapptheme.h"

#include <QLabel>

TupSceneNameDialog::TupSceneNameDialog(DialogType type, const QString &name, QWidget *parent) : QDialog(parent)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupSceneNameDialog::TupSceneNameDialog()]";
    #endif

    setModal(true);
    actionType = type;
    sceneName = name;

    QString actionDesc = tr("Rename Scene");
    if (actionType == Add)
        actionDesc = tr("Add Scene");
    else
        oldName = name;

    setWindowTitle(actionDesc);
    setWindowIcon(QIcon(QPixmap(THEME_DIR + "icons/scenes.png")));
    setStyleSheet(TAppTheme::themeSettings());

    layout = new QVBoxLayout(this);
    setUI(sceneName);
}

TupSceneNameDialog::~TupSceneNameDialog()
{
}

void TupSceneNameDialog::setUI(const QString &sceneName)
{
    QLabel *sceneLabel = new QLabel(tr("Scene Name:"));
    sceneInput = new QLineEdit(sceneName);
    connect(sceneInput, SIGNAL(textChanged(const QString &)), this, SLOT(checkInput(const QString &)));

    QWidget *formWidget = new QWidget;
    QHBoxLayout *formLayout = new QHBoxLayout(formWidget);
    formLayout->addWidget(sceneLabel);
    formLayout->addWidget(sceneInput);

    layout->addWidget(formWidget);

    okButton = new QPushButton(QIcon(QPixmap(THEME_DIR + "icons/apply.png")), "");
    okButton->setToolTip(actionDesc);
    if (actionType == Rename)
        okButton->setVisible(false);

    connect(okButton, SIGNAL(clicked()), this, SLOT(accept()));

    QPushButton *closeButton = new QPushButton(QIcon(QPixmap(THEME_DIR + "icons/close.png")), "");
    closeButton->setToolTip(tr("Close"));
    connect(closeButton, SIGNAL(clicked()), this, SLOT(close()));

    QWidget *buttonsWidget = new QWidget;
    QHBoxLayout *buttonLayout = new QHBoxLayout(buttonsWidget);
    buttonLayout->addWidget(okButton);
    buttonLayout->addWidget(closeButton);

    layout->addWidget(buttonsWidget, 1, Qt::AlignRight);
    layout->addStretch(1);
}

QString TupSceneNameDialog::getSceneName() const
{
    return sceneName;
}

void TupSceneNameDialog::checkInput(const QString &input)
{
    bool flag = false;
    if (!input.isEmpty()) {
        if (actionType == Rename) {
            if (input.compare(oldName) != 0) {
                flag = true;
                sceneName = input;
            }
        } else {
            flag = true;
            sceneName = input;
        }
    }

    okButton->setVisible(flag);
}

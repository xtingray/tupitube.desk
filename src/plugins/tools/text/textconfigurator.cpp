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

#include "textconfigurator.h"
#include "tapplicationproperties.h"
#include "tseparator.h"

#include <QPushButton>

TextConfigurator::TextConfigurator(QWidget *parent) : QWidget(parent)
{
    /*
    #ifdef TUP_DEBUG
        qDebug() << "[TextConfigurator()]";
    #endif
    */

    QBoxLayout *mainLayout = new QBoxLayout(QBoxLayout::TopToBottom, this);

    QLabel *pencilTitle = new QLabel;
    pencilTitle->setAlignment(Qt::AlignHCenter);
    QPixmap pencilPic(THEME_DIR + "icons/text.png");
    pencilTitle->setPixmap(pencilPic.scaledToWidth(16, Qt::SmoothTransformation));
    pencilTitle->setToolTip(tr("Text Properties"));

    mainLayout->addWidget(pencilTitle);
    mainLayout->addWidget(new TSeparator(Qt::Horizontal));

    m_fontChooser = new TFontChooser;
    mainLayout->addWidget(m_fontChooser);

    QLabel *textLabel = new QLabel(tr("Text"));
    mainLayout->addWidget(textLabel);
    m_text = new QTextEdit(this);
    mainLayout->addWidget(m_text);

    QHBoxLayout *buttonLayout = new QHBoxLayout;
    QPushButton *addButton = new QPushButton(QIcon(QPixmap(THEME_DIR + "icons/plus_sign.png")), "");
    addButton->setToolTip(tr("Add Text"));
    addButton->setMaximumWidth(50);
    connect(addButton, SIGNAL(clicked()), this, SIGNAL(textAdded()));

    buttonLayout->addStretch(1);
    buttonLayout->addWidget(new QWidget);
    buttonLayout->addWidget(addButton, Qt::AlignHCenter);
    buttonLayout->addWidget(new QWidget);
    buttonLayout->addStretch(1);

    mainLayout->addLayout(buttonLayout);
    mainLayout->addStretch(1);

    connect(m_fontChooser, SIGNAL(fontChanged()), this, SLOT(changeFont()));
    changeFont();
}

TextConfigurator::~TextConfigurator()
{
}

QString TextConfigurator::text() const
{
    return m_text->toPlainText();
}

QFont TextConfigurator::textFont() const
{
    return font;
}

void TextConfigurator::changeFont()
{
    font = m_fontChooser->currentFont();

    // qDebug() << "[TextConfigurator::changeFont()] - Font Family -> " << font.family();
    // qDebug() << "[TextConfigurator::changeFont()] - Font Style -> " << font.style();
    // qDebug() << "[TextConfigurator::changeFont()] - Font Size -> " << font.pointSize();

    m_text->setFont(font);
    adjustSize();
}

void TextConfigurator::setDocument(QTextDocument *doc)
{
    m_text->setDocument(doc);
}

/*
void TextConfigurator::sendText()
{
    emit textAdded(m_text->toPlainText());
}
*/

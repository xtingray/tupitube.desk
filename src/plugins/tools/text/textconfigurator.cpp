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

#include <QBoxLayout>
#include <QPushButton>

TextConfigurator::TextConfigurator(QWidget *parent) : QWidget(parent)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TextConfigurator()]";
    #endif

    mode = Add;
    QBoxLayout *mainLayout = new QBoxLayout(QBoxLayout::TopToBottom, this);

    QLabel *pencilTitle = new QLabel;
    pencilTitle->setAlignment(Qt::AlignHCenter);
    QPixmap pencilPic(THEME_DIR + "icons/text.png");
    pencilTitle->setPixmap(pencilPic.scaledToWidth(16, Qt::SmoothTransformation));
    pencilTitle->setToolTip(tr("Text Properties"));

    mainLayout->addWidget(pencilTitle);
    mainLayout->addWidget(new TSeparator(Qt::Horizontal));

    fontChooser = new TFontChooser;
    mainLayout->addWidget(fontChooser);

    QLabel *textLabel = new QLabel(tr("Text"));
    mainLayout->addWidget(textLabel);
    textBox = new QTextEdit(this);
    textBox->setAlignment(Qt::AlignLeft);
    mainLayout->addWidget(textBox);

    QHBoxLayout *buttonLayout = new QHBoxLayout;
    addButton = new QPushButton(QIcon(QPixmap(THEME_DIR + "icons/plus_sign.png")), "");
    addButton->setToolTip(tr("Add Text"));
    addButton->setMaximumWidth(50);
    connect(addButton, SIGNAL(clicked()), this, SLOT(callAction()));

    clearButton = new QPushButton(QIcon(QPixmap(THEME_DIR + "icons/new.png")), "");
    clearButton->setToolTip(tr("Clear Text"));
    clearButton->setMaximumWidth(50);
    connect(clearButton, SIGNAL(clicked()), this, SLOT(clearText()));

    buttonLayout->addStretch(1);
    buttonLayout->addWidget(new QWidget);
    buttonLayout->addWidget(addButton, Qt::AlignHCenter);
    buttonLayout->addWidget(clearButton, Qt::AlignHCenter);
    buttonLayout->addWidget(new QWidget);
    buttonLayout->addStretch(1);

    mainLayout->addLayout(buttonLayout);
    mainLayout->addStretch(1);

    connect(fontChooser, SIGNAL(fontChanged()), this, SLOT(changeFont()));
    connect(fontChooser, SIGNAL(alignmentUpdated(Qt::Alignment)), this, SLOT(updateTextAlignment(Qt::Alignment)));
    changeFont();
}

TextConfigurator::~TextConfigurator()
{
}

QString TextConfigurator::text() const
{
    return textBox->toPlainText();
}

QFont TextConfigurator::textFont() const
{
    return font;
}

void TextConfigurator::changeFont()
{
    font = fontChooser->currentFont();
    textBox->setFont(font);
    adjustSize();
}

void TextConfigurator::loadTextSettings(const QFont &itemFont, const QString &text, const QColor &color)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TextConfigurator::loadTextSettings()] - text -> " << text;
        qDebug() << "[TextConfigurator::loadTextSettings()] - color -> " << color.name(QColor::HexArgb);
    #endif

    font = itemFont;
    textBox->setFont(itemFont);
    textBox->setTextColor(color);
    textBox->setPlainText(text);

    fontChooser->updateFontSettings(itemFont);
    updateMode(Update);
}

void TextConfigurator::callAction()
{
    if (mode == Add)
        emit textAdded();
    else
        emit textUpdated();
}

void TextConfigurator::clearText()
{
    textBox->clear();
}

void TextConfigurator::updateMode(Mode action)
{
    if (action != mode) {
        mode = action;

        QString tip = tr("Update Text");
        QString icon = "edit_sign.png";
        if (mode == Add) {
            tip = tr("Add Text");
            icon = "plus_sign.png";
        }

        addButton->setIcon(QIcon(QPixmap(THEME_DIR + "icons/" + icon)));
        addButton->setToolTip(tip);
    }
}

void TextConfigurator::updateTextAlignment(Qt::Alignment value)
{
    textBox->selectAll();
    textBox->setAlignment(value);
    textAlignmentValue = value;

    QTextCursor cursor = textBox->textCursor();
    cursor.movePosition(QTextCursor::End);
    textBox->setTextCursor(cursor);
}

Qt::Alignment TextConfigurator::textAlignment()
{
    return textAlignmentValue;
}

void TextConfigurator::resetText()
{
    textBox->clear();
    updateMode(Add);
}

void TextConfigurator::setTextColor(const QColor &color)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TextConfigurator::setTextColor()] - color -> " << color.name(QColor::HexArgb);
    #endif

    QString text = textBox->toPlainText();
    textBox->clear();
    textBox->setTextColor(color);
    textBox->setText(text);
}

QColor TextConfigurator::getTextColor() const
{
    return textBox->textColor();
}

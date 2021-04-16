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

#include "tfontchooser.h"
#include "tapplicationproperties.h"
#include "tseparator.h"

#include <QPushButton>

TFontChooser::TFontChooser(QWidget *parent) : QFrame(parent)
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    m_families = new QFontComboBox;
    connect(m_families, SIGNAL(currentFontChanged(const QFont &)), this, SLOT(loadFontInfo(const QFont &)));
    
    QHBoxLayout *fontFamilyLayout = new QHBoxLayout;
    fontFamilyLayout->addStretch();
    fontFamilyLayout->addLayout(TFormFactory::makeLine(tr("Family"), m_families));

    m_fontSize = new QComboBox;
    connect(m_fontSize, SIGNAL(activated (int)), this, SLOT(emitFontChanged(int)));

    fontFamilyLayout->addLayout(TFormFactory::makeLine(tr("Size"), m_fontSize));
    fontFamilyLayout->addStretch();
    mainLayout->addLayout(fontFamilyLayout);

    QHBoxLayout *buttonsBarLayout = new QHBoxLayout;
    buttonsBarLayout->addStretch();

    boldButton = new QPushButton(QIcon(QPixmap(THEME_DIR + "icons/bold.png")), "");
    boldButton->setCheckable(true);
    boldButton->setToolTip(tr("Bold"));
    buttonsBarLayout->addWidget(boldButton);
    connect(boldButton, SIGNAL(clicked()), this, SLOT(setBoldFlag()));

    italicButton = new QPushButton(QIcon(QPixmap(THEME_DIR + "icons/italic.png")), "");
    italicButton->setCheckable(true);
    italicButton->setToolTip(tr("Italic"));
    buttonsBarLayout->addWidget(italicButton);
    connect(italicButton, SIGNAL(clicked()), this, SLOT(setItalicFlag()));

    underlineButton = new QPushButton(QIcon(QPixmap(THEME_DIR + "icons/underline.png")), "");
    underlineButton->setCheckable(true);
    underlineButton->setToolTip(tr("Underline"));
    buttonsBarLayout->addWidget(underlineButton);
    connect(underlineButton, SIGNAL(clicked()), this, SLOT(setUnderlineFlag()));

    overlineButton = new QPushButton(QIcon(QPixmap(THEME_DIR + "icons/overline.png")), "");
    overlineButton->setCheckable(true);
    overlineButton->setToolTip(tr("Overline"));
    buttonsBarLayout->addWidget(overlineButton);
    connect(overlineButton, SIGNAL(clicked()), this, SLOT(setOverlineFlag()));

    buttonsBarLayout->addWidget(new TSeparator(Qt::Vertical));

    leftButton = new QPushButton(QIcon(QPixmap(THEME_DIR + "icons/align_left.png")), "");
    leftButton->setCheckable(true);
    leftButton->setChecked(true);
    leftButton->setToolTip(tr("Align Text To Left"));
    buttonsBarLayout->addWidget(leftButton);
    connect(leftButton, SIGNAL(clicked()), this, SLOT(alignTextToLeft()));

    centerButton = new QPushButton(QIcon(QPixmap(THEME_DIR + "icons/align_center.png")), "");
    centerButton->setCheckable(true);
    centerButton->setToolTip(tr("Align Text To Center"));
    buttonsBarLayout->addWidget(centerButton);
    connect(centerButton, SIGNAL(clicked()), this, SLOT(alignTextToCenter()));

    rightButton = new QPushButton(QIcon(QPixmap(THEME_DIR + "icons/align_right.png")), "");
    rightButton->setCheckable(true);
    rightButton->setToolTip(tr("Align Text To Right"));
    buttonsBarLayout->addWidget(rightButton);
    connect(rightButton, SIGNAL(clicked()), this, SLOT(alignTextToRight()));

    buttonsBarLayout->addStretch();
    mainLayout->addLayout(buttonsBarLayout);

    initFont();
}

TFontChooser::~TFontChooser()
{
}

void TFontChooser::setFontSizeRange(const QString &family)
{
    m_fontSize->blockSignals(true);

    QFontDatabase fdb;
    m_fontSize->clear();
    QList<int> points = fdb.pointSizes(family);
    if (!points.isEmpty()) {
        foreach (int point, points)
            m_fontSize->addItem(QString::number(point));
    } else {
        #ifdef TUP_DEBUG
            qDebug() << "[TFontChooser::setFontSizeRange()] - Fatal Error: No sizes for family -> " << family;
        #endif
        for (int i=1; i<30; i++)
            m_fontSize->addItem(QString::number(i));
    }

    m_fontSize->blockSignals(false);
}

void TFontChooser::loadFontInfo(const QFont &newFont)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TFontChooser::loadFontInfo()] - newFont -> " << newFont.styleName();
    #endif

    QString currentSize = m_fontSize->currentText();
    // QString currentStyle = tr("Normal");
    QString family = newFont.family();

    /*
    QFontDatabase fdb;
    m_fontSize->clear();    
    QList<int> points = fdb.pointSizes(family);
    if (!points.isEmpty()) {
        foreach (int point, points)
            m_fontSize->addItem(QString::number(point));
    } else {
        #ifdef TUP_DEBUG
            qDebug() << "[TFontChooser::loadFontInfo()] - Fatal Error: No sizes for family -> " << family;
        #endif
        for (int i=1; i<30; i++)
            m_fontSize->addItem(QString::number(i));
    }
    */

    setFontSizeRange(family);

    int sizeIndex = m_fontSize->findText(currentSize);
    if (sizeIndex >= 0)
        m_fontSize->setCurrentIndex(sizeIndex);

    m_families->blockSignals(true);
    m_currentFont = newFont;
    m_currentFont.setPointSize(m_fontSize->currentText().toInt());
    m_families->blockSignals(false);
    
    emit fontChanged();    
}

void TFontChooser::updateFontSettings(const QFont &itemFont)
{
    QString family = itemFont.family();

    m_families->blockSignals(true);
    m_families->setCurrentFont(itemFont);
    m_families->blockSignals(false);

    int pointSize = itemFont.pointSize();
    int sizeIndex = m_fontSize->findText(QString::number(pointSize));
    if (sizeIndex >= 0) {
        m_fontSize->blockSignals(true);
        m_fontSize->setCurrentIndex(sizeIndex);
        m_fontSize->blockSignals(false);
    }

    boldButton->blockSignals(true);
    boldButton->setChecked(itemFont.bold());
    boldButton->blockSignals(false);

    italicButton->blockSignals(true);
    italicButton->setChecked(itemFont.italic());
    italicButton->blockSignals(false);

    underlineButton->blockSignals(true);
    underlineButton->setChecked(itemFont.underline());
    underlineButton->blockSignals(false);

    overlineButton->blockSignals(true);
    overlineButton->setChecked(itemFont.overline());
    overlineButton->blockSignals(false);
}

void TFontChooser::emitFontChanged(int)
{
    m_currentFont = m_families->currentFont();
    m_currentFont.setPointSize(m_fontSize->currentText().toInt());

    emit fontChanged();
}

void TFontChooser::setCurrentFont(const QFont &font)
{
    m_families->setCurrentIndex(m_families->findText(font.family()));
    m_fontSize->setCurrentIndex(m_fontSize->findText(QString::number(font.pointSize())));
}

void TFontChooser::initFont()
{
    m_families->setCurrentIndex(0);
    m_fontSize->setCurrentIndex(0);
}

QFont TFontChooser::currentFont() const
{
     return m_currentFont;
}

int TFontChooser::currentSize() const
{
    return m_fontSize->currentText().toInt();
}

void TFontChooser::setBoldFlag()
{
    m_currentFont.setBold(boldButton->isChecked());
    emit fontChanged();
}

void TFontChooser::setItalicFlag()
{
    m_currentFont.setItalic(italicButton->isChecked());
    emit fontChanged();
}

void TFontChooser::setUnderlineFlag()
{
    m_currentFont.setUnderline(underlineButton->isChecked());
    emit fontChanged();
}

void TFontChooser::setOverlineFlag()
{
    m_currentFont.setOverline(overlineButton->isChecked());
    emit fontChanged();
}

void TFontChooser::alignTextToLeft()
{
    centerButton->blockSignals(true);
    centerButton->setChecked(false);
    centerButton->blockSignals(false);

    rightButton->blockSignals(true);
    rightButton->setChecked(false);
    rightButton->blockSignals(false);

    emit alignmentUpdated(Qt::AlignLeft);
}

void TFontChooser::alignTextToCenter()
{
    leftButton->blockSignals(true);
    leftButton->setChecked(false);
    leftButton->blockSignals(false);

    rightButton->blockSignals(true);
    rightButton->setChecked(false);
    rightButton->blockSignals(false);

    emit alignmentUpdated(Qt::AlignCenter);
}

void TFontChooser::alignTextToRight()
{
    leftButton->blockSignals(true);
    leftButton->setChecked(false);
    leftButton->blockSignals(false);

    centerButton->blockSignals(true);
    centerButton->setChecked(false);
    centerButton->blockSignals(false);

    emit alignmentUpdated(Qt::AlignRight);
}

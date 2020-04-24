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

#include "tsizebox.h"

TSizeBox::TSizeBox(const QString &title, const QSize &size, QWidget *parent) : QGroupBox(title, parent)
{
    dimension = size;
    QHBoxLayout *layout = new QHBoxLayout(this);

    QGridLayout *internal = new QGridLayout;
    m_textX = new QLabel(tr("Width:"));
    internal->addWidget(m_textX, 0, 0, Qt::AlignLeft);

    m_x = new QSpinBox(this);
    m_x->setAlignment(Qt::AlignRight);
    m_x->setSingleStep(1);
    m_x->setMinimum(100);
    m_x->setMaximum(5000);
    m_x->setValue(dimension.width());
    m_x->setMinimumWidth(60);
    internal->addWidget(m_x, 0, 1);

    m_textX->setBuddy(m_x);

    m_textY = new QLabel(tr("Height:"));
    internal->addWidget(m_textY, 1, 0, Qt::AlignLeft);

    m_y = new QSpinBox();
    m_y->setAlignment(Qt::AlignRight);
    m_y->setSingleStep(1);
    m_y->setMinimum(100);
    m_y->setMaximum(5000);
    m_y->setValue(dimension.height());
    m_y->setMinimumWidth(60);
    internal->addWidget(m_y, 1, 1);

    connect(m_x, SIGNAL(valueChanged(int)), this, SLOT(updateYValue()));
    connect(m_y, SIGNAL(valueChanged(int)), this, SLOT(updateXValue()));

    m_textY->setBuddy(m_y);
    layout->addLayout(internal);

    setLayout(layout);
}

TSizeBox::~TSizeBox()
{
}

void TSizeBox::updateXValue()
{
    int y = m_y->value();
    int x = (y * dimension.width()) / dimension.height();
    m_x->blockSignals(true);
    m_x->setValue(x);
    m_x->blockSignals(false);
}

void TSizeBox::updateYValue()
{
    int x = m_x->value();
    int y = (x * dimension.height()) / dimension.width();
    m_y->blockSignals(true);
    m_y->setValue(y);
    m_y->blockSignals(false);
}

int TSizeBox::x()
{
    return m_x->value();
}

int TSizeBox::y()
{
    return m_y->value();
}

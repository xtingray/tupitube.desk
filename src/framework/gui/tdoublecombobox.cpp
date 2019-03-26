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

#include "tdoublecombobox.h"

#define PERCENT(v) (v * 100.0) / maximum()
#define VALUE(p) (p * maximum()) / 100.0

TDoubleComboBox::TDoubleComboBox(double min, double max, QWidget *parent) : QComboBox(parent)
{
    validator = new QDoubleValidator(this);
    editor = new QLineEdit;
    editor->setValidator(validator);
    setLineEdit(editor);
    
    setValidator(validator);
    setMinimum(min);
    setMaximum(max);

    setDuplicatesEnabled(false);
    setInsertPolicy(QComboBox::InsertAlphabetically);
    
    connect(this, SIGNAL(activated(int)), this, SLOT(emitActivated(int)));
    connect(this, SIGNAL(highlighted(int)), this, SLOT(emitHighlighted(int)));
    connect(this, SIGNAL(currentIndexChanged(int)), this, SLOT(emitCurrentIndexChanged(int)));
    
    connect(editor, SIGNAL(editingFinished()), this, SIGNAL(editingFinished()));
    connect(editor, SIGNAL(returnPressed()), this, SIGNAL(editingFinished()));
    
    checkShowAsPercent = false;
    
    setDecimals(2);
}

TDoubleComboBox::~TDoubleComboBox()
{
    delete validator;
    delete editor;
}

void TDoubleComboBox::setShowAsPercent(bool p)
{
    if (checkShowAsPercent == p)
        return;
    
    checkShowAsPercent = p;
    
    for (int index = 0; index < count(); index++) {
         if (checkShowAsPercent) {
             setItemText(index, QString::number(PERCENT(itemText(index).toDouble())));
         } else {
             double value = VALUE(itemText(index).toDouble());
             setItemText(index, QString::number(value));
        }
    }
}

bool TDoubleComboBox::showAsPercent() const
{
    return checkShowAsPercent;
}

void TDoubleComboBox::setDecimals(int n)
{
    validator->setDecimals(n);
}

int TDoubleComboBox::decimals() const
{
    return validator->decimals();
}

void TDoubleComboBox::setMaximum(double max)
{
    validator->setTop(max);
}

void TDoubleComboBox::setMinimum(double min)
{
    validator->setBottom(min);
}

double TDoubleComboBox::maximum() const
{
    return validator->top();
}

double TDoubleComboBox::minimum() const
{
    return validator->bottom();
}

void TDoubleComboBox::addValue(double v)
{
    if (minimum() < v && v < maximum()) {
        if (checkShowAsPercent)
            addItem(QString::number(PERCENT(v)));
        else
            addItem(QString::number(v));
    }
}

void TDoubleComboBox::addPercent(double p)
{
    if (p >= 0 && p <= 100) {
        if (checkShowAsPercent)
            addItem(QString::number(p));
        else
            addItem(QString::number(VALUE(p)));
    }
}

void TDoubleComboBox::emitHighlighted(int index)
{
    emit highlighted(itemText(index).toDouble());
}

void TDoubleComboBox::emitActivated(int index)
{
    emit activated(itemText(index).toDouble());
}

void TDoubleComboBox::emitCurrentIndexChanged(int index)
{
    emit currentIndexChanged(itemText(index).toDouble());
}

double TDoubleComboBox::value()
{
    if (checkShowAsPercent)
        return (VALUE(currentText().toDouble()));
    
    return currentText().toDouble();
}

void TDoubleComboBox::setValue(int index, double v)
{
    if (checkShowAsPercent)
        setItemText(index, QString::number(PERCENT(v)));
    else
        setItemText(index, QString::number(v));
}

void TDoubleComboBox::setPercent(int index, double p)
{
    if (checkShowAsPercent)
        setItemText(index, QString::number(p));
    else
        setItemText(index, QString::number(VALUE(p)));
}

double TDoubleComboBox::percent()
{
    if (checkShowAsPercent)
        return currentText().toDouble();
    
    return PERCENT(currentText().toDouble());
}

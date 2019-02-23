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

#ifndef TUPNEWITEMDIALOG_H
#define TUPNEWITEMDIALOG_H

#include "tglobal.h"
#include "talgorithm.h"
#include "tapplicationproperties.h"

#include <QDialog>
#include <QLineEdit>
#include <QSpinBox>
#include <QComboBox>

/**
 * @author Gustav Gonzalez
**/

class TUPITUBE_EXPORT TupNewItemDialog : public QDialog
{
    Q_OBJECT

    public:
        enum DialogType { Raster = 0, Vector };

        TupNewItemDialog(QString &name, DialogType type, QSize size);
        ~TupNewItemDialog();

        QString getItemName() const;
        QSize itemSize() const;
        QString itemExtension() const;
        QColor getBackground() const;
        QString getSoftware() const;

    private slots:
        void checkValues();
        void updateExtension(int index);
        void updateBackground(int index);
        void updateEditor(const QString &editor);

    private:
        QLineEdit *itemName;
        QComboBox *extension;
        QSpinBox *width;
        QSpinBox *height;
        QComboBox *background;
        QComboBox *editor;
        QString name;
        QString software;
        QString fileExtension;
        QSize size;
        QColor colors[3];
        QColor bg;
};

#endif

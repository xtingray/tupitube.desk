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

#ifndef TEXTCONFIGURATOR_H
#define TEXTCONFIGURATOR_H

#include "tglobal.h"
#include "tfontchooser.h"

#include <QWidget>
#include <QLineEdit>
#include <QFont>
#include <QTextEdit>
#include <QCheckBox>
#include <QBoxLayout>
#include <QFontDatabase>

class TUPITUBE_PLUGIN TextConfigurator : public QWidget
{
    Q_OBJECT

    public:
        enum Mode {Add, Update};
        TextConfigurator(QWidget *parent = nullptr);
        ~TextConfigurator();

        QString text() const;
        QFont textFont() const;
        // void setDocument(QTextDocument *doc);
        void loadTextSettings(const QFont &itemFont, const QString &text);
        void updateMode(Mode action);
        Qt::Alignment textAlignment();
        void resetText();

    signals:
        void textAdded();
        void textUpdated();

    private slots:
        void changeFont();
        void callAction();
        void updateTextAlignment(Qt::Alignment flag);

    private:
        QTextEdit *textBox;
        TFontChooser *fontChooser;
        QFont font;
        QPushButton *addButton;
        Mode mode;
        Qt::Alignment textAlignmentValue;
};
#endif

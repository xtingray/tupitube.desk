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

#ifndef NODESETTINGS_H
#define NODESETTINGS_H

#include "tglobal.h"
#include "tapplicationproperties.h"

#include <QSpinBox>
#include <QSlider>
#include <QComboBox>
#include <QTextEdit>
#include <QPushButton>

class TUPITUBE_PLUGIN NodeSettings : public QWidget
{
    Q_OBJECT

    public:
        NodeSettings(QWidget *parent = nullptr);
        ~NodeSettings();

        void setNodesTotal(int value);
        void undo();
        void redo();

        NodePosition policyParam();
        void showClearPanel(bool show);

    signals:
        void nodesChanged(int total);
        void policyChanged();

    private slots:
        void updateNodesFromBox(int value);
        void updateNodesFromSlider(int value);
        void updatePolicyParam(int index);
        void openTipPanel();

    private:
        QWidget *clearWidget;
        QSpinBox *clearSpinBox;
        QSlider *clearSlider;
        QComboBox *policyCombo;

        QList<int> undoValues;
        QList<int> redoValues;

        NodePosition policy;

        QPushButton *tips;
        QWidget *help;
        QTextEdit *textArea;
};

#endif

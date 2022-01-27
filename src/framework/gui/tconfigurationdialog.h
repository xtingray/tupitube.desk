/***************************************************************************
 *   Project TUPITUBE DESK                                                 *
 *   Project Contact: info@maefloresta.com                                 *
 *   Project Leader: Gustav Gonzalez <info@maefloresta.com>                *
 *                                                                         *
 *   Developers:                                                           *
 *   2019:                                                                 *
 *    Alejandro Carrasco Rodríguez                                         *
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
 ***************************************************************************/

#ifndef TCONFIGURATIONDIALOG_H
#define TCONFIGURATIONDIALOG_H

#include "tglobal.h"
#include "twidgetlistview.h"
#include "tflatbutton.h"
#include "tseparator.h"

#include <QDialog>
#include <QMap>
#include <QButtonGroup>
#include <QVBoxLayout>
#include <QPushButton>
#include <QHBoxLayout>
#include <QStackedWidget>
#include <QLabel>
#include <QHeaderView>
#include <QDialogButtonBox>
#include <QListWidget>

// class QStackedWidget;
// class QListWidget;
// class QListWidgetItem;

class T_GUI_EXPORT TConfigurationDialog : public QDialog
{
    Q_OBJECT

    public:
        TConfigurationDialog(QWidget *parent = nullptr);
        ~TConfigurationDialog();
        
        void addPage(QWidget *page, const QString &label, const QIcon &icon);
        QWidget *currentPage() const;
        void setCurrentItem(int row);
        
    public slots:
        virtual void apply();
        
    private slots:
        void changePage(QListWidgetItem *, QListWidgetItem *);
        
    private:
        QListWidget *list;
        QStackedWidget *pageArea;
};

#endif

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

#ifndef TUPPREFERENCESDIALOG_H
#define TUPPREFERENCESDIALOG_H

#include "tglobal.h"
#include "tconfigurationdialog.h"
#include "tupgeneralpreferences.h"
#include "tuppaintareapreferences.h"
#include "tupthemepreferences.h"

class TUPITUBE_EXPORT TupPreferencesDialog : public TConfigurationDialog
{
    Q_OBJECT
    
    public:
        enum TabName { General = 0, Theme, PaintArea };
        TupPreferencesDialog(QWidget *parent = nullptr);
        ~TupPreferencesDialog();

    protected:
        QSize sizeHint() const;
        
    public slots:
        void apply();

    private slots:
        void testThemeColor(const QColor &);

    signals:
        void timerChanged();

    private:
        TupGeneralPreferences *general;
        TupThemePreferences *theme;
        TupPaintAreaPreferences *workspace;
};

#endif

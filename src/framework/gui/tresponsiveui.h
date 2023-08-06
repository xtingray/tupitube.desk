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

#ifndef TRESPONSIVEUI_H
#define TRESPONSIVEUI_H

#include "tglobal.h"
#include "tuivalues.h"

class T_GUI_EXPORT TResponsiveUI
{
    public:
        static QSize fitMainTopBarIconSize();
        static int fitTitleIconSize();
        static int fitToolViewIconSize();
        static int fitActionBarIconSize();
        static QSize fitStatusIconSize();
        static QPair<int, int> fitColorPaletteWidth();
        static QSize fitLateralToolBarIconSize(bool bigScreen, int screenWidth);
        static int fitRightPanelIconSize();
        static QPair<int, int> fitSelectionOptionIconsSize();
        static int fitRightPanelWidth();
        static int fitExposureVerticalHeader();
        static int fitExposureRowWidth();
        static int fitExposureRowHeight();
        static int fitStatusPanelWidth();
        static int fitStatusPanelFormIconSize();
        static int fitStatusPanelRotateWidth();
        static int fitSmallIconSize();       
        static int fitPlayerButtonSize();
        static int fitSmallPlayerButtonSize();
};

#endif

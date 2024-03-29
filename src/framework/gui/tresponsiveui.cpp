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

#include "tresponsiveui.h"
#include "talgorithm.h"

#include <QDebug>

QSize TResponsiveUI::fitMainTopBarIconSize()
{
    QPair<int, int> dimension = TAlgorithm::screenDimension();
    int screenWidth = dimension.first;
    int iconSize = MAIN_TOPBAR_ICON_SIZE;
    // Big resolutions
    if (screenWidth > HD_WIDTH)
        iconSize = FIT_MAIN_TOPBAR_ICON_SIZE(screenWidth);

    return QSize(iconSize, iconSize);
}

int TResponsiveUI::fitTitleIconSize()
{
    QPair<int, int> dimension = TAlgorithm::screenDimension();
    int screenWidth = dimension.first;
    int titleIconSize = PANEL_TITLE_ICON_SIZE;
    // Big resolutions
    if (screenWidth > HD_WIDTH)
        titleIconSize = FIT_TITLE_ICON_SIZE(screenWidth);

    return titleIconSize;
}

int TResponsiveUI::fitToolViewIconSize()
{
    QPair<int, int> dimension = TAlgorithm::screenDimension();
    int screenWidth = dimension.first;
    int iconSize = TOOLVIEW_ICON_SIZE;
    // Big resolutions
    if (screenWidth > HD_WIDTH)
        iconSize = FIT_TOOLVIEW_ICON_SIZE(screenWidth);

    return iconSize;
}

int TResponsiveUI::fitActionBarIconSize()
{
    QPair<int, int> dimension = TAlgorithm::screenDimension();
    int screenWidth = dimension.first;
    int iconSize = ACTIONBAR_ICON_SIZE;
    // Big resolutions
    if (screenWidth > HD_WIDTH)
        iconSize = FIT_ACTIONBAR_ICON_SIZE(screenWidth);

    return iconSize;
}

QSize TResponsiveUI::fitStatusIconSize()
{
    QPair<int, int> dimension = TAlgorithm::screenDimension();
    int screenWidth = dimension.first;
    QSize iconSize(STATUS_ICON_SIZE, STATUS_ICON_SIZE);
    // Big resolutions
    if (screenWidth > HD_WIDTH) {
        int iconWidth = FIT_STATUS_ICON_SIZE(screenWidth);
        iconSize = QSize(iconWidth, iconWidth);
    }

    return iconSize;
}

QPair<int, int> TResponsiveUI::fitColorPaletteWidth()
{
    QPair<int, int> widthValues;
    QPair<int, int> dimension = TAlgorithm::screenDimension();
    int screenWidth = dimension.first;
    // Big resolutions
    if (screenWidth > HD_WIDTH) {
        int width = FIT_MIN_COLORPALETTE_WIDTH(screenWidth);
        widthValues.first = width;
        widthValues.second = width*2;
    } else {
        widthValues.first = MIN_COLORPALETTE_WIDTH;
        widthValues.second = MAX_COLORPALETTE_WIDTH;
    }

    return widthValues;
}

QSize TResponsiveUI::fitLateralToolBarIconSize(bool bigScreen, int screenWidth)
{
    int iconSize = PLUGIN_ICON_SIZE;
    if (bigScreen)
        iconSize = FIT_PLUGIN_ICON_SIZE(screenWidth);

    return QSize(iconSize, iconSize);
}

int TResponsiveUI::fitRightPanelIconSize()
{
    QPair<int, int> dimension = TAlgorithm::screenDimension();
    int screenWidth = dimension.first;
    int iconSize = RIGHT_PANEL_ICON_SIZE;
    // Big resolutions
    if (screenWidth > HD_WIDTH)
        iconSize = FIT_RIGHT_PANEL_ICON_SIZE(screenWidth);

    return iconSize;
}

int TResponsiveUI::fitRightPanelWidth()
{
    QPair<int, int> dimension = TAlgorithm::screenDimension();
    int screenWidth = dimension.first;
    int iconSize = RIGHT_PANEL_MIN_WIDTH;
    // Big resolutions
    if (screenWidth > HD_WIDTH)
        iconSize = FIT_RIGHT_PANEL_MIN_WIDTH(screenWidth);

    return iconSize;
}

int TResponsiveUI::fitTweenerPanelWidth()
{
    QPair<int, int> dimension = TAlgorithm::screenDimension();
    int screenWidth = dimension.first;
    int iconSize = TWEENER_PANEL_MIN_WIDTH;
    // Big resolutions
    if (screenWidth > HD_WIDTH)
        iconSize = FIT_TWEENER_PANEL_MIN_WIDTH(screenWidth);

    return iconSize;
}

QPair<int, int> TResponsiveUI::fitSelectionOptionIconsSize()
{
    QPair<int, int> widthValues;
    QPair<int, int> dimension = TAlgorithm::screenDimension();
    int screenWidth = dimension.first;
    widthValues.first = 15;
    widthValues.second = 18;
    if (screenWidth > HD_WIDTH) {
        float value = (screenWidth*0.8)/100;
        widthValues.first = (int) value;
        value = (screenWidth*0.95)/100;
        widthValues.second = (int) value;
    }

    return widthValues;
}

int TResponsiveUI::fitNodesPanelWidth()
{
    QPair<int, int> dimension = TAlgorithm::screenDimension();
    int screenWidth = dimension.first;
    int minWidth = NODES_PANEL_MIN_WIDTH;
    if (screenWidth > HD_WIDTH)
        minWidth = FIT_NODES_PANEL_MIN_WIDTH(screenWidth);

    return minWidth;
}

int TResponsiveUI::fitLipsyncPanelWidth()
{
    QPair<int, int> dimension = TAlgorithm::screenDimension();
    int screenWidth = dimension.first;
    int minWidth = LIPSYNC_PANEL_MIN_WIDTH;
    if (screenWidth > HD_WIDTH)
        minWidth = FIT_LIPSYNC_PANEL_MIN_WIDTH(screenWidth);

    return minWidth;
}

int TResponsiveUI::fitTextPanelWidth()
{
    QPair<int, int> dimension = TAlgorithm::screenDimension();
    int screenWidth = dimension.first;
    int minWidth = TEXT_PANEL_MIN_WIDTH;
    if (screenWidth > HD_WIDTH)
        minWidth = FIT_TEXT_PANEL_MIN_WIDTH(screenWidth);

    return minWidth;
}

int TResponsiveUI::fitExposureVerticalHeader()
{
    QPair<int, int> dimension = TAlgorithm::screenDimension();
    int screenWidth = dimension.first;
    int width = EXPOSURE_VERTICAL_HEADER_WIDTH;
    if (screenWidth > HD_WIDTH)
        width = FIT_EXPOSURE_VERTICAL_HEADER_WIDTH(screenWidth);

    return width;
}

int TResponsiveUI::fitExposureRowWidth()
{
    QPair<int, int> dimension = TAlgorithm::screenDimension();
    int screenWidth = dimension.first;
    int width = EXPOSURE_ROW_WIDTH;
    if (screenWidth > HD_WIDTH)
        width = FIT_EXPOSURE_ROW_WIDTH(screenWidth);

    return width;
}

int TResponsiveUI::fitExposureRowHeight()
{
    QPair<int, int> dimension = TAlgorithm::screenDimension();
    int screenWidth = dimension.first;
    int height = EXPOSURE_ROW_HEIGHT;
    if (screenWidth > HD_WIDTH)
        height = FIT_EXPOSURE_ROW_HEIGHT(screenWidth);

    return height;
}

int TResponsiveUI::fitStatusPanelWidth()
{
    QPair<int, int> dimension = TAlgorithm::screenDimension();
    int screenWidth = dimension.first;
    int width = STATUS_PANEL_WIDTH;
    if (screenWidth > HD_WIDTH)
        width = FIT_STATUS_PANEL_WIDTH(screenWidth);

    return width;
}

int TResponsiveUI::fitStatusPanelFormIconSize()
{
    QPair<int, int> dimension = TAlgorithm::screenDimension();
    int screenWidth = dimension.first;
    int width = STATUS_PANEL_ICON_SIZE;
    if (screenWidth > HD_WIDTH)
        width = FIT_STATUS_PANEL_ICON_SIZE(screenWidth);

    return width;
}

int TResponsiveUI::fitStatusPanelRotateWidth()
{
    QPair<int, int> dimension = TAlgorithm::screenDimension();
    int screenWidth = dimension.first;
    int width = STATUS_PANEL_ROTATE_WIDTH;
    if (screenWidth > HD_WIDTH)
        width = FIT_STATUS_PANEL_ROTATE_WIDTH(screenWidth);

    return width;
}

int TResponsiveUI::fitSmallIconSize()
{
    QPair<int, int> dimension = TAlgorithm::screenDimension();
    int screenWidth = dimension.first;
    int width = SMALL_ICON_SIZE;
    if (screenWidth > HD_WIDTH)
        width = FIT_SMALL_ICON_SIZE(screenWidth);

    return width;
}

int TResponsiveUI::fitPlayerButtonSize()
{
    QPair<int, int> dimension = TAlgorithm::screenDimension();
    int screenWidth = dimension.first;
    int width = PLAYER_BUTTON_SIZE;
    if (screenWidth > HD_WIDTH)
        width = FIT_PLAYER_BUTTON_SIZE(screenWidth);

    return width;
}

int TResponsiveUI::fitSmallPlayerButtonSize()
{
    QPair<int, int> dimension = TAlgorithm::screenDimension();
    int screenWidth = dimension.first;
    int width = SMALL_PLAYER_BUTTON_SIZE;
    if (screenWidth > HD_WIDTH)
        width = FIT_SMALL_PLAYER_BUTTON_SIZE(screenWidth);

    return width;
}

QString TResponsiveUI::fitTitleFontSize()
{
    QPair<int, int> dimension = TAlgorithm::screenDimension();
    int screenWidth = dimension.first;
    int fontSize = TITLE_FONT_SIZE;
    if (screenWidth > HD_WIDTH)
        fontSize = FIT_TITLE_FONT_SIZE(screenWidth);

    return QString::number(fontSize);
}

QString TResponsiveUI::fitMsgFontSize()
{
    QPair<int, int> dimension = TAlgorithm::screenDimension();
    int screenWidth = dimension.first;
    int fontSize = MSG_FONT_SIZE;
    if (screenWidth > HD_WIDTH)
        fontSize = FIT_MSG_FONT_SIZE(screenWidth);

    return QString::number(fontSize);
}

QString TResponsiveUI::fitInfoIconSize()
{
    QPair<int, int> dimension = TAlgorithm::screenDimension();
    int screenWidth = dimension.first;
    int iconSize = INFO_ICON_SIZE;
    if (screenWidth > HD_WIDTH)
        iconSize = FIT_INFO_ICON_SIZE(screenWidth);

    return QString::number(iconSize);
}

QString TResponsiveUI::fitMsgIconSize()
{
    QPair<int, int> dimension = TAlgorithm::screenDimension();
    int screenWidth = dimension.first;
    int iconSize = MSG_FONT_SIZE;
    if (screenWidth > HD_WIDTH)
        iconSize = FIT_MSG_FONT_SIZE(screenWidth);

    return QString::number(iconSize);
}

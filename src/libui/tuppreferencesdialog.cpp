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

#include "tuppreferencesdialog.h"
#include "tapplicationproperties.h"
#include "tosd.h"

TupPreferencesDialog::TupPreferencesDialog(QWidget *parent) : TConfigurationDialog(parent)
{
    setWindowTitle(tr("TupiTube Preferences"));

    general = new TupGeneralPreferences;
    addPage(general, tr("General"), QPixmap(THEME_DIR + "icons/tupi_general_preferences.png"));

    theme = new TupThemePreferences;
    connect(theme, SIGNAL(colorPicked(const QColor&)), this, SLOT(testThemeColor(const QColor&)));
    addPage(theme, tr("Theme"), QPixmap(THEME_DIR + "icons/tupi_theme_preferences.png"));

    workspace = new TupPaintAreaPreferences;
    addPage(workspace, tr("Workspace"), QIcon(THEME_DIR + "icons/tupi_workspace_preferences.png"));

    setCurrentItem(General);
}

TupPreferencesDialog::~TupPreferencesDialog()
{
}

void TupPreferencesDialog::apply()
{
    if (general->saveValues()) {
        theme->saveValues();
        workspace->saveValues();
        if (general->showWarning())
            TOsd::self()->display(TOsd::Warning, tr("Please restart TupiTube"));
        else
            TOsd::self()->display(TOsd::Info, tr("Preferences saved successfully"));
        accept();
    }
}

QSize TupPreferencesDialog::sizeHint() const
{
    return QSize(600, 430);
}

void TupPreferencesDialog::testThemeColor(const QColor &color)
{
    QString r = QString::number(color.red());
    QString g = QString::number(color.green());
    QString b = QString::number(color.blue());
    QString uiStyleSheet = "QWidget { background-color: rgb(" + r + "," + g + "," + b + ") }";
    setStyleSheet(uiStyleSheet);
}

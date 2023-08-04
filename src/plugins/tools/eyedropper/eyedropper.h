/***************************************************************************
 *   Project TUPITUBE DESK                                                *
 *   Project Contact: info@maefloresta.com                                 *
 *   Project Website: http://www.maefloresta.com                           *
 *   Project Leader: Gustav Gonzalez <info@maefloresta.com>                *
 *                                                                         *
 *   Developers:                                                           *
 *   2010:                                                                 *
 *    Gustav Gonzalez / xtingray                                           *
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

#ifndef EYEDROPPERPLUGIN_H
#define EYEDROPPERPLUGIN_H

#include "tglobal.h"
#include "tuptoolplugin.h"
#include "eyedroppersettings.h"

class TUPITUBE_PLUGIN EyeDropper: public TupToolPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "com.maefloresta.tupi.TupToolInterface" FILE "eyedropper.json")
    
    public:
        EyeDropper();
        ~EyeDropper();

        virtual QList<TAction::ActionId> keys() const;

        void init(TupGraphicsScene *scene);
        
        virtual void press(const TupInputDeviceInformation *input, TupBrushManager *brushManager, 
                           TupGraphicsScene *gScene);
        virtual void move(const TupInputDeviceInformation *input, TupBrushManager *brushManager, 
                          TupGraphicsScene *gScene);

        virtual void release(const TupInputDeviceInformation *input, TupBrushManager *brushManager, 
                    TupGraphicsScene *gScene);
        
        virtual QMap<TAction::ActionId, TAction *> actions() const;
        TAction * getAction(TAction::ActionId toolId);

        int toolType() const;
        
        virtual QWidget *configurator();
        void aboutToChangeScene(TupGraphicsScene *scene);
        virtual void aboutToChangeTool();
        
        virtual void saveConfig();
        virtual void keyPressEvent(QKeyEvent *event);
        virtual QCursor toolCursor();

        virtual void updateColorType(TColorCell::FillType type);
        QColor grabColorFromScreen() const;
        void refreshEyeDropperPanel();

    signals:
        void closeHugeCanvas();
        void callForPlugin(int, int);
        void colorPicked(TColorCell::FillType type, const QColor &color);

    private:
        void setupActions();

    private:
        QCursor cursor;
        QMap<TAction::ActionId, TAction *> eyedropperActions;
        TupGraphicsScene *scene;
        EyeDropperSettings *settings;
        TColorCell::FillType fillType;
        const QDesktopWidget *desktop = QApplication::desktop();
};

#endif

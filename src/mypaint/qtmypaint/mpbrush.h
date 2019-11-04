/*
    Copyright © 2015 by The QTMyPaint Project

    This file is part of QTMyPaint, a Qt-based interface for MyPaint C++ library.

    QTMyPaint is libre software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    QTMyPaint is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with QTMyPaint. If not, see <http://www.gnu.org/licenses/>.
*/
#ifndef MPBRUSH_H
#define MPBRUSH_H

#include <QColor>

#include "mypaint-brush.h"

class MPBrush
{
    public:
        MPBrush();
        ~MPBrush();

        MyPaintBrush *brush;

        void initBrush();
        void load(const QByteArray &content);

        QColor getColor();

        void setColor(QColor newColor);

        float getValue(MyPaintBrushSetting setting);
        void setValue(MyPaintBrushSetting setting, float value);

    private:
        QColor color;
};

#endif // MPBRUSH_H

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
#include "mpbrush.h"

MPBrush::MPBrush()
{
    // Init Brush
    initBrush();

    // Set Default color to black
    setColor(QColor(0, 0, 0));
}

MPBrush::~MPBrush()
{
    mypaint_brush_unref(brush);
}

void MPBrush::initBrush()
{
    brush = mypaint_brush_new();
    mypaint_brush_from_defaults(brush);

    setValue(MYPAINT_BRUSH_SETTING_COLOR_H, 0);
    setValue(MYPAINT_BRUSH_SETTING_COLOR_S, 0);
    setValue(MYPAINT_BRUSH_SETTING_COLOR_V, 0);
    setValue(MYPAINT_BRUSH_SETTING_SNAP_TO_PIXEL, 0.0);
    setValue(MYPAINT_BRUSH_SETTING_ANTI_ALIASING, 1.0);
    setValue(MYPAINT_BRUSH_SETTING_RADIUS_LOGARITHMIC, static_cast<float>(0.3));
    // setValue(MYPAINT_BRUSH_SETTING_STROKE_DURATION_LOGARITHMIC, 4.0);
    // setValue(MYPAINT_BRUSH_SETTING_SPEED2_SLOWNESS, 0.8);
    // setValue(MYPAINT_BRUSH_SETTING_SPEED2_GAMMA, 10);
    // setValue(MYPAINT_BRUSH_SETTING_SPEED1_SLOWNESS, 0.04);
    // setValue(MYPAINT_BRUSH_SETTING_SPEED1_GAMMA, 10);
    // setValue(MYPAINT_BRUSH_SETTING_SMUDGE_LENGTH, 0.5);
    // setValue(MYPAINT_BRUSH_SETTING_SLOW_TRACKING_PER_DAB, 1.5);
    // setValue(MYPAINT_BRUSH_SETTING_SLOW_TRACKING, 1.03);
    // setValue(MYPAINT_BRUSH_SETTING_OFFSET_BY_RANDOM, 0.5);
    setValue(MYPAINT_BRUSH_SETTING_DIRECTION_FILTER, 10.0);
    setValue(MYPAINT_BRUSH_SETTING_DABS_PER_ACTUAL_RADIUS, 4.0);
}

void MPBrush::load(const QByteArray &content)
{
    mypaint_brush_from_defaults(brush);

    if (!mypaint_brush_from_string(brush, content.constData()))
    {
        // Not able to load the selected brush. Let's execute some backup code...
        qDebug("Trouble when reading the selected brush !");
    }
    setColor(color);
}

QColor MPBrush::getColor()
{
    return color;
}

void MPBrush::setColor(QColor newColor)
{
    color = newColor;

    float h = static_cast<float>(color.hue()/360.0);
    float s = static_cast<float>(color.saturation()/255.0);
    float v = static_cast<float>(color.value()/255.0);

    // Opacity is not handled here as it is defined by the brush settings.
    // If you wish to force opacity, use MPHandler::setBrushValue()
    //
    // float opacity = m_color.alpha()/255.0;
    // mypaint_brush_set_base_value(brush, MYPAINT_BRUSH_SETTING_OPAQUE, opacity);

    setValue(MYPAINT_BRUSH_SETTING_COLOR_H, h);
    setValue(MYPAINT_BRUSH_SETTING_COLOR_S, s);
    setValue(MYPAINT_BRUSH_SETTING_COLOR_V, v);
}

float MPBrush::getValue(MyPaintBrushSetting setting)
{
    return mypaint_brush_get_base_value(brush, setting);
}

void MPBrush::setValue(MyPaintBrushSetting setting, float value)
{
    mypaint_brush_set_base_value(brush, setting, value);
}

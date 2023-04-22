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
#include "tuptextitem.h"

#include <QWidget>
#include <QFont>
#include <QTextEdit>
#include <QSpinBox>
#include <QCheckBox>

class TUPITUBE_PLUGIN TextConfigurator : public QWidget
{
    Q_OBJECT

    public:
        enum Mode {Add, Update};
        TextConfigurator(QWidget *parent = nullptr);
        ~TextConfigurator();

        QString text() const;
        QFont textFont() const;
        void loadTextSettings(const QFont &itemFont, const QString &text, const QColor &color);
        void updateMode(Mode action);
        Qt::Alignment textAlignment();
        void resetText();
        void setTextColor(const QColor &color);
        QColor getTextColor() const;

        void displayControls(bool flag, const QPointF &point = QPointF(), const QDomElement &element = QDomElement());

        void updatePositionCoords(int x, int y);
        void updateRotationAngle(int angle);
        void updateScaleFactor(double x, double y);

        void setProportionState(int flag);

    signals:
        void textAdded();
        void textUpdated();

        void xPosChanged(int x);
        void yPosChanged(int y);
        void rotationChanged(int angle);
        void scaleChanged(double xFactor, double yFactor);
        void resetActionCalled();

        void scaleUpdated(double xFactor, double yFactor);
        void activateProportion(bool enable);

        void textObjectReleased();

    public slots:
        void clearText();

    private slots:
        void changeFont();
        void callAction();
        void updateTextAlignment(Qt::Alignment flag);

        void notifyRotation(int angle);
        void notifyXScale(double factor);
        void notifyYScale(double factor);
        void enableProportion(int flag);

    private:
        QWidget * createTransformationTools();

        QTextEdit *textBox;
        TFontChooser *fontChooser;
        QFont font;
        QPushButton *addButton;
        QPushButton *resetButton;
        QPushButton *clearButton;
        Mode mode;
        Qt::Alignment textAlignmentValue;

        QWidget *controlsWidget;
        QSpinBox *xPosField;
        QSpinBox *yPosField;
        QSpinBox *angleField;
        QDoubleSpinBox *factorXField;
        QDoubleSpinBox *factorYField;
        QCheckBox *propCheck;

        int currentX;
        int currentY;
        double currentXFactor;
        double currentYFactor;

        QMargins margins;
};
#endif

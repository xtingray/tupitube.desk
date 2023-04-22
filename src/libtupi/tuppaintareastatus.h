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

#ifndef TUPPAINTAREASTATUS_H
#define TUPPAINTAREASTATUS_H

#include "tglobal.h"
#include "tupbrushstatus.h"
#include "tuptoolstatus.h"

#include <QStatusBar>
#include <QPushButton>
#include <QComboBox>
#include <QCheckBox>
#include <QLineEdit>

class TUPITUBE_EXPORT TupPaintAreaStatus : public QStatusBar
{
    Q_OBJECT

    public:
        enum StatusType { Vector = 0, Raster };
        TupPaintAreaStatus(StatusType type, QPen pen = QPen(), QBrush brush = QBrush(), QWidget *parent = nullptr);
        ~TupPaintAreaStatus();

        void updateTool(const QString &label, const QPixmap &pixmap);
        void setZoomPercent(const QString &percent);
        void setRotationAngle(const QString &angle);
        void updateZoomFactor(double factor);
        qreal currentZoomFactor();
        void enableFullScreenFeature(bool flag);
        void updatePosition(const QString &position);
        void setFramePointer(int index);

    public slots:
        // void selectAntialiasingHint();
        void updateRotationAngle(int angle);
        void applyZoom(const QString &text);
        void setPen(const QPen &pen);
        void setBrush(const QBrush  &brush);
        void applyRotation(const QString &text);
        void showPos(const QPointF &point);
        void updateFrameIndex(int index);

    private slots:
        void updateFramePointer();

    signals:
        void newFramePointer(int index);
        void clearFrameClicked();
        void resetClicked();
        void safeAreaClicked();
        void gridClicked();
        void angleChanged(int angle);
        void zoomChanged(qreal factor);
        void antialiasChanged(bool flag);
        void fullClicked();

    private:
        void updateZoomField(const QString &text);
        void updateRotationField(const QString &degree);

        QPushButton *fullScreenButton;
        QLineEdit *frameField;
        QComboBox *zoomCombo;
        QComboBox *rotationCombo;

        TupBrushStatus *contourStatus;
        TupBrushStatus *fillStatus;

        TupToolStatus *toolStatus;
        TColorCell::FillType colorContext;
        qreal scaleFactor;
        int angle;
        int currentFrame;

        QMargins margins;
};

#endif

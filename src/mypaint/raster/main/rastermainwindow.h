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

#ifndef RASTERMAINWINDOW_H
#define RASTERMAINWINDOW_H

#include "tmainwindow.h"
#include "rastercanvas.h"
#include "rasterbrusheswidget.h"
#include "rastercolorwidget.h"
#include "tuppaintareastatus.h"
#include "tupbackground.h"

#include <QDockWidget>
#include <QPushButton>
#include <QColorDialog>
#include <QCloseEvent>

class TUPITUBE_EXPORT RasterMainWindow : public TMainWindow
{
    Q_OBJECT

    public:
        enum Perspective { Raster = 0x01 };

        explicit RasterMainWindow(TupProject *project, const QString &winKey, TupProject::Mode context,
                                  int scene, const QColor contourColor, const QString &zoomFactor,
                                  QWidget *parent = nullptr);
        ~RasterMainWindow();

        void setTabletDevice(QTabletEvent *event);

    public slots:
        void openProject();
        void exportImage();
        void saveCanvas();
        void processColorEvent(const TupPaintAreaEvent *);

        void resetWorkSpaceTransformations();
        void drawActionSafeArea();
        void drawGrid();
        void setZoomFactor(qreal factor);
        void applyZoomIn();
        void applyZoomOut();
        void setRotationAngle(int angle);

    private slots:
        void setBackgroundDirection(int direction);
        void updateBackgroundShiftProperty(int shift);
        // void undo();
        // void redo();

    signals:
         void paintAreaEventTriggered(const TupPaintAreaEvent *event);
         void closeWindow(const QString &path);

    protected:
        void closeEvent(QCloseEvent *event);
        void keyPressEvent(QKeyEvent *event);
        void resizeEvent(QResizeEvent *event);

    private:
        void createTopResources();
        void createCentralWidget(TupProject * project, const QColor contourColor);

        TupBackground *tupBg;
        QToolBar *topBar;
        RasterCanvas *rasterCanvas;
        RasterBrushesWidget *brushesWidget;
        RasterColorWidget *colorWidget;
        ToolView *brushesView;
        ToolView *colorView;
        TupPaintAreaStatus *status;

        QSize projectSize;
        int sceneIndex;
        QPushButton *colorBtn;
        QPushButton *clearBtn;
        QPushButton *saveBtn;
        QPushButton *openBtn;

        bool tabletIsActive;
        TupProject::Mode spaceContext;
};

#endif // RASTERMAINWINDOW_H

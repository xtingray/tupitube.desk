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

#ifndef TUPDOCUMENTVIEW_H
#define TUPDOCUMENTVIEW_H

#include "tglobal.h"
#include "tuppaintarea.h"
#include "tactionmanager.h"
#include "tupconfigurationarea.h"
#include "tupstoryboard.h"
#include "tupbrushmanager.h"
#include "tupproject.h"
#include "tupcanvas.h"
#include "tupruler.h"
#include "tuppaintareastatus.h"
#include "tupexportinterface.h"
#include "rastermainwindow.h"

#include <QMainWindow>
#include <QActionGroup>
#include <QTimer>
#include <QCursor>
#include <QAction>
#include <QActionGroup>
#include <QMenu>
#include <QSpinBox>
#include <QComboBox>
#include <QScreen>

class TupProjectRequest;
class TupProject;
class TupBrushManager;
class TupPaintAreaCommand;
class TupPaintAreaEvent;
class TupProjectResponse;

class Q_DECL_EXPORT TupDocumentView: public QMainWindow
{
    Q_OBJECT

    public:
        enum DockType { None = 0, ExposureSheet, TimeLine };

        TupDocumentView(TupProject *work, bool netFlag = true, const QStringList &users = QStringList(), QWidget *parent = nullptr);
        ~TupDocumentView();

        void setWorkSpaceSize(int width, int height);
        void closeInterface();
        QSize sizeHint() const;

        TupBrushManager *brushManager() const;
        QPen contourPen() const;
        QBrush fillBrush() const;

        TupPaintAreaCommand *createPaintCommand(const TupPaintAreaEvent *event);
        TupProject::Mode spaceContext();
        TupProject *currentProject();
        int currentFramesTotal();
        int currentSceneIndex();
        void setZoomPercent(const QString &percent);
        QSize workSpaceSize() const;
        void updateUsersOnLine(const QString &login, int state);
        void resizeProjectDimension(const QSize dimension);
        void updatePerspective();
        QColor projectBGColor() const;
        void updateWorkspace();
        void setFillTool(TColorCell::FillType type);

    private slots:
        void showModesSettings();
        void setRotationAngle(int angle);
        void setZoomFactor(qreal factor);
        void goToFrame(int index);
        void setNextOnionSkin(int n);
        void setPreviousOnionSkin(int n);
        void updateZoomVars(qreal factor);
        void applyZoomIn();
        void applyZoomOut();
        void updateRotationVars(int angle);
        void changeRulerOrigin(const QPointF &zero);
        void saveTimer();
        void showFullScreen();
        void closeFullScreen();
        void loadPlugin(int menu, int actionId);
        // void updateStatusBgColor(const QColor color);
        // void updatePenThickness(int size);
        void updateOnionOpacity(double opacity);
        void setVectorBackgroundDirection(int direction);
        void updateBackgroundShiftProperty(int shift);
        void renderVectorDynamicFrame();
        void fullScreenRightClick();
        void cameraInterface();
        void insertPictureInFrame(int id, const QString path);
        void papagayoManager();
        void openRasterMode();

        void refreshEyeDropperPanel();
        void launchEyeDropperTool();

    private slots: 
        // Plugins
        void loadPlugins();
        void setSpaceContext();
        void enableOnionFeature();
        void setDefaultOnionFactor();
        void setOnionFactor(double value);
        void exportImage();
        void postImage();
        void storyboardSettings();
        void selectFrame(int frame, int layer, int scene);
        void selectScene(int scene);

        // void showPos(const QPointF &point);
        void setCursor(const QCursor &cursor);
        void selectToolFromMenu(QAction *action);
        void callAutoSave();
        void sendStoryboard(TupStoryboard *storyboard, int sceneIndex);
        void updateStaticOpacity(double opacity);
        void updateDynamicOpacity(double opacity);

        void updatePen(const QPen &pen);
        void updateBrush(const QBrush &brush);
        void updateCameraMode();
        void closeRasterWindow(const QString &path);

        void importImageToLibrary(const QString &imgPath);

    public slots:
        void undo();
        void redo();
        void selectTool();
        void applyFilter();
        void drawGrid();
        void drawActionSafeArea();
        bool handleProjectResponse(TupProjectResponse *response);
        void updateNodesScale(qreal factor);
        void importPapagayoLipSync();
        void resetWorkSpaceTransformations();
        void updateBgColor(const QColor color);
        void updatePaintArea();
        void updateActiveDock(TupDocumentView::DockType currentDock);
        void setAntialiasing(bool useIt);
        void requestRasterStroke();
        void requestClearRasterCanvas();
        void updateBgSettings(QList<TupBackground::BgType>, QList<bool>);
        void clearFrame();
        void enableEyeDropperTool(TColorCell::FillType fillType);

    signals:
        void requestTriggered(const TupProjectRequest *event);
        void localRequestTriggered(const TupProjectRequest *event);
        void paintAreaEventTriggered(const TupPaintAreaEvent *event);
        void autoSave();
        void modeHasChanged(TupProject::Mode mode);
        void requestExportImageToServer(int frameIndex, int sceneIndex, const QString &title, const QString &topics, const QString &description);
        void openColorDialog(const QColor &);
        // void updateColorFromFullScreen(const QColor &color);
        // void penColorChanged(const QColor &color);
        void colorChanged(TColorCell::FillType fillType, const QColor color);

        // void updatePenFromFullScreen(const QPen &pen);
        void updateStoryboard(TupStoryboard *storyboard, int sceneIndex);
        void postStoryboard(int sceneIndex);
        // void projectHasChanged();
        void closePolyLine();
        void closeLine();
        void projectSizeHasChanged(const QSize dimension);
        void updateFPS(int fps);
        void newPerspective(int index);
        void contourColorChanged(const QColor &color);
        void fillColorChanged(const QColor &color);
        void bgColorChanged(const QColor &color);
        void penWidthChanged(int width);
        void colorModeChanged(TColorCell::FillType mode);
        void fillToolEnabled();
        void projectHasChanged();
        void imagePostRequested(const QString &);
        // void notifyLipsyncSoundMetadata(const QString &, int);

    private:
        void setupDrawActions();
        void createToolBar();
        void createMenu();
        void createLateralToolBar();
        void updateRotationAngleFromRulers(int angle);
        double backgroundOpacity(TupFrame::FrameType type);
        void updateOnionColorSchemeStatus(bool status);
        void updateToolsMenu(TAction::ActionId id, const QString &actionId);

        QMenu *shapesMenu;
        QMenu *motionMenu;
        QMenu *miscMenu;
        QMenu *filterMenu;

        TupPaintArea *paintArea;
        TupProject *project;
        QSize wsDimension;
        QScreen *screen;
        
        QToolBar *barGrid;
        QToolBar *toolbar;
        QToolBar *dynamicPropertiesBar;
        QToolBar *staticPropertiesBar;

        QDoubleSpinBox *onionFactorSpin;
        QSpinBox *prevOnionSkinSpin;
        QSpinBox *nextOnionSkinSpin;

        QComboBox *dirCombo;
        QSpinBox *shiftSpin;

        bool onionEnabled;
        int prevOnionValue;
        int nextOnionValue;
        double opacityFactor;
        int viewAngle;
        // int autoSaveTime;
        bool fullScreenOn;
        bool rasterWindowOn;
        bool isNetworked;
        QStringList onLineUsers;

        TAction *pencilAction;
        TAction *inkAction;
        // Note: Enable it only for debugging
        // TAction *schemeAction;
        TAction *polyLineAction;
        TAction *textAction;
        TAction *selectionAction;
        TAction *nodesAction;
        TAction *fillAction;
        TAction *papagayoAction;
        TAction *eyedropperAction;

        TupCanvas *fullScreen;
        RasterMainWindow *rasterWindow;

        TupRuler *verticalRuler;
        TupRuler *horizontalRuler;

        TActionManager *actionManager;
        TupConfigurationArea *configurationArea;
        TupToolPlugin *currentTool;
        TupPaintAreaStatus *status;
        QComboBox *spaceModeCombo;
        QString zoomFactor;

        bool dynamicFlag;
        bool staticFlag;
        QSize cameraSize;
        int photoCounter;

        QTimer *timer;

        TupExportInterface *imagePlugin;

        qreal nodesScaleFactor;
        qreal cacheScaleFactor;

        QActionGroup *actionGroup;

        DockType currentDock;
        bool cameraMode;
        TColorCell::FillType colorSpace;
        QColor contourColor;
};

#endif

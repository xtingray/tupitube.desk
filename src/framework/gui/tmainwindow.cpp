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

#include "tmainwindow.h"

class T_GUI_EXPORT DefaultSettings : public TMainWindowAbstractSettings
{
    public:
        DefaultSettings(QObject *parent);
        ~DefaultSettings();

        void save(TMainWindow *window);
        void restore(TMainWindow *window);
};

DefaultSettings::DefaultSettings(QObject *parent) : TMainWindowAbstractSettings(parent)
{
}

DefaultSettings::~DefaultSettings()
{
}

void DefaultSettings::save(TMainWindow *window)
{
    #ifdef TUP_DEBUG
        qWarning() << "TMainWindow::DefaultSettings::save() - Saving UI settings [ " + qApp->applicationName() + " ]";
    #endif

    QSettings settings(qApp->applicationName(), "ideality", this);

    QHash<Qt::ToolBarArea, TButtonBar *> buttonBars = window->buttonBars();
    QHash<TButtonBar *, QList<ToolView*> > toolViews = window->toolViews();

    foreach (TButtonBar *bar, buttonBars.values()) {
        foreach (ToolView *view, toolViews[bar]) {
            settings.beginGroup(view->objectName());

            settings.setValue("area", int(view->button()->area()));
            settings.setValue("style", view->button()->toolButtonStyle());
            settings.setValue("visible", view->isVisible());
            settings.setValue("floating", view->isFloating());
            settings.setValue("position", view->pos());

            settings.endGroup();
        }
    }

    settings.beginGroup("MainWindow");
    settings.setValue("size", window->size());
    settings.setValue("maximized", window->isMaximized());
    settings.setValue("position", window->pos());
    settings.endGroup();
}

void DefaultSettings::restore(TMainWindow *window)
{
    #ifdef TUP_DEBUG
        qWarning() << "TMainWindow::DefaultSettings::restore() - Restoring UI settings [ " + qApp->applicationName() + " ]";
    #endif

    QSettings settings(qApp->applicationName(), "ideality", this);

    QHash<Qt::ToolBarArea, TButtonBar*> buttonBars = window->buttonBars();
    QHash<TButtonBar*, QList<ToolView*>> toolViews = window->toolViews();

    QList<ToolView*> toHide;

    foreach (TButtonBar *bar, buttonBars.values()) {
        foreach (ToolView *view, toolViews[bar]) {
            settings.beginGroup(view->objectName());
            view->button()->setToolButtonStyle(Qt::ToolButtonStyle(settings.value("style", 
                                               int(view->button()->toolButtonStyle())).toInt()));

            bool visible = settings.value("visible", false).toBool();
            if (visible && view->button()->isVisible()) {
                view->button()->setChecked(true);
                view->show();
            } else {
                toHide << view;
            }

            settings.endGroup();
        }
    }
	
    foreach (ToolView *view, toHide) {
        view->button()->setChecked(false);
        view->setVisible(false);
        view->close();
    }

    settings.beginGroup("MainWindow");
    window->resize(settings.value("size").toSize());
    bool maximized = settings.value("maximized", false).toBool();

    if (maximized)
        window->showMaximized();

    window->move(settings.value("position").toPoint());
    settings.endGroup();
}

TMainWindow::TMainWindow(QWidget *parent): QMainWindow(parent), m_forRelayout(nullptr),
                                           perspective(DefaultPerspective), m_autoRestore(false)
{
    setObjectName("TMainWindow");
    settings = new DefaultSettings(this);

    specialToolBar = new QToolBar(tr("Show Top Panel"), this);
    specialToolBar->setIconSize(QSize(9, 5));
    specialToolBar->setMovable(false);

    addToolBar(Qt::LeftToolBarArea, specialToolBar);
    addButtonBar(Qt::LeftToolBarArea);
    addButtonBar(Qt::RightToolBarArea);
    addButtonBar(Qt::TopToolBarArea);
    addButtonBar(Qt::BottomToolBarArea);

    setDockNestingEnabled(false);
}

TMainWindow::~TMainWindow()
{
}

void TMainWindow::addButtonBar(Qt::ToolBarArea area)
{
    TButtonBar *bar = new TButtonBar(area, this);
    addToolBar(area, bar);
    m_buttonBars.insert(area, bar);
}

void TMainWindow::enableSpecialBar(bool flag)
{
    specialToolBar->setVisible(flag);
}

void TMainWindow::addSpecialButton(TAction *action)
{
    specialToolBar->addAction(action);
}

ToolView *TMainWindow::addToolView(QWidget *widget, Qt::DockWidgetArea area, int perspective, const QString &code, QKeySequence shortcut)
{
    /*
    #ifdef TUP_DEBUG
        qDebug() << "TMainWindow::addToolView() - code: " << code;
    #endif
    */

    ToolView *toolView = new ToolView(widget->windowTitle(), widget->windowIcon(), code);
    toolView->setShortcut(shortcut);
    toolView->setWidget(widget);
    toolView->setPerspective(perspective);
    toolView->button()->setArea(toToolBarArea(area));

    m_buttonBars[toToolBarArea(area)]->addButton(toolView->button());
    m_toolViews[m_buttonBars[toToolBarArea(area)]] << toolView;

    addDockWidget(area, toolView);
    // SQA: This line is a hack to avoid self-resizing docks issue
    // resizeDocks({toolView}, {200}, Qt::Horizontal);

    return toolView;
}

void TMainWindow::removeToolView(ToolView *view)
{
    #ifdef TUP_DEBUG
        qDebug() << "TMainWindow::removeToolView()";
    #endif

    bool findIt = false;

    foreach (TButtonBar *bar, m_buttonBars.values()) {
        QList<ToolView *> views = m_toolViews[bar];
        QList<ToolView *>::iterator it = views.begin();

        while (it != views.end()) {
            ToolView *toolView = *it;
            if (toolView == view) {
                views.erase(it);
                bar->removeButton(view->button());
                findIt = true;
                break;
            }
            ++it;
        }

        if (findIt) 
            break;
    }

    if (findIt)
        removeDockWidget(view);
}

void TMainWindow::enableToolViews(bool isEnabled)
{
    #ifdef TUP_DEBUG
        qDebug() << "TMainWindow::enableToolViews() - enable: " << isEnabled;
    #endif

    foreach (TButtonBar *bar, m_buttonBars.values()) {
        QList<ToolView *> views = m_toolViews[bar];
        QList<ToolView *>::iterator it = views.begin();

        while (it != views.end()) {
            ToolView *view = *it;
            view->enableButton(isEnabled);
            ++it;
        }
    }
}

void TMainWindow::addToPerspective(QWidget *widget, int workSpace)
{
    #ifdef TUP_DEBUG
        qDebug() << "TMainWindow::addToPerspective()";
    #endif

    if (QToolBar *bar = dynamic_cast<QToolBar*>(widget)) {
        if (toolBarArea(bar) == 0)
            addToolBar(bar);
    }

    if (!m_managedWidgets.contains(widget)) {
        m_managedWidgets.insert(widget, workSpace);

        if (!(workSpace & perspective))
            widget->hide();
    }
}

void TMainWindow::removeFromPerspective(QWidget *widget)
{
    #ifdef TUP_DEBUG
        qDebug() << "TMainWindow::removeFromPerspective()";
    #endif

    m_managedWidgets.remove(widget);
}

// Add action list to perspective
void TMainWindow::addToPerspective(const QList<QAction *> &actions, int workSpace)
{
    #ifdef TUP_DEBUG
        qDebug() << "TMainWindow::addToPerspective()";
    #endif

    foreach (QAction *action, actions)
        addToPerspective(action, workSpace);
}

// Add action to perspective
void TMainWindow::addToPerspective(QAction *action, int workSpace)
{
    #ifdef TUP_DEBUG
        qDebug() << "TMainWindow::addToFromPerspective()";
    #endif

    if (!m_managedActions.contains(action)) {
        m_managedActions.insert(action, workSpace);

        if (!(workSpace & perspective))
            action->setVisible(false);
    }
}

// Remove action from perspective
void TMainWindow::removeFromPerspective(QAction *action)
{
    m_managedActions.remove(action);
}

Qt::DockWidgetArea TMainWindow::toDockWidgetArea(Qt::ToolBarArea area)
{
    switch (area) {
        case Qt::LeftToolBarArea:
           {
             return Qt::LeftDockWidgetArea;
           }
        case Qt::RightToolBarArea:
           {
             return Qt::RightDockWidgetArea;
           }
        case Qt::TopToolBarArea:
           {
             return Qt::TopDockWidgetArea;
           }
        case Qt::BottomToolBarArea:
           {
             return Qt::BottomDockWidgetArea;
           }
        default: 
           {
              #ifdef TUP_DEBUG
                  qWarning() << "TMainWindow::toDockWidgetArea() - Floating -> " + QString::number(area);
              #endif
           }
           break;
    }

    return Qt::LeftDockWidgetArea;
}

Qt::ToolBarArea TMainWindow::toToolBarArea(Qt::DockWidgetArea area)
{
    switch (area) {
        case Qt::LeftDockWidgetArea:
           {
             return Qt::LeftToolBarArea;
           }
        case Qt::RightDockWidgetArea:
           {
             return Qt::RightToolBarArea;
           }
        case Qt::TopDockWidgetArea:
           {
             return Qt::TopToolBarArea;
           }
        case Qt::BottomDockWidgetArea:
           {
             return Qt::BottomToolBarArea;
           }
        default: 
           {
             #ifdef TUP_DEBUG
                 qWarning() << "TMainWindow::toToolBarArea() - Floating -> " + QString::number(area);
             #endif
           }
    }

    return Qt::LeftToolBarArea;
}

void TMainWindow::setCurrentPerspective(int workSpace)
{
    #ifdef TUP_DEBUG
        qDebug() << "TMainWindow::setCurrentPerspective()";
    #endif

    if (perspective == workSpace)
        return;

    if (workSpace != 1)
        specialToolBar->setVisible(false);
    else
        specialToolBar->setVisible(true);

    typedef QList<ToolView *> Views;
    QList<Views > viewsList = m_toolViews.values();

    QHash<TButtonBar *, int> hideButtonCount;
    foreach (Views views, viewsList) {
        foreach (ToolView *view, views) {
            TButtonBar *bar = m_buttonBars[view->button()->area()];

            if (view->perspective() & workSpace) {
                bar->enable(view->button());
                if (view->isExpanded()) {
                    view->blockSignals(true);
                    view->show(); 
                    view->blockSignals(false);
                }
            } else {
                bar->disable(view->button());
                if (view->isExpanded()) {
                    view->blockSignals(true);
                    view->close();
                    view->blockSignals(false);
                }
                hideButtonCount[bar]++;
            }

            if (bar->isEmpty() && bar->isVisible()) {
                bar->hide();
            } else {
                if (!bar->isVisible())
                    bar->show();
            }
        }
    }

    QHashIterator<TButtonBar *, int> barIt(hideButtonCount);
    // This loop hides the bars with no buttons
    while (barIt.hasNext()) {
        barIt.next();
        if (barIt.key()->count() == barIt.value())
            barIt.key()->hide();
    }

    perspective = workSpace;
    emit perspectiveChanged(perspective);
}

int TMainWindow::currentPerspective() const
{
    return perspective;
}

// if autoRestore is true, the widgets will be loaded when main window is showed (position and properties)
void TMainWindow::setAutoRestore(bool autoRestore)
{
    m_autoRestore = autoRestore;
}

bool TMainWindow::autoRestore() const
{
    return m_autoRestore;
}

void TMainWindow::setSettingsHandler(TMainWindowAbstractSettings *config)
{
    delete settings;

    settings = config;
    settings->setParent(this);
}

void TMainWindow::closeEvent(QCloseEvent *e)
{
    /*
    #ifdef TUP_DEBUG
        qDebug() << "TMainWindow::closeEvent()";
    #endif
    */

    saveGUI();
    QMainWindow::closeEvent(e);
}

void TMainWindow::showEvent(QShowEvent *e)
{
    /*
    #ifdef TUP_DEBUG
        qDebug() << "TMainWindow::showEvent() - m_autoRestore: " << m_autoRestore;
    #endif
    */

    QMainWindow::showEvent(e);

    if (!m_autoRestore) {
        m_autoRestore = true;
        restoreGUI();
        int cwsp = perspective;
        perspective -= 1;
        setCurrentPerspective(cwsp);
    }
}

void TMainWindow::saveGUI()
{
    #ifdef TUP_DEBUG
        qDebug() << "TMainWindow::saveGUI()";
    #endif

    settings->save(this);
}

void TMainWindow::restoreGUI()
{
    #ifdef TUP_DEBUG
        qDebug() << "TMainWindow::restoreGUI()";
    #endif

    setUpdatesEnabled(false);
    settings->restore(this);
    setUpdatesEnabled(true);
}

QHash<Qt::ToolBarArea, TButtonBar *> TMainWindow::buttonBars() const
{
    return m_buttonBars;
}

QHash<TButtonBar *, QList<ToolView*> > TMainWindow::toolViews() const
{
    return m_toolViews;
}

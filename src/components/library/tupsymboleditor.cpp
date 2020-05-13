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

#include "tupsymboleditor.h"

View::View()
{
}

View::~View()
{
}

TupSymbolEditor::TupSymbolEditor(QWidget *parent) : QMainWindow(parent)
{
    setWindowTitle(tr("Symbol editor"));
    
    view = new View;
    view->setRenderHints(QPainter::Antialiasing);
    
    scene = new QGraphicsScene;
    view->setScene(scene);
    
    setCentralWidget(view);
    
    brushTools = new QToolBar(tr("Brushes"));
    addToolBar(Qt::BottomToolBarArea, brushTools);
    
    selectionTools = new QToolBar(tr("Selection"));
    addToolBar(Qt::BottomToolBarArea, selectionTools);
    
    fillTools = new QToolBar(tr("Fill"));
    addToolBar(Qt::BottomToolBarArea, fillTools);
    
    viewTools = new QToolBar(tr("View"));
    addToolBar(Qt::BottomToolBarArea, viewTools);
    
    QTimer::singleShot(0, this, SLOT(loadTools()));
}

TupSymbolEditor::~TupSymbolEditor()
{
}

void TupSymbolEditor::setSymbol(TupLibraryObject *object)
{
    if (QGraphicsItem *item = qvariant_cast<QGraphicsItem *>(object->getData())) {
        symbol = object;
        scene->addItem(item);
    }
}

void TupSymbolEditor::loadTools()
{
    QActionGroup *group = new QActionGroup(this);
    group->setExclusive(true);
    
    foreach (QObject *plugin, TupPluginManager::instance()->getTools()) {
             TupToolPlugin *tool = qobject_cast<TupToolPlugin *>(plugin);
        
             QStringList::iterator it;
             QStringList keys = tool->keys();
            
             for (it = keys.begin(); it != keys.end(); ++it) {
                  #ifdef TUP_DEBUG
                      qDebug() << "TupSymbolEditor::loadTools() - *** Tool Loaded: " + *it;
                  #endif
            
                  TAction *act = tool->actions()[*it];
                  if (act) {
                      connect(act, SIGNAL(triggered()), this, SLOT(selectTool()));
                
                      switch (tool->toolType()) {
                              case TupToolInterface::Selection:
                              {
                                   selectionTools->addAction(act);
                              }
                              break;
                              case TupToolInterface::Fill:
                              {
                                   fillTools->addAction(act);
                              }
                              break;
                              case TupToolInterface::View:
                              {
                                   viewTools->addAction(act);
                              }
                              break;
                              case TupToolInterface::Brush:
                              {
                                   brushTools->addAction(act);
                              }
                              break;
                      }
                
                      group->addAction(act);
                      act->setCheckable(true);
                      act->setParent(plugin);
                  }
             }
    }
}

void TupSymbolEditor::selectTool()
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupSymbolEditor::selectTool()]";
    #endif

    TAction *action = qobject_cast<TAction *>(sender());
    
    if (action) {
        TupToolPlugin *tool = qobject_cast<TupToolPlugin *>(action->parent());
        tool->setName(action->text());
    }
}

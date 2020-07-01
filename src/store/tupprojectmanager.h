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

#ifndef TUPPROJECTMANAGER_H
#define TUPPROJECTMANAGER_H

#include "tglobal.h"

#include <QUndoStack>
#include <QDir>
#include <QFileInfo>
#include <QObject>
#include <QSize>

class TupProject;
class TupProjectRequest;
class TupProjectCommand;
class TupProjectManagerParams;
class TupAbstractProjectHandler;
class QUndoStack;
class TupCommandExecutor;
class TupProjectResponse;

class TUPITUBE_EXPORT TupProjectManager : public QObject
{
    Q_OBJECT

    public:
        TupProjectManager(QObject *parent = nullptr);
        virtual ~TupProjectManager();

        void setParams(TupProjectManagerParams *getParams);
        TupProjectManagerParams *getParams() const;

        virtual void setupNewProject();
        virtual void closeProject();

        bool isOpen() const;
        bool projectWasModified() const;
        TupProject *getProject();
        void setHandler(TupAbstractProjectHandler *getHandler, bool isNetworked);
        TupAbstractProjectHandler *getHandler() const;

        void createCommand(TupProjectCommand *command);
        void clearUndoStack();

        virtual bool saveProject(const QString &fileName);
        virtual bool loadProject(const QString &fileName);

        bool isValid() const;
        void setModificationStatus(bool changed);
        void setOpen(bool isOpen);

        void updateProjectDimension(const QSize size);
        bool removeProjectPath(const QString &projectPath);

        int framesCount(int sceneIndex);

    protected slots:
        virtual void handleProjectRequest(const TupProjectRequest *request);
        virtual void handleLocalRequest(const TupProjectRequest *request);
        virtual void createCommand(const TupProjectRequest *request, bool addToStack);

    private slots:
        void emitResponse(TupProjectResponse *response);
        void undo();
        void redo();

    signals:
        void responsed(TupProjectResponse *reponse);
        void requestOpenProject(const QString &filename);

    private:
        bool modified;
        int sceneIndex;
        int layerIndex;
        int frameIndex;
        bool isNetworked;

        TupProject *project;
        QUndoStack *undoStack;
        TupAbstractProjectHandler *handler;
        TupProjectManagerParams *params;
        TupCommandExecutor *commandExecutor;
};

#endif

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

#ifndef TUPNETPROJECTMANAGERHANDLER_H
#define TUPNETPROJECTMANAGERHANDLER_H

#include "tglobal.h"
#include "tupabstractprojectmanagerhandler.h"
#include "tupstoryboard.h"

#include "tupnetprojectmanagerparams.h"
#include "tupprojectresponse.h"
#include "tosd.h"
#include "tupprojectcommand.h"
#include "tupcommandexecutor.h"
#include "tupnetsocket.h"
#include "tupprojectrequest.h"
#include "tupnewprojectpackage.h"
#include "tupconnectpackage.h"
#include "tupimageexportpackage.h"
#include "tupvideoexportpackage.h"
#include "tupstoryboardupdatepackage.h"
#include "tupstoryboardexportpackage.h"
#include "tupstoryboardparser.h"
#include "tupnetfilemanager.h"
#include "tupopenpackage.h"
#include "tupchatpackage.h"
#include "tupnotificationparser.h"
#include "tupprojectlistparser.h"
#include "tupprojectparser.h"
#include "tuprequestparser.h"
#include "tupackparser.h"
#include "tupcommunicationparser.h"
#include "tuprequestbuilder.h"
#include "tupproject.h"
#include "tuplistprojectdialog.h"
#include "tupchat.h"
#include "tupnotice.h"

#include <QDomDocument>
#include <QTabWidget>
#include <QTemporaryFile>
#include <QTabWidget>
#include <QDesktopWidget>
#include <QMessageBox>

class TupNetSocket;

/*
class TupChat;
class TupProjectCommand;
class TupNetProjectManagerParams;
*/

class TUPITUBE_EXPORT TupNetProjectManagerHandler : public TupAbstractProjectHandler
{
    Q_OBJECT

    public:
        TupNetProjectManagerHandler(QObject *parent = nullptr);
        ~TupNetProjectManagerHandler();

        virtual void initialize(TupProjectManagerParams *params);
        virtual bool setupNewProject(TupProjectManagerParams *params);
        virtual bool closeProject();

        virtual void handleProjectRequest(const TupProjectRequest* event);
        virtual bool commandExecuted(TupProjectResponse *response);
        virtual bool saveProject(const QString &fileName, TupProject *project);
        virtual bool loadProject(const QString &fileName, TupProject *project);

        void handlePackage(const QString &root, const QString &package);
        virtual bool isValid() const;
        void sendPackage(const QDomDocument &doc);

        QTabWidget *communicationWidget();
        void closeConnection();

    signals:
        void savingSuccessful();
        void postOperationDone();
        void connectionHasBeenLost();

    public slots:
        void sendExportImageRequest(int frameIndex, int sceneIndex, const QString &title, const QString &topics, const QString &description);
        void updateStoryboardRequest(TupStoryboard *storyboard, int sceneIndex);
        void postStoryboardRequest(int sceneIndex);
        void sendVideoRequest(const QString &title, const QString &topics, const QString &description, int fps, const QList<int> sceneIndexes);

    private slots:
        void sendChatMessage(const QString &message);
        void connectionLost();

    private:
        void loadProjectFromServer(const QString &projectID, const QString &owner);
        void emitRequest(TupProjectRequest *request, bool toStack);
        void setProject(TupProject *project);

        TupNetProjectManagerParams *params;
        TupNetSocket *socket;
        QString projectName;
        QString username;
        TupProject *project;

        QString sign;
        bool ownPackage;
        bool doAction;

        QTabWidget *communicationModule;
        TupChat *chat;
        TupNotice *notices;

        bool projectIsOpen;
        bool dialogIsOpen;
        TupListProjectDialog *dialog;
};

#endif

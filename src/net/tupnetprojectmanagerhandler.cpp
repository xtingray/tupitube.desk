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

#include "tupnetprojectmanagerhandler.h"

#include <QScreen>

TupNetProjectManagerHandler::TupNetProjectManagerHandler(QObject *parent) : TupAbstractProjectHandler(parent)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupNetProjectManagerHandler()]";
    #endif

    socket = new TupNetSocket(this);
    connect(socket, SIGNAL(disconnected()), this, SLOT(connectionLost()));

    project = nullptr;
    params = nullptr;
    ownPackage = false;
    doAction = true;
    projectIsOpen = false;
    dialogIsOpen = false;
    
    communicationModule = new QTabWidget;
    communicationModule->setWindowTitle(tr("Communications"));
    communicationModule->setWindowIcon(QPixmap(THEME_DIR + "icons/chat.png"));

    chat = new TupChat;
    communicationModule->addTab(chat, tr("Chat"));
    
    connect(chat, SIGNAL(requestSendMessage(const QString&)), this, SLOT(sendChatMessage(const QString&)));
    
    notices = new TupNotice;
    communicationModule->addTab(notices, tr("Notices"));
    
    // connect(notices, SIGNAL(requestSendMessage(const QString&)), this, SLOT(sendNoticeMessage(const QString&)));
}

TupNetProjectManagerHandler::~TupNetProjectManagerHandler()
{
    #ifdef TUP_DEBUG
        qDebug() << "[~TupNetProjectManagerHandler()]";
    #endif


    chat->close();
}

void TupNetProjectManagerHandler::handleProjectRequest(const TupProjectRequest* request)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupNetProjectManagerHandler::handleProjectRequest()]";
    #endif

    // This comes from the project before the command execution
    // SQA: Save a copy of the events or queued packages and resend to the GUI when the "Ok" package 
    // comes from the server 

    if (socket->state() == QAbstractSocket::ConnectedState) {
        #ifdef TUP_DEBUG
            qWarning() << "TupNetProjectManagerHandler::handleProjectRequest() - SENDING PACKAGE: " + request->getXml();
        #endif

        if (request->isValid()) {
            emit sendCommand(request, true);
            socket->send(request->getXml());
        } else {
            #ifdef TUP_DEBUG
                qDebug() << "TupNetProjectManagerHandler::handleProjectRequest() - INVALID REQUEST! ID: " + QString::number(request->getId());
            #endif
        }
    }
}

bool TupNetProjectManagerHandler::commandExecuted(TupProjectResponse *response)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupNetProjectManagerHandler::commandExecuted()]";
    #endif

    if (response->getMode() == TupProjectResponse::Do) {
        doAction = true;
        return true;
    } 

    TupProjectRequest request = TupRequestBuilder::fromResponse(response);
    doAction = false;

    if (response->getMode() != TupProjectResponse::Undo && response->getMode() != TupProjectResponse::Redo) {
        handleProjectRequest(&request);
    } else { 
        if (socket->state() == QAbstractSocket::ConnectedState) {
            if (request.isValid())
                socket->send(request.getXml());
        }
    }

    return true;
}

bool TupNetProjectManagerHandler::saveProject(const QString &fileName, TupProject *project)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupNetProjectManagerHandler::saveProject()]";
    #endif

    Q_UNUSED(fileName);
    Q_UNUSED(project);

    /*
    TupiNetFileManager manager(params->server(), params->port());
    return manager.save(fileName, project);
    */

    return true;
}

bool TupNetProjectManagerHandler::loadProject(const QString &fileName, TupProject *project)
{
    // SQA: Check why this two variables are required as parameters here
    Q_UNUSED(fileName);
    Q_UNUSED(project);

    /*
    if (socket->state() != QAbstractSocket::ConnectedState)
        return false;
    
    return loadProjectFromServer(params->projectName());
    */

    return true;
}

void TupNetProjectManagerHandler::loadProjectFromServer(const QString &projectID, const QString &owner)
{
    TupOpenPackage package(projectID, owner);
    socket->send(package);
}

void TupNetProjectManagerHandler::initialize(TupProjectManagerParams *params)
{
    TupNetProjectManagerParams *netParams = dynamic_cast<TupNetProjectManagerParams*>(params);

    if (!netParams)
        return;
    
    // params = netParams;

    #ifdef TUP_DEBUG
        qWarning() << "TupNetProjectManagerHandler::initialize() - Connecting to " + netParams->server() + ":" + QString::number(netParams->port());
    #endif

    socket->connectToHost(netParams->server(), netParams->port());
    bool connected = socket->waitForConnected(1000);

    if (connected) {
        TupConnectPackage connectPackage(netParams->server(), netParams->login(), netParams->password());
        socket->send(connectPackage);
        username = netParams->login();
    } else {
        TOsd::self()->display(TOsd::Error, tr("Unable to connect to server"));
    }
}

bool TupNetProjectManagerHandler::setupNewProject(TupProjectManagerParams *params)
{
    TupNetProjectManagerParams *netParams = dynamic_cast<TupNetProjectManagerParams*>(params);
    
    if (!netParams)
        return false;
    
    #ifdef TUP_DEBUG
        qWarning() << "netParams->projectName() : " << netParams->getProjectManager();
        SHOW_VAR(netParams->getProjectManager());
    #endif    

    projectName = netParams->getProjectManager();
    // author = netParams->author();
    /* 
    if (! socket->isOpen()) {
        bool connected = initialize(params);
        if (!connected) 
            return false;
    }
    */

    QString dimension = QString::number(netParams->getDimension().width()) + "," + QString::number(netParams->getDimension().height());

    TupNewProjectPackage newProjectPackage(netParams->getProjectManager(), netParams->getAuthor(), netParams->getDescription(),
                                           netParams->getBgColor().name(), dimension, QString::number(netParams->getFPS()));
    socket->send(newProjectPackage);
    
    return true;
}

bool TupNetProjectManagerHandler::closeProject()
{
    projectIsOpen = false;

    closeConnection();

    return TupAbstractProjectHandler::closeProject();
}

void TupNetProjectManagerHandler::emitRequest(TupProjectRequest *request, bool toStack)
{
    emit sendCommand(request, toStack);
}

void TupNetProjectManagerHandler::handlePackage(const QString &root, const QString &package)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupNetProjectManagerHandler::handlePackage()]";
        qDebug() << "TupNetProjectManagerHandler::handlePackage() - PKG:";
        qWarning() << package;
    #endif

    if (root == "user_denied") {
        closeConnection();
        QMessageBox msgBox;
        msgBox.setWindowTitle(tr("Fatal Error"));
        msgBox.setIcon(QMessageBox::Critical);
        msgBox.setText(tr("User \"%1\" is disabled.\nPlease, contact the animation server admin to get access.").arg(params->login()));
        msgBox.exec();
    } else if (root == "project_request") {
               TupRequestParser parser;

               if (parser.parse(package)) {
                   if (parser.getSign() == sign)
                       ownPackage = true;
                   else
                       ownPackage = false;

                   if (ownPackage && !doAction) {
                       if (parser.getResponse()->getPart() == TupProjectRequest::Item) {
                           TupItemResponse *response = static_cast<TupItemResponse *>(parser.getResponse());
                           TupProjectRequest request = TupRequestBuilder::createFrameRequest(response->getSceneIndex(), 
                                                      response->getLayerIndex(), response->getFrameIndex(), TupProjectRequest::Select);
                           request.setExternal(!ownPackage);
                           emit sendLocalCommand(&request);
                       }
                       return;
                   } else {
                       TupProjectRequest request = TupRequestBuilder::fromResponse(parser.getResponse());
                       request.setExternal(!ownPackage);
                       emitRequest(&request, doAction && ownPackage);
                   }
               } else { // SQA: show error 
                   #ifdef TUP_DEBUG
                       qDebug() << "TupNetProjectManagerHandler::handlePackage() - Error parsing net request";
                   #endif
               }
    } else if (root == "project_storyboard_update") {
               // tError() << "TupNetProjectManagerHandler::handlePackage() - Updating the storyboard...";
               TupStoryboardParser parser(package);

               if (parser.checksum()) {
                   if ((parser.sceneIndex() >= 0) && (parser.storyboardXml().length() > 0)) {
                       TupStoryboard *storyboard = new TupStoryboard(username);
                       storyboard->fromXml(parser.storyboardXml());
                       project->sceneAt(parser.sceneIndex())->setStoryboard(storyboard);
                   } else {
                       #ifdef TUP_DEBUG
                           qDebug() << "ProjectManager::handlePackage() - [ Fatal Error ] - Can't parse project_storyboard package";
                       #endif
                   }
               } else {
                   #ifdef TUP_DEBUG
                       qDebug() << "ProjectManager::handlePackage() - [ Fatal Error ] - Can't parse project_storyboard package"; 
                   #endif

               }
    } else if (root == "server_ack") {
               // Checking the package
               TupAckParser parser;
               if (parser.parse(package)) {
                   sign = parser.sign();
                   // TOsd::self()->display(tr("Information"), tr("Login successful!")); 
                   // TOsd::self()->display(tr("Information"), parser.motd());
                   emit authenticationSuccessful(); 
               }
    } else if (root == "server_project") {
               TupProjectParser parser;
               if (parser.parse(package)) {
                   QTemporaryFile file;
                   if (file.open()) {
                       file.write(parser.data());
                       file.flush();
            
                       if (project) {
                           TupFileManager *manager = new TupFileManager;
                           bool isOk = manager->load(file.fileName(), project);
                           if (isOk) {
                               projectIsOpen = true;
                               emit openNewArea(project->getName(), parser.partners());
                           } else {
                               #ifdef TUP_DEBUG
                                   qDebug() << "TupNetProjectManagerHandler::handlePackage() - Error: Net project can't be opened";
                               #endif
                           }
                           delete manager;
                       } else {
                           #ifdef TUP_DEBUG
                               qDebug() << "TupNetProjectManagerHandler::handlePackage() - Error: Can't open project";
                           #endif
                       }
                   }
               }
    } else if (root == "server_projectlist") {
               TupProjectListParser parser;
               if (parser.parse(package)) {
                   int works = parser.workSize();
                   int contributions = parser.contributionSize();
                   if ((works + contributions) > 0) {
                       dialog = new TupListProjectDialog(works, contributions, params->server());
                       QDesktopWidget desktop;
                       dialog->show();

                       /*
                       dialog->move((int) (desktop.screenGeometry().width() - dialog->width())/2,
                                       (int) (desktop.screenGeometry().height() - dialog->height())/2);
                       */

                       QScreen *screen = QGuiApplication::screens().at(0);
                       dialog->move(static_cast<int> ((screen->geometry().width() - dialog->width()) / 2),
                                    static_cast<int> ((screen->geometry().height() - dialog->height()) / 2));

                       dialogIsOpen = true;

                       foreach (TupProjectListParser::ProjectInfo info, parser.works())
                                dialog->addWork(info.file, info.name, info.description, info.date);

                       foreach (TupProjectListParser::ProjectInfo info, parser.contributions())
                                dialog->addContribution(info.file, info.name, info.author, info.description, info.date);

                       if (dialog->exec() == QDialog::Accepted && !dialog->projectID().isEmpty()) {
                           #ifdef TUP_DEBUG
                               qWarning() << "TupNetProjectManagerHandler::handlePackage() - opening project -> " + dialog->projectID();
                           #endif
                           dialogIsOpen = false;
                           if (dialog->workIsMine())
                               loadProjectFromServer(dialog->projectID(), username);
                           else
                               loadProjectFromServer(dialog->projectID(), dialog->owner());
                       } else {
                           dialogIsOpen = false;
                           closeConnection();
                       }
                   } else {
                       TOsd::self()->display(TOsd::Warning, tr("User has no available projects in the server"));
                       #ifdef TUP_DEBUG
                           qDebug() << "TupNetProjectManagerHandler::handlePackage() - Info: User has no available projects in the server";
                       #endif
                       closeConnection();
                   }
               }
    } else if (root == "communication_notification") {
               TupNotificationParser parser;
               if (parser.parse(package)) {
                   int code = parser.notification().code;

                   switch(code) {
                          case 380:
                               emit savingSuccessful();
                          break;
                          case 100:
                          case 101:
                          case 102:
                          case 382:
                          case 383:
                          case 384:
                               emit postOperationDone();
                          break;
                   }

                   TOsd::Level level = TOsd::Level(parser.notification().level);
                   /*
                   QString title = "Information";
                   if (level == TOsd::Warning) {
                       title = tr("Warning");
                   } else if (level == TOsd::Error) {
                              title = tr("Error");
                   }
                   */
                   TOsd::self()->display(level, parser.notification().message);
               }
    } else if (root == "communication_chat") {
               TupCommunicationParser parser;
               if (parser.parse(package)) {
                   chat->addMessage(parser.login(), parser.message());
               }
    } else if (root == "communication_notice") {
               TupCommunicationParser parser;
               if (parser.parse(package)) {
                   QString login = parser.login();
                   int state = parser.state();

                   emit updateUsersList(login, state);

                   QString message = "<b>" + login + "</b>" + " has left the project"; 
                   if (state == 1)
                       message = "<b>" + login + "</b>" + " has joined the project";

                   TOsd::self()->display(TOsd::Info, message);
                   notices->addMessage(message);
               } 
    } else if (root == "communication_wall") {
               TupCommunicationParser parser;
               if (parser.parse(package)) {
                   QString message = QObject::tr("Wall from") + ": "+ parser.login() + "\n" + parser.message();
                   TOsd::self()->display(TOsd::Info, message);
               }
    } else if (root == "storyboard_update") {
               // SQA: storyboard package must be parsed and the related scene must be updated  
    } else {
      #ifdef TUP_DEBUG
          qDebug() << "TupNetProjectManagerHandler::handlePackage() - Error: Unknown package: " + root;
      #endif
    }
}

bool TupNetProjectManagerHandler::isValid() const
{
    return socket->state() == QAbstractSocket::ConnectedState;
}

void TupNetProjectManagerHandler::sendPackage(const QDomDocument &doc)
{
    // qDebug() << "TupNetProjectManagerHandler::sendPackage() - xml: " << doc.toString();
    socket->send(doc);
}

QTabWidget *TupNetProjectManagerHandler::communicationWidget()
{
    return communicationModule;
}

void TupNetProjectManagerHandler::setProject(TupProject *work)
{
    project = work;
}

void TupNetProjectManagerHandler::sendChatMessage(const QString &message)
{
    TupChatPackage package(message);
    sendPackage(package);
}

void TupNetProjectManagerHandler::connectionLost()
{
    #ifdef TUP_DEBUG
        qWarning() << "TupNetProjectManagerHandler::connectionLost() - The socket has been closed";
    #endif


    if (dialogIsOpen) {
        if (dialog) {
            if (dialog->isVisible())
                dialog->close();
        }
        emit connectionHasBeenLost();
    } else if (projectIsOpen) {
               emit connectionHasBeenLost();
    }
}

void TupNetProjectManagerHandler::closeConnection()
{
    if (socket->isOpen())
        socket->close();
}

void TupNetProjectManagerHandler::sendExportImageRequest(int frameIndex, int sceneIndex, 
                                                         const QString &title, const QString &topics, const QString &description)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupNetProjectManagerHandler::sendExportImageRequest()]";
    #endif

    TupImageExportPackage package(frameIndex, sceneIndex, title, topics, description);
    sendPackage(package);
}

void TupNetProjectManagerHandler::sendVideoRequest(const QString &title, const QString &topics, const QString &description, int fps, const QList<int> sceneIndexes)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupNetProjectManagerHandler::sendVideoRequest()]";
    #endif

    TupVideoExportPackage package(title, topics, description, fps, sceneIndexes);
    sendPackage(package);
}

void TupNetProjectManagerHandler::updateStoryboardRequest(TupStoryboard *storyboard, int sceneIndex)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupNetProjectManagerHandler::updateStoryboardRequest()]";
    #endif

    QDomDocument doc;
    QDomElement story = storyboard->toXml(doc);
    TupStoryboardUpdatePackage package(story, sceneIndex);
    sendPackage(package);
}

void TupNetProjectManagerHandler::postStoryboardRequest(int sceneIndex)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupNetProjectManagerHandler::postStoryboardRequest()]";
    #endif

    TupStoryboardExportPackage package(sceneIndex);
    sendPackage(package);
}

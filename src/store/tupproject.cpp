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

#include "tupproject.h"
#include "tupscene.h"
#include "tuplayer.h"
#include "tupsoundlayer.h"
#include "tupframe.h"
#include "tupbackground.h"
#include "tuplibrary.h"
#include "tuplibraryobject.h"
#include "tupgraphiclibraryitem.h"
#include "tupitemfactory.h"
#include "tupprojectresponse.h"
#include "tupprojectloader.h"

TupProject::TupProject(QObject *parent) : QObject(parent)
{
    #ifdef TUP_DEBUG
        #ifdef Q_OS_WIN
            qDebug() << "[TupProject()]";
        #else
            TINIT;
        #endif
    #endif    
    
    spaceMode = TupProject::NONE;
    bgColor = QColor("#fff");
    projectTags = tr("#animation #2D");
    sceneCounter = 0;
    isOpen = false;
    library = new TupLibrary("library", this);
    cachePath = "";
}

TupProject::~TupProject()
{
    #ifdef TUP_DEBUG
        #ifdef Q_OS_WIN
            qDebug() << "[~TupProject()]";
        #else
            TEND;
        #endif
    #endif    
        
    // deleteDataDir(cachePath);
    scenesList.clear();
}

void TupProject::loadLibrary(const QString &filename)
{
    #ifdef TUP_DEBUG
        #ifdef Q_OS_WIN
            qDebug() << "[TupProject::loadLibrary()]";
        #else
            T_FUNCINFO;
        #endif
    #endif

    QFile file(filename);

    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        library->fromXml(QString::fromLocal8Bit(file.readAll()));
        file.close();
    } else {               
        #ifdef TUP_DEBUG
            QString msg = "TupProject::loadLibrary() - Cannot open library from: " + filename;
            #ifdef Q_OS_WIN
                qDebug() << msg;
            #else
                tFatal("library") << msg;
            #endif
        #endif
    }
}

void TupProject::clear()
{
    for (int i=0; i<scenesList.count(); i++) {
         TupScene *scene = scenesList.takeAt(i);
         scene->clear();
         delete scene;
         scene = nullptr;
    }

    scenesList.clear();
    sceneCounter = 0;

    // deleteDataDir(cachePath);
}

void TupProject::setProjectName(const QString &name)
{
    projectName = name;
}

void TupProject::setAuthor(const QString &author)
{
    projectAuthor = author;
}

void TupProject::setTags(const QString &tags)
{
    projectTags = tags;
}

void TupProject::setBgColor(const QColor color)
{
    bgColor = color;

    int totalScenes = scenesList.size();
    for (int i = 0; i < totalScenes; i++) {
         TupScene *scene = scenesList.at(i);
         scene->setBgColor(color);
    }
}

void TupProject::setDescription(const QString &description)
{
    projectDesc = description;
}

void TupProject::setDimension(const QSize size)
{
    dimension = size;
}

void TupProject::setFPS(const int value)
{
    fps = value;
}

void TupProject::setDataDir(const QString &path)
{
    cachePath = path;
}

QString TupProject::getName() const
{
    return projectName;
}

QString TupProject::getAuthor() const
{
    return projectAuthor;
}

QString TupProject::getTags() const
{
    return projectTags;
}

QColor TupProject::getBgColor() const
{
    return bgColor;
}

QString TupProject::getDescription() const
{
    return projectDesc;
}

QSize TupProject::getDimension() const
{
    return dimension;
}

int TupProject::getFPS() const
{
    return fps;
}

QString TupProject::getDataDir() const
{
    return cachePath;
}

TupScene *TupProject::createScene(QString name, int position, bool loaded)
{
    #ifdef TUP_DEBUG
        #ifdef Q_OS_WIN
            qDebug() << "[TupProject::createScene()]";
        #else
            T_FUNCINFO;
        #endif
    #endif

    if (position < 0 || position > scenesList.count())
        return nullptr;

    TupScene *scene = new TupScene(this, dimension, bgColor);
    scenesList.insert(position, scene);
    sceneCounter++;
    scene->setSceneName(name);
    
    if (loaded)
        TupProjectLoader::createScene(scene->getSceneName(), position, this);

    return scene;
}

void TupProject::updateScene(int position, TupScene *scene)
{
    scenesList.insert(position, scene);
}

bool TupProject::restoreScene(int position)
{
    if (undoScenes.count() > 0) {
        TupScene *scene = undoScenes.takeLast();
        if (scene) {
            scenesList.insert(position, scene);
            sceneCounter++;
            return true;
        }
        return false;
    }

    return false;
}

bool TupProject::removeScene(int pos)
{
    #ifdef TUP_DEBUG
        qInfo() << "[TupProject::removeScene()] - scene index: " + QString::number(pos);
    #endif

    TupScene *toRemove = sceneAt(pos);
    if (toRemove) {
        QString path = getDataDir() + "/scene" + QString::number(pos) + ".tps";
        if (QFile::exists(path)) {
            if (!QFile::remove(path)) {
                #ifdef TUP_DEBUG
                    qCritical() << "TupProject::removeScene() - Error removing file " + path;
                #endif
            
                return false;
            }
        }

        int total = sceneCounter - 1;
        if (pos < total) {
            for (int i=pos + 1; i<=total; i++) {
                 QString oldName = getDataDir() + "/scene" + QString::number(i) + ".tps";
                 QString newName = getDataDir() + "/scene" + QString::number(i-1) + ".tps";
                 QFile::rename(oldName, newName); 
            }
        }

        undoScenes << scenesList.takeAt(pos);
        sceneCounter--;

        return true;
    } 

    return false;
}

bool TupProject::resetScene(int pos, const QString &newName)
{
    #ifdef TUP_DEBUG
        #ifdef Q_OS_WIN
            qDebug() << "[TupProject::resetScene()]";
        #else
            T_FUNCINFO;
        #endif
    #endif
   
    TupScene *scene = sceneAt(pos);
    if (scene) {
        undoScenes << scenesList.takeAt(pos);

        TupScene *basic = new TupScene(this, dimension, "#ffffff");
        basic->setSceneName(newName);
        basic->setBasicStructure();
        scenesList.insert(pos, basic);

        return true;
    } else {
        #ifdef TUP_DEBUG
            QString msg = "TupProject::resetScene() - No scene at index -> " + QString::number(pos);
            #ifdef Q_OS_WIN
                qDebug() << msg;
            #else
                tError() << msg;
            #endif
        #endif
    }

    return false;
}

QString TupProject::recoverScene(int pos)
{
    TupScene *scene = undoScenes.takeLast();
    if (scene) {
        scenesList[pos] = scene;
        return scene->getSceneName();
    }

    return "";
}

bool TupProject::moveScene(int position, int newPosition)
{
    if (position < 0 || newPosition < 0) {
        #ifdef TUP_DEBUG
            QString msg = "TupProject::moveScene() - Failed moving scene from " + QString::number(position) + " to " + QString::number(newPosition);
            #ifdef Q_OS_WIN
                qDebug() << msg;
            #else
                tError() << msg;
            #endif
        #endif
        return false;
    }

    TupScene *scene = scenesList.takeAt(position);
    scenesList.insert(newPosition, scene);

    return true;
}

TupScene *TupProject::sceneAt(int position) const
{    
    #ifdef TUP_DEBUG
        #ifdef Q_OS_WIN
            qDebug() << "[TupProject::sceneAt()] - position: " << position;
        #else
            T_FUNCINFOX("project") << position;
        #endif
    #endif    

    if (position < 0) {
        #ifdef TUP_DEBUG
            QString msg = "TupProject::sceneAt() - FATAL ERROR: index out of bound (" + QString::number(position) + ")";
            #ifdef Q_OS_WIN
                qDebug() << msg;
            #else
                tError() << msg;
            #endif
        #endif
        return nullptr;
    }

    return scenesList.value(position);
}

int TupProject::visualIndexOf(TupScene *scene) const
{
    return scenesList.indexOf(scene);
}

void TupProject::fromXml(const QString &xml)
{
    QDomDocument document;

    if (!document.setContent(xml))
        return;

    QDomElement root = document.documentElement();
    QDomNode n = root.firstChild();

    int i = 0;
    while (!n.isNull()) {
           QDomElement e = n.toElement();

           if (!e.isNull()) {
               if (e.tagName() == "project") {
                   setProjectName(e.attribute("name", getName()));
                   QDomNode n1 = e.firstChild();
                   e = n1.toElement();

                   if (e.tagName() == "meta") {
                       QDomNode n1 = e.firstChild();

                       while (!n1.isNull()) {
                              QDomElement e1 = n1.toElement();

                              if (e1.tagName() == "author") {
                                  if (e1.firstChild().isText()) 
                                      setAuthor(e1.text());
                              } else if (e1.tagName() == "tags") {
                                         if (e1.text().isEmpty())
                                             setTags(tr("#animation #2D"));
                                         else
                                             setTags(e1.text());
                              } else if (e1.tagName() == "bgcolor") {
                                         if (e1.text().isEmpty())
                                             setBgColor(QColor("#ffffff"));
                                         else
                                             setBgColor(QColor(e1.text()));

                              } else if (e1.tagName() == "description") {
                                         if (e1.firstChild().isText())
                                             setDescription(e1.text());

                                } else if (e1.tagName() == "dimension") {
                                           if (e1.firstChild().isText()) {
                                               QStringList list = e1.text().split(",");
                                               int x = list.at(0).toInt();
                                               int y = list.at(1).toInt();
                                               QSize size(x,y);
                                               setDimension(size);
                                           }

                                } else if (e1.tagName() == "fps") {
                                           if (e1.firstChild().isText())
                                               setFPS(e1.text().toInt());
                                }

                                n1 = n1.nextSibling();
                          }
                   }
               }
           } 
           n = n.nextSibling();
           i++;
    }
}

QDomElement TupProject::toXml(QDomDocument &doc) const
{
    QDomElement tupi = doc.createElement("Tupi");
    tupi.setAttribute("version", "1");

    QDomElement project = doc.createElement("project");
    project.setAttribute("name", projectName);

    QDomElement meta = doc.createElement("meta");

    QDomElement author = doc.createElement("author");
    author.appendChild(doc.createTextNode(projectAuthor));

    QDomElement tags = doc.createElement("tags");
    tags.appendChild(doc.createTextNode(projectTags));

    QDomElement description = doc.createElement("description");
    description.appendChild(doc.createTextNode(projectDesc));

    QDomElement color = doc.createElement("bgcolor");
    color.appendChild(doc.createTextNode(bgColor.name()));

    QDomElement size = doc.createElement("dimension");
    QString xy = QString::number(dimension.width()) + "," + QString::number(dimension.height());
    size.appendChild(doc.createTextNode(xy));

    QDomElement fpsElement = doc.createElement("fps");
    QString frames = QString::number(fps);
    fpsElement.appendChild(doc.createTextNode(frames));

    meta.appendChild(author);
    meta.appendChild(tags);
    meta.appendChild(color);
    meta.appendChild(description);
    meta.appendChild(size);
    meta.appendChild(fpsElement);

    project.appendChild(meta);
    tupi.appendChild(project);
    
    return tupi;
}

Scenes TupProject::getScenes() const
{
    return scenesList;
}

bool TupProject::createSymbol(int type, const QString &name, const QByteArray &data, const QString &folder)
{
    #ifdef TUP_DEBUG
        #ifdef Q_OS_WIN
            qDebug() << "[TupProject::createSymbol()]";
        #else
            T_FUNCINFOX("symbol");
        #endif
    #endif
   
    if (!isOpen) {
        #ifdef TUP_DEBUG
            QString msg = "TupProject::createSymbol() - Fatal error: project is NOT open!";
            #ifdef Q_OS_WIN
                qDebug() << msg;
            #else
                tError() << msg;
            #endif
        #endif
        
        return false;
    }

    if (library->createSymbol(TupLibraryObject::Type(type), name, data, folder) == nullptr) {
        #ifdef TUP_DEBUG
            QString msg = "TupProject::createSymbol() - Fatal error: object can't be created. Data is NULL!";
            #ifdef Q_OS_WIN
                qDebug() << msg;
            #else
                tError() << msg;
            #endif
        #endif    

        return false;
    }         

    #ifdef TUP_DEBUG
        QString msg = "TupProject::createSymbol() - Object added successfully -> " + name;
        #ifdef Q_OS_WIN
            qWarning() << msg;
        #else
            tWarning() << msg;
        #endif
    #endif    

    return true;
}

bool TupProject::removeSymbol(const QString &name, TupLibraryObject::Type type)
{
    #ifdef TUP_DEBUG
        #ifdef Q_OS_WIN
            qDebug() << "[TupProject::removeSymbol()]";
        #else
            T_FUNCINFO;
        #endif
    #endif    
    
    int totalScenes = scenesList.size();
    for (int i = 0; i < totalScenes; i++) {
         TupScene *scene = scenesList.at(i);

         TupBackground *bg = scene->sceneBackground();
         if (bg) {
             TupFrame *frame = bg->staticFrame();
             if (frame) {
                 if (type != TupLibraryObject::Svg)
                     frame->removeImageItemFromFrame(name);
                 else
                     frame->removeSvgItemFromFrame(name);
             } 

             frame = bg->dynamicFrame();
             if (frame) {
                 bool found = false;

                 if (type != TupLibraryObject::Svg)
                     found = frame->removeImageItemFromFrame(name);
                 else
                     found = frame->removeSvgItemFromFrame(name);

                 if (found)
                     bg->scheduleRender(true);
             }
         }

         int totalLayers = scene->getLayers().size();
         for (int j = 0; j < totalLayers; j++) {
              TupLayer *layer = scene->getLayers().at(j);
              int totalFrames = layer->getFrames().size();
              for (int t = 0; t < totalFrames; t++) {
                   TupFrame *frame = layer->getFrames().at(t);
                   if (type != TupLibraryObject::Svg)
                       frame->removeImageItemFromFrame(name);
                   else
                       frame->removeSvgItemFromFrame(name);
              }
         }
    }

    return library->removeObject(name, true);
}

bool TupProject::addFolder(const QString &name)
{
    #ifdef TUP_DEBUG
        #ifdef Q_OS_WIN
            qDebug() << "[TupProject::addFolder()]";
        #else
            T_FUNCINFO;
        #endif
    #endif

    TupLibraryFolder *folder = new TupLibraryFolder(name, this);

    return library->addFolder(folder);
}

bool TupProject::removeFolder(const QString &name)
{
    #ifdef TUP_DEBUG
        #ifdef Q_OS_WIN
            qDebug() << "[TupProject::removeFolder()]";
        #else
            T_FUNCINFO;
        #endif
    #endif

    return library->removeFolder(name);
}

bool TupProject::removeSound(const QString &name)
{
    return library->removeObject(name, true);
}

bool TupProject::insertSymbolIntoFrame(TupProject::Mode spaceMode, const QString &name, int sceneIndex, 
                                       int layerIndex, int frameIndex)
{    
    #ifdef TUP_DEBUG
        #ifdef Q_OS_WIN
            qDebug() << "[TupProject::insertSymbolIntoFrame()]";
        #else
            T_FUNCINFO;
        #endif
    #endif        
    
    TupFrame *frame = nullptr;
    TupScene *scene = this->sceneAt(sceneIndex);

    if (scene) {
        if (spaceMode == TupProject::FRAMES_EDITION) {
            TupLayer *layer = scene->layerAt(layerIndex);

            if (layer)
                frame = layer->frameAt(frameIndex);
            else
                return false;
        } else if (spaceMode == TupProject::STATIC_BACKGROUND_EDITION) { 
            TupBackground *bg = scene->sceneBackground();

            if (bg)
                frame = bg->staticFrame();
            else
                return false;
        } else if (spaceMode == TupProject::DYNAMIC_BACKGROUND_EDITION) {
            TupBackground *bg = scene->sceneBackground();

            if (bg) {
                frame = bg->dynamicFrame();
                bg->scheduleRender(true);
            } else {
                return false;
            }
        } else {
            #ifdef TUP_DEBUG
                QString msg = "TupProject::insertSymbolIntoFrame() - Fatal Error: invalid spaceMode!";
                #ifdef Q_OS_WIN
                    qDebug() << msg;
                #else
                    tError() << msg;
                #endif
            #endif        
            return false;
        }

        if (frame) {
            TupLibraryObject *object = library->getObject(name);
            if (object) {
                switch (object->getType()) {
                        case TupLibraryObject::Item:
                        {
                             TupGraphicLibraryItem *libraryItem = new TupGraphicLibraryItem(object);
                             int zLevel = frame->getTopZLevel();
                             libraryItem->setZValue(zLevel);
                             frame->addLibraryItem(name, libraryItem);
                        }
                        break;
                        case TupLibraryObject::Image:
                        {
                             TupGraphicLibraryItem *libraryItem = new TupGraphicLibraryItem(object);
                             int imageW = static_cast<int>(libraryItem->boundingRect().width());
                             int imageH = static_cast<int> (libraryItem->boundingRect().height());

                             qreal xPos = 0;
                             qreal yPos = 0;
                             if (dimension.width() > imageW)
                                 xPos = (dimension.width() - imageW) / 2;
                             if (dimension.height() > imageH)
                                 yPos = (dimension.height() - imageH) / 2;

                             libraryItem->moveBy(xPos, yPos);

                             int zLevel = frame->getTopZLevel();
                             libraryItem->setZValue(zLevel);
                             frame->addItem(name, libraryItem);
                        }
                        break;
                        case TupLibraryObject::Svg:
                        {
                             QString path(object->getDataPath());
                             TupSvgItem *svgItem = new TupSvgItem(path, frame);
                             svgItem->setSymbolName(name);

                             int svgW = static_cast<int> (svgItem->boundingRect().width());
                             int svgH = static_cast<int> (svgItem->boundingRect().height());
                             if (dimension.width() < svgW || dimension.height() < svgH) {
                                 qreal factorW = static_cast<qreal>(dimension.width()) / static_cast<qreal>(svgW);
                                 qreal factorH = static_cast<qreal>(dimension.height()) / static_cast<qreal>(svgH);
                                 if (svgW < svgH) {
                                     svgItem->setScale(factorW);
                                     svgW = static_cast<int> (svgW * factorW);
                                     svgH = static_cast<int> (svgH * factorW);
                                 } else {
                                     svgItem->setScale(factorH);
                                     svgW = static_cast<int> (svgW * factorH);
                                     svgH = static_cast<int> (svgH * factorH);
                                 }
                             }

                             qreal xPos = 0;
                             qreal yPos = 0;
                             if (dimension.width() > svgW)
                                 xPos = (dimension.width() - svgW) / 2;
                             if (dimension.height() > svgH)
                                 yPos = (dimension.height() - svgH) / 2;

                             svgItem->moveBy(xPos, yPos);

                             int zLevel = frame->getTopZLevel();
                             svgItem->setZValue(zLevel);
                             frame->addSvgItem(name, svgItem);
                        }
                        break;
                        case TupLibraryObject::Text:
                        {
                             // SQA: Just out of curiosity, check if this case really happens!
                             // tFatal() << "TupProject::insertSymbolIntoFrame() - Just tracing text!";
                             TupGraphicLibraryItem *libraryItem = new TupGraphicLibraryItem(object);

                             int zLevel = frame->getTopZLevel();
                             libraryItem->setZValue(zLevel);
                             frame->addItem(name, libraryItem);
                        }
                        break;
                        case TupLibraryObject::Sound:
                        {
                             TupSoundLayer *sound = scene->createSoundLayer(scene->getSoundLayers().count());
                             sound->fromSymbol(object->getSymbolName());
                        }
                        break;
                        default:
                             #ifdef TUP_DEBUG
                                 QString msg = "TupProject::insertSymbolIntoFrame() -> Unknown Object Type";
                                 #ifdef Q_OS_WIN
                                     qDebug() << msg;
                                 #else
                                     tFatal() << msg;
                                 #endif
                             #endif                         
                        break;
                }

                return true;
            } else {                
                #ifdef TUP_DEBUG
                    QString msg = "TupProject::insertSymbolIntoFrame() - Object NOT found at library! " + name;
                    #ifdef Q_OS_WIN
                        qDebug() << msg;
                    #else
                        tError() << msg;
                    #endif
                #endif
                
                return false;
            }
        } else {
                #ifdef TUP_DEBUG
                    QString msg = "TupProject::insertSymbolIntoFrame() - Invalid frame!";
                    #ifdef Q_OS_WIN
                        qDebug() << msg;
                    #else
                        tError() << msg;
                    #endif
                #endif
        }
    } else {        
        #ifdef TUP_DEBUG
            QString msg = "TupProject::insertSymbolIntoFrame() - Invalid scene!";
            #ifdef Q_OS_WIN
                qDebug() << msg;
            #else
                tError() << msg;
            #endif
        #endif        
    }

    return false;
}

bool TupProject::removeSymbolFromFrame(const QString &name, TupLibraryObject::Type type)
{
    #ifdef TUP_DEBUG
        QString msg = "TupProject::removeSymbolFromFrame() - Removing symbol " + name + " from project...";
        #ifdef Q_OS_WIN
            qDebug() << msg;
        #else
            T_FUNCINFO << msg;
        #endif
    #endif
    
    if (type == TupLibraryObject::Folder)
        return true;

    int totalScenes = scenesList.size();
    for (int i = 0; i < totalScenes; i++) {
         TupScene *scene =  scenesList.at(i);
         int totalLayers = scene->getLayers().size();
         for (int j = 0; j < totalLayers; j++) {
              TupLayer *layer = scene->getLayers().at(j);
              int totalFrames = layer->getFrames().size(); 
              for (int t = 0; t < totalFrames; t++) {
                   TupFrame *frame = layer->getFrames().at(t);
                   if (type != TupLibraryObject::Svg)
                       frame->removeImageItemFromFrame(name);
                   else
                       frame->removeSvgItemFromFrame(name);
              }
         }

         TupBackground *bg = scene->sceneBackground();
         if (bg) {
             TupFrame *frame = bg->staticFrame();
             if (frame) {
                 if (type == TupLibraryObject::Svg)
                     frame->removeSvgItemFromFrame(name);
                 else
                     frame->removeImageItemFromFrame(name);
             }

             frame = bg->dynamicFrame();
             if (frame) {
                 bool found = false;

                 if (type == TupLibraryObject::Svg)
                     found = frame->removeSvgItemFromFrame(name);
                 else
                     found = frame->removeImageItemFromFrame(name);

                 if (found)
                     bg->scheduleRender(true);
             }
         }
    }

    // library->removeObject(name, true);

    return true;
}

bool TupProject::updateSymbolId(TupLibraryObject::Type type, const QString &oldId, const QString &newId)
{
    int scenesCount = scenesList.size();
    for (int i = 0; i < scenesCount; i++) {
         TupScene *scene = scenesList.at(i);
         int layersCount = scene->getLayers().size();
         for (int j = 0; j < layersCount; j++) {
              TupLayer *layer = scene->getLayers().at(j);
              int framesCount = layer->getFrames().size();  
              for (int t = 0; t < framesCount; t++) {
                   TupFrame *frame = layer->getFrames().at(t); 
                   if (type != TupLibraryObject::Svg)
                       frame->updateIdFromFrame(oldId, newId);
                   else 
                       frame->updateSvgIdFromFrame(oldId, newId);
              }
         }

         TupBackground *bg = scene->sceneBackground();
         if (bg) {
             TupFrame *frame = bg->staticFrame();
             if (frame) {
                 if (type != TupLibraryObject::Svg)
                     frame->updateIdFromFrame(oldId, newId);
                 else
                     frame->updateSvgIdFromFrame(oldId, newId);
             }

             frame = bg->dynamicFrame();
             if (frame) {
                 if (type != TupLibraryObject::Svg)
                     frame->updateIdFromFrame(oldId, newId);
                 else
                     frame->updateSvgIdFromFrame(oldId, newId);
             }
         }
    }

    return true;
}

void TupProject::reloadLibraryItem(TupLibraryObject::Type type, const QString &id, TupLibraryObject *object)
{
    int scenesCount = scenesList.size();
    for (int i = 0; i < scenesCount; i++) {
         TupScene *scene = scenesList.at(i);
         int layersCount = scene->getLayers().size();
         for (int j = 0; j < layersCount; j++) {
              TupLayer *layer = scene->getLayers().at(j);
              int framesCount = layer->getFrames().size();
              for (int t = 0; t < framesCount; t++) {
                   TupFrame *frame = layer->getFrames().at(t);
                   if (type == TupLibraryObject::Svg)
                       frame->reloadSVGItem(id, object);
                   else
                       frame->reloadGraphicItem(id, object->getDataPath());
              }
         }

         TupBackground *bg = scene->sceneBackground();
         if (bg) {
             TupFrame *frame = bg->staticFrame();
             if (frame) {
                 if (type == TupLibraryObject::Svg)
                     frame->reloadSVGItem(id, object);
                 else
                     frame->reloadGraphicItem(id, object->getDataPath());
             }

             frame = bg->dynamicFrame();
             if (frame) {
                 if (type == TupLibraryObject::Svg)
                     frame->reloadSVGItem(id, object);
                 else
                     frame->reloadGraphicItem(id, object->getDataPath());
             }
         }
    }
}

TupLibrary *TupProject::getLibrary()
{
    return library;
}

void TupProject::emitResponse(TupProjectResponse *response)
{
    emit responsed(response);
}

void TupProject::setOpen(bool open)
{
    isOpen = open;
}

bool TupProject::isProjectOpen()
{
    return isOpen;
}

/*
bool TupProject::deleteDataDir(const QString &path)
{
    #ifdef TUP_DEBUG
        #ifdef Q_OS_WIN
            qDebug() << "[TupProject::deleteDataDir()]";
        #else
            T_FUNCINFO;
            tWarning() << "Removing project path: " << path;
        #endif
    #endif

    QDir dir(path);
    if (dir.exists()) {
        QStringList contentList = dir.entryList();
        foreach (QString item, contentList) {
            QString absolute = dir.absolutePath() + "/" + item;
            QFileInfo itemInfo(absolute);
            if (itemInfo.isFile()) {
                tError() << "FILE: " << absolute;
                // dir.remove(absolute); 
            } else {
                if (!absolute.endsWith(".")) {
                    tError() << "DIR: " << absolute;
                    // deleteDataDir(absolute);
                }
            }
        } 
        // dir.rmdir(path);
        tError() << "path: " << path;
        return true;
    } else {
        #ifdef TUP_DEBUG
            QString msg = "TupProject::deleteDataDir() - Warning: directory doesn't exist -> " + path;
            #ifdef Q_OS_WIN
                qWarning() << msg;
            #else
                tWarning() << msg;
            #endif
        #endif
    }

    return false;
}
*/

int TupProject::scenesCount() const
{
    return scenesList.count();
}

void TupProject::updateSpaceContext(TupProject::Mode mode)
{
    spaceMode = mode;
}

TupProject::Mode TupProject::spaceContext()
{
    return spaceMode;
}

void TupProject::importLayer(int sceneIndex, const QString &layer)
{
    TupScene *scene = scenesList.at(sceneIndex);
    scene->addLayer(layer);
}

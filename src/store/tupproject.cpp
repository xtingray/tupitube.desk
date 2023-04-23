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
       qDebug() << "[TupProject()]";
    #endif    
    
    spaceMode = TupProject::NONE;
    currentBgColor = QColor(255, 255, 255);
    dimension = QSize(1920, 1080);
    fps = 24;
    sceneCounter = 0;
    isOpen = false;
    library = new TupLibrary("library", this);
    cachePath = "";
}

TupProject::~TupProject()
{
    #ifdef TUP_DEBUG
        qDebug() << "[~TupProject()]";
    #endif    
        
    scenesList.clear();
}

void TupProject::loadLibrary(const QString &filename)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupProject::loadLibrary()]";
    #endif

    QFile file(filename);

    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        library->fromXml(QString::fromLocal8Bit(file.readAll()));
        file.close();
    } else {
        #ifdef TUP_DEBUG
            qDebug() << "[TupProject::loadLibrary()] - Cannot open library from -> " << filename;
        #endif
    }
}

void TupProject::registerLibrarySounds()
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupProject::registerLibrarySounds()]";
    #endif

    QList<TupLibraryObject *> soundList = library->getLibrarySoundItems();
    foreach(TupLibraryObject *sound, soundList)
        addSoundResource(sound);
}

void TupProject::clear()
{
    for (int i=0; i<scenesList.count(); i++) {
         TupScene *scene = scenesList.takeAt(i);
         scene->clear();

         scene = nullptr;
         delete scene;
    }

    scenesList.clear();
    sceneCounter = 0;
}

void TupProject::setProjectName(const QString &name)
{
    projectName = name;
    kAppProp->setProjectDir(projectName);
}

void TupProject::setTempProjectName(const QString &tempFolder, const QString &name)
{
    projectName = name;
    kAppProp->setTempProjectDir(tempFolder, projectName);
}

void TupProject::setAuthor(const QString &author)
{
    projectAuthor = author;
}

void TupProject::setCurrentBgColor(const QColor &bgColor)
{
    currentBgColor = bgColor;
}

void TupProject::setSceneBgColor(int sceneIndex, const QColor &bgColor)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupProject::setSceneBgColor()] - sceneIndex -> " << sceneIndex;
        qDebug() << "[TupProject::setSceneBgColor()] - bgColor.name() -> " << bgColor.name();
    #endif

    setCurrentBgColor(bgColor);

    TupScene *scene = scenesList.at(sceneIndex);
    scene->setBgColor(bgColor);
}

void TupProject::setDescription(const QString &description)
{
    projectDesc = description;
}

void TupProject::setDimension(const QSize size)
{
    dimension = size;
}

void TupProject::setFPS(const int value, const int sceneIndex)
{
    if (sceneIndex == 0)
        fps = value;

    if (sceneIndex < scenesList.count()) {
        TupScene *scene = scenesList.at(sceneIndex);
        if (scene)
            scene->setFPS(value);
    }
}

void TupProject::setVersion(const QString &value)
{
    version = value.toFloat();
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

QString TupProject::getDescription() const
{
    return projectDesc;
}

QSize TupProject::getDimension() const
{
    return dimension;
}

int TupProject::getFPS(const int sceneIndex) const
{
    if (sceneIndex == 0)
        return fps;

    TupScene *scene = scenesList.at(sceneIndex);
    int value = fps;
    if (scene)
        value = scene->getFPS();

    return value;
}

float TupProject::getVersion()
{
    return version;
}

QColor TupProject::getCurrentBgColor() const
{
    return currentBgColor;
}

QColor TupProject::getSceneBgColor(int sceneIndex) const
{
    return scenesList.at(sceneIndex)->getBgColor();
}

QString TupProject::getDataDir() const
{
    return cachePath;
}

TupScene *TupProject::createScene(QString name, int position, bool loaded)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupProject::createScene()]";
    #endif

    if (position < 0 || position > scenesList.count())
        return nullptr;

    TupScene *scene = new TupScene(this, position, dimension, currentBgColor);
    scene->setFPS(fps);
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
        qInfo() << "[TupProject::removeScene()] - scene index -> " << pos;
    #endif

    TupScene *toRemove = sceneAt(pos);
    if (toRemove) {
        QString path = getDataDir() + "/scene" + QString::number(pos) + ".tps";
        if (QFile::exists(path)) {
            if (!QFile::remove(path)) {
                #ifdef TUP_DEBUG
                    qCritical() << "[TupProject::removeScene()] - Error removing file -> " << path;
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
        qDebug() << "[TupProject::resetScene()]";
    #endif
   
    TupScene *scene = sceneAt(pos);
    if (scene) {
        undoScenes << scenesList.takeAt(pos);

        TupScene *basic = new TupScene(this, pos, dimension, Qt::white);
        basic->setSceneName(newName);
        basic->setBasicStructure();
        scenesList.insert(pos, basic);

        return true;
    } else {
        #ifdef TUP_DEBUG
            qDebug() << "[TupProject::resetScene()] - No scene at index -> " << pos;
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
            qDebug() << "[TupProject::moveScene()] - Failed moving scene from " << position
                     << " to " << newPosition;
        #endif

        return false;
    }

    TupScene *scene = scenesList.takeAt(position);
    scenesList.insert(newPosition, scene);

    return true;
}

TupScene *TupProject::sceneAt(int sceneIndex) const
{    
    #ifdef TUP_DEBUG
        qDebug() << "[TupProject::sceneAt()] - position -> " << sceneIndex;
    #endif    

    if (sceneIndex < 0) {
        #ifdef TUP_DEBUG
            qDebug() << "[TupProject::sceneAt()] - FATAL ERROR: index out of bound -> " << sceneIndex;
        #endif

        return nullptr;
    }

    return scenesList.value(sceneIndex);
}

TupBackground * TupProject::getBackgroundFromScene(int sceneIndex)
{
    if (sceneIndex < 0) {
        #ifdef TUP_DEBUG
            qDebug() << "[TupProject::getBackgroundFromScene()] - FATAL ERROR: index out of bound -> " << sceneIndex;
        #endif

        return nullptr;
    }

    return scenesList.value(sceneIndex)->sceneBackground();
}

int TupProject::visualIndexOf(TupScene *scene) const
{
    return scenesList.indexOf(scene);
}

void TupProject::fromXml(const QString &xml)
{
    QDomDocument document;

    if (!document.setContent(xml)) {
        #ifdef TUP_DEBUG
            qWarning() << "[TupProject::fromXml()] - Fatal Error: Xml input is corrupt! -> " << xml;
        #endif

        return;
    }

    QDomElement root = document.documentElement();
    setVersion(root.attribute("version"));

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
                        } else if (e1.tagName() == "bgcolor") { // Legacy parameter
                            if (!e1.text().isEmpty())
                                currentBgColor = QColor(e1.text());
                        } else if (e1.tagName() == "description") {
                            if (e1.firstChild().isText())
                                setDescription(e1.text());
                        } else if (e1.tagName() == "dimension") {
                            if (e1.firstChild().isText()) {
                                QStringList list = e1.text().split(",");
                                int x = list.at(0).toInt();
                                int y = list.at(1).toInt();
                                if (x % 2)
                                    x++;
                                if (y % 2)
                                    y++;
                                QSize size(x, y);
                                setDimension(size);
                            }
                        } else if (e1.tagName() == "fps") {
                            if (e1.firstChild().isText())
                                fps = e1.text().toInt();
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
    tupi.setAttribute("version", "1.1");

    QDomElement project = doc.createElement("project");
    project.setAttribute("name", projectName);

    QDomElement meta = doc.createElement("meta");

    QDomElement author = doc.createElement("author");
    author.appendChild(doc.createTextNode(projectAuthor));

    QDomElement description = doc.createElement("description");
    description.appendChild(doc.createTextNode(projectDesc));

    QDomElement size = doc.createElement("dimension");
    int w = dimension.width();
    if (w % 2)
        w++;
    int h = dimension.height();
    if (h % 2)
        h++;
    QString xy = QString::number(w) + "," + QString::number(h);
    size.appendChild(doc.createTextNode(xy));

    QDomElement fpsElement = doc.createElement("fps");
    QString frames = QString::number(fps);
    fpsElement.appendChild(doc.createTextNode(frames));

    meta.appendChild(author);
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
        qDebug() << "[TupProject::createSymbol()] - name -> " << name;
        qDebug() << "[TupProject::createSymbol()] - folder -> " << folder;
    #endif

    if (!isOpen) {
        #ifdef TUP_DEBUG
            qWarning() << "[TupProject::createSymbol()] - Fatal Error: Project is NOT open!";
        #endif

        return false;
    }

    if (library->createSymbol(TupLibraryObject::ObjectType(type), name, data, folder) == nullptr) {
        #ifdef TUP_DEBUG
            qWarning() << "[TupProject::createSymbol()] - Fatal Error: Object can't be created. Data is NULL!";
        #endif    

        return false;
    }         

    #ifdef TUP_DEBUG
        qDebug() << "[TupProject::createSymbol()] - Object added successfully -> " << name;
    #endif

    return true;
}

bool TupProject::removeSymbol(const QString &name, TupLibraryObject::ObjectType type)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupProject::removeSymbol()]";
    #endif    
    
    int totalScenes = scenesList.size();
    for (int i = 0; i < totalScenes; i++) {
         TupScene *scene = scenesList.at(i);

         TupBackground *bg = scene->sceneBackground();
         if (bg) {
             TupFrame *frame = bg->vectorStaticFrame();
             if (frame) {
                 if (type != TupLibraryObject::Svg)
                     frame->removeImageItemFromFrame(name);
                 else
                     frame->removeSvgItemFromFrame(name);
             }

             frame = bg->vectorForegroundFrame();
             if (frame) {
                 if (type != TupLibraryObject::Svg)
                     frame->removeImageItemFromFrame(name);
                 else
                     frame->removeSvgItemFromFrame(name);
             }

             frame = bg->vectorDynamicFrame();
             if (frame) {
                 bool found = false;

                 if (type != TupLibraryObject::Svg)
                     found = frame->removeImageItemFromFrame(name);
                 else
                     found = frame->removeSvgItemFromFrame(name);

                 if (found)
                     bg->scheduleVectorRender(true);
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
        qDebug() << "[TupProject::addFolder()]";
    #endif

    TupLibraryFolder *folder = new TupLibraryFolder(name, this);

    return library->addFolder(folder);
}

bool TupProject::removeFolder(const QString &name)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupProject::removeFolder()]";
    #endif

    return library->removeFolder(name);
}

bool TupProject::removeSound(const QString &name)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupProject::removeSound()] - item -> " << name;
    #endif

    return library->removeObject(name, true);
}

bool TupProject::insertSymbolIntoFrame(TupProject::Mode spaceMode, const QString &name, int sceneIndex, 
                                       int layerIndex, int frameIndex)
{    
    #ifdef TUP_DEBUG
        qDebug() << "[TupProject::insertSymbolIntoFrame()] - spaceMode -> " << spaceMode;
    #endif
    
    TupFrame *frame = nullptr;
    TupScene *scene = this->sceneAt(sceneIndex);

    if (scene) {
        if (spaceMode == TupProject::FRAMES_MODE) {
            TupLayer *layer = scene->layerAt(layerIndex);

            if (layer)
                frame = layer->frameAt(frameIndex);
            else
                return false;

        } else if (spaceMode == TupProject::VECTOR_STATIC_BG_MODE) {
            TupBackground *bg = scene->sceneBackground();

            if (bg)
                frame = bg->vectorStaticFrame();
            else
                return false;

        } else if (spaceMode == TupProject::VECTOR_DYNAMIC_BG_MODE) {
            TupBackground *bg = scene->sceneBackground();

            if (bg) {
                frame = bg->vectorDynamicFrame();
                bg->scheduleVectorRender(true);
            } else {
                return false;
            }

        } else if (spaceMode == TupProject::VECTOR_FG_MODE) {
            TupBackground *bg = scene->sceneBackground();

            if (bg)
                frame = bg->vectorForegroundFrame();
            else
                return false;

        } else {
            #ifdef TUP_DEBUG
                qDebug() << "[TupProject::insertSymbolIntoFrame()] - Fatal Error: invalid spaceMode!";
            #endif

            return false;
        }

        if (frame) {
            TupLibraryObject *object = library->getObject(name);
            if (object) {
                switch (object->getObjectType()) {
                    case TupLibraryObject::Item:
                    {
                         TupGraphicLibraryItem *libraryItem = new TupGraphicLibraryItem(object->clone());
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
                    /*
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
                    */
                    case TupLibraryObject::Audio:
                    {
                         TupSoundLayer *sound = scene->createSoundLayer(scene->getSoundLayers().count());
                         sound->fromSymbol(object->getSymbolName());
                    }
                    break;
                    default:
                         #ifdef TUP_DEBUG
                             qDebug() << "[TupProject::insertSymbolIntoFrame()] -> Unknown Object Type";
                         #endif
                    break;
                }

                return true;
            } else {                
                #ifdef TUP_DEBUG
                    qDebug() << "[TupProject::insertSymbolIntoFrame()] - Object NOT found at library! -> " << name;
                #endif
                
                return false;
            }
        } else {
                #ifdef TUP_DEBUG
                    qDebug() << "[TupProject::insertSymbolIntoFrame()] - Invalid frame!";
                #endif
        }
    } else {        
        #ifdef TUP_DEBUG
            qDebug() << "[TupProject::insertSymbolIntoFrame()] - Invalid scene!";
        #endif
    }

    return false;
}

bool TupProject::removeSymbolFromFrame(const QString &name, TupLibraryObject::ObjectType type)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupProject::removeSymbolFromFrame()] - Removing symbol from project -> " << name;
    #endif
    
    if (type == TupLibraryObject::Folder) {
        #ifdef TUP_DEBUG
            qWarning() << "[TupProject::removeSymbolFromFrame()] - Ignoring! Object is a folder -> " << name;
        #endif

        return true;
    }

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
             TupFrame *frame = bg->vectorStaticFrame();
             if (frame) {
                 if (type == TupLibraryObject::Svg)
                     frame->removeSvgItemFromFrame(name);
                 else
                     frame->removeImageItemFromFrame(name);
             }

             frame = bg->vectorForegroundFrame();
             if (frame) {
               if (type == TupLibraryObject::Svg)
                   frame->removeSvgItemFromFrame(name);
               else
                   frame->removeImageItemFromFrame(name);
             }

             frame = bg->vectorDynamicFrame();
             if (frame) {
                 bool found = false;

                 if (type == TupLibraryObject::Svg)
                     found = frame->removeSvgItemFromFrame(name);
                 else
                     found = frame->removeImageItemFromFrame(name);

                 if (found)
                     bg->scheduleVectorRender(true);
             }
         }
    }

    return true;
}

bool TupProject::updateSymbolId(TupLibraryObject::ObjectType type, const QString &oldId, const QString &newId)
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
             TupFrame *frame = bg->vectorStaticFrame();
             if (frame) {
                 if (type != TupLibraryObject::Svg)
                     frame->updateIdFromFrame(oldId, newId);
                 else
                     frame->updateSvgIdFromFrame(oldId, newId);
             }

             frame = bg->vectorForegroundFrame();
             if (frame) {
               if (type != TupLibraryObject::Svg)
                   frame->updateIdFromFrame(oldId, newId);
               else
                   frame->updateSvgIdFromFrame(oldId, newId);
             }

             frame = bg->vectorDynamicFrame();
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

void TupProject::reloadLibraryItem(TupLibraryObject::ObjectType type, const QString &id, TupLibraryObject *object)
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
             TupFrame *frame = bg->vectorStaticFrame();
             if (frame) {
                 if (type == TupLibraryObject::Svg)
                     frame->reloadSVGItem(id, object);
                 else
                     frame->reloadGraphicItem(id, object->getDataPath());
             }

             frame = bg->vectorForegroundFrame();
             if (frame) {
                 if (type == TupLibraryObject::Svg)
                     frame->reloadSVGItem(id, object);
                 else
                     frame->reloadGraphicItem(id, object->getDataPath());
             }

             frame = bg->vectorDynamicFrame();
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

void TupProject::setLibrary(TupLibrary *lib)
{
    library = lib;
}

bool TupProject::isLibraryEmpty()
{
    return library->isEmpty();
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

void TupProject::updateRasterBackground(TupProject::Mode spaceContext, int sceneIndex, const QString &imgPath)
{
    scenesList.at(sceneIndex)->updateRasterBackground(spaceContext, imgPath);
}

void TupProject::releaseLipSyncVoices(int sceneIndex, const QString &lipSyncName)
{
    TupScene *scene = scenesList.at(sceneIndex);
    if (scene) {
        TupLipSync *lipsync = scene->getLipSync(lipSyncName);
        if (lipsync) {
            QString soundKey = lipsync->getSoundFile();
            library->releaseLipSyncVoices(soundKey);
            if (!updateSoundType(soundKey, Effect)) {
                #ifdef TUP_DEBUG
                    qWarning() << "[TupProject::releaseLipSyncVoices()] - "
                                  "Fatal Error: Couldn't update sound resource type -> "
                               << soundKey;
                #endif
            }
        }
    }
}

// Sound Records API

void TupProject::addSoundResource(TupLibraryObject *object)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupProject::addSoundResource()] - Symbol name -> " << object->getSymbolName();
    #endif

    SoundResourceParams record;
    record.key = object->getSymbolName();
    record.frame = object->frameToPlay();
    record.path = object->getDataPath();
    record.muted = object->isMuted();
    record.type = object->getSoundType();

    soundRecords << record;
}

bool TupProject::removeSoundResource(const QString &id)
{
    #ifdef TUP_DEBUG
        qDebug() << "---";
        qDebug() << "[TupProject::removeSoundResource()] - id -> " << id;
        qDebug() << "[TupProject::removeSoundResource()] - soundRecords.size() -> " << soundRecords.size();
    #endif

    for (int i=0; i<soundRecords.size(); i++) {
        SoundResourceParams item = soundRecords.at(i);
        #ifdef TUP_DEBUG
            qDebug() << "[TupProject::removeSoundResource()] - record key -> " << item.key;
        #endif
        if (item.key.compare(id) == 0) {
            soundRecords.removeAt(i);
            #ifdef TUP_DEBUG
                qDebug() << "[TupProject::removeSoundResource()] - Success!";
                qDebug() << "---";
            #endif

            return true;
        }
    }
    #ifdef TUP_DEBUG
        qDebug() << "[TupProject::removeSoundResource()] - Failed!";
        qDebug() << "---";
    #endif

    return false;
}

bool TupProject::updateSoundResourcesItem(TupLibraryObject *item)
{
    int size = soundRecords.count();
    #ifdef TUP_DEBUG
        qDebug() << "[TupProject::updateSoundResourcesItem()] - item -> "
                 << item->getSymbolName();
        qDebug() << "[TupProject::updateSoundResourcesItem()] - item path -> "
                 << item->getDataPath();
        qDebug() << "[TupProject::updateSoundResourcesItem()] - soundRecords.count() -> "
                 << size;
    #endif

    for(int i=0; i<size; i++) {
        SoundResourceParams record = soundRecords.at(i);
        #ifdef TUP_DEBUG
            qDebug() << "[TupProject::updateSoundResourcesItem()] - record path -> "
                     << record.path;
        #endif
        if (item->getDataPath().compare(record.path) == 0) {
            record.key = item->getSymbolName();
            record.frame = item->frameToPlay();
            record.muted = item->isMuted();
            record.type = item->getSoundType();
            soundRecords.replace(i, record);

            #ifdef TUP_DEBUG
                qDebug() << "[TupProject::updateSoundResourcesItem()] - Record updated successfully!";
            #endif

            return true;
        }
    }

    return false;
}

void TupProject::updateLibraryPaths(const QString &newPath)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupProject::updateLibraryPaths()] - newPath -> " << newPath;
    #endif

    library->updatePaths(newPath);
    updateSoundPaths(newPath);
}

void TupProject::updateSoundPaths(const QString &newPath)
{
    #ifdef TUP_DEBUG
        qDebug() << "---";
        qDebug() << "[TupProject::updateSoundPaths()] - newPath -> " << newPath;
        qDebug() << "[TupProject::updateSoundPaths()] - old path -> " << kAppProp->projectDir();
        qDebug() << "[TupProject::updateSoundPaths()] - soundRecords.size() -> " << soundRecords.size();
        qDebug() << "---";
    #endif

    for (int i=0; i<soundRecords.size(); i++) {
        SoundResourceParams item = soundRecords.at(i);
        QString oldPath = item.path;
        int range = oldPath.length() - oldPath.indexOf("audio");
        QString path = newPath + "/" + oldPath.right(range);

        #ifdef TUP_DEBUG
            qDebug() << "[TupProject::updateSoundPaths()] - oldPath -> " << oldPath;
            qDebug() << "[TupProject::updateSoundPaths()] - path -> " << path;
        #endif

        item.path = path;
        soundRecords[i] = item;
    }
}

bool TupProject::updateSoundType(const QString audioId, SoundType type)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupProject::updateSoundType()] - audioId -> " << audioId;
    #endif

    for (int i=0; i<soundRecords.size(); i++) {
        SoundResourceParams item = soundRecords.at(i);
        if (item.key.compare(audioId) == 0) {
            item.type = type;
            soundRecords[i] = item;

            return true;
        }
    }

    return false;
}

bool TupProject::updateSoundFrame(const QString audioId, int frame)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupProject::updateSoundFrame()] - audioId -> " << audioId;
    #endif

    for (int i=0; i<soundRecords.size(); i++) {
        SoundResourceParams item = soundRecords.at(i);
        if (item.key.compare(audioId) == 0) {
            library->updateSoundFrameToPlay(audioId, frame);
            item.frame = frame + 1;
            soundRecords[i] = item;

            return true;
        }
    }

    return false;
}

QList<SoundResourceParams> TupProject::soundResourcesList()
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupProject::soundResourcesList()] - items size -> "
                 << soundRecords.size();
    #endif

    return soundRecords;
}

int TupProject::soundsListSize()
{
    return soundRecords.size();
}

void TupProject::resetSoundRecordsList()
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupProject::resetSoundRecordsList()]";
    #endif

    if (!soundRecords.isEmpty())
        soundRecords.clear();
}

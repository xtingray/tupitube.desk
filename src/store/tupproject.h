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

#ifndef TUPPROJECT_H
#define TUPPROJECT_H

#include "tglobal.h"
#include "tupabstractserializable.h"
#include "tapplicationproperties.h"
#include "tuplibraryobject.h"

#include <QObject>
#include <QSize>

class TupScene;
class TupLayer;
class TupFrame;
class TupBackground;
class TupProjectRequest;
class TupProjectResponse;
class QGraphicsItem;
class TupLibrary;

typedef QList<TupScene *> Scenes;

class TUPITUBE_EXPORT TupProject : public QObject, public TupAbstractSerializable
{
    Q_OBJECT

    public:
        enum Mode 
        {
            FRAMES_MODE = 0,
            VECTOR_STATIC_BG_MODE,
            VECTOR_DYNAMIC_BG_MODE,
            RASTER_STATIC_BG_MODE,
            RASTER_DYNAMIC_BG_MODE,
            VECTOR_FG_MODE,
            NONE
        };

        TupProject(QObject *parent = nullptr);
        ~TupProject();

        void setProjectName(const QString &name);
        void setAuthor(const QString &author);
        // void setTags(const QString &tags);
        void setBgColor(const QColor color);
        void setDescription(const QString &desc);
        void setDimension(const QSize size);
        void setFPS(const int value, const int sceneIndex=0);

        QString getName() const;
        QString getAuthor() const;
        // QString getTags() const;
        QColor getBgColor() const;
        QString getDescription() const;
        QSize getDimension() const;
        int getFPS(const int sceneIndex=0) const;

        void setDataDir(const QString &path);
        QString getDataDir() const;

        TupScene *sceneAt(int pos) const;

        int visualIndexOf(TupScene *scene) const;

        Scenes getScenes() const;

        TupScene *createScene(QString name, int pos, bool loaded = false);
        void updateScene(int pos, TupScene *scene);
        bool restoreScene(int pos);
        bool removeScene(int pos);
        bool resetScene(int pos, const QString &newName);
        QString recoverScene(int pos);
        bool moveScene(int pos, int newPos);
        TupBackground * getBackgroundFromScene(int sceneIndex);

        bool createSymbol(int type, const QString &name, const QByteArray &data, const QString &folder = QString());
        bool removeSymbol(const QString &name, TupLibraryObject::ObjectType type);
        bool addFolder(const QString &name);
        bool removeFolder(const QString &name);

        bool removeSound(const QString &name);

        bool insertSymbolIntoFrame(TupProject::Mode spaceMode, const QString &name, int scene, int layer, int frame);
        bool removeSymbolFromFrame(const QString &name, TupLibraryObject::ObjectType type);

        bool updateSymbolId(TupLibraryObject::ObjectType type, const QString &oldId, const QString &newId);
        void reloadLibraryItem(TupLibraryObject::ObjectType type, const QString &id, TupLibraryObject *object);

        void clear();
        void loadLibrary(const QString &filename);

        TupLibrary *getLibrary();
        void setLibrary(TupLibrary *lib);
        void resetSoundRecordsList();

        void emitResponse(TupProjectResponse *response);

        virtual void fromXml(const QString &xml);
        virtual QDomElement toXml(QDomDocument &doc) const;

        void setOpen(bool open);
        bool isProjectOpen();
        int scenesCount() const;

        void updateSpaceContext(TupProject::Mode mode);
        TupProject::Mode spaceContext();

        void importLayer(int sceneIndex, const QString &layer);
        void updateRasterBackground(TupProject::Mode spaceContext, int sceneIndex, const QString &imgPath);

        void releaseLipSyncVoices(int sceneIndex, const QString &lipSyncName);

        // Sound Records API
        void addSoundResource(TupLibraryObject *object);
        bool removeSoundResource(const QString &key);
        QList<SoundResource> soundResourcesList();
        int soundsListSize();
        bool updateSoundResourcesItem(TupLibraryObject *item);
        bool updateSoundType(const QString audioId, SoundType type);
        bool updateSoundFrame(const QString audioId, int frame);
        void updateLibraryPaths(const QString &newPath);
        void updateSoundPaths(const QString &newPath);

    signals:
        void responsed(TupProjectResponse *response);

    private:
        QString projectName;
        QString projectAuthor;
        // QString projectTags;
        QColor bgColor;
        QString projectDesc;
        QSize dimension;
        int fps;
        QString cachePath;

        Scenes scenesList;
        Scenes undoScenes;

        int sceneCounter;
        TupLibrary *library;
        bool isOpen;
        TupProject::Mode spaceMode;

        QList<SoundResource> soundRecords;
};
#endif

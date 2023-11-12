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

#ifndef TUPLIBRARYWIDGET_H
#define TUPLIBRARYWIDGET_H

#include "tglobal.h"
#include "tupmodulewidgetbase.h"
#include "tuplibrarydisplay.h"
#include "timagebutton.h"
#include "tupitemmanager.h"
#include "tupnewitemdialog.h"
#include "tuplibraryobject.h"
#include "tapplication.h"
#include "toptionaldialog.h"
#include "tconfig.h"
#include "tuplibrary.h"
#include "tupproject.h"
#include "tupsymboleditor.h"
#include "tuprequestbuilder.h"
#include "tosd.h"
#include "talgorithm.h"
#include "toptionaldialog.h"
#include "tupsearchdialog.h"

#include <QScreen>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QMap>
#include <QDir>
#include <QMouseEvent>
#include <QApplication>
#include <QGroupBox>
#include <QFileDialog>
#include <QGraphicsItem>
#include <QLabel>
#include <QMenu>
#include <QMessageBox>
#include <QDesktopWidget>
#include <QBuffer>
#include <QGraphicsSvgItem>
#include <QSvgRenderer>
#include <QSvgGenerator>
#include <QComboBox>
#include <QTreeWidgetItemIterator>
#include <QProcess>
#include <QFileSystemWatcher>
#include <QChar>
#include <QPainter>

#include <cstdlib>
#include <ctime>

typedef QMap<QString, TupLibraryObject *> LibraryObjects;
typedef QMap<QString, TupLibraryFolder *> Folders;

class TUPITUBE_EXPORT TupLibraryWidget : public TupModuleWidgetBase
{
    Q_OBJECT

    public:
        TupLibraryWidget(QWidget *parent = nullptr);
        ~TupLibraryWidget();

        void resetGUI();
        void setLibrary(TupLibrary *assets);
        void setNetworking(bool isNetworked);
        void stopSoundPlayer();
        void updateSpaceContext(TupProject::Mode mode);
        void initCurrentFrame();
        void removeSoundItem(const QString &soundKey);

    protected:
        virtual void sceneResponse(TupSceneResponse *response);
        virtual void layerResponse(TupLayerResponse *response);
        virtual void libraryResponse(TupLibraryResponse *response);
        virtual void frameResponse(TupFrameResponse *response);

    private slots:
        void previewItem(QTreeWidgetItem *item);
        void insertObjectInWorkspace();
        void removeCurrentItem();
        void cloneObject(QTreeWidgetItem *item);
        void exportObject(QTreeWidgetItem *item);
        void createRasterObject();
        void createVectorObject();
        void renameObject(QTreeWidgetItem *item);
        void importLibraryObject();
        void refreshItem(QTreeWidgetItem *item);
        void updateLibrary(QString node, QString target);
        void activeRefresh(QTreeWidgetItem *item);
        void openInkscapeToEdit(QTreeWidgetItem *item);
        void openGimpToEdit(QTreeWidgetItem *item);
        void openKritaToEdit(QTreeWidgetItem *item);
        void openMyPaintToEdit(QTreeWidgetItem *item);
        void updateItemFromSaveAction();
        void updateItemEditionState();
        void importNativeObjects();
        // void updateSoundTiming(int frame);
        void updateSoundMuteStatus(bool mute);
        void importAsset(const QString &name, TupSearchDialog::AssetType assetType, const QString &extension,
                         int extensionId, QByteArray &data);
        void callLipsyncEditor(QTreeWidgetItem* item);

    public slots:
        void addFolder(const QString &folderName = QString());
        void importImageGroup();
        void importImage(const QString &image, const QString &folder = QString());
        void importSvgGroup();
        void importSvg(const QString &svgPath, const QString &folder = QString());
        void importImageSequence();
        void importSvgSequence();
        void importSoundFile();
        void importVideoFile();
        void resetSoundPlayer();
        void importLocalDroppedAsset(const QString &path, TupLibraryObject::ObjectType type);
        void importLocalDroppedAsset(const QString &path, TupLibraryObject::ObjectType type,
                                     const QString &folder);
        void importExternalLibraryAsset(const QString &path, TupLibraryObject::ObjectType type,  const QString &folder);
        void importWebDroppedAsset(const QString &imageName, const QString &extension,
                                   TupLibraryObject::ObjectType type, QByteArray array);

    private slots:
        void openSearchDialog();
        void recoverMode();
        void importLocalSoundFile(const QString &filePath);
        void importSoundFileFromFolder(const QString &filePath, const QString &folder = QString());
        void importVideoFileFromFolder(const QString &filePath);
        void loadSequenceFromDirectory(ImportAction action, const QString &path, bool resizeFlag = false);

    public slots:
        void updateSoundPlayer();

    signals:
        void requestCurrentGraphic();
        void soundUpdated();
        void lipsyncModuleCalled(PapagayoAppMode mode, const QString &filePath);
        void soundRemoved(ModuleSource source, const QString &objectKey);
        void folderWithAudiosRemoved();
        void imagesImportationDone();
        void projectSizeHasChanged(const QSize dimension);
        void msgSent(const QString &msg);

    private:
        void callExternalEditor(QTreeWidgetItem *item, const QString &software);
        void executeSoftware(const QString &software, QString &path);
        void updateItem(const QString &name, const QString &extension, TupLibraryObject *object);
        bool itemNameEndsWithDigit(QString &name);
        int getItemNameIndex(QString &name) const;
        QString nameForClonedItem(QString &name, QString &extension, int index, QString &path) const;
        QString nameForClonedItem(QString &smallId, QString &extension, QString &path) const;
        QString verifyNameAvailability(QString &name, QString &extension, bool isCloningAction);
        void verifyFramesAvailability(int filesTotal);
        void setDefaultPath(const QString &path);
        void saveDefaultPath(const QString &dir);
        void importNativeObject(const QString &object, const QString &folder = QString());
        void refreshItemFromCollection(LibraryObjects collection);
        void importImageFromByteArray(const QString &filename, const QString &extension, QByteArray data,
                                      const QString &folder = QString());
        void importSvgFromByteArray(const QString &filename, QByteArray data, const QString &folder = QString());
        void importNativeObjectFromByteArray(const QString &filename, QByteArray data, const QString &folder = QString());
        void importSoundFileFromByteArray(const QString &filename, QByteArray data, const QString &folder = QString());
        void importVideoFileFromByteArray(const QString &filename, QByteArray data);
        bool fileIsImage(const QString &extension);
        bool importImageRecord(const QString &photogram, const QString &extension, const QSize imageSize, const QSize projectSize,
                               bool resize, const QString &directory);
        QScreen *screen;
        TupLibrary *library;
        TupProject *project;
        TupProject::Mode currentMode;
        TupProject::Mode previousMode;
        TupLibraryDisplay *display;
        TupItemManager *libraryTree;
        int childCount;
        QDir libraryDir;
        QComboBox *itemType;
        int currentPlayerId;
        QString oldId;
        bool renaming;
        bool mkdir;
        bool isNetworked;
        bool nativeFromFileSystem;
        bool isExternalLibraryAsset;

        QTreeWidgetItem *lastItemEdited;
        QTreeWidgetItem *currentItemDisplayed;
        QFileSystemWatcher *watcher;
        QList<QString> editorItems;
        bool isEffectSound;
        TupLibraryObject *currentSound;
        QMap<QString, QPixmap> nativeMap;

        struct Frame
        {
           int scene;
           int layer;
           int frame;
        } currentFrame;

        bool removeTempVideo;
        QString tempVideoPath;
};

#endif

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
#include <QProgressDialog>
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

    protected:
        virtual void sceneResponse(TupSceneResponse *response);
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
        void updateSoundTiming(int frame);
        void importAsset(const QString &name, const QString &extension, int extensionId, QByteArray &data);
        void importImageAsset(const QString &name, const QString &extension, QByteArray &data);
        void importNativeAsset(const QString &name, const QString &extension, QByteArray &data);

    public slots:
        void addFolder(const QString &folderName = QString());
        void importImageGroup();
        void importImage(const QString &image);
        void importSvgGroup();
        void importSvg(const QString &svgPath);
        void importImageSequence();
        void importSvgSequence();
        void importSoundFile();

    private slots:
        void openSearchDialog();

    signals:
        void requestCurrentGraphic();
        void soundUpdated();

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
        void importNativeObject(const QString &object);
        QStringList naturalSort(QStringList photograms);
        void refreshItem(LibraryObjects collection);

        QScreen *screen;
        TupLibrary *library;
        TupProject *project;
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
        QTreeWidgetItem *lastItemEdited;
        QTreeWidgetItem *currentItemDisplayed;
        QFileSystemWatcher *watcher;
        QList<QString> editorItems;
        bool isEffectSound;
        TupLibraryObject *currentSound;

        struct Frame
        {
           int scene;
           int layer;
           int frame;
        } currentFrame;
};

#endif

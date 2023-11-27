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

#include "tuplibrarywidget.h"
#include "tuplayer.h"
#include "tupitemfactory.h"
#include "tupitemgroup.h"
#include "tuppathitem.h"
#include "tuptextitem.h"
#include "tuprectitem.h"
#include "tupellipseitem.h"
#include "tupsounddialog.h"
#include "tupvideoimporterdialog.h"

#define RETURN_IF_NOT_LIBRARY if (!library) return;

TupLibraryWidget::TupLibraryWidget(QWidget *parent) : TupModuleWidgetBase(parent)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupLibraryWidget()]";
    #endif

    libraryIsLoaded = false;
    childCount = 0;
    renaming = false;
    mkdir = false;
    isEffectSound = false;
    currentMode = TupProject::FRAMES_MODE;
    nativeFromFileSystem = false;
    isExternalLibraryAsset = false;
    removeTempVideo = false;
    removeTempVideo = "";

    setWindowIcon(QPixmap(ICONS_DIR + "library.png"));
    setWindowTitle(tr("Library"));

    screen = QGuiApplication::screens().at(0);

    libraryDir = QDir(CONFIG_DIR + "libraries");

    display = new TupLibraryDisplay();
    connect(display, SIGNAL(muteEnabled(bool)), this, SLOT(updateSoundMuteStatus(bool)));
    connect(display, SIGNAL(soundResourceModified(SoundResource)),
            this, SLOT(updateSoundResource(SoundResource)));

    libraryTree = new TupItemManager;

    connect(libraryTree, SIGNAL(itemSelected(QTreeWidgetItem *)), this,
                                   SLOT(previewItem(QTreeWidgetItem *)));

    connect(libraryTree, SIGNAL(itemRemoved()), this,
                                   SLOT(removeCurrentItem()));

    connect(libraryTree, SIGNAL(itemCloned(QTreeWidgetItem*)), this,
                                   SLOT(cloneObject(QTreeWidgetItem*)));

    connect(libraryTree, SIGNAL(itemExported(QTreeWidgetItem*)), this,
                                   SLOT(exportObject(QTreeWidgetItem*)));

    connect(libraryTree, SIGNAL(itemRenamed(QTreeWidgetItem*)), this,
                                   SLOT(renameObject(QTreeWidgetItem*)));

    connect(libraryTree, SIGNAL(itemChanged(QTreeWidgetItem*, int)), this,
                                   SLOT(refreshItem(QTreeWidgetItem*)));

    connect(libraryTree, SIGNAL(editorClosed()), this,
                                   SLOT(updateItemEditionState()));

    connect(libraryTree, SIGNAL(itemMoved(QString, QString)), this,
                                   SLOT(updateLibrary(QString, QString)));

    connect(libraryTree, SIGNAL(itemRequired()), this,
                                   SLOT(insertObjectInWorkspace()));

    connect(libraryTree, SIGNAL(itemCreated(QTreeWidgetItem*)), this,
                                   SLOT(activeRefresh(QTreeWidgetItem*)));

    connect(libraryTree, SIGNAL(inkscapeEditCall(QTreeWidgetItem*)), this,
                                   SLOT(openInkscapeToEdit(QTreeWidgetItem*)));

    connect(libraryTree, SIGNAL(gimpEditCall(QTreeWidgetItem*)), this,
                                   SLOT(openGimpToEdit(QTreeWidgetItem*)));

    connect(libraryTree, SIGNAL(kritaEditCall(QTreeWidgetItem*)), this,
                                   SLOT(openKritaToEdit(QTreeWidgetItem*)));

    connect(libraryTree, SIGNAL(myPaintEditCall(QTreeWidgetItem*)), this,
                                   SLOT(openMyPaintToEdit(QTreeWidgetItem*)));

    connect(libraryTree, SIGNAL(newRasterCall()), this,
                                   SLOT(createRasterObject()));

    connect(libraryTree, SIGNAL(newVectorCall()), this,
                                   SLOT(createVectorObject()));

    connect(libraryTree, SIGNAL(lipSyncCall(QTreeWidgetItem*)), this,
                                   SLOT(callLipsyncEditor(QTreeWidgetItem*)));

    /* SQA: These connections don't work on Windows
    connect(libraryTree, &TupItemManager::itemSelected, this, &TupLibraryWidget::previewItem);

    connect(libraryTree, &TupItemManager::itemRemoved, this,
                         &TupLibraryWidget::removeCurrentItem);

    connect(libraryTree, &TupItemManager::itemCloned, this,
                         &TupLibraryWidget::cloneObject);

    connect(libraryTree, &TupItemManager::itemExported, this,
                         &TupLibraryWidget::exportObject);

    connect(libraryTree, &TupItemManager::itemRenamed, this,
                         &TupLibraryWidget::renameObject);

    connect(libraryTree, &TupItemManager::itemChanged, this,
                         &TupLibraryWidget::refreshItem);

    connect(libraryTree, &TupItemManager::editorClosed, this,
                         &TupLibraryWidget::updateItemEditionState);

    connect(libraryTree, &TupItemManager::itemMoved, this,
                         &TupLibraryWidget::updateLibrary);

    connect(libraryTree, &TupItemManager::itemRequired, this,
                         &TupLibraryWidget::insertObjectInWorkspace);

    connect(libraryTree, &TupItemManager::itemCreated, this,
                         &TupLibraryWidget::activeRefresh);

    connect(libraryTree, &TupItemManager::inkscapeEditCall, this,
                         &TupLibraryWidget::openInkscapeToEdit);

    connect(libraryTree, &TupItemManager::gimpEditCall, this,
                         &TupLibraryWidget::openGimpToEdit);

    connect(libraryTree, &TupItemManager::kritaEditCall, this,
                         &TupLibraryWidget::openKritaToEdit);

    connect(libraryTree, &TupItemManager::myPaintEditCall, this,
                         &TupLibraryWidget::openMyPaintToEdit);

    connect(libraryTree, &TupItemManager::newRasterCall, this,
                         &TupLibraryWidget::createRasterObject);

    connect(libraryTree, &TupItemManager::newVectorCall, this,
                         &TupLibraryWidget::createVectorObject);
    */

    QGroupBox *buttons = new QGroupBox;
    QHBoxLayout *buttonLayout = new QHBoxLayout(buttons);
    buttonLayout->setMargin(0);
    buttonLayout->setSpacing(0);

    TImageButton *searchButton = new TImageButton(QPixmap(THEME_DIR + "icons/search.png"), 22, buttons);
    connect(searchButton, SIGNAL(clicked()), this, SLOT(openSearchDialog()));
    searchButton->setToolTip(tr("Search for Assets"));
    buttonLayout->addWidget(searchButton);

    QHBoxLayout *comboLayout = new QHBoxLayout;
    comboLayout->setMargin(0);
    comboLayout->setSpacing(0);

    itemType = new QComboBox();
    itemType->setIconSize(QSize(15, 15));
    itemType->setMaximumWidth(120);
    
    itemType->addItem(QIcon(THEME_DIR + "icons/bitmap.png"), tr("Image"));
    itemType->addItem(QIcon(THEME_DIR + "icons/svg.png"), tr("Svg File"));
    itemType->addItem(QIcon(THEME_DIR + "icons/drawing_object.png"), tr("Native Object"));
    itemType->addItem(QIcon(THEME_DIR + "icons/bitmap_array.png"), tr("Image Sequence"));
    itemType->addItem(QIcon(THEME_DIR + "icons/svg_array.png"), tr("Svg Sequence"));
    itemType->addItem(QIcon(THEME_DIR + "icons/sound_object.png"), tr("Audio File"));
    itemType->addItem(QIcon(THEME_DIR + "icons/player.png"), tr("Video File"));

    comboLayout->addWidget(itemType);

    connect(itemType, SIGNAL(currentIndexChanged(int)), this, SLOT(importLibraryObject()));

    TImageButton *addGC = new TImageButton(QPixmap(THEME_DIR + "icons/plus_sign.png"), 22, buttons);
    addGC->setToolTip(tr("Add an object to library"));
    connect(addGC, SIGNAL(clicked()), this, SLOT(importLibraryObject()));
    comboLayout->addWidget(addGC);

    buttonLayout->addLayout(comboLayout);

    TImageButton *addFolderGC = new TImageButton(QPixmap(THEME_DIR + "icons/create_folder.png"), 22, buttons);
    connect(addFolderGC, SIGNAL(clicked()), this, SLOT(addFolder()));
    addFolderGC->setToolTip(tr("Create new folder"));
    buttonLayout->addWidget(addFolderGC);
    // SQA: Temporary code
    // addFolderGC->setEnabled(false);

    TImageButton *gctoDrawingArea = new TImageButton(QPixmap(THEME_DIR + "icons/library_to_ws.png"), 22, buttons);
    connect(gctoDrawingArea, SIGNAL(clicked()), this, SLOT(insertObjectInWorkspace()));
    gctoDrawingArea->setToolTip(tr("Insert library item into frame"));
    buttonLayout->addWidget(gctoDrawingArea);

    buttons->setLayout(buttonLayout);

    addChild(display, Qt::AlignTop);
    addChild(buttons, Qt::AlignTop);
    addChild(libraryTree);

    watcher = new QFileSystemWatcher(this);
    connect(watcher, SIGNAL(fileChanged(QString)), this, SLOT(updateItemFromSaveAction()));
}

TupLibraryWidget::~TupLibraryWidget()
{
    /*
    #ifdef TUP_DEBUG
        qDebug() << "[~TupLibraryWidget()]";
    #endif
    */
}

void TupLibraryWidget::resetGUI()
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupLibraryWidget::resetGUI()]";
    #endif

    libraryIsLoaded = false;

    if (display)
        display->reset();

    if (library)
        library->reset();

    if (libraryTree)
        libraryTree->cleanUI();

    currentSound = nullptr;
    delete currentSound;
}

void TupLibraryWidget::setLibrary(TupLibrary *assets)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupLibraryWidget::setLibrary()]";
    #endif

    library = assets;
    project = library->getProject();
    // libraryIsLoaded = true;
}

void TupLibraryWidget::initCurrentFrame()
{
    currentFrame.frame = 0;
    currentFrame.layer = 0;
    currentFrame.scene = 0;
}

void TupLibraryWidget::updateSpaceContext(TupProject::Mode mode)
{
    currentMode = mode;
}

void TupLibraryWidget::setNetworking(bool netOn)
{
    isNetworked = netOn;
}

void TupLibraryWidget::addFolder(const QString &folderName)
{
    libraryTree->createFolder(folderName);
    mkdir = true;
}

void TupLibraryWidget::updateItemEditionState()
{
    if (editorItems.count() == 2) {
        TupProjectRequest request = TupRequestBuilder::createLibraryRequest(TupProjectRequest::Remove, 
                                                                            editorItems.at(0), TupLibraryObject::Folder);
        emit requestTriggered(&request);
    }

    editorItems.clear();
}

void TupLibraryWidget::activeRefresh(QTreeWidgetItem *item)
{
    mkdir = true;
    refreshItem(item);
}

void TupLibraryWidget::previewItem(QTreeWidgetItem *item)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupLibraryWidget::previewItem()]";
    #endif

    RETURN_IF_NOT_LIBRARY;

    if (item) {
        currentItemDisplayed = item;

        if (item->text(2).length() == 0) {
            display->showDisplay();
            QGraphicsTextItem *msg = new QGraphicsTextItem(tr("Directory"));
            display->render(false, static_cast<QGraphicsItem *>(msg));
            return;
        }

        QString objectName = item->text(1) + "." + item->text(2).toLower();
        #ifdef TUP_DEBUG
            qDebug() << "[TupLibraryWidget::previewItem()] - Getting object -> " << objectName;
        #endif
        TupLibraryObject *object = library->getObject(objectName);
        if (!object) {
            #ifdef TUP_DEBUG
                qDebug() << "[TupLibraryWidget::previewItem()] - Fatal Error: Cannot find the object -> "
                            + item->text(1) + "." + item->text(2).toLower();
            #endif

            display->showDisplay();
            QGraphicsTextItem *text = new QGraphicsTextItem(tr("No preview available"));
            display->render(false, static_cast<QGraphicsItem *>(text));

            return;
        }

        switch (object->getObjectType()) {
                case TupLibraryObject::Svg:
                   {
                     display->showDisplay();
                     QGraphicsSvgItem *svg = new QGraphicsSvgItem(object->getDataPath()); 
                     display->render(true, static_cast<QGraphicsItem *>(svg));
                   }
                   break;
                case TupLibraryObject::Image:
                   {
                     display->showDisplay();
                     display->render(true, qvariant_cast<QGraphicsItem *>(object->getData()));
                   }
                   break;
                case TupLibraryObject::Item:
                   {
                     display->showDisplay();
                     if (object->getItemType() == TupLibraryObject::Text || object->getItemType() == TupLibraryObject::Path) {
                         display->render(true, qvariant_cast<QGraphicsItem *>(object->getData()));
                     } else {
                         display->render(true, nativeMap[objectName]);
                     }

                     /* SQA: Just a test
                     TupSymbolEditor *editor = new TupSymbolEditor;
                     editor->setSymbol(object);
                     emit postPage(editor);
                     */
                   }
                   break;
                case TupLibraryObject::Audio:
                   {
                     currentSound = object;

                     qDebug() << "[TupLibraryWidget::previewItem()] - project->getSceneNames() ->" << project->getSceneNames();

                     display->setSoundParams(object->getSoundResourceParams(),
                                             project->getSceneNames(), project->getFrameLimits());
                     display->showSoundPlayer();
                   }
                   break;
                default:
                   {
                     #ifdef TUP_DEBUG
                         qDebug() << "[TupLibraryWidget::previewItem()] - "
                                     "Unknown symbol type ->" << object->getObjectType();
                     #endif
                   }
                   break;
        }
    } else {
        QGraphicsTextItem *msg = new QGraphicsTextItem(tr("No preview available"));
        display->render(false, static_cast<QGraphicsItem *>(msg));
    }
}

void TupLibraryWidget::insertObjectInWorkspace()
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupLibraryWidget::insertObjectInWorkspace()] - currentMode -> " << currentMode;
    #endif

    if (libraryTree->topLevelItemCount() == 0) {
        TOsd::self()->display(TOsd::Error, tr("Library is empty!"));
        #ifdef TUP_DEBUG
            qDebug() << "[TupLibraryWidget::insertObjectInWorkspace()] - Library is empty!";
        #endif
        return;
    }

    if (!libraryTree->currentItem()) {
        TOsd::self()->display(TOsd::Error, tr("There's no current selection!"));
        #ifdef TUP_DEBUG
            qDebug() << "[TupLibraryWidget::insertObjectInWorkspace()] - There's no current selection!";
        #endif
        return;
    }

    QString extension = libraryTree->currentItem()->text(2);
    if (extension.length() == 0) {
        TOsd::self()->display(TOsd::Error, tr("It's a directory! Please, pick a library object"));
        #ifdef TUP_DEBUG
            qDebug() << "[TupLibraryWidget::insertObjectInWorkspace()] - It's a directory!";
        #endif
        return;
    }

    if ((extension.compare("OGG") == 0) || (extension.compare("WAV") == 0) || (extension.compare("MP3") == 0)) {
        TOsd::self()->display(TOsd::Error, tr("It's an audio file! Please, pick a graphic object"));
        #ifdef TUP_DEBUG
            qDebug() << "[TupLibraryWidget::insertObjectInWorkspace()] - It's an audio file!";
        #endif
        return;
    } 

    QString key = libraryTree->currentItem()->text(1) + "." + extension.toLower();
    int objectType = libraryTree->itemType();
    TupProjectRequest request = TupRequestBuilder::createLibraryRequest(TupProjectRequest::InsertSymbolIntoFrame,
                                key, TupLibraryObject::ObjectType(objectType), currentMode, nullptr, QString(),
                                currentFrame.scene, currentFrame.layer, currentFrame.frame);

    emit requestTriggered(&request);
}

void TupLibraryWidget::removeCurrentItem()
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupLibraryWidget::removeCurrentItem()]";
    #endif

    if (!libraryTree->currentItem())
        return;

    TCONFIG->beginGroup("General");
    bool ask = TCONFIG->value("ConfirmRemoveObject", true).toBool();

    if (ask) {
        TOptionalDialog dialog(tr("Do you want to remove this object from Library?"), tr("Confirmation"));
        dialog.setModal(true);
        dialog.move(static_cast<int> ((screen->geometry().width() - dialog.sizeHint().width()) / 2),
                    static_cast<int> ((screen->geometry().height() - dialog.sizeHint().height()) / 2));

        if (dialog.exec() == QDialog::Rejected)
            return;

        TCONFIG->beginGroup("General");
        TCONFIG->setValue("ConfirmRemoveObject", dialog.shownAgain());
        TCONFIG->sync();
    }

    QString objectKey = libraryTree->currentItem()->text(1);
    QString extension = libraryTree->currentItem()->text(2);
    TupLibraryObject::ObjectType type = TupLibraryObject::Folder;

    // If it's NOT a directory
    if (extension.length() > 0) {
        objectKey = libraryTree->currentItem()->text(3);
        if (extension.compare("JPEG") == 0 || extension.compare("JPG") == 0 || extension.compare("PNG") == 0 ||
            extension.compare("GIF") == 0 || extension.compare("WEBP") == 0)
            type = TupLibraryObject::Image;
        if (extension.compare("SVG")==0)
            type = TupLibraryObject::Svg;
        if (extension.compare("TOBJ")==0)
            type = TupLibraryObject::Item;
        if ((extension.compare("OGG") == 0) || (extension.compare("WAV") == 0) || (extension.compare("MP3") == 0))
            type = TupLibraryObject::Audio;
    } 

    if (type == TupLibraryObject::Audio) {
        resetSoundPlayer();
        emit soundRemoved(Library, objectKey);
    } else  {
        if (type == TupLibraryObject::Folder) {
            if (library->folderHasAudioObjects(objectKey)) {
                #ifdef TUP_DEBUG
                    qDebug() << "[TupLibraryWidget::removeCurrentItem()] - Folder has audio items! -> " << objectKey;
                #endif
                emit folderWithAudiosRemoved();
            }
        }

        TupProjectRequest request = TupRequestBuilder::createLibraryRequest(TupProjectRequest::Remove, objectKey, type);
        emit requestTriggered(&request);
    }
}

void TupLibraryWidget::removeSoundItem(const QString &soundKey)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupLibraryWidget::removeSoundItem()] - soundKey -> " << soundKey;
    #endif

    TupProjectRequest request = TupRequestBuilder::createLibraryRequest(TupProjectRequest::Remove,
                                                                        soundKey, TupLibraryObject::Audio);
    emit requestTriggered(&request);
}

void TupLibraryWidget::cloneObject(QTreeWidgetItem* item)
{
    if (item) {
        QString id = item->text(3);
        TupLibraryObject *object = library->getObject(id);

        if (object) {
            QString smallId = object->getShortId();
            QString extension = object->getExtension();
            TupLibraryObject::ObjectType type = object->getObjectType();
            QString path = object->getDataPath();
            int limit = path.lastIndexOf("/");
            QString newPath = path.left(limit + 1); 

            QString symbolName = "";

            if (itemNameEndsWithDigit(smallId)) {
                int index = getItemNameIndex(smallId);
                symbolName = nameForClonedItem(smallId, extension, index, newPath);
                newPath += symbolName;
            } else {
                symbolName = nameForClonedItem(smallId, extension, newPath);
                newPath += symbolName;
            }

            QString baseName = symbolName.section('.', 0, 0);
            baseName = verifyNameAvailability(baseName, extension, true);
            symbolName = baseName + "." + extension.toLower();

            bool isOk = QFile::copy(path, newPath);

            if (!isOk) {
                #ifdef TUP_DEBUG
                    qDebug() << "[TupLibraryWidget::cloneObject()] - Fatal Error: Object file couldn't be cloned!";
                #endif
                return;
            }

            TupLibraryObject *newObject = new TupLibraryObject();
            newObject->setSymbolName(symbolName);
            newObject->setObjectType(type);
            newObject->setDataPath(newPath);
            isOk = newObject->loadData(newPath);

            if (!isOk) {
                #ifdef TUP_DEBUG
                    qDebug() << "[TupLibraryWidget::cloneObject()] - Fatal Error: Object file couldn't be loaded!";
                #endif
                return;
            } 

            library->addObject(newObject);

            QTreeWidgetItem *item = new QTreeWidgetItem(libraryTree);
            item->setText(1, baseName);
            item->setText(2, extension);
            item->setText(3, symbolName);
            item->setFlags(item->flags() | Qt::ItemIsEditable | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled);

            switch (object->getObjectType()) {
                case TupLibraryObject::Item:
                    {
                        item->setIcon(0, QIcon(THEME_DIR + "icons/drawing_object.png"));
                        libraryTree->setCurrentItem(item);
                        previewItem(item);
                    }
                    break;
                case TupLibraryObject::Image:
                    {
                        item->setIcon(0, QIcon(THEME_DIR + "icons/bitmap.png"));
                        libraryTree->setCurrentItem(item);
                        previewItem(item);
                    }
                    break;
                case TupLibraryObject::Svg:
                    {
                        item->setIcon(0, QIcon(THEME_DIR + "icons/svg.png"));
                        libraryTree->setCurrentItem(item);
                        previewItem(item);
                    }
                    break;
                case TupLibraryObject::Audio:
                    {
                        item->setIcon(0, QIcon(THEME_DIR + "icons/sound_object.png"));
                        previewItem(item);
                    }
                    break;
                default:
                    {
                    }
            }
        } else {
            #ifdef TUP_DEBUG
                qDebug() << "[TupLibraryWidget::cloneObject()] - Fatal Error: Object doesn't exist! -> " << id;
            #endif

            return;
        }
    }
}

void TupLibraryWidget::exportObject(QTreeWidgetItem *item)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupLibraryWidget::exportObject()]";
    #endif

    if (item) {
        QString id = item->text(3);
        TupLibraryObject *object = library->getObject(id);
        if (object) {
            QString path = object->getDataPath();
            if (path.length() > 0) {
                TupLibraryObject::ObjectType type = object->getObjectType();
                QString fileExtension = object->getExtension();
                QString filter;

                if (type == TupLibraryObject::Image) {
                    filter = tr("Images") + " ";
                    if (fileExtension.compare("PNG") == 0)
                        filter += "(*.png)";
                    if ((fileExtension.compare("JPG") == 0) || (fileExtension.compare("JPEG") == 0))
                        filter += "(*.jpg *.jpeg)";
                    if (fileExtension.compare("GIF") == 0)
                        filter += "(*.gif)";
                    if (fileExtension.compare("WEBP") == 0)
                        filter += "(*.webp)";
                    if (fileExtension.compare("XPM") == 0)
                        filter += "(*.xpm)";
                    if (fileExtension.compare("SVG") == 0)
                        filter += "(*.svg)";
                } else if (type == TupLibraryObject::Audio) {
                    filter = tr("Audio") + " ";
                    if (fileExtension.compare("OGG") == 0)
                        filter += "(*.ogg)";
                    if (fileExtension.compare("MP3") == 0)
                        filter += "(*.mp3)";
                    if (fileExtension.compare("WAV") == 0)
                        filter += "(*.wav)";
                } else if (type == TupLibraryObject::Item) {
                    filter = tr("Native Objects") + " " + "(*.tobj)";
                }

                TCONFIG->beginGroup("General");
                QString defaultPath = TCONFIG->value("DefaultPath", QDir::homePath()).toString();
                QString target = QFileDialog::getSaveFileName(this, tr("Export object..."), defaultPath + "/" + id , filter);

                if (target.isEmpty())
                    return;

                QString filename = target.toUpper();
                if (type == TupLibraryObject::Image) {
                    if (fileExtension.compare("PNG") == 0 && !filename.endsWith(".PNG"))
                        target += ".png";
                    if ((fileExtension.compare("JPG") == 0) && (!filename.endsWith(".JPG") || !filename.endsWith(".JPEG")))
                        target += ".jpg";
                    if (fileExtension.compare("GIF") == 0 && !filename.endsWith(".GIF"))
                        target += ".gif";
                    if (fileExtension.compare("WEBP") == 0 && !filename.endsWith(".WEBP"))
                        target += ".webp";
                    if (fileExtension.compare("XPM") == 0 && !filename.endsWith(".XPM"))
                        target += ".xpm";
                    if (fileExtension.compare("SVG") == 0 && !filename.endsWith(".SVG"))
                        target += ".svg";
                } else if (type == TupLibraryObject::Audio) {
                    if (fileExtension.compare("OGG") == 0 && !filename.endsWith(".OGG"))
                        target += ".ogg";
                    if (fileExtension.compare("MP3") == 0 && !filename.endsWith(".MP3"))
                        target += ".mp3";
                    if (fileExtension.compare("WAV") == 0 && !filename.endsWith(".WAV"))
                        target += ".wav";
                } else if (type == TupLibraryObject::Item && !filename.endsWith(".TOBJ")) {
                    target += ".tobj";
                }

                if (QFile::exists(target)) {
                    if (!QFile::remove(target)) {
                        #ifdef TUP_DEBUG
                            qDebug() << "[TupLibraryWidget::exportObject()] - Fatal Error: Destination path already exists! -> " << id;
                        #endif
                        return;
                    }
                }

                if (QFile::copy(path, target)) {
                    setDefaultPath(target);
                    TOsd::self()->display(TOsd::Info, tr("Item exported successfully!"));
                } else {
                    #ifdef TUP_DEBUG
                        qDebug() << "[TupLibraryWidget::exportObject()] - Error: Object file couldn't be exported! -> " << id;
                    #endif
                    return;
                }
            } else {
                #ifdef TUP_DEBUG
                    qDebug() << "[TupLibraryWidget::exportObject()] - Error: Object path is null! -> " << id;
                #endif
                return;
            }
        } else {
            #ifdef TUP_DEBUG
                qDebug() << "[TupLibraryWidget::exportObject()] - Error: Object doesn't exist! -> " << id;
            #endif
            return;
        }
    }
}

void TupLibraryWidget::renameObject(QTreeWidgetItem *item)
{
    if (item) {
        renaming = true;
        oldId = item->text(1);
        libraryTree->editItem(item, 1);
    }
}

void TupLibraryWidget::createRasterObject()
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupLibraryWidget::createRasterObject()]";
    #endif

    QString name = "object00";
    QString extension = "PNG";
    name = verifyNameAvailability(name, extension, true);

    QSize size = project->getDimension();
    int w = QString::number(size.width()).length();
    int h = QString::number(size.height()).length();

    int width = 1;
    int height = 1; 
    for(int i=0; i<w; i++)
        width *= 10;
    for(int i=0; i<h; i++)
        height *= 10;

    size = QSize(width, height);
    TupNewItemDialog dialog(name, TupNewItemDialog::Raster, size);

    if (dialog.exec() == QDialog::Accepted) {
        QString name = dialog.getItemName();
        QSize size = dialog.itemSize();
        QColor background = dialog.getBackground();
        QString extension = dialog.itemExtension();
        QString editor = dialog.getSoftware();

        QString imagesDir = project->getDataDir() + "/images/";
        if (!QFile::exists(imagesDir)) {
            QDir dir;
            if (!dir.mkpath(imagesDir)) {
                #ifdef TUP_DEBUG
                    qDebug() << "[TupLibraryWidget::createRasterObject()] - Fatal Error: Couldn't create directory -> " << imagesDir;
                #endif
                TOsd::self()->display(TOsd::Error, tr("Couldn't create images directory!"));
                return;
            }
        }
             
        QString path = imagesDir + name + "." + extension.toLower();
        QString symbolName = name; 
        if (QFile::exists(path)) {
            symbolName = nameForClonedItem(name, extension, imagesDir);
            path = imagesDir + symbolName + "." + extension.toLower();
        }

        symbolName += "." + extension.toLower();

        QImage::Format format = QImage::Format_RGB32;
        if (extension.compare("PNG")==0)
            format = QImage::Format_ARGB32;

        QImage *image = new QImage(size, format); 
        image->fill(background);

        bool isOk = image->save(path);

        if (isOk) {
            TupLibraryObject *newObject = new TupLibraryObject();
            newObject->setSymbolName(symbolName);
            newObject->setObjectType(TupLibraryObject::Image);
            newObject->setDataPath(path);
            isOk = newObject->loadData(path);

            if (isOk) {
                library->addObject(newObject);

                QTreeWidgetItem *item = new QTreeWidgetItem(libraryTree);
                item->setText(1, name);
                item->setText(2, extension);
                item->setText(3, symbolName);
                item->setFlags(item->flags() | Qt::ItemIsEditable | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled);
                item->setIcon(0, QIcon(THEME_DIR + "icons/bitmap.png"));
                libraryTree->setCurrentItem(item);

                previewItem(item);

                lastItemEdited = item;
                executeSoftware(editor, path);
            } else {
                #ifdef TUP_DEBUG
                    qDebug() << "[TupLibraryWidget::createRasterObject()] - Fatal Error: Object file couldn't be loaded from -> " << path;
                #endif
                return;
            }
        } else {
            #ifdef TUP_DEBUG
                qDebug() << "[TupLibraryWidget::createRasterObject()] - Fatal Error: Object file couldn't be saved at -> " << path;
            #endif
            return;
        }
    }
}

void TupLibraryWidget::createVectorObject()
{
    QString name = "object00";
    QString extension = "SVG";
    name = verifyNameAvailability(name, extension, true);

    QSize size = project->getDimension();
    int w = QString::number(size.width()).length();
    int h = QString::number(size.height()).length();

    int width = 1;
    int height = 1;
    for(int i=0; i<w; i++) 
        width *= 10;
    for(int i=0; i<h; i++) 
        height *= 10;

    size = QSize(width, height);

    TupNewItemDialog dialog(name, TupNewItemDialog::Vector, size);
    if (dialog.exec() == QDialog::Accepted) {
        QString name = dialog.getItemName();
        QSize size = dialog.itemSize();
        QString extension = dialog.itemExtension();
        QString editor = dialog.getSoftware();

        QString vectorDir = project->getDataDir() + "/svg/";
        if (!QFile::exists(vectorDir)) {
            QDir dir;
            if (!dir.mkpath(vectorDir)) {
                #ifdef TUP_DEBUG
                    qDebug() << "[TupLibraryWidget::createVectorObject()] - Fatal Error: Couldn't create directory -> " << vectorDir;
                #endif

                TOsd::self()->display(TOsd::Error, tr("Couldn't create vector directory!"));
                return;
            }
        }

        QString path = vectorDir + name + "." + extension.toLower();
        QString symbolName = name;
        if (QFile::exists(path)) {
            symbolName = nameForClonedItem(name, extension, vectorDir);
            path = vectorDir + symbolName + "." + extension.toLower();
        }

        symbolName += "." + extension.toLower();

        QSvgGenerator generator;
        generator.setFileName(path);
        generator.setSize(size);
        generator.setViewBox(QRect(0, 0, size.width(), size.height()));
        generator.setTitle(name);
        generator.setDescription(tr("TupiTube library item"));
        QPainter painter;
        painter.begin(&generator);
        bool isOk = painter.end();

        if (isOk) {
            QDomDocument doc;
            QFile file(path);
            if (!file.open(QIODevice::ReadOnly)) {
                #ifdef TUP_DEBUG
                    qDebug() << "[TupLibraryWidget::createVectorObject()] - Fatal Error: SVG file couldn't be opened -> " << path;
                #endif
                return;
            }
            if (!doc.setContent(&file)) {
                #ifdef TUP_DEBUG
                    qDebug() << "[TupLibraryWidget::createVectorObject()] - Fatal Error: SVG file couldn't be parsed as XML -> " << path;
                #endif
                return;
            }
            file.close();

            QDomNodeList roots = doc.elementsByTagName("svg");
            QDomElement root = roots.at(0).toElement(); 
            root.setAttribute("width", size.width());
            root.setAttribute("height", size.height());
            if (!file.open(QIODevice::Truncate | QIODevice::WriteOnly)) {
                #ifdef TUP_DEBUG
                    qDebug() << "[TupLibraryWidget::createVectorObject()] - Fatal Error: SVG file couldn't be updated -> " << path;
                #endif
                return;
            } 
            QByteArray xml = doc.toByteArray();
            file.write(xml);
            file.close();

            TupLibraryObject *newObject = new TupLibraryObject();
            newObject->setSymbolName(symbolName);
            newObject->setObjectType(TupLibraryObject::Svg);
            newObject->setDataPath(path);
            isOk = newObject->loadData(path);

            if (isOk) {
                library->addObject(newObject);
                QTreeWidgetItem *item = new QTreeWidgetItem(libraryTree);
                item->setText(1, name);
                item->setText(2, extension);
                item->setText(3, symbolName);
                item->setFlags(item->flags() | Qt::ItemIsEditable | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled);
                item->setIcon(0, QIcon(THEME_DIR + "icons/svg.png"));

                libraryTree->setCurrentItem(item);
                previewItem(item);

                lastItemEdited = item;
                executeSoftware(editor, path);
            } else {
                #ifdef TUP_DEBUG
                    qDebug() << "[TupLibraryWidget::createVectorObject()] - Fatal Error: Object file couldn't be loaded from -> " << path;
                #endif
                return;
            }
        } else {
            #ifdef TUP_DEBUG
                qDebug() << "[TupLibraryWidget::createVectorObject()] - Fatal Error: Object file couldn't be saved at -> " << path;
            #endif
            return;
        }

    }
}

void TupLibraryWidget::importImageGroup()
{
    TCONFIG->beginGroup("General");
    QString path = TCONFIG->value("DefaultPath", QDir::homePath()).toString();

    QFileDialog dialog(this, tr("Import images..."), path);
    dialog.setNameFilter(tr("Images") + " (*.png *.xpm *.jpg *.jpeg *.gif *.webp)");
    dialog.setFileMode(QFileDialog::ExistingFiles);

    if (dialog.exec() == QDialog::Accepted) {
        QStringList files = dialog.selectedFiles();
        int size = files.size();
        for (int i = 0; i < size; ++i)
             importImage(files.at(i));

        setDefaultPath(files.at(0));
    }
}

void TupLibraryWidget::importExternalLibraryAsset(const QString &path, TupLibraryObject::ObjectType type,
                                                  const QString &folder)
{
    isExternalLibraryAsset = true;
    importLocalDroppedAsset(path, type, folder);
}

void TupLibraryWidget::importLocalDroppedAsset(const QString &path, TupLibraryObject::ObjectType type)
{
    importLocalDroppedAsset(path, type, "");
}

void TupLibraryWidget::importLocalDroppedAsset(const QString &path, TupLibraryObject::ObjectType type, const QString &folder)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupLibraryWidget::importLocalDroppedAsset()] - path -> " << path;
        qDebug() << "[TupLibraryWidget::importLocalDroppedAsset()] - folder -> " << folder;
    #endif

    if (type == TupLibraryObject::Image) {
        importImage(path, folder);
    } if (type == TupLibraryObject::Svg) {
        importSvg(path, folder);
    } if (type == TupLibraryObject::Item) {
        nativeFromFileSystem = true;
        importNativeObject(path, folder);
    } if (type == TupLibraryObject::Audio) {
        importSoundFileFromFolder(path, folder);
    } if (type == TupLibraryObject::Video) {
        importVideoFileFromFolder(path);
    }
}

void TupLibraryWidget::importWebDroppedAsset(const QString &assetName, const QString &extension,
                                             TupLibraryObject::ObjectType type, QByteArray data)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupLibraryWidget::importWebDroppedAsset()]";
    #endif

    if (data.size() == 0) {
        #ifdef TUP_DEBUG
            qDebug() << "[TupLibraryWidget::importWebDroppedAsset()] - Fatal Error: Byte array is empty!";
        #endif
        return;
    }

    if (type == TupLibraryObject::Image) {
        importImageFromByteArray(assetName, extension, data);
    } if (type == TupLibraryObject::Svg) {
        importSvgFromByteArray(assetName, data);
    } if (type == TupLibraryObject::Item) {
        nativeFromFileSystem = true;
        importNativeObjectFromByteArray(assetName, data);
    } if (type == TupLibraryObject::Audio) {
        importSoundFileFromByteArray(assetName, data);
    } if (type == TupLibraryObject::Video) {
        importVideoFileFromByteArray(assetName, data);
    }
}

void TupLibraryWidget::importImageFromByteArray(const QString &imageName, const QString &extension, QByteArray data,
                                                const QString &folder)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupLibraryWidget::importImageFromByteArray()]";
    #endif

    QString key = library->getItemKey(imageName);
    QByteArray ba = extension.toLatin1();
    const char* ext = ba.data();
    QPixmap *pixmap = new QPixmap();
    if (pixmap->loadFromData(data, ext)) {
        int picWidth = pixmap->width();
        int picHeight = pixmap->height();
        int projectWidth = project->getDimension().width();
        int projectHeight = project->getDimension().height();

        #ifdef TUP_DEBUG
            qDebug() << "[TupLibraryWidget::importImageFromByteArray()] - Image filename: " << key << " | Raw Size: " << data.size();
            qDebug() << "[TupLibraryWidget::importImageFromByteArray()] - Image Size: " << "[" << picWidth << ", " << picHeight << "]"
                    << " | Project Size: " << "[" << projectWidth << ", " << projectHeight << "] - Extension -> " << ext;
        #endif

        if (!isExternalLibraryAsset) {
            if (picWidth > projectWidth || picHeight > projectHeight) {
                QMessageBox msgBox;
                msgBox.setWindowTitle(tr("File:") + " " + imageName);
                msgBox.setIcon(QMessageBox::Question);
                msgBox.setText(tr("Image is bigger than workspace."));
                msgBox.setInformativeText(tr("Do you want to resize it?"));
                msgBox.setStandardButtons(QMessageBox::No | QMessageBox::Yes);
                msgBox.setDefaultButton(QMessageBox::Ok);
                msgBox.show();

                msgBox.move(static_cast<int> ((screen->geometry().width() - msgBox.width()) / 2),
                            static_cast<int> ((screen->geometry().height() - msgBox.height()) / 2));

                if (msgBox.exec() == QMessageBox::Yes) {
                    msgBox.close();
                    pixmap = new QPixmap();
                    if (pixmap->loadFromData(data, ext)) {
                        QPixmap newpix;
                        if (picWidth > picHeight) {
                            if (picWidth > projectWidth)
                                newpix = QPixmap(pixmap->scaledToWidth(projectWidth, Qt::SmoothTransformation));
                            else
                                newpix = QPixmap(pixmap->scaledToHeight(projectHeight, Qt::SmoothTransformation));
                        } else { // picHeight >= picWidth
                            if (picHeight > projectHeight)
                                newpix = QPixmap(pixmap->scaledToHeight(projectHeight, Qt::SmoothTransformation));
                            else
                                newpix = QPixmap(pixmap->scaledToWidth(projectWidth, Qt::SmoothTransformation));
                        }

                        QBuffer buffer(&data);
                        buffer.open(QIODevice::WriteOnly);
                        if (newpix.save(&buffer, ext)) {
                            #ifdef TUP_DEBUG
                                qDebug() << "[TupLibraryWidget::importImageFromByteArray()] - Image resize successfully! -> " << imageName;
                            #endif
                        } else {
                            #ifdef TUP_DEBUG
                                qWarning() << "[TupLibraryWidget::importImageFromByteArray()] - Fatal Error: Can't save resized image -> " << imageName;
                            #endif
                        }
                    } else {
                        #ifdef TUP_DEBUG
                            qWarning() << "[TupLibraryWidget::importImageFromByteArray()] - Fatal Error: Can't load large image -> " << imageName;
                        #endif
                    }
                }
            }
        }

        TupProjectRequest request = TupRequestBuilder::createLibraryRequest(TupProjectRequest::Add, key,
                                                                            TupLibraryObject::Image, project->spaceContext(), data, folder,
                                                                            currentFrame.scene, currentFrame.layer, currentFrame.frame);
        emit requestTriggered(&request);
    }
}

void TupLibraryWidget::importImage(const QString &imagePath, const QString &folder)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupLibraryWidget::importImage()] - imagePath -> " << imagePath;
    #endif

    if (imagePath.isEmpty()) {
        #ifdef TUP_DEBUG
            qDebug() << "[TupLibraryWidget::importImage()] - Warning: Image path is empty!";
        #endif

        return;
    }

    QFile imageFile(imagePath);
    if (imageFile.open(QIODevice::ReadOnly)) { 
        QFileInfo fileInfo(imageFile);
        QString key = library->getItemKey(fileInfo.fileName().toLower());
        QString extension = fileInfo.suffix().toUpper();
        QByteArray ba = extension.toLatin1();

        QByteArray data = imageFile.readAll();
        imageFile.close();
        importImageFromByteArray(key, extension, data, folder);
        data.clear();
    } else {
        #ifdef TUP_DEBUG
            qDebug() << "[TupLibraryWidget::importImage()] - Error: Can't open image - imagePath -> " << imagePath;
        #endif
        TOsd::self()->display(TOsd::Error, tr("Cannot open file: %1").arg(imagePath));
    }
}

void TupLibraryWidget::importSvgGroup()
{
    TCONFIG->beginGroup("General");
    QString path = TCONFIG->value("DefaultPath", QDir::homePath()).toString();

    QFileDialog dialog(this, tr("Import SVG files..."), path);
    dialog.setNameFilter(tr("Vector") + " (*.svg)");
    dialog.setFileMode(QFileDialog::ExistingFiles);

    if (dialog.exec() == QDialog::Accepted) {
        QStringList files = dialog.selectedFiles();
        int size = files.size();
        for (int i = 0; i < size; ++i)
            importSvg(files.at(i));

        setDefaultPath(files.at(0));
    }
}

void TupLibraryWidget::importSvgFromByteArray(const QString &filename, QByteArray data, const QString &folder)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupLibraryWidget::importSvg()] - Inserting SVG into project -> " << project->getName();
        int projectWidth = project->getDimension().width();
        int projectHeight = project->getDimension().height();
        qDebug() << "[TupLibraryWidget::importSvg()] - Project Size: [" << projectWidth << ","
                 << projectHeight << "]";
    #endif

    TupProjectRequest request = TupRequestBuilder::createLibraryRequest(TupProjectRequest::Add, filename,
                                                   TupLibraryObject::Svg, project->spaceContext(), data, folder,
                                                   currentFrame.scene, currentFrame.layer, currentFrame.frame);
    emit requestTriggered(&request);
}

void TupLibraryWidget::importSvg(const QString &svgPath, const QString &folder)
{
    if (svgPath.isEmpty()) {
        #ifdef TUP_DEBUG
            qWarning() << "[TupLibraryWidget::importSvg()] - Warning: SVG path is empty!";
        #endif

        return;
    }

    QFile file(svgPath);
    if (file.open(QIODevice::ReadOnly)) {
        QFileInfo fileInfo(file);
        QString key = library->getItemKey(fileInfo.fileName().toLower());
        QByteArray data = file.readAll();
        file.close();

        importSvgFromByteArray(key, data, folder);
    } else {
        TOsd::self()->display(TOsd::Error, tr("Cannot open file: %1").arg(svgPath));
    }
}

void TupLibraryWidget::importNativeObjects()
{
    nativeFromFileSystem = true;

    TCONFIG->beginGroup("General");
    QString path = TCONFIG->value("DefaultPath", QDir::homePath()).toString();

    QFileDialog dialog(this, tr("Import objects..."), path);
    dialog.setNameFilter(tr("Native Objects") + " (*.tobj)");
    dialog.setFileMode(QFileDialog::ExistingFiles);

    if (dialog.exec() == QDialog::Accepted) {
        QStringList files = dialog.selectedFiles();
        int size = files.size();
        for (int i = 0; i < size; ++i)
            importNativeObject(files.at(i));

        setDefaultPath(files.at(0));
    }
}

void TupLibraryWidget::importNativeObjectFromByteArray(const QString &filename, QByteArray data, const QString &folder)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupLibraryWidget::importNativeObjectFromByteArray()] - Inserting native object into project -> "
                 << project->getName();
    #endif

    TupProjectRequest request = TupRequestBuilder::createLibraryRequest(TupProjectRequest::Add, filename,
                                                   TupLibraryObject::Item, project->spaceContext(), data, folder,
                                                   currentFrame.scene, currentFrame.layer, currentFrame.frame);
    emit requestTriggered(&request);
}

void TupLibraryWidget::importNativeObject(const QString &nativePath, const QString &folder)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupLibraryWidget::importNativeObject()] - object -> ";
        qDebug() << nativePath;
    #endif

    if (nativePath.isEmpty()) {
        #ifdef TUP_DEBUG
            qDebug() << "[TupLibraryWidget::importNativeObject()] - Error: object data is empty!";
        #endif
        return;
    }

    QFile file(nativePath);
    if (file.open(QIODevice::ReadOnly)) {
        QFileInfo fileInfo(file);
        QString key = library->getItemKey(fileInfo.fileName().toLower());

        QByteArray data = file.readAll();
        file.close();

        importNativeObjectFromByteArray(key, data, folder);
    } else {
        TOsd::self()->display(TOsd::Error, tr("Cannot open file: %1").arg(nativePath));
    }
}

void TupLibraryWidget::verifyFramesAvailability(int filesTotal)
{
    TupScene *scene = project->sceneAt(currentFrame.scene);
    TupLayer *layer = scene->layerAt(currentFrame.layer);
    int framesTotal = layer->framesCount();
    int initFrame = currentFrame.frame;
    int scope = initFrame + filesTotal;
    if (scope > framesTotal) {
        for (int i=framesTotal; i<scope; i++) {
             TupProjectRequest request = TupRequestBuilder::createFrameRequest(currentFrame.scene, currentFrame.layer,
                                                                               i, TupProjectRequest::Add, tr("Frame"));
             emit requestTriggered(&request);
        }

        QString selection = QString::number(currentFrame.layer) + "," + QString::number(currentFrame.layer) + ","
                            + QString::number(initFrame) + "," + QString::number(initFrame);

        TupProjectRequest request = TupRequestBuilder::createFrameRequest(currentFrame.scene, currentFrame.layer, initFrame,
                                                                          TupProjectRequest::Select, selection);
        emit requestTriggered(&request);
    }
}

void TupLibraryWidget::importImageSequence()
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupLibraryWidget::importImageSequence()]";
    #endif

    TCONFIG->beginGroup("General");
    QString path = TCONFIG->value("DefaultPath", QDir::homePath()).toString();

    QFileDialog dialog(this, tr("Choose the images directory..."), path);
    dialog.setFileMode(QFileDialog::Directory);
    dialog.setOption(QFileDialog::ShowDirsOnly);
    dialog.setOption(QFileDialog::DontResolveSymlinks);

    if (dialog.exec() == QDialog::Accepted) {
        QStringList files = dialog.selectedFiles();
        path = files.at(0);
        loadSequenceFromDirectory(FolderAction, path);
    }
}

bool TupLibraryWidget::importImageRecord(const QString &photogram, const QString &extension, const QSize imageSize,
                                         const QSize projectSize, bool resize, const QString &directory)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupLibraryWidget::importImageRecord()] - photogram -> " << photogram;
    #endif

    QFile file(photogram);
    QFileInfo fileInfo(file);
    QString symName = fileInfo.fileName().toLower();
    symName = symName.replace("(","_");
    symName = symName.replace(")","_");

    if (file.open(QIODevice::ReadOnly)) {
        QByteArray data = file.readAll();
        file.close();

        if (resize) {
            QPixmap *pixmap = new QPixmap();
            QByteArray ba = extension.toLatin1();
            const char* ext = ba.data();
            if (pixmap->loadFromData(data, ext)) {
                QPixmap newpix;
                if (imageSize.width() > imageSize.height())
                    newpix = QPixmap(pixmap->scaledToWidth(projectSize.width(), Qt::SmoothTransformation));
                else
                    newpix = QPixmap(pixmap->scaledToHeight(projectSize.height(), Qt::SmoothTransformation));

                QBuffer buffer(&data);
                buffer.open(QIODevice::WriteOnly);
                newpix.save(&buffer, ext);
            }
        }

        TupProjectRequest request = TupRequestBuilder::createLibraryRequest(TupProjectRequest::Add, symName,
                                    TupLibraryObject::Image, project->spaceContext(), data, directory);
        emit requestTriggered(&request);
    } else {
        QMessageBox::critical(this, tr("ERROR!"),
                              tr("ERROR: Can't open file %1. Please, check file permissions and try again.").arg(symName),
                              QMessageBox::Ok);
        QApplication::restoreOverrideCursor();

        return false;
    }

    return true;
}

bool TupLibraryWidget::fileIsImage(const QString &extension)
{
    if (extension.compare("JPEG")==0 || extension.compare("JPG")==0 || extension.compare("PNG")==0 ||
        extension.compare("GIF")==0 || extension.compare("XPM")==0 || extension.compare("WEBP")==0)
        return true;

    return false;
}

void TupLibraryWidget::loadSequenceFromDirectory(ImportAction action, const QString &path, bool resizeFlag)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupLibraryWidget::loadSequenceFromDirectory()] - path -> " << path;
    #endif

    QDir source(path);
    QFileInfoList records = source.entryInfoList(QDir::Files, QDir::Name);
    int imagesTotal = records.size();

    // Ensuring to get only graphic files here. Check extensions! (PNG, JPG, GIF, XPM, WEBP)
    QString extension = "";
    QStringList photograms;
    for (int i = 0; i < imagesTotal; ++i) { // Getting images list
         if (records.at(i).isFile()) {
             extension = records.at(i).suffix().toUpper();
             if (fileIsImage(extension))
                 photograms << records.at(i).absoluteFilePath();
         }
    }

    imagesTotal = photograms.size();
    if (imagesTotal == 0) {
        TOsd::self()->display(TOsd::Error, tr("No image files were found.<br/>Please, try another directory"));

        return;
    }
    photograms = TAlgorithm::naturalSort(photograms);

    QString text = tr("Image files found: %1.").arg(imagesTotal);
    bool resize = false;

    QPixmap *pixmap = new QPixmap(photograms.at(0));
    int picWidth = pixmap->width();
    int picHeight = pixmap->height();
    int projectWidth = project->getDimension().width();
    int projectHeight = project->getDimension().height();

    if (action == FolderAction) { // Importing image sequence from a folder
        if (picWidth > projectWidth || picHeight > projectHeight) {
            text = text + "\n" + tr("Files are too big, so they will be resized.") + "\n"
                   + tr("Note: This task can take a while.");
            resize = true;
        }

        QMessageBox msgBox;
        msgBox.setWindowTitle(tr("Information"));
        msgBox.setIcon(QMessageBox::Question);
        msgBox.setText(text);
        msgBox.setInformativeText(tr("Do you want to continue?"));
        msgBox.setStandardButtons(QMessageBox::Cancel | QMessageBox::Ok);
        msgBox.setDefaultButton(QMessageBox::Ok);
        msgBox.show();

        msgBox.move(static_cast<int> ((screen->geometry().width() - msgBox.width()) / 2),
                    static_cast<int> ((screen->geometry().height() - msgBox.height()) / 2));

        int answer = msgBox.exec();
        if (answer == QMessageBox::Ok) {
            msgBox.close();
            QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

            verifyFramesAvailability(imagesTotal);

            QString directory = source.dirName();
            libraryTree->createFolder(directory);
            TupLibraryFolder *folder = new TupLibraryFolder(directory, project);
            library->addFolder(folder);

            int initFrame = currentFrame.frame;
            for(int i = 0; i < imagesTotal; i++) {
                importImageRecord(photograms.at(i), extension, QSize(picWidth, picHeight),
                                  QSize(projectWidth, projectHeight), resize, directory);
                if (i < imagesTotal-1) {
                    int layer = currentFrame.layer;
                    int frame = currentFrame.frame + 1;
                    QString selection = QString::number(layer) + "," + QString::number(layer) + ","
                                        + QString::number(frame) + "," + QString::number(frame);
                    TupProjectRequest request = TupRequestBuilder::createFrameRequest(currentFrame.scene, layer, frame,
                                                                    TupProjectRequest::Select, selection);
                    emit requestTriggered(&request);
                }
            }

            saveDefaultPath(path);
            QString selection = QString::number(currentFrame.layer) + "," + QString::number(currentFrame.layer) + ","
                                + QString::number(initFrame) + "," + QString::number(initFrame);
            TupProjectRequest request = TupRequestBuilder::createFrameRequest(currentFrame.scene, currentFrame.layer, initFrame,
                                                                              TupProjectRequest::Select, selection);
            emit requestTriggered(&request);

            QApplication::restoreOverrideCursor();
        }
    } else if (action == VideoAction) { // Importing images sequence extrated from video file
        verifyFramesAvailability(imagesTotal);

        QString directory = library->getFolderKey(tr("Video"));
        libraryTree->createFolder(directory);
        TupLibraryFolder *folder = new TupLibraryFolder(directory, project);
        library->addFolder(folder);

        int initFrame = currentFrame.frame;
        for(int i = 0; i < imagesTotal; i++) {
            importImageRecord(photograms.at(i), extension, QSize(picWidth, picHeight),
                              QSize(projectWidth, projectHeight), resizeFlag, directory);
            QString msg = tr("Importing image %1 of %2").arg(i).arg(imagesTotal);
            emit msgSent(msg);

            if (i < imagesTotal-1) {
                int layer = currentFrame.layer;
                int frame = currentFrame.frame + 1;
                QString selection = QString::number(layer) + "," + QString::number(layer) + ","
                                    + QString::number(frame) + "," + QString::number(frame);
                TupProjectRequest request = TupRequestBuilder::createFrameRequest(currentFrame.scene, layer, frame,
                                                                                  TupProjectRequest::Select, selection);
                emit requestTriggered(&request);
            }
        }

        saveDefaultPath(path);
        QString selection = QString::number(currentFrame.layer) + "," + QString::number(currentFrame.layer) + ","
                            + QString::number(initFrame) + "," + QString::number(initFrame);
        TupProjectRequest request = TupRequestBuilder::createFrameRequest(currentFrame.scene, currentFrame.layer, initFrame,
                                                                          TupProjectRequest::Select, selection);

        emit requestTriggered(&request);
        emit imagesImportationDone();

        // Removing temporary video file
        if (removeTempVideo && !tempVideoPath.isEmpty()) {
            QFileInfo info(tempVideoPath);
            QString tempDir = info.absolutePath();
            QDir videoDir(tempDir);
            #ifdef TUP_DEBUG
                qDebug() << "[TupLibraryWidget::loadSequenceFromDirectory()] - Removing temporary (video) folder -> "
                         << tempDir;
            #endif
            if (videoDir.exists()) {
                if (!videoDir.removeRecursively()) {
                    #ifdef TUP_DEBUG
                        qWarning() << "[TupLibraryWidget::loadSequenceFromDirectory()] - Error: Can't remove temporary folder -> "
                                   << tempDir;
                    #endif
                } else {
                    #ifdef TUP_DEBUG
                        qDebug() << "[TupLibraryWidget::loadSequenceFromDirectory()] - Temporary (video) folder was removed succesfully! -> "
                                 << tempDir;
                    #endif
                }
            }
            removeTempVideo = false;
        }
    }
}

void TupLibraryWidget::importSvgSequence() 
{
    TCONFIG->beginGroup("General");
    QString path = TCONFIG->value("DefaultPath", QDir::homePath()).toString();

    QFileDialog dialog(this, tr("Choose the SVG files directory..."), path);
    dialog.setFileMode(QFileDialog::Directory);
    dialog.setOption(QFileDialog::ShowDirsOnly);
    dialog.setOption(QFileDialog::DontResolveSymlinks);

    if (dialog.exec() == QDialog::Accepted) {
        QStringList files = dialog.selectedFiles();
        path = files.at(0);

        QDir source(path); 
        QFileInfoList records = source.entryInfoList(QDir::Files, QDir::Name);
        int filesTotal = records.size();
        QStringList photograms;

        // Ensuring to get only SVG files here. Check extension! (SVG)
        int svgCounter = 0;
        for (int i = 0; i < filesTotal; ++i) {
             if (records.at(i).isFile()) {
                 QString extension = records.at(i).suffix().toUpper();
                 if (extension.compare("SVG")==0) {
                     svgCounter++;
                     photograms << records.at(i).absoluteFilePath(); 
                 }
             }
        }

        if (svgCounter > 0) {
            QString text = tr("%1 SVG files will be loaded.").arg(svgCounter);

            QMessageBox msgBox;
            msgBox.setWindowTitle(tr("Information"));  
            msgBox.setIcon(QMessageBox::Question);
            msgBox.setText(text);
            msgBox.setInformativeText(tr("Do you want to continue?"));
            msgBox.setStandardButtons(QMessageBox::Cancel | QMessageBox::Ok);
            msgBox.setDefaultButton(QMessageBox::Ok);
            msgBox.show();
            msgBox.move(static_cast<int> ((screen->geometry().width() - msgBox.width()) / 2),
                        static_cast<int> ((screen->geometry().height() - msgBox.height()) / 2));

            int answer = msgBox.exec();

            if (answer == QMessageBox::Ok) {
                msgBox.close();
                verifyFramesAvailability(filesTotal);
                QString directory = source.dirName();
                libraryTree->createFolder(directory);

                QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

                TupLibraryFolder *folder = new TupLibraryFolder(directory, project);
                library->addFolder(folder);

                photograms = TAlgorithm::naturalSort(photograms);

                int initFrame = currentFrame.frame;
                filesTotal = photograms.size();
                QByteArray data;
                for (int i = 0; i < filesTotal; ++i) {
                     QFile file(photograms.at(i));
                     QFileInfo fileInfo(file);
                     QString extension = fileInfo.suffix().toUpper();
                     if (extension.compare("SVG")==0) {
                         QString symName = fileInfo.fileName().toLower();
                         symName = symName.replace("(","_");
                         symName = symName.replace(")","_");

                         if (file.open(QIODevice::ReadOnly)) {
                             data = file.readAll();
                             file.close();

                             TupProjectRequest request = TupRequestBuilder::createLibraryRequest(TupProjectRequest::Add, symName,
                                                                            TupLibraryObject::Svg, project->spaceContext(), data, directory);
                             emit requestTriggered(&request);
                             if (i < filesTotal-1) {
                                 int layer = currentFrame.layer;
                                 int frame = currentFrame.frame + 1;
                                 QString selection = QString::number(layer) + "," + QString::number(layer) + ","
                                                     + QString::number(frame) + "," + QString::number(frame);
                                 request = TupRequestBuilder::createFrameRequest(currentFrame.scene, layer, frame,
                                                                                 TupProjectRequest::Select, selection);
                                 emit requestTriggered(&request);
                             }
                         } else {
                             QMessageBox::critical(this, tr("ERROR!"), tr("ERROR: Can't open file %1. Please, check file permissions and try again.").arg(symName), QMessageBox::Ok);
                             QApplication::restoreOverrideCursor();
                             return;
                         }
                     }
                }
                saveDefaultPath(path);
                
                QString selection = QString::number(currentFrame.layer) + "," + QString::number(currentFrame.layer) + ","
                                    + QString::number(initFrame) + "," + QString::number(initFrame);
                TupProjectRequest request = TupRequestBuilder::createFrameRequest(currentFrame.scene, currentFrame.layer, initFrame,
                                                                                  TupProjectRequest::Select, selection);
                emit requestTriggered(&request);
                QApplication::restoreOverrideCursor();
            }
        } else {
            TOsd::self()->display(TOsd::Error, tr("No SVG files were found.<br/>Please, try another directory"));
        }
    }
}

void TupLibraryWidget::importSoundFile()
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupLibraryWidget::importSoundFile()]";
    #endif

    TupSoundDialog *soundDialog = new TupSoundDialog();

    connect(soundDialog, SIGNAL(soundFilePicked(const QString &)),
            this, SLOT(importLocalSoundFile(const QString &)));
    connect(soundDialog, &TupSoundDialog::lipsyncModuleCalled,
            this, &TupLibraryWidget::lipsyncModuleCalled);
    soundDialog->show();
}

void TupLibraryWidget::importVideoFile()
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupLibraryWidget::importVideoFile()]";
    #endif

    TCONFIG->beginGroup("General");
    QString videoPath = TCONFIG->value("DefaultPath", QDir::homePath()).toString();

    QFileDialog dialog(this, tr("Choose a video file..."), videoPath, "(*.mp4 *.mov)");
    dialog.setFileMode(QFileDialog::ExistingFile);

    if (dialog.exec() == QDialog::Accepted) {
        QStringList files = dialog.selectedFiles();
        importVideoFileFromFolder(files.at(0));
    }
}

void TupLibraryWidget::importSoundFileFromByteArray(const QString &filename, QByteArray data, const QString &folder)
{
    isEffectSound = true;
    TupProjectRequest request = TupRequestBuilder::createLibraryRequest(TupProjectRequest::Add, filename,
                                                   TupLibraryObject::Audio, project->spaceContext(), data, folder);
    emit requestTriggered(&request);
}

void TupLibraryWidget::importVideoFileFromByteArray(const QString &filename, QByteArray data)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupLibraryWidget::importVideoFileFromByteArray()] - filename -> " << filename;
    #endif

    removeTempVideo = true;
    QString tempFolder = TAlgorithm::randomString(10);
    tempVideoPath = CACHE_DIR + tempFolder;
    if (!QFile::exists(tempVideoPath)) {
        QDir dir;
        if (!dir.mkpath(tempVideoPath)) {
            #ifdef TUP_DEBUG
                qWarning() << "[TupLibraryWidget::importVideoFileFromByteArray()] - Fatal Error: Couldn't create directory -> "
                           << tempVideoPath;
            #endif
            TOsd::self()->display(TOsd::Error, tr("Couldn't create temporary directory!"));

            return;
        }
    }

    tempVideoPath += "/" + filename;

    QFile videoFile(tempVideoPath);
    if (videoFile.open(QIODevice::WriteOnly)) {
        videoFile.write(data);
        videoFile.close();

        importVideoFileFromFolder(tempVideoPath);
    } else {
        #ifdef TUP_DEBUG
            qWarning() << "[TupLibraryWidget::importVideoFileFromByteArray()] - Fatal Error: Can't save temporary video file -> "
                       << tempVideoPath;
        #endif
        videoFile.close();
    }
}

void TupLibraryWidget::importLocalSoundFile(const QString &filePath)
{
    importSoundFileFromFolder(filePath);
}

void TupLibraryWidget::importSoundFileFromFolder(const QString &filePath, const QString &folder)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupLibraryWidget::importSoundFileFromFolder()] - filePath -> " << filePath;
        qDebug() << "[TupLibraryWidget::importSoundFileFromFolder()] - folder -> " << folder;
    #endif

    QFile file(filePath);
    if (file.open(QIODevice::ReadOnly)) {
        QByteArray data = file.readAll();
        file.close();

        QFileInfo fileInfo(file);
        QString key = library->getItemKey(fileInfo.fileName().toLower());
        importSoundFileFromByteArray(key, data, folder);
        setDefaultPath(filePath);
    } else {
        file.close();
        TOsd::self()->display(TOsd::Error, tr("Error while opening file: %1").arg(filePath));
    }
}

void TupLibraryWidget::importVideoFileFromFolder(const QString &videoPath)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupLibraryWidget::importVideoFileFromFolder()] - videoPath -> " << videoPath;
    #endif

    QFile file(videoPath);
    double fileSize = static_cast<double>(file.size()) / static_cast<double>(1000000);
    if (fileSize <= 2) {
        TupVideoCutter *videoCutter = new TupVideoCutter();
        QString tempFolder = TAlgorithm::randomString(10);
        QString imagesPath = CACHE_DIR + tempFolder + "/";
        if (videoCutter->loadFile(videoPath, imagesPath)) {
            TupVideoImporterDialog *dialog = new TupVideoImporterDialog(videoPath, imagesPath, project->getDimension(), videoCutter);
            connect(dialog, SIGNAL(extractionDone(ImportAction, const QString &, bool)),
                    SLOT(loadSequenceFromDirectory(ImportAction, const QString &, bool)));
            connect(dialog, SIGNAL(projectSizeHasChanged(const QSize)), this, SIGNAL(projectSizeHasChanged(const QSize)));
            connect(this, SIGNAL(imagesImportationDone()), dialog, SLOT(endProcedure()));
            connect(this, SIGNAL(msgSent(const QString &)), dialog, SLOT(updateStatus(const QString &)));

            dialog->show();
        } else {
            TOsd::self()->display(TOsd::Error, tr("Can't load video file!"));
        }
    } else {
        TOsd::self()->display(TOsd::Error, tr("Video file is larger than 2 MB. Too big!"));
    }
}

void TupLibraryWidget::sceneResponse(TupSceneResponse *response)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupLibraryWidget::sceneResponse()] - response->action() -> " << response->getAction();
    #endif

    switch (response->getAction()) {
        case TupProjectRequest::Select:
        {
            currentFrame.frame = 0;
            currentFrame.layer = 0;
            currentFrame.scene = response->getSceneIndex();
        }
        break;
        case TupProjectRequest::Add:
        case TupProjectRequest::Remove:
        {
            qDebug() << "[TupLibraryWidget::sceneResponse()] - project->getSceneNames() ->" << project->getSceneNames();

            if (project->hasLibrarySounds()) {
                if (display->isSoundPanelVisible())
                    display->setSoundParams(currentSound->getSoundResourceParams(), project->getSceneNames(),
                                            project->getFrameLimits());
            }
        }
        break;
    }
}

void TupLibraryWidget::layerResponse(TupLayerResponse *event)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupLibraryWidget::layerResponse()] - event -> " << event->getAction();
    #endif

    switch(event->getAction()) {
        case TupProjectRequest::AddLipSync:
        case TupProjectRequest::UpdateLipSync:
        {
            TupLipSync *lipSync = new TupLipSync();
            lipSync->fromXml(event->getArg().toString());

            QString soundID = lipSync->getSoundFile();
            int frameIndex = lipSync->getInitFrame();
            TupLibraryObject *sound = library->getObject(soundID);
            if (sound) {
                QList<int> frames;
                frames << frameIndex;
                sound->updateFramesToPlay(currentFrame.scene, frames);
                // if (display->getSoundID().compare(soundID) == 0)
                //     display->updateSoundInitFrame(frameIndex + 1);
            } else {
                #ifdef TUP_DEBUG
                    qDebug() << "[TupLibraryWidget::layerResponse()] - "
                                "Fatal Error: Can't find audio item -> " << soundID;
                #endif
            }
        }
        break;
        case TupProjectRequest::RemoveLipSync:
        {
            if (display->isSoundPanelVisible()) {
                QString id = event->getArg().toString();
                TupLibraryObject *sound = library->findSoundFile(id);
                if (sound) {
                    QString currentId = display->getSoundID();
                    if (currentId.compare(sound->getSymbolName()) == 0) {
                        // display->enableLipSyncInterface(sound->getSoundType(), sound->frameToPlay() + 1);
                        display->enableLipSyncInterface(sound->getSoundType(), sound->getAudioScenes());
                    }
                } else {
                    #ifdef TUP_DEBUG
                        qDebug() << "[TupLibraryWidget::layerResponse()] - "
                                    "Fatal Error: Can't find audio item -> " << id;
                    #endif
                }
            }
        }
        break;
        default:
        break;
    }
}

void TupLibraryWidget::libraryResponse(TupLibraryResponse *response)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupLibraryWidget::libraryResponse()] - response->action() ->"
                 << response->getAction();
    #endif

    RETURN_IF_NOT_LIBRARY;

    switch (response->getAction()) {
        case TupProjectRequest::Add:
          {
             if (response->symbolType() == TupLibraryObject::Folder) {
                 libraryTree->createFolder(response->getArg().toString());
                 return;
             }

             QString folderName = response->getParent();
             QString id = response->getArg().toString();

             #ifdef TUP_DEBUG
                 qDebug() << "[TupLibraryWidget::libraryResponse()] - response->getArg() ->" << id;
             #endif

             int index = id.lastIndexOf(".");
             QString name = id.mid(0, index);
             QString extension = id.mid(index + 1, id.length() - index).toUpper();
             TupLibraryObject *object = library->getObject(id);

             if (index < 0)
                 extension = "TOBJ";

             QTreeWidgetItem *item;
             if (folderName.length() > 0 && folderName.compare("library") != 0)
                 item = new QTreeWidgetItem(libraryTree->getFolder(folderName));
             else
                 item = new QTreeWidgetItem(libraryTree);

             item->setText(1, name);
             item->setText(2, extension);
             item->setText(3, id);
             item->setFlags(item->flags() | Qt::ItemIsEditable | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled);

             if (object) {
                 switch (object->getObjectType()) {
                     case TupLibraryObject::Item:
                       {
                         if (object->getItemType() != TupLibraryObject::Text && object->getItemType() != TupLibraryObject::Path)
                             nativeMap[id] = TupLibraryObject::generateImage(object->toString(), width());

                         item->setIcon(0, QIcon(THEME_DIR + "icons/drawing_object.png"));
                         libraryTree->setCurrentItem(item);
                         previewItem(item);

                         if (nativeFromFileSystem) {
                             if (!isNetworked && !folderName.endsWith(".pgo") && !library->isLoadingAssets()
                                 && !isExternalLibraryAsset)
                                 insertObjectInWorkspace();
                             nativeFromFileSystem = false;
                         } else {
                             if ((currentMode == TupProject::VECTOR_STATIC_BG_MODE
                                  || currentMode == TupProject::VECTOR_DYNAMIC_BG_MODE)
                                 && !isExternalLibraryAsset)
                             insertObjectInWorkspace();
                         }
                       }
                     break;
                     case TupLibraryObject::Image:
                       {
                         item->setIcon(0, QIcon(THEME_DIR + "icons/bitmap.png"));
                         libraryTree->setCurrentItem(item);
                         previewItem(item);

                         if (!isNetworked && !folderName.endsWith(".pgo") && !library->isLoadingAssets()
                             && folderName.compare(tr("Raster Objects")) != 0 && !isExternalLibraryAsset)
                             insertObjectInWorkspace();
                       }
                     break;
                     case TupLibraryObject::Svg:
                       {
                         item->setIcon(0, QIcon(THEME_DIR + "icons/svg.png"));
                         libraryTree->setCurrentItem(item);
                         previewItem(item);
                         if (!isNetworked && !library->isLoadingAssets() && !isExternalLibraryAsset)
                             insertObjectInWorkspace();
                       }
                     break;
                     case TupLibraryObject::Audio:
                       {
                         if (!library->isLoadingAssets()) {
                             if (isEffectSound) {
                                 library->updateObjectSoundType(id, Effect);
                                 isEffectSound = false;
                                 library->updateSoundFramesToPlay(id, currentFrame.scene,
                                                                  object->framesToPlayAt(currentFrame.scene));
                             } else {
                                 library->updateObjectSoundType(id, Lipsync);
                             }
                         }

                         library->registerSoundResource(id);
                         item->setIcon(0, QIcon(THEME_DIR + "icons/sound_object.png"));
                         libraryTree->setCurrentItem(item);
                         previewItem(item);
                       }
                     break;
                     default:
                       {
                       }
                     break;
                 }

                 if (isExternalLibraryAsset)
                     isExternalLibraryAsset = false;
             } else {
                 #ifdef TUP_DEBUG
                     qDebug() << "[TupLibraryWidget::libraryResponse()] - No object found ->" << id;
                 #endif
             }
          }
        break;
        case TupProjectRequest::InsertSymbolIntoFrame:
          {
             #ifdef TUP_DEBUG
                 qDebug() << "[TupLibraryWidget::libraryResponse()] - InsertSymbolIntoFrame -> No action taken";
             #endif
          }
        break;
        case TupProjectRequest::RemoveSymbolFromFrame:
          {
             #ifdef TUP_DEBUG
                 qDebug() << "[TupLibraryWidget::libraryResponse()] - RemoveSymbolFromFrame -> No action taken";
             #endif
          }
        break;
        case TupProjectRequest::Remove:
          {
             QString id = response->getArg().toString();
             #ifdef TUP_DEBUG
                 qDebug() << "[TupLibraryWidget::libraryResponse()] - Removing item ->" << id;
             #endif

             bool isFolder = false;
             QTreeWidgetItemIterator it(libraryTree);
             while ((*it)) {
                    // If target is NOT a folder
                    if ((*it)->text(2).length() > 0) {
                        if (id == (*it)->text(3)) {
                            delete (*it);
                            break;
                        }
                    } else {
                        // If target is a folder
                        if (id == (*it)->text(1)) {
                            isFolder = true;
                            delete (*it);
                            break;
                        }
                    }
                    ++it;
             }

             if (!isFolder) {
                 if (response->symbolType() == TupLibraryObject::Audio) {
                     if (project->removeSoundResource(id)) {
                         resetSoundPlayer();
                     } else {
                         #ifdef TUP_DEBUG
                             qWarning() << "[TupLibraryWidget::libraryResponse()] - "
                                           "Warning: Can't remove sound resource record ->" << id;
                             #endif
                     }
                 }
             }

             if (libraryTree->topLevelItemCount() > 0) {
                 previewItem(libraryTree->currentItem());
             } else  {
                 display->showDisplay();
                 display->reset();
             }
          }
        break;
        default:
          {
             #ifdef TUP_DEBUG
                 qDebug() << "[TupLibraryWidget::libraryResponse()] - Unknown/Unhandled project action ->"
                             << response->getAction();
             #endif
          }
        break;
    }
}

void TupLibraryWidget::updateCurrentFrameIndex(int sceneIndex, int layerIndex, int frameIndex)
{
    currentFrame.scene = sceneIndex;
    currentFrame.layer = layerIndex;
    currentFrame.frame = frameIndex;
}

void TupLibraryWidget::frameResponse(TupFrameResponse *response)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupLibraryWidget::frameResponse()] - frameIndex -> "
                 << response->getFrameIndex() << " - action -> " << response->getAction();
    #endif

    int sceneIndex = response->getSceneIndex();
    int layerIndex = response->getLayerIndex();
    int frameIndex = response->getFrameIndex();

    updateCurrentFrameIndex(sceneIndex, layerIndex, frameIndex);

    switch (response->getAction()) {
        case TupProjectRequest::Add:
        case TupProjectRequest::Remove:
        case TupProjectRequest::RemoveSelection:
        {
            if (project->hasLibrarySounds()) {
                if (project->sceneAt(sceneIndex)) {
                    int framesCount = project->sceneAt(sceneIndex)->framesCount();
                     if (display->isSoundPanelVisible())
                         display->updateFrameLimit(sceneIndex, framesCount);
                }
            }
        }
        break;
    }

    qDebug() << "[TupLibraryWidget::frameResponse()] - END!";
}

void TupLibraryWidget::importLibraryObject()
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupLibraryWidget::importLibraryObject()]";
    #endif

    QString option = itemType->currentText();

    if (option.compare(tr("Image")) == 0) {
        importImageGroup();
        return;
    }

    if (option.compare(tr("Image Sequence")) == 0) {
        importImageSequence();
        return;
    }

    if (option.compare(tr("Svg File")) == 0) {
        importSvgGroup();
        return;
    }

    if (option.compare(tr("Svg Sequence")) == 0) {
        importSvgSequence();
        return;
    }

    if (option.compare(tr("Native Object")) == 0) {
        importNativeObjects();
        return;
    }

    if (option.compare(tr("Audio File")) == 0) {
        importSoundFile();
        return;
    }

    if (option.compare(tr("Video File")) == 0) {
        importVideoFile();
        return;
    }
}

void TupLibraryWidget::refreshItem(QTreeWidgetItem *item)
{
    if (!item)
        return;

    if (mkdir) {
        mkdir = false;

        QString base = item->text(1);
        if (base.length() == 0)
            return;

        QString tag = base;
        int i = 0;
        while (library->folderExists(tag)) {
               int index = tag.lastIndexOf("-");
               if (index < 0) {
                   tag = base + "-1";
               } else {
                   QString name = base.mid(0, index);
                   i++;
                   tag = name + "-" + QString::number(i);
               }
        }

        item->setText(1, tag);

        TupLibraryFolder *folder = new TupLibraryFolder(tag, project);
        library->addFolder(folder);

        QGraphicsTextItem *msg = new QGraphicsTextItem(tr("Directory"));
        display->render(false, static_cast<QGraphicsItem *>(msg));

        editorItems << tag;

        return;
    }

    if (renaming) {
        // Renaming directory
        if (libraryTree->isFolder(item)) {
            QString base = item->text(1);
            if (oldId.length() == 0 || base.length() == 0)
                return;

            if (oldId.compare(base) == 0)
                return;

            int i = 0;
            QString tag = base;
            while (library->folderExists(tag)) {
                   int index = tag.lastIndexOf("-");
                   if (index < 0) {
                       tag = base + "-1";
                   } else {
                       QString name = base.mid(0, index);
                       i++;
                       tag = name + "-" + QString::number(i);
                   }
            }

            if (!library->folderExists(tag)) {
                // rename directory here!
                if (library->folderExists(oldId)) {
                    bool renamed = library->renameFolder(oldId, tag);
                    if (renamed)
                        item->setText(1, tag);
                }
            } 
        } else {
            // Renaming item
            if (oldId.length() == 0)
                return;

            QString newId = item->text(1);
            QString extension = item->text(2);

            if (oldId.compare(newId) != 0) {
                newId = verifyNameAvailability(newId, extension, false);
                QString oldRef = oldId + "." + extension.toLower();
                item->setText(1, newId);

                newId = newId + "." + extension.toLower();
                item->setText(3, newId);

                QTreeWidgetItem *parent = item->parent();
                if (parent) 
                    library->renameObject(parent->text(1), oldRef, newId);
                else
                    library->renameObject("", oldRef, newId);

                TupLibraryObject::ObjectType type = TupLibraryObject::Image;
                if (extension.compare("SVG")==0)
                    type = TupLibraryObject::Svg;
                if (extension.compare("TOBJ")==0)
                    type = TupLibraryObject::Item;

                project->updateSymbolId(type, oldRef, newId);
            }
        }

        renaming = false;
    }
}

void TupLibraryWidget::updateLibrary(QString file, QString folder) 
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupLibraryWidget::updateLibrary()] - folder -> " << folder;
        qDebug() << "[TupLibraryWidget::updateLibrary()] - file -> " << file;
    #endif

    if (folder.length() > 0)
        library->moveObject(file, folder);
    else
        library->moveObjectToRoot(file);
}

void TupLibraryWidget::openInkscapeToEdit(QTreeWidgetItem *item)
{
    callExternalEditor(item, "Inkscape");
}

void TupLibraryWidget::openGimpToEdit(QTreeWidgetItem *item)
{
    callExternalEditor(item, "Gimp");
}

void TupLibraryWidget::openKritaToEdit(QTreeWidgetItem *item)
{
    callExternalEditor(item, "Krita");
}

void TupLibraryWidget::openMyPaintToEdit(QTreeWidgetItem *item)
{
    callExternalEditor(item, "MyPaint");
}

void TupLibraryWidget::callExternalEditor(QTreeWidgetItem *item, const QString &software)
{
    if (item) {
        lastItemEdited = item;
        QString id = item->text(1) + "." + item->text(2).toLower();
        TupLibraryObject *object = library->getObject(id);

        if (object) {
            QString path = object->getDataPath();
            executeSoftware(software, path);
        } else {
            #ifdef TUP_DEBUG
                qDebug() << "[TupLibraryWidget::callExternalEditor()] - Fatal Error: No object related to the current library item was found! -> " << id;
            #endif
        }
    } else {
        #ifdef TUP_DEBUG
            qDebug() << "[TupLibraryWidget::callExternalEditor()] - Error: Current library item is invalid!";
        #endif
    }
}

void TupLibraryWidget::executeSoftware(const QString &software, QString &path)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupLibraryWidget::executeSoftware()] - Application -> " << software;
        qDebug() << "[TupLibraryWidget::executeSoftware()] - File Path -> " << path;
    #endif

    if (path.length() > 0 && QFile::exists(path)) {
        QString program = "/usr/bin/" + software.toLower(); 

        QStringList arguments;
        arguments << path;

        QProcess *editor = new QProcess(this);
        editor->start(program, arguments);

        // SQA: Check the path list and if it doesn't exist yet, then add it to 
        watcher->addPath(path);
    } else {
        #ifdef TUP_DEBUG
            qDebug() << "[TupLibraryWidget::executeSoftware()] - Fatal Error: Item path either doesn't exist or is empty";
        #endif
    }
}

void TupLibraryWidget::refreshItemFromCollection(LibraryObjects collection)
{
    QMapIterator<QString, TupLibraryObject *> i(collection);
    while (i.hasNext()) {
       i.next();
       TupLibraryObject *object = i.value();
       if (object) {
           updateItem(object->getShortId(), object->getExtension().toLower(), object);
       } else {
           #ifdef TUP_DEBUG
               qDebug() << "[TupLibraryWidget::updateItemFromSaveAction()] - "
                           "Fatal Error: The library item modified was not found!";
           #endif
       }
    }
}

void TupLibraryWidget::updateItemFromSaveAction()
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupLibraryWidget::updateItemFromSaveAction()]";
    #endif

    refreshItemFromCollection(library->getObjects());

    foreach (TupLibraryFolder *folder, library->getFolders()) {
        LibraryObjects objects = folder->getObjects();
        refreshItemFromCollection(objects);
    }

    QString selection = QString::number(currentFrame.layer) + "," + QString::number(currentFrame.layer) + ","
                        + QString::number(currentFrame.frame) + "," + QString::number(currentFrame.frame);
    TupProjectRequest request = TupRequestBuilder::createFrameRequest(currentFrame.scene, currentFrame.layer, currentFrame.frame,
                                                                      TupProjectRequest::Select, selection);
    emit requestTriggered(&request);
}

void TupLibraryWidget::updateItem(const QString &name, const QString &extension, TupLibraryObject *object)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupLibraryWidget::updateItem()]";
    #endif

    QString onEdition = name + "." + extension;
    QString onDisplay = currentItemDisplayed->text(1) + "." + currentItemDisplayed->text(2).toLower();

    TupLibraryObject::ObjectType type = TupLibraryObject::Image;
    if (extension.compare("svg") == 0)
        type = TupLibraryObject::Svg;

    if (library->reloadObject(onEdition)) {
        project->reloadLibraryItem(type, onEdition, object);

        if (onDisplay.compare(onEdition) == 0)
            previewItem(lastItemEdited);
    } else {
        #ifdef TUP_DEBUG
            qDebug() << "[TupLibraryWidget::updateItemFromSaveAction()] - "
                        "Fatal Error: Couldn't reload item from Library!";
        #endif
    }
}

bool TupLibraryWidget::itemNameEndsWithDigit(QString &name)
{
    QByteArray array = name.toLocal8Bit();

    QChar letter(array.at(array.size() - 1));
    if (letter.isDigit())
        return true;

    return false;
}

int TupLibraryWidget::getItemNameIndex(QString &name) const
{
    QByteArray array = name.toLocal8Bit();
    int index = 0;
    for (int i = array.size()-1; i >= 0; i--) {
        QChar letter(array.at(i));
        if (!letter.isDigit()) {
            index = i + 1;
            break;
        }
    }

    return index;
}

QString TupLibraryWidget::nameForClonedItem(QString &name, QString &extension, int index, QString &path) const
{
    QString symbolName = "";

    QString base = name.left(index);
    QString right = name.right(index);
    int counter = right.toInt();

    while (true) {
        counter++;
        QString number = QString::number(counter);
        if (counter < 10)
           number = "0" + number;
        symbolName = base + number + "." + extension.toLower();
        QString tester = path + symbolName;
        if (!QFile::exists(tester))
           break;
    }

    return symbolName;
}

QString TupLibraryWidget::nameForClonedItem(QString &smallId, QString &extension, QString &path) const
{
    QString symbolName = "";
    int index = 0;

    while(true) {
        QString number = QString::number(index);
        if (index < 10)
          number = "0" + number;

        QString base = smallId + number;
        symbolName = base + "." + extension.toLower();
        QString tester = path + symbolName;

        if (!QFile::exists(tester))
          break;

        index++;
    }

    return symbolName;
}

QString TupLibraryWidget::verifyNameAvailability(QString &name, QString &extension, bool isCloningAction) {

    int limit = 1;
    if (isCloningAction)
        limit = 0; 

    QList<QTreeWidgetItem *> list = libraryTree->findItems(name, Qt::MatchExactly, 1);
    if (list.size() > limit) {
        int total = 0;
        QTreeWidgetItem *node;
        for (int i=0; i<list.size(); i++) {
             node = list.at(i);
             if (node->text(2).compare(extension) == 0)
                 total++;
        }

        if (total > limit) {
            bool ok = false;
            if (itemNameEndsWithDigit(name)) {
                int index = getItemNameIndex(name);
                QString base = name.left(index);
                QString right = name.right(name.length() - index);
                int counter = right.toInt(&ok);
                if (ok) {
                    while (true) {
                           counter++;
                           QString number = QString::number(counter);
                           if (counter < 10)
                               number = "0" + number;
                           name = base + number;
                           QList<QTreeWidgetItem *> others = libraryTree->findItems(name, Qt::MatchExactly, 1);
                           if (others.size() == 0)
                               break;
                    }
                } else {
                    name = TAlgorithm::randomString(8);
                    #ifdef TUP_DEBUG
                        qWarning() << "[TupLibraryWidget::verifyNameAvailability()] - Warning: error while processing item name!";
                    #endif
                }
            } else {
                int index = name.lastIndexOf("-");
                if (index < 0) {
                    name += "-1";
                } else {
                    QString first = name.mid(0, index);
                    QString last = name.mid(index+1, name.length() - index);
                    int newIndex = last.toInt(&ok);
                    if (ok) {
                        newIndex++;
                        name = first + "-" + QString::number(newIndex);
                    } else {
                        name = TAlgorithm::randomString(8);
                        #ifdef TUP_DEBUG
                            qWarning() << "[TupLibraryWidget::verifyNameAvailability()] - Warning: error while processing item name!";
                        #endif
                    }
                }
            }
        }
    }

    return name;
}

void TupLibraryWidget::setDefaultPath(const QString &path)
{
    int last = path.lastIndexOf("/");
    QString dir = path.left(last);
    saveDefaultPath(dir);
}

void TupLibraryWidget::saveDefaultPath(const QString &dir)
{
    TCONFIG->beginGroup("General");
    TCONFIG->setValue("DefaultPath", dir);
    TCONFIG->sync();
}

void TupLibraryWidget::callLipsyncEditor(QTreeWidgetItem *item)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupLibraryWidget::callLipsyncEditor()]";
    #endif

    QString id = item->text(1) + "." + item->text(2).toLower();
    QString audioPath = library->getObjectPath(id);
    emit lipsyncModuleCalled(AudioFromLibrary, audioPath);
}

/*
void TupLibraryWidget::updateSoundTiming(int frame)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupLibraryWidget::updateSoundTiming()] - frame -> " << frame;
    #endif

    if (currentSound) {
        currentSound->updateFrameToPlay(frame);
        if (!project->updateSoundResourcesItem(currentSound)) {
            #ifdef TUP_DEBUG
                qWarning() << "[TupLibraryWidget::updateSoundTiming()] - "
                              "Warning: Can't update audio object -> " << currentSound->getSymbolName();
            #endif
        }
        emit soundUpdated();
    }
}
*/

void TupLibraryWidget::updateSoundMuteStatus(bool mute)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupLibraryWidget::updateSoundMuteStatus()] - mute ->" << mute;
    #endif

    if (currentSound) {
        currentSound->enableMute(mute);
        project->updateSoundResourcesItem(currentSound);
        emit soundUpdated();
    }
}

void TupLibraryWidget::updateSoundResource(SoundResource params)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupLibraryWidget::updateSoundResource()]"
                    " - params.isBackgroundTrack ->" << params.isBackgroundTrack;
        qDebug() << "[TupLibraryWidget::updateSoundResource()]"
                    " - params.scenes.count() ->" << params.scenes.count();
        foreach(SoundScene scene, params.scenes) {
            qDebug() << "[TupLibraryWidget::updateSoundResource()] - scene index ->" << scene.sceneIndex;
            foreach(int frameIndex, scene.frames)
                qDebug() << "[TupLibraryWidget::updateSoundResource()] - frame index ->" << frameIndex;
        }
    #endif

    if (currentSound) {
        currentSound->updateSoundResourceParams(params);
        project->updateSoundResourcesItem(currentSound);
        emit soundUpdated();
    }
}

void TupLibraryWidget::stopSoundPlayer()
{
    if (display)
        display->stopSoundPlayer();
}

void TupLibraryWidget::openSearchDialog()
{
    previousMode = currentMode;
    TupSearchDialog *dialog = new TupSearchDialog(project->getDimension());
    connect(dialog, SIGNAL(assetStored(const QString &, TupSearchDialog::AssetType, const QString &, int, QByteArray &)), this,
            SLOT(importAsset(const QString &, TupSearchDialog::AssetType, const QString &, int, QByteArray &)));
    connect(dialog, SIGNAL(accepted()), this, SLOT(recoverMode()));

    /* SQA: These connections don't work on Windows
    connect(dialog, &TupSearchDialog::assetStored, this, &TupLibraryWidget::importAsset);
    connect(dialog, &TupSearchDialog::accepted, this, &TupLibraryWidget::recoverMode);
    */
    dialog->show();
}

void TupLibraryWidget::recoverMode()
{
    currentMode = previousMode;
}

void TupLibraryWidget::importAsset(const QString &name, TupSearchDialog::AssetType assetType,
                                   const QString &extension, int extensionId, QByteArray &data)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupLibraryWidget::importAsset()] - name -> " << name;
    #endif

    QString key = name + "." + extension;
    int i = 0;
    while (library->exists(key)) {
        i++;
        key = name + "-" + QString::number(i) + "." + extension;
    }

    TupLibraryObject::ObjectType type = TupLibraryObject::Item;
    switch(extensionId) {
      case TupSearchDialog::JPG:
      case TupSearchDialog::PNG:
        {
            type = TupLibraryObject::Image;
        }
      break;
      case TupSearchDialog::SVG:
        {
            type = TupLibraryObject::Svg;
        }
      break;
      /*
      case TupSearchDialog::TOBJ:
        {
            type = TupLibraryObject::Item;
        }
      break;
      */
    }

    currentMode = project->spaceContext();
    if (assetType == TupSearchDialog::StaticBg)
        currentMode = TupProject::VECTOR_STATIC_BG_MODE;
    else if (assetType == TupSearchDialog::DynamicBg)
        currentMode = TupProject::VECTOR_DYNAMIC_BG_MODE;

    TupProjectRequest request = TupRequestBuilder::createLibraryRequest(TupProjectRequest::Add, key,
                                                   type, currentMode, data, QString(),
                                                   currentFrame.scene, currentFrame.layer, currentFrame.frame);
    emit requestTriggered(&request);
    data.clear();
}

void TupLibraryWidget::resetSoundPlayer()
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupLibraryWidget::resetSoundPlayer()]";
    #endif

    currentSound = nullptr;
    delete currentSound;

    if (display)
        display->resetSoundPlayer();
}

void TupLibraryWidget::updateSoundPlayer()
{
    #ifdef TUP_DEBUG
        qDebug() << "---";
        qDebug() << "[TupLibraryWidget::updateSoundPlayer()]";
    #endif

    if (currentSound) {
        QString key = currentSound->getSymbolName();
        currentSound = library->getObject(key);
        #ifdef TUP_DEBUG
            QString path = currentSound->getDataPath();
            qDebug() << "[TupLibraryWidget::updateSoundPlayer()] - Sound key -> " << key;
            qDebug() << "[TupLibraryWidget::updateSoundPlayer()] - Sound path -> " << path;
            if (QFile::exists(path))
                qDebug() << "[TupLibraryWidget::updateSoundPlayer()] - Sound file exists!";
            else
                qDebug() << "[TupLibraryWidget::updateSoundPlayer()] - Sound file doesn't exist!";
        #endif

        if (display) {
            if (display->isSoundPanelVisible()) {
                qDebug() << "[TupLibraryWidget::updateSoundPlayer()] - project->getSceneNames() ->" << project->getSceneNames();
                display->setSoundParams(currentSound->getSoundResourceParams(), project->getSceneNames(),
                                        project->getFrameLimits());
            }
        }
    }

    #ifdef TUP_DEBUG
        qDebug() << "---";
    #endif
}

void TupLibraryWidget::updateSoundItems()
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupLibraryWidget::updateSoundItems()]";
    #endif

    if (display) {
        if (display->isSoundPanelVisible()) {
            display->setSoundParams(currentSound->getSoundResourceParams(), project->getSceneNames(),
                                    project->getFrameLimits());
        } else {
            #ifdef TUP_DEBUG
                qDebug() << "[TupLibraryWidget::updateSoundItems()] - SoundPanel is NOT visible!";
            #endif
        }
    } else {
        #ifdef TUP_DEBUG
            qDebug() << "[TupLibraryWidget::updateSoundItems()] - display is NULL!";
        #endif
    }
}

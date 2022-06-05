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

#define RETURN_IF_NOT_LIBRARY if (!library) return;

TupLibraryWidget::TupLibraryWidget(QWidget *parent) : TupModuleWidgetBase(parent)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupLibraryWidget()]";
    #endif

    childCount = 0;
    renaming = false;
    mkdir = false;
    isEffectSound = false;
    currentMode = TupProject::FRAMES_MODE;
    nativeFromFileSystem = false;

    setWindowIcon(QPixmap(THEME_DIR + "icons/library.png"));
    setWindowTitle(tr("Library"));

    screen = QGuiApplication::screens().at(0);

    libraryDir = QDir(CONFIG_DIR + "libraries");

    display = new TupLibraryDisplay;
    connect(display, SIGNAL(frameUpdated(int)), this, SLOT(updateSoundTiming(int)));
    connect(display, SIGNAL(muteEnabled(bool)), this, SLOT(updateSoundMuteStatus(bool)));

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

    addChild(display);
    addChild(buttons);
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

    if (display)
        display->reset();

    if (library)
        library->reset();

    if (libraryTree)
        libraryTree->cleanUI();
}

void TupLibraryWidget::setLibrary(TupLibrary *assets)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupLibraryWidget::setLibrary()]";
    #endif

    library = assets;
    project = library->getProject();
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
            display->render(static_cast<QGraphicsItem *>(msg));
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
            display->render(static_cast<QGraphicsItem *>(text));

            return;
        }

        switch (object->getObjectType()) {
                case TupLibraryObject::Svg:
                   {
                     display->showDisplay();
                     QGraphicsSvgItem *svg = new QGraphicsSvgItem(object->getDataPath()); 
                     display->render(static_cast<QGraphicsItem *>(svg));
                   }
                   break;
                case TupLibraryObject::Image:
                   {
                     display->showDisplay();
                     display->render(qvariant_cast<QGraphicsItem *>(object->getData()));
                   }
                   break;
                case TupLibraryObject::Item:
                   {
                     display->showDisplay();
                     if (object->getItemType() == TupLibraryObject::Text || object->getItemType() == TupLibraryObject::Path) {
                         display->render(qvariant_cast<QGraphicsItem *>(object->getData()));
                     } else {
                         display->render(nativeMap[objectName]);
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

                     display->setSoundParams(object);
                     display->showSoundPlayer();
                   }
                   break;
                default:
                   {
                     #ifdef TUP_DEBUG
                         qDebug() << "[TupLibraryWidget::previewItem()] - Unknown symbol id -> " << object->getObjectType();
                     #endif
                   }
                   break;
        }
    } else {
        QGraphicsTextItem *msg = new QGraphicsTextItem(tr("No preview available"));
        display->render(static_cast<QGraphicsItem *>(msg));
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

    TupProjectRequest request = TupRequestBuilder::createLibraryRequest(TupProjectRequest::Remove, objectKey, type);
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

void TupLibraryWidget::importImage(const QString &imagePath)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupLibraryWidget::importImage()]";
    #endif

    if (imagePath.isEmpty())
        return;

    QFile imageFile(imagePath);

    if (imageFile.open(QIODevice::ReadOnly)) {
        QFileInfo fileInfo(imageFile);
        QString key = fileInfo.fileName().toLower();
        key = key.replace("(","_");
        key = key.replace(")","_");
        int index = key.lastIndexOf(".");

        QString name = key.mid(0, index);
        if (name.length() > 30)
            name = key.mid(0, 30);

        QString extension = key.mid(index, key.length() - index);
        QByteArray data = imageFile.readAll();
        imageFile.close();

        QPixmap *pixmap = new QPixmap(imagePath);
        int picWidth = pixmap->width();
        int picHeight = pixmap->height();
        int projectWidth = project->getDimension().width();
        int projectHeight = project->getDimension().height();

        #ifdef TUP_DEBUG
            qDebug() << "[TupLibraryWidget::importImage()] - Image filename: " << key << " | Raw Size: " << data.size();
            qDebug() << "[TupLibraryWidget::importImage()] - Image Size: " << "[" << picWidth << ", " << picHeight << "]"
                    << " | Project Size: " << "[" << projectWidth << ", " << projectHeight << "]";
        #endif

        if (picWidth > projectWidth || picHeight > projectHeight) {
            QMessageBox msgBox;
            msgBox.setWindowTitle(tr("Information"));
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
                QString extension = fileInfo.suffix().toUpper();
                QByteArray ba = extension.toLatin1();
                const char* ext = ba.data();
                if (pixmap->loadFromData(data, ext)) {
                    QPixmap newpix;
                    if (picWidth > picHeight) {
                        newpix = QPixmap(pixmap->scaledToHeight(projectHeight, Qt::SmoothTransformation));
                    } else {
                        newpix = QPixmap(pixmap->scaledToWidth(projectWidth, Qt::SmoothTransformation));
                    }
                    QBuffer buffer(&data);
                    buffer.open(QIODevice::WriteOnly);
                    newpix.save(&buffer, ext);
                }
            } 
        }

        int i = 0;
        while (library->exists(key)) {
            i++;
            key = name + "-" + QString::number(i) + extension;
        }

        TupProjectRequest request = TupRequestBuilder::createLibraryRequest(TupProjectRequest::Add, key,
                                                                            TupLibraryObject::Image, project->spaceContext(), data, QString(),
                                                                            currentFrame.scene, currentFrame.layer, currentFrame.frame);
        emit requestTriggered(&request);

        data.clear();
    } else {
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

void TupLibraryWidget::importSvg(const QString &svgPath)
{
    if (svgPath.isEmpty())
        return;

    QFile file(svgPath);

    if (file.open(QIODevice::ReadOnly)) {
        QFileInfo fileInfo(file);
        QString key = fileInfo.fileName().toLower();
        key = key.replace("(","_");
        key = key.replace(")","_");

        int index = key.lastIndexOf(".");
        QString name = key.mid(0, index);
        if (name.length() > 30)
            name = key.mid(0, 30);

        QString extension = key.mid(index, key.length() - index);

        QByteArray data = file.readAll();
        file.close();

        #ifdef TUP_DEBUG
            qDebug() << "[TupLibraryWidget::importSvg()] - Inserting SVG into project -> " << project->getName();
            int projectWidth = project->getDimension().width();
            int projectHeight = project->getDimension().height();
            qDebug() << "[TupLibraryWidget::importSvg()] - Project Size: [" << projectWidth << ","
                     << projectHeight << "]";
        #endif

        int i = 0;
        while (library->exists(key)) {
            i++;
            key = name + "-" + QString::number(i) + extension;
        }

        TupProjectRequest request = TupRequestBuilder::createLibraryRequest(TupProjectRequest::Add, key,
                                                       TupLibraryObject::Svg, project->spaceContext(), data, QString(),
                                                       currentFrame.scene, currentFrame.layer, currentFrame.frame);
        emit requestTriggered(&request);
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

void TupLibraryWidget::importNativeObject(const QString &object)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupLibraryWidget::importNativeObject()] - object -> ";
        qDebug() << object;
    #endif

    if (object.isEmpty()) {
        #ifdef TUP_DEBUG
            qDebug() << "[TupLibraryWidget::importNativeObject()] - Error: object data is empty!";
        #endif
        return;
    }

    QFile file(object);
    if (file.open(QIODevice::ReadOnly)) {
        QFileInfo fileInfo(file);
        QString key = fileInfo.fileName().toLower();
        key = key.replace("(","_");
        key = key.replace(")","_");
        QByteArray data = file.readAll();
        file.close();

        if (object.startsWith("<group") || object.startsWith("<rect") || object.startsWith("<ellipse"))
            nativeMap[key] = TupLibraryObject::generateImage(object, width());

        #ifdef TUP_DEBUG
            qDebug() << "[TupLibraryWidget::importNativeObject()] - Inserting native object into project -> "
                     << project->getName();
        #endif

        int i = 0;
        int index = key.lastIndexOf(".");
        QString name = key.mid(0, index);
        QString extension = key.mid(index, key.length() - index);
        while (library->exists(key)) {
            i++;
            key = name + "-" + QString::number(i) + extension;
        }

        TupProjectRequest request = TupRequestBuilder::createLibraryRequest(TupProjectRequest::Add, key,
                                                       TupLibraryObject::Item, project->spaceContext(), data, QString(),
                                                       currentFrame.scene, currentFrame.layer, currentFrame.frame);
        emit requestTriggered(&request);
    } else {
        TOsd::self()->display(TOsd::Error, tr("Cannot open file: %1").arg(object));
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
        TupProjectRequest request = TupRequestBuilder::createFrameRequest(currentFrame.scene, currentFrame.layer, initFrame,
                                                                          TupProjectRequest::Select);
        emit requestTriggered(&request);
    }
}

QStringList TupLibraryWidget::naturalSort(QStringList photograms)
{ 
    QCollator coll;
    coll.setNumericMode(true);
    for (int i = photograms.size()-1; i >= 0; i--) {
         for (int j = 1; j <= i; j++) {
              if (coll.compare(photograms.at(j-1), photograms.at(j)) > 0)
                  photograms.swapItemsAt(j-1, j);
         }
    }

    return photograms;
}

void TupLibraryWidget::importImageSequence()
{
    TCONFIG->beginGroup("General");
    QString path = TCONFIG->value("DefaultPath", QDir::homePath()).toString();

    QFileDialog dialog(this, tr("Choose the images directory..."), path);
    dialog.setFileMode(QFileDialog::Directory);
    dialog.setOption(QFileDialog::ShowDirsOnly);
    dialog.setOption(QFileDialog::DontResolveSymlinks);

    if (dialog.exec() == QDialog::Accepted) {
        QStringList files = dialog.selectedFiles();
        path = files.at(0);

        QDir source(path); 
        QFileInfoList records = source.entryInfoList(QDir::Files, QDir::Name);
        int filesTotal = records.size();

        // Ensuring to get only graphic files here. Check extensions! (PNG, JPG, GIF, XPM, WEBP)
        int imagesCounter = 0; 
        QStringList photograms;
        for (int i = 0; i < filesTotal; ++i) {
             if (records.at(i).isFile()) {
                 QString extension = records.at(i).suffix().toUpper();
                 if (extension.compare("JPEG")==0 || extension.compare("JPG")==0 || extension.compare("PNG")==0 ||
                     extension.compare("GIF")==0 || extension.compare("XPM")==0 || extension.compare("WEBP")==0) {
                     imagesCounter++;
                     photograms << records.at(i).absoluteFilePath();
                 }
             }
        }

        if (imagesCounter > 0) {
            QString text = tr("Image files found: %1.").arg(imagesCounter);
            bool resize = false;

            QPixmap *pixmap = new QPixmap(photograms.at(0));
            int picWidth = pixmap->width();
            int picHeight = pixmap->height(); 
            int projectWidth = project->getDimension().width();
            int projectHeight = project->getDimension().height();

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
                verifyFramesAvailability(filesTotal);

                QString directory = source.dirName();
                libraryTree->createFolder(directory);

                QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

                TupLibraryFolder *folder = new TupLibraryFolder(directory, project);
                library->addFolder(folder);

                photograms = naturalSort(photograms);

                int initFrame = currentFrame.frame;
                int filesTotal = photograms.size();
                QPixmap newpix;
                for (int i = 0; i < filesTotal; i++) {
                     QFile file(photograms.at(i));
                     QFileInfo fileInfo(file);
                     QString extension = fileInfo.suffix().toUpper();
                     if (extension.compare("JPEG")==0 || extension.compare("JPG")==0 || extension.compare("PNG")==0 ||
                         extension.compare("GIF")==0 || extension.compare("XPM")==0 || extension.compare("WEBP")==0) {
                         QString symName = fileInfo.fileName().toLower();
                         symName = symName.replace("(","_");
                         symName = symName.replace(")","_");

                         if (file.open(QIODevice::ReadOnly)) {
                             QByteArray data = file.readAll();
                             file.close();

                             if (resize) {
                                 pixmap = new QPixmap();
                                 QString extension = fileInfo.suffix().toUpper();
                                 QByteArray ba = extension.toLatin1();
                                 const char* ext = ba.data();
                                 if (pixmap->loadFromData(data, ext)) {
                                     if (picWidth > picHeight)
                                         newpix = QPixmap(pixmap->scaledToWidth(projectWidth, Qt::SmoothTransformation));
                                     else
                                         newpix = QPixmap(pixmap->scaledToHeight(projectHeight, Qt::SmoothTransformation));
                                     QBuffer buffer(&data);
                                     buffer.open(QIODevice::WriteOnly);
                                     newpix.save(&buffer, ext);
                                 }
                             }
                           
                             TupProjectRequest request = TupRequestBuilder::createLibraryRequest(TupProjectRequest::Add, symName,
                                                         TupLibraryObject::Image, project->spaceContext(), data, directory);
                             emit requestTriggered(&request);
                             if (i < filesTotal-1) {
                                 request = TupRequestBuilder::createFrameRequest(currentFrame.scene, currentFrame.layer, currentFrame.frame + 1,
                                                                                 TupProjectRequest::Select);
                                 emit requestTriggered(&request);
                             }

                             /*
                             progressDialog.setLabelText(tr("Loading image #%1").arg(index));
                             progressDialog.setValue(index);
                             index++;
                             */
                         } else {
                             QMessageBox::critical(this, tr("ERROR!"), tr("ERROR: Can't open file %1. Please, check file permissions and try again.").arg(symName), QMessageBox::Ok);
                             QApplication::restoreOverrideCursor();
                             return;
                         }
                     }
                }
                saveDefaultPath(path);
                TupProjectRequest request = TupRequestBuilder::createFrameRequest(currentFrame.scene, currentFrame.layer, initFrame,
                                                                                  TupProjectRequest::Select);
                emit requestTriggered(&request);

                QApplication::restoreOverrideCursor();
            }
        } else {
            TOsd::self()->display(TOsd::Error, tr("No image files were found.<br/>Please, try another directory"));
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

                photograms = naturalSort(photograms);

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
                                 request = TupRequestBuilder::createFrameRequest(currentFrame.scene, currentFrame.layer, currentFrame.frame + 1,
                                                                                 TupProjectRequest::Select);
                                 emit requestTriggered(&request);
                             }

                             /*
                             progressDialog.setLabelText(tr("Loading SVG file #%1").arg(index));
                             progressDialog.setValue(index);
                             index++;
                             */
                         } else {
                             QMessageBox::critical(this, tr("ERROR!"), tr("ERROR: Can't open file %1. Please, check file permissions and try again.").arg(symName), QMessageBox::Ok);
                             QApplication::restoreOverrideCursor();
                             return;
                         }
                     }
                }
                saveDefaultPath(path);
                
                TupProjectRequest request = TupRequestBuilder::createFrameRequest(currentFrame.scene, currentFrame.layer, initFrame,
                                                                                  TupProjectRequest::Select);
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
    connect(soundDialog, &TupSoundDialog::soundFilePicked,
            this, &TupLibraryWidget::importSoundFileFromFolder);
    connect(soundDialog, &TupSoundDialog::lipsyncModuleCalled,
            this, &TupLibraryWidget::lipsyncModuleCalled);
    soundDialog->show();
}

void TupLibraryWidget::importSoundFileFromFolder(const QString &filePath)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupLibraryWidget::importSoundFileFromFolder()] - filePath -> " << filePath;
    #endif

    QFile file(filePath);
    QFileInfo fileInfo(file);
    QString key = fileInfo.fileName().toLower();
    key = key.replace("(","_");
    key = key.replace(")","_");

    if (file.open(QIODevice::ReadOnly)) {
        QByteArray data = file.readAll();
        file.close();

        isEffectSound = true;
        TupProjectRequest request = TupRequestBuilder::createLibraryRequest(TupProjectRequest::Add, key,
                                                       TupLibraryObject::Audio, project->spaceContext(), data);
        emit requestTriggered(&request);
        setDefaultPath(filePath);
    } else {
        TOsd::self()->display(TOsd::Error, tr("Error while opening file: %1").arg(filePath));
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
    }
}

void TupLibraryWidget::layerResponse(TupLayerResponse *event)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupLibraryWidget::layerResponse()] - event -> " << event->getAction();
    #endif

    if (event->getAction() == TupProjectRequest::UpdateLipSync) {
        TupLipSync *lipSync = new TupLipSync();
        lipSync->fromXml(event->getArg().toString());

        QString soundID = lipSync->getSoundFile();
        int frameIndex = lipSync->getInitFrame();
        TupLibraryObject *sound = library->getObject(soundID);
        if (sound) {
            sound->updateFrameToPlay(frameIndex);
            if (display->isSoundPanelVisible()) {
                if (display->getSoundID().compare(soundID) == 0)
                    display->updateSoundInitFrame(frameIndex);
            }
        } else {
            #ifdef TUP_DEBUG
                qDebug() << "[TupLibraryWidget::layerResponse()] - Fatal Error: Can't find audio item -> " << soundID;
            #endif
        }
    } else if (event->getAction() == TupProjectRequest::RemoveLipSync) {
        if (display->isSoundPanelVisible()) {
            QString id = event->getArg().toString();
            TupLibraryObject *sound = library->findSoundFile(id);
            QString currentId = display->getSoundID();
            if (currentId.compare(sound->getSymbolName()) == 0)
                display->enableLipSyncInterface(sound->getSoundType(), sound->frameToPlay() + 1);
        }
    }
}

void TupLibraryWidget::libraryResponse(TupLibraryResponse *response)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupLibraryWidget::libraryResponse()] - response->action() -> " << response->getAction();
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
                 qDebug() << "[TupLibraryWidget::libraryResponse()] - response->getArg() -> " << id;
             #endif

             int index = id.lastIndexOf(".");
             QString name = id.mid(0, index);
             QString extension = id.mid(index + 1, id.length() - index).toUpper();
             TupLibraryObject *obj = library->getObject(id);

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

             if (obj) {
                 switch (obj->getObjectType()) {
                     case TupLibraryObject::Item:
                       {
                         if (obj->getItemType() != TupLibraryObject::Text && obj->getItemType() != TupLibraryObject::Path)
                             nativeMap[id] = TupLibraryObject::generateImage(obj->toString(), width());

                         item->setIcon(0, QIcon(THEME_DIR + "icons/drawing_object.png"));
                         libraryTree->setCurrentItem(item);
                         previewItem(item);

                         if (nativeFromFileSystem) {
                             if (!isNetworked && !folderName.endsWith(".pgo") && !library->isLoadingProject())
                                 insertObjectInWorkspace();
                             nativeFromFileSystem = false;
                         } else {
                             if (currentMode == TupProject::VECTOR_STATIC_BG_MODE
                                 || currentMode == TupProject::VECTOR_DYNAMIC_BG_MODE)
                             insertObjectInWorkspace();
                         }
                       }
                     break;
                     case TupLibraryObject::Image:
                       {
                         item->setIcon(0, QIcon(THEME_DIR + "icons/bitmap.png"));
                         libraryTree->setCurrentItem(item);
                         previewItem(item);
                         if (!isNetworked && !folderName.endsWith(".pgo") && !library->isLoadingProject()
                             && folderName.compare(tr("Raster Objects")) != 0)
                             insertObjectInWorkspace();
                       }
                     break;
                     case TupLibraryObject::Svg:
                       {
                         item->setIcon(0, QIcon(THEME_DIR + "icons/svg.png"));
                         libraryTree->setCurrentItem(item);
                         previewItem(item);
                         if (!isNetworked && !library->isLoadingProject())
                             insertObjectInWorkspace();
                       }
                     break;
                     case TupLibraryObject::Audio:
                       {
                         TupLibraryObject *object = library->getObject(id);
                         if (object) {
                             if (!library->isLoadingProject()) {
                                 if (isEffectSound) {
                                     object->setSoundType(Effect);
                                     // object->setSoundResourceFlag(true);
                                     isEffectSound = false;
                                 } else {
                                     object->setSoundType(Lipsync);
                                     // object->setLipsyncVoiceFlag(true);
                                 }
                             }

                             if (!library->isLoadingProject())
                                 object->updateFrameToPlay(currentFrame.frame + 1);

                             library->updateSoundResourcesItem(object);

                             item->setIcon(0, QIcon(THEME_DIR + "icons/sound_object.png"));
                             libraryTree->setCurrentItem(item);
                             previewItem(item);
                         } else {
                             #ifdef TUP_DEBUG
                                 qDebug() << "[TupLibraryWidget::libraryResponse()] - Fatal Error: No object with id -> " << id;
                             #endif
                         }
                       }
                     break;
                     default:
                       {
                       }
                     break;
                 }

             } else {
                 #ifdef TUP_DEBUG
                     qDebug() << "[TupLibraryWidget::libraryResponse()] - No object found: " << id;
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
                            delete (*it);
                            break;
                        }
                    }
                    ++it;
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
                 qDebug() << "[TupLibraryWidget::libraryResponse()] - Unknown/Unhandled project action -> "
                             << response->getAction();
             #endif
          }
        break;
    }
}

void TupLibraryWidget::frameResponse(TupFrameResponse *response)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupLibraryWidget::frameResponse()] - frameIndex -> "
                 << response->getFrameIndex() << " - action -> " << response->getAction();
    #endif

    if (response->getAction() == TupProjectRequest::Add
        || response->getAction() == TupProjectRequest::Select) {
        currentFrame.frame = response->getFrameIndex();
        currentFrame.layer = response->getLayerIndex();
        currentFrame.scene = response->getSceneIndex();
    }
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
        display->render(static_cast<QGraphicsItem *>(msg));

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

    TupProjectRequest request = TupRequestBuilder::createFrameRequest(currentFrame.scene, currentFrame.layer, currentFrame.frame,
                                                                      TupProjectRequest::Select);
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
            qDebug() << "[TupLibraryWidget::updateItemFromSaveAction()] - Fatal Error: Couldn't reload item from Library!";
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

void TupLibraryWidget::updateSoundTiming(int frame)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupLibraryWidget::updateSoundTiming()] - frame -> " << frame;
    #endif

    if (currentSound) {
        currentSound->updateFrameToPlay(frame);
        library->updateSoundResourcesItem(currentSound);
        emit soundUpdated();
    }
}

void TupLibraryWidget::updateSoundMuteStatus(bool mute)
{
    if (currentSound) {
        currentSound->enableMute(mute);
        library->updateSoundResourcesItem(currentSound);
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

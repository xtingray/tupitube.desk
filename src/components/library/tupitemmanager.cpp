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

#include "tupitemmanager.h"

/**
 * This class provides some methods for the TupLibraryWidget.
 * Here is where folder methods are defined for the Library widget.
 **/

TupItemManager::TupItemManager(QWidget *parent) : TreeListWidget(parent), m_currentFolder(0)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupItemManager()]";
    #endif

    currentSelection = "";
    setHeaderLabels(QStringList() << "" << "");

    header()->setSectionResizeMode(QHeaderView::ResizeToContents);

    setItemDelegate(new TupTreeDelegate(this));
    setColumnCount(3);

    setAcceptDrops(true);
    setDragEnabled(true);
    setDropIndicatorShown(true); 
    setDragDropMode(QAbstractItemView::InternalMove);

    foldersTotal = 1;
    parentNode = "";
}

TupItemManager::~TupItemManager()
{
}

void TupItemManager::createFolder(const QString &name)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupItemManager::createFolder()]";
    #endif

    if (name.isNull())
        folderName = tr("New folder %1").arg(foldersTotal);
    else
        folderName = name;

    QTreeWidgetItem *newFolder = new QTreeWidgetItem(this);
    newFolder->setFlags(newFolder->flags() | Qt::ItemIsEditable | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled);
    newFolder->setIcon(0, QIcon(THEME_DIR + "icons/open.png"));
    newFolder->setText(1, folderName);
    newFolder->setText(2, "");

    newFolder->setFlags(newFolder->flags() | Qt::ItemIsEditable);

    foldersTotal++;

    m_currentFolder = newFolder;
    setCurrentItem(m_currentFolder);

    if (name.isNull()) {
        editItem(m_currentFolder, 1);
        emit itemCreated(newFolder);
    } 
}

QTreeWidgetItem *TupItemManager::getFolder(const QString &folderName)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupItemManager::getFolder()] - folderName -> " << folderName;
    #endif

    QList<QTreeWidgetItem *> nodes = findItems(folderName, Qt::MatchExactly, 1);
    for (int i = 0; i < nodes.size(); ++i) {
         QTreeWidgetItem *node = nodes.at(i);
         if ((node->text(1).compare(folderName) == 0) && (node->text(2).length() == 0))
             return node; 
    }

    return 0;
}

QString TupItemManager::oldFolder()
{
    return folderName;
}

QTreeWidgetItem *TupItemManager::currentFolder()
{
    return m_currentFolder;
}

void TupItemManager::setCurrentFolder(QTreeWidgetItem *cf)
{
    if (cf)
        m_currentFolder = cf;
}

void TupItemManager::removeCurrentFolder()
{
    if (m_currentFolder) {
        int index = indexOfTopLevelItem(m_currentFolder) - 1;

        delete m_currentFolder;

        m_currentFolder = topLevelItem (index);
        setCurrentItem(m_currentFolder);
    }
}

void TupItemManager::renameItem() 
{
    QTreeWidgetItem *item = currentItem();
    if (item)
        emit itemRenamed(item);
}

void TupItemManager::cloneItem()
{
    QTreeWidgetItem *item = currentItem();
    if (item)
        emit itemCloned(item);
}

void TupItemManager::exportItem()
{
    QTreeWidgetItem *item = currentItem();
    if (item)
        emit itemExported(item);
}

void TupItemManager::callInkscapeToEdit()
{
    QTreeWidgetItem *item = currentItem();
    if (item)
        emit inkscapeEditCall(item);
}

void TupItemManager::callGimpToEdit()
{
    QTreeWidgetItem *item = currentItem();
    if (item)
        emit gimpEditCall(item);
}

void TupItemManager::callKritaToEdit()
{
    QTreeWidgetItem *item = currentItem();
    if (item)
        emit kritaEditCall(item);
}

void TupItemManager::callMyPaintToEdit()
{
    QTreeWidgetItem *item = currentItem();
    if (item)
        emit myPaintEditCall(item);
}

void TupItemManager::createNewRaster()
{
     emit newRasterCall();
}

void TupItemManager::createNewSVG()
{
     emit newVectorCall();
}

void TupItemManager::callLipSync()
{
    QTreeWidgetItem *item = currentItem();
    if (item)
        emit lipSyncCall(item);
}

void TupItemManager::mouseDoubleClickEvent(QMouseEvent *event)
{
    if (event->buttons() == Qt::LeftButton) {
        QTreeWidgetItem *item = currentItem();
        if (item) {
            if (isFolder(item)) 
                emit itemRenamed(item);    
            else
                emit itemRequired();
        }
    }
}

bool TupItemManager::isFolder(QTreeWidgetItem *item) 
{
    if (item->text(2).length() == 0)
        return true;
   
    return false;
}

int  TupItemManager::indexOf(QTreeWidgetItem *item)
{
    return indexOfTopLevelItem(item) + 1;
}

void TupItemManager::mousePressEvent(QMouseEvent *event)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupItemManager::mousePressEvent()]";
    #endif

    parentNode = "";

    QTreeWidgetItem *item = itemAt(event->pos());
    if (item) {
        setCurrentItem(item);
        emit itemSelected(item);

        if (event->buttons() == Qt::RightButton) { // Opening options menu
            QMenu *menu = new QMenu(tr("Options"));

            if (isFolder(item)) { // Options for folders
                QAction *rename = new QAction(tr("Rename"), this);
                connect(rename, SIGNAL(triggered()), this, SLOT(renameItem()));

                QAction *remove = new QAction(tr("Delete"), this);
                connect(remove, SIGNAL(triggered()), this, SIGNAL(itemRemoved()));

                menu->addAction(rename);
                menu->addAction(remove);
            } else { // Options for Graphic Objects and Sound
                QString extension = item->text(2);
                bool isSound = false;
                bool isNative = false;

                if ((extension.compare("OGG") == 0) || (extension.compare("MP3") == 0) || (extension.compare("WAV") == 0))
                    isSound = true; 
                if (extension.compare("TOBJ") == 0)
                    isNative = true; 

                if (extension.compare("SVG") == 0) {
                    QAction *edit = new QAction(tr("Edit with Inkscape"), this);
                    connect(edit, SIGNAL(triggered()), this, SLOT(callInkscapeToEdit()));
                    #ifdef Q_OS_UNIX
                        if (!QFile::exists("/usr/bin/inkscape"))
                            edit->setDisabled(true);
                    #else
                        edit->setDisabled(true);
                    #endif
                    menu->addAction(edit);
                } else if ((extension.compare("TOBJ") != 0) && !isSound) {
                    QAction *gimpEdit = new QAction(tr("Edit with Gimp"), this);
                    connect(gimpEdit, SIGNAL(triggered()), this, SLOT(callGimpToEdit()));
                    #ifdef Q_OS_UNIX
                       if (!QFile::exists("/usr/bin/gimp"))
                           gimpEdit->setDisabled(true);
                    #else
                       gimpEdit->setDisabled(true);
                    #endif
                    menu->addAction(gimpEdit);

                    QAction *kritaEdit = new QAction(tr("Edit with Krita"), this);
                    connect(kritaEdit, SIGNAL(triggered()), this, SLOT(callKritaToEdit()));
                    #ifdef Q_OS_UNIX
                       if (!QFile::exists("/usr/bin/krita"))
                           kritaEdit->setDisabled(true);
                    #else
                           kritaEdit->setDisabled(true);
                    #endif
                    menu->addAction(kritaEdit);

                    QAction *myPaintEdit = new QAction(tr("Edit with MyPaint"), this);
                    connect(myPaintEdit, SIGNAL(triggered()), this, SLOT(callMyPaintToEdit()));
                    #ifdef Q_OS_UNIX
                       if (!QFile::exists("/usr/bin/mypaint"))
                           myPaintEdit->setDisabled(true);
                    #else
                       myPaintEdit->setDisabled(true);
                    #endif
                    menu->addAction(myPaintEdit);
                }

                if (!isSound && !isNative) { // Option for graphics only
                    QAction *clone = new QAction(tr("Clone"), this);
                    connect(clone, SIGNAL(triggered()), this, SLOT(cloneItem()));
                    menu->addAction(clone);
                }

                QAction *exportObject = new QAction(tr("Export"), this);
                connect(exportObject, SIGNAL(triggered()), this, SLOT(exportItem()));

                QAction *rename = new QAction(tr("Rename"), this);
                connect(rename, SIGNAL(triggered()), this, SLOT(renameItem()));

                QAction *remove = new QAction(tr("Delete"), this);
                connect(remove, SIGNAL(triggered()), this, SIGNAL(itemRemoved()));

                menu->addAction(exportObject);
                menu->addAction(rename);
                menu->addAction(remove);
                menu->addSeparator();

                if (isSound) {
                    QTreeWidgetItem *top = item->parent();
                    if (!top) {
                        QAction *lipsyncAction = new QAction(tr("Open lip-sync editor"), this);
                        connect(lipsyncAction, SIGNAL(triggered()), this, SLOT(callLipSync()));
                        menu->addAction(lipsyncAction);
                    }
                }

                #ifdef Q_OS_UNIX
                    if ((extension.compare("TOBJ") != 0) && !isSound) {
                        if (QFile::exists("/usr/bin/gimp") || QFile::exists("/usr/bin/krita") || QFile::exists("/usr/bin/mypaint")) {
                            QAction *raster = new QAction(tr("Create new raster item"), this);
                            connect(raster, SIGNAL(triggered()), this, SLOT(createNewRaster()));
                            menu->addAction(raster);
                        }

                        if (QFile::exists("/usr/bin/inkscape")) {
                            QAction *svg = new QAction(tr("Create new svg item"), this);
                            connect(svg, SIGNAL(triggered()), this, SLOT(createNewSVG()));
                            menu->addAction(svg);
                        }
                    }
                #endif
            }

            menu->exec(event->globalPos());
        } else if (event->buttons() == Qt::LeftButton) { // Moving item
            if (isFolder(item)) { // If folder, expand it
                bool flag = true;
                if (item->isExpanded())
                    flag = false;
                item->setExpanded(flag);
                return;
            }

            QTreeWidgetItem *top = item->parent();
            if (top)
                parentNode = top->text(1);

            // For directories, get the children
            nodeChildren.clear();
            if (item->text(2).length()==0 && item->childCount() > 0) {
                for (int i=0; i<item->childCount(); i++) {
                    QTreeWidgetItem *node = item->child(i);
                    nodeChildren << node;
                }
            }

            QPixmap pixmap = item->icon(0).pixmap(15, 15);

            QByteArray itemData;
            QDataStream dataStream(&itemData, QIODevice::WriteOnly);
            dataStream << pixmap << item->text(1) << item->text(2) << item->text(3);

            QMimeData *mimeData = new QMimeData;
            mimeData->setData("application/x-dnditemdata", itemData);
            QUrl url("asset://");
            QList<QUrl> list;
            list << url;
            mimeData->setUrls(list);

            QDrag *drag = new QDrag(this);
            drag->setMimeData(mimeData);
            drag->setPixmap(pixmap);            

            if (drag->exec(Qt::MoveAction) == Qt::MoveAction)
                delete takeTopLevelItem(indexOfTopLevelItem(item));
        }
    } else {
        if (event->buttons() == Qt::RightButton) {
            QMenu *menu = new QMenu(tr("Options"));

            #ifdef Q_OS_UNIX
            if (QFile::exists("/usr/bin/gimp") || QFile::exists("/usr/bin/krita") || QFile::exists("/usr/bin/mypaint")) {
                QAction *raster = new QAction(tr("Create new raster item"), this);
                connect(raster, SIGNAL(triggered()), this, SLOT(createNewRaster()));
                menu->addAction(raster);
            }
            if (QFile::exists("/usr/bin/inkscape")) {
                QAction *svg = new QAction(tr("Create new svg item"), this);
                connect(svg, SIGNAL(triggered()), this, SLOT(createNewSVG()));
                menu->addAction(svg);
            }
            #endif

            menu->exec(event->globalPos());
        }
    }
}

void TupItemManager::dropEvent(QDropEvent *event)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupItemManager::dropEvent()]";
    #endif

    bool eventAccept = false;

    if (event->mimeData()->hasFormat("application/x-dnditemdata")) {
        #ifdef TUP_DEBUG
            qDebug() << "[TupItemManager::dropEvent()] - Processing action...";
        #endif

        QByteArray itemData = event->mimeData()->data("application/x-dnditemdata");
        QDataStream dataStream(&itemData, QIODevice::ReadOnly);

        QPixmap pixmap;
        QString label;
        QString extension;
        QString key;
        dataStream >> pixmap >> label >> extension >> key;

        #ifdef TUP_DEBUG
            qDebug() << "[TupItemManager::dropEvent()] - item metadata (label, extension, key) -> "
                     << label << " - " << extension << " - " << key;
            qDebug() << "[TupItemManager::dropEvent()] - parentNode -> " << parentNode;
        #endif

        QTreeWidgetItem *item;
        QTreeWidgetItem *parent = itemAt(event->pos().x(), event->pos().y());

        if (parent) { // Target != NULL            
            #ifdef TUP_DEBUG
                qDebug() << "[TupItemManager::dropEvent()] - parent->text(0) -> " << parent->text(0);
                qDebug() << "[TupItemManager::dropEvent()] - parent->text(1) -> " << parent->text(1);
                qDebug() << "[TupItemManager::dropEvent()] - parent->text(2) -> " << parent->text(2);
            #endif

            // Checking if both target and origin are directories
            if (parentNode.length() > 0) {
                if ((parent->text(2).length() == 0) && (extension.length() == 0)) {
                    #ifdef TUP_DEBUG
                        qDebug() << "[TupItemManager::dropEvent()] - Error: can't move folder inside folder!";
                    #endif
                    return;
                }
            } else { // Parent is root
                if (extension.length() == 0) {
                    #ifdef TUP_DEBUG
                        qDebug() << "[TupItemManager::dropEvent()] - Error: can't move folder to root";
                    #endif
                    return;
                }
            }

            // Ensure target is a directory
            if (parent->text(2).length() > 0) {
                #ifdef TUP_DEBUG
                    qDebug() << "[TupItemManager::dropEvent()] - Warning: parent MUST BE a folder!";
                #endif
                return;
            }

            // Check if target is going to the same parent
            int childrenTotal = parent->childCount();
            for (int i=0; i < childrenTotal; i++) {
                QTreeWidgetItem *child = parent->child(i);
                QString name = child->text(1);
                if (name.compare(label) == 0 && child->text(2).compare(extension) == 0) {
                    #ifdef TUP_DEBUG
                        qDebug() << "[TupItemManager::dropEvent()] - Error: origin and target are the same!";
                    #endif
                    return;
                }
            }

            if (extension.length() > 0) { // Origin is not a folder
                #ifdef TUP_DEBUG
                    qDebug() << "[TupItemManager::dropEvent()] - Origin is NOT a folder";
                #endif

                if (parentNode.length() > 0) { // Parent is NOT root
                    QList<QTreeWidgetItem *> nodes = findItems(parentNode, Qt::MatchExactly, 1);
                    for (int i = 0; i < nodes.size(); ++i) {
                        QTreeWidgetItem *node = nodes.at(i);
                        if (node->text(1).compare(parentNode) == 0) { // Old parent of origin found
                            int childrenTotal = node->childCount();
                            for (int i=0;i < childrenTotal; i++) {
                                QTreeWidgetItem *child = node->child(i);
                                if (child->text(1).compare(label) == 0 && child->text(2).compare(extension) == 0) {
                                    node->removeChild(child); // Removing origin from old position
                                    break;
                                }
                            }
                            break;
                        }
                    }
                }

                #ifdef TUP_DEBUG
                    qDebug() << "[TupItemManager::dropEvent()] - Adding item to folder";
                #endif

                // Adding origin to the target
                item = new QTreeWidgetItem(parent);
                item->setIcon(0, QIcon(pixmap));
                item->setText(1, label);
                item->setText(2, extension);
                item->setText(3, key);
                item->setFlags(item->flags() | Qt::ItemIsEditable | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled);
                setCurrentItem(item);

                if (key.length() > 0) // Notifying item was moved
                    emit itemMoved(key, parent->text(1));

                eventAccept = true;
            } else {
                #ifdef TUP_DEBUG
                    qDebug() << "[TupItemManager::dropEvent()] - Origin is a folder";
                #endif
                bool flag = true;
                if (parent->isExpanded())
                    flag = false;
                parent->setExpanded(flag);
            }
        } else { // Moving object to root
            #ifdef TUP_DEBUG
                qDebug() << "[TupItemManager::dropEvent()] - Moving object to root...";
            #endif

            if (parentNode.length() > 0) {
                QList<QTreeWidgetItem *> nodes = findItems(parentNode, Qt::MatchExactly, 1);
                if (nodes.size() > 0) {
                    #ifdef TUP_DEBUG
                        qDebug() << "[TupItemManager::dropEvent()] - Removing origin from old position...";
                    #endif
                    for (int i = 0; i < nodes.size(); ++i) {
                        QTreeWidgetItem *node = nodes.at(i);
                        if (node->text(1).compare(parentNode) == 0) { // Old parent of origin found
                            int childrenTotal = node->childCount();
                            for (int i=0;i < childrenTotal; i++) {
                                QTreeWidgetItem *child = node->child(i);
                                if (child->text(1).compare(label) == 0 && child->text(2).compare(extension) == 0) {
                                    node->removeChild(child); // Removing origin from old position
                                    break;
                                }
                            }
                            break;
                        }
                    }
                }
            } else {
                // if (extension.length() == 0 && nodeChildren.size() > 0) {
                if (extension.length() == 0) {
                    #ifdef TUP_DEBUG
                        qDebug() << "[TupItemManager::dropEvent()] - Error: can't move folder to root!";
                    #endif
                    return;
                }
            }

            #ifdef TUP_DEBUG
                qDebug() << "[TupItemManager::dropEvent()] - Adding item to root";
            #endif

            item = new QTreeWidgetItem(this);
            item->setIcon(0, QIcon(pixmap));
            item->setText(1, label);
            item->setText(2, extension);
            item->setText(3, key);
            item->setFlags(item->flags() | Qt::ItemIsEditable | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled);
            setCurrentItem(item);

            #ifdef TUP_DEBUG
                qDebug() << "[TupItemManager::dropEvent()] - Item moved successfully!";
            #endif

            emit itemMoved(key, ""); // Notifying item was moved to the root
            eventAccept = true;
        }
    } else { // Item wasn't recognized as library element
        #ifdef TUP_DEBUG
            qDebug() << "[TupItemManager::dropEvent()] - Error: item wasn't recognized as library element";
        #endif
        event->ignore();
        return;
    }

    if (eventAccept) { // Accepting action
        if (event->source() == this) {
            #ifdef TUP_DEBUG
                qDebug() << "[TupItemManager::dropEvent()] - Move action accepted!";
            #endif
            event->setDropAction(Qt::MoveAction);
            event->accept();
        } else {
            event->acceptProposedAction();
        }
    }
}

void TupItemManager::dragEnterEvent(QDragEnterEvent *event)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupItemManager::dragEnterEvent()]";
    #endif

    if (event->mimeData()->hasFormat("application/x-dnditemdata")) {
        if (event->source() == this) {
            event->setDropAction(Qt::MoveAction);
            event->accept();
        } else {
            event->acceptProposedAction();
        }
    } else {
        event->ignore();
    }
}

void TupItemManager::dragMoveEvent(QDragMoveEvent *event)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupItemManager::dragMoveEvent()]";
    #endif

    if (event->mimeData()->hasFormat("application/x-dnditemdata")) {
        if (event->source() == this) {
            #ifdef TUP_DEBUG
                qDebug() << "[TupItemManager::dragMoveEvent()] - Move action accepted!";
            #endif
            event->setDropAction(Qt::MoveAction);
            event->accept();
        } else {
            event->acceptProposedAction();
        }
    } else {
        #ifdef TUP_DEBUG
            qDebug() << "[TupItemManager::dragMoveEvent()] - Action ignored!";
        #endif
        event->ignore();
    }
}

void TupItemManager::keyPressEvent(QKeyEvent * event)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupItemManager::keyPressEvent()]";
    #endif

    if (event->key() == Qt::Key_Up) {
        QTreeWidgetItem *current = currentItem();
        if (current) {
            QTreeWidgetItem *up = itemAbove(current);
            if (up) {
                setCurrentItem(up);
                emit itemSelected(up);
            }
        }
        return;
    }

    if (event->key() == Qt::Key_Down) {
        QTreeWidgetItem *current = currentItem();
        if (current) {
            QTreeWidgetItem *down = itemBelow(current);
            if (down) {
                setCurrentItem(down);
                emit itemSelected(down);
            }
        }
        return;
    }

    if (event->key() == Qt::Key_Return) {
        QTreeWidgetItem *item = currentItem();
        if (item) {
            if (item->text(2).length() == 0) {
                if (item->childCount() > 0) {
                    if (item->isExpanded())
                        item->setExpanded(false);
                    else
                        item->setExpanded(true);
                }
            }
        }
        return;
    }
}

void TupItemManager::cleanUI()
{
    clear();
    foldersTotal = 1;
}

int TupItemManager::itemType()
{
    int type = 0;
    QTreeWidgetItem *item = currentItem();
    if (item)
        type = item->data(1, 3216).toInt();

    return type;
}

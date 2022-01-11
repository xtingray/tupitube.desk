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

#include "tupviewcolorcells.h"

TupViewColorCells::TupViewColorCells(QWidget *parent) : QFrame(parent)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupViewColorCells()]";
    #endif

    numColorRecent = 0;
    currentCell = nullptr;
    viewLayout = new QVBoxLayout;
    viewLayout->setMargin(0);
    viewLayout->setSpacing(0);

    setFrameStyle(QFrame::Box | QFrame::Raised);
    setupForm();

    setLayout(viewLayout);
}

TupViewColorCells::~TupViewColorCells()
{
    #ifdef TUP_DEBUG
        qDebug() << "[~TupViewColorCells()]";
    #endif

    TCONFIG->beginGroup("ColorPalette");
    TCONFIG->setValue("LastPalette", chooserPalette->currentIndex());

    QDir brushesDir(CONFIG_DIR + "palettes");

    if (! brushesDir.exists()) 
        brushesDir.mkdir(brushesDir.path());

    #ifdef TUP_DEBUG
        qWarning() << "[TupViewColorCells::~TupViewColorCells()] - Saving color palettes in -> " << brushesDir.path();
    #endif

    for (int i = 0; i < containerPalette->count(); i++) {
         TupCellsColor *palette = qobject_cast<TupCellsColor *>(containerPalette->widget(i));
         if (palette) {
             if (!palette->isReadOnly())
                 palette->save(CONFIG_DIR + "palettes/" + palette->getName() + ".tpal");
         }
    }	
}

void TupViewColorCells::setupForm()
{
    chooserPalette = new QComboBox(this);
    chooserPalette->setStyleSheet("combobox-popup: 0;");

    containerPalette = new QStackedWidget(this);
    viewLayout->addWidget(chooserPalette);
    viewLayout->addWidget(containerPalette);

    // Default Palette
    defaultPalette = new TupCellsColor(containerPalette);
    defaultPalette->setName(tr("Default Palette"));
    defaultPalette->setReadOnly(true);
    addPalette(defaultPalette);

    // Named Colors
    qtColorPalette = new TupCellsColor(containerPalette);
    qtColorPalette->setReadOnly(true);
    qtColorPalette->setName(tr("Named Colors"));
    fillNamedColor();
    addPalette(qtColorPalette);

    // Custom Color Palette
    // SQA: This palette must be implemented
    customColorPalette = new TupCellsColor(containerPalette);
    customColorPalette->setName(tr("Custom Color Palette"));
    addPalette(customColorPalette);

    // Custom Gradient Palette
    // SQA: This palette must be implemented
    customGradientPalette = new TupCellsColor(containerPalette);
    customGradientPalette->setName(tr("Custom Gradient Palette"));
    customGradientPalette->setType(TupCellsColor::Gradient);
    addPalette(customGradientPalette);

#ifdef Q_OS_WIN
    QString palettesPath = SHARE_DIR + "palettes";
#else
    QString palettesPath = SHARE_DIR + "data/palettes";
#endif
    readPalettes(palettesPath); // Pre-installed
    readPalettes(CONFIG_DIR + "palettes"); // Locals

    connect(chooserPalette, SIGNAL(activated(int)), containerPalette, SLOT(setCurrentIndex(int)));

    TCONFIG->beginGroup("ColorPalette");
    int lastIndex = TCONFIG->value("LastPalette").toInt();

    if (lastIndex < 0)
        lastIndex = 0;

    chooserPalette->setCurrentIndex(lastIndex);
    containerPalette->setCurrentIndex(lastIndex);

    buttonPanel = new TupColorButtonPanel(Qt::SolidPattern, QSize(22, 22), 10, "6,4,2", this);
    connect(buttonPanel, SIGNAL(clickColor(const QColor&)), this, SLOT(updateColorFromPanel(const QColor&)));

    /* SQA: This connection doesn't work on Windows
    connect(buttonPanel, &TupColorButtonPanel::clickColor, this, &TupViewColorCells::updateColorFromPanel);
    */

    buttonPanel->setFixedHeight(35);

    QHBoxLayout *basicLayout = new QHBoxLayout;
    basicLayout->addWidget(buttonPanel);

    viewLayout->addLayout(basicLayout);
}

void TupViewColorCells::readPalettes(const QString &paletteDir)
{
    #ifdef TUP_DEBUG
        qWarning() << "[TupViewColorCells::readPalettes()] - Reading palettes from: " << paletteDir;
    #endif

    QDir dir(paletteDir);
    if (dir.exists()) {
        QStringList files = dir.entryList(QStringList() << "*.tpal");
        // QStringList::ConstIterator it = files.begin();

        for (int i = 0; i < files.size(); ++i)
            readPaletteFile(dir.path() + "/" + files.at(i));

        /*
        while (it != files.end()) {
            readPaletteFile(dir.path() + "/" + *it);
            ++it;
        }
        */
    } else {
        #ifdef TUP_DEBUG
            qDebug() << "[TupViewColorCells::readPalettes()] - Error: Palettes path doesn't exist -> " << paletteDir;
        #endif

        if (dir.mkpath(paletteDir)) {
            #ifdef TUP_DEBUG
                qDebug() << "[TupViewColorCells::readPalettes()] - Creating path -> " << paletteDir;
            #endif
        }
    }
}

void TupViewColorCells::readPaletteFile(const QString &paletteFile)
{
    QFile file(paletteFile);
    if (file.exists()) {
        if (file.open(QFile::ReadOnly | QFile::Text)) {
            TupPaletteParser parser(&file);
            if (parser.processPalette()) {
                QList<QBrush> brushes = parser.getBrushes();
                QString name = parser.getPaletteName();
                bool editable = parser.paletteIsEditable();
                addPalette(name, brushes, editable);
            } else {
                #ifdef TUP_DEBUG
                    qDebug() << "[TupViewColorCells::readPaletteFile()] - Fatal error while parsing palette file -> " << paletteFile;
                #endif
            }
        } else {
            #ifdef TUP_DEBUG
                qDebug() << "[TupViewColorCells::readPaletteFile()] - Fatal error while open palette file -> " << paletteFile;
            #endif
        }
    } else {
        #ifdef TUP_DEBUG
            qDebug() << "[TupViewColorCells::readPaletteFile()] - Fatal error: palette file doesn't exist! -> " << paletteFile;
        #endif
    }

    /*
    if (file.exists()) {
        if (parser.parse(&file)) {
            QList<QBrush> brushes = parser.getBrushes();
            QString name = parser.getPaletteName();
            bool editable = parser.paletteIsEditable();
            addPalette(name, brushes, editable);
        } else {
            #ifdef TUP_DEBUG
                qDebug() << "[TupViewColorCells::readPaletteFile()] - Fatal error while parsing palette file: " + paletteFile;
            #endif
        }
    } else {
        #ifdef TUP_DEBUG
            qDebug() << "[TupViewColorCells::readPaletteFile()] - Fatal error: palette file doesn't exist! -> " + paletteFile;
        #endif
    }
    */
}

void TupViewColorCells::addPalette(const QString & name, const QList<QBrush> & brushes, bool editable)
{
    /*
    #ifdef TUP_DEBUG
        qDebug() << "[TupViewColorCells::addPalette()] - name -> " << name;
    #endif
    */

    if (name == "Default Palette") {
        QList<QBrush>::ConstIterator it = brushes.begin();
        while (it != brushes.end()) {
               defaultPalette->addItem(*it);
               ++it;
        }
    } else if (name == customColorPalette->getName()) {
        QList<QBrush>::ConstIterator it = brushes.begin();

        while (it != brushes.end()) {
               customColorPalette->addItem(*it);
               ++it;
        }
    } else if (name == customGradientPalette->getName()) {
               QList<QBrush>::ConstIterator it = brushes.begin();

               while (it != brushes.end()) {
                      customGradientPalette->addItem(*it);
                      ++it;
               }
    } else {
           TupCellsColor *palette = new  TupCellsColor(containerPalette);
           QList<QBrush>::ConstIterator it = brushes.begin();

           while (it != brushes.end()) {
                  palette->addItem(*it);
                  ++it;
           }

           palette->setName(name);
           addPalette(palette);
           palette->setReadOnly(!editable);
    }
}

void TupViewColorCells::addPalette(TupCellsColor *palette)
{
    connect(palette, SIGNAL(itemEntered(QTableWidgetItem*)), this, SLOT(changeColor(QTableWidgetItem*)));
    connect(palette, SIGNAL(itemPressed(QTableWidgetItem*)), this, SLOT(changeColor(QTableWidgetItem*)));
    chooserPalette->addItem(palette->getName());
    containerPalette->addWidget(palette);
}

void TupViewColorCells::changeColor(QTableWidgetItem* item)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupViewColorCells::changeColor()]";
    #endif

    if (item) {
        buttonPanel->resetPanel();
        if (currentCell) {
            QColor currentColor = currentCell->background().color();
            QColor newColor = item->background().color(); 
            if (newColor != currentColor) {
                currentCell = item;
                emit colorSelected(item->background());
            }
        } else {
            currentCell = item;
            emit colorSelected(item->background());
        }
    }
}

void TupViewColorCells::clearSelection()
{
    if (currentCell)
        currentCell->setSelected(false);
}

void TupViewColorCells::resetBasicPanel()
{
    buttonPanel->resetPanel();
}

void TupViewColorCells::enableTransparentColor(bool flag)
{
    buttonPanel->enableTransparentColor(flag);
}

void TupViewColorCells::fillNamedColor()
{
    QStringList colorStrings = QColor::colorNames();
    // QStringList::ConstIterator it = colorStrings.begin();

    for (int i = 0; i < colorStrings.size(); ++i)
        qtColorPalette->addItem(QColor(colorStrings.at(i)));

    /*
    while (it != it.end()) {
        qtColorPalette->addItem(QColor(*it));
        ++it;
    }
    */

    qtColorPalette->addItem(QColor(0,0,0,0));
    qtColorPalette->addItem(QColor(0,0,0,50));
}

void TupViewColorCells::addCurrentColor()
{
    TupCellsColor *palette = qobject_cast<TupCellsColor*>(containerPalette->currentWidget());

    if (palette) {
        if (palette->isReadOnly() || (currentColor.gradient()  && palette->getType() == TupCellsColor::Color)
            || (currentColor.color().isValid() && palette->getType() == TupCellsColor::Gradient)) {
            if (15 <= currentColor.style() && currentColor.style() < 18) {
                palette = customGradientPalette;
                chooserPalette->setCurrentIndex(chooserPalette->findText(customGradientPalette->getName()));
                containerPalette->setCurrentWidget(customGradientPalette);
            } else {
                palette = customColorPalette;
                chooserPalette->setCurrentIndex(chooserPalette->findText(customColorPalette->getName()));
                containerPalette->setCurrentWidget(customColorPalette);
            }
        }
        palette->addItem(currentColor);
    }
}

void TupViewColorCells::removeCurrentColor()
{
     TCellView *palette = qobject_cast<TCellView *>(containerPalette->currentWidget());
     if (palette) {
         if (defaultPalette != palette) {
             // SQA: Add function removeItem in TCellView
         }
     }
}

void TupViewColorCells::setupButtons()
{
    QGroupBox *containerButtons = new QGroupBox(this);
    QBoxLayout *bLayout = new QBoxLayout(QBoxLayout::LeftToRight);
    bLayout->setMargin(0);
    bLayout->setSpacing(0);

    containerButtons->setLayout(bLayout);

    TImageButton *addItem = new TImageButton(QPixmap(THEME_DIR  + "icons/plus_sign.png") , 22);
    connect(addItem, SIGNAL(clicked()), SLOT(addCurrentColor()));
    addItem->setToolTip(tr("Add Color"));
    bLayout->addWidget(addItem);
    // SQA instruction - temporary code
    addItem->setEnabled(false);

    TImageButton *removeColor = new TImageButton(QPixmap(THEME_DIR + "icons/minus_sign.png"), 22);
    connect(removeColor, SIGNAL(clicked()), SLOT(removeCurrentColor()));
    removeColor->setToolTip(tr("Remove Color"));
    bLayout->addWidget(removeColor);
    // SQA instruction - temporary code
    removeColor->setEnabled(false);

    layout()->addWidget(containerButtons);
}

void TupViewColorCells::setColor(const QBrush& brush)
{
    currentColor = brush;
}

void TupViewColorCells::updateColorFromPanel(const QColor &color)
{
    clearSelection();

    QBrush brush(color);
    emit colorSelected(brush);
}

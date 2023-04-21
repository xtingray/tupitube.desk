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

#include "rastercolorwidget.h"

#include "timagebutton.h"
#include "tseparator.h"
#include "tupmodulewidgetbase.h"
#include "ticon.h"
#include "tcolorcell.h"
#include "tconfig.h"
#include "tuppaintareaevent.h"
#include "tvhbox.h"

#include <QBoxLayout>
#include <QPushButton>
#include <QFrame>
#include <QToolTip>
#include <QComboBox>
#include <QGroupBox>
#include <QMenu>

RasterColorWidget::RasterColorWidget(const QColor contourColor, const QColor bgColor, QWidget *parent) : TupModuleWidgetBase(parent)
{
    #ifdef TUP_DEBUG
        qDebug() << "[RasterColorWidget()]";
    #endif

    setWindowTitle(tr("Color Palette"));
    setWindowIcon(QPixmap(THEME_DIR + "icons/color_palette.png"));

    currentSpace = TColorCell::Contour;
    TCONFIG->beginGroup("ColorPalette");
    TCONFIG->setValue("CurrentColorMode", 0);

    currentContourBrush = QBrush(contourColor);
    bgBrush = QBrush(bgColor);
    splitter = new QSplitter(Qt::Vertical, this);

    tab = new QTabWidget;
    connect(tab, SIGNAL(currentChanged(int)), this, SLOT(updateColorType(int)));

    setupColorDisplay();

    addChild(splitter);

    setupMainPalette();
    setupColorChooser();

    tab->setPalette(palette());
    tab->setMinimumHeight(320);
    splitter->addWidget(tab);

    setMinimumWidth(316);
}

RasterColorWidget::~RasterColorWidget()
{
    #ifdef TUP_DEBUG
        qDebug() << "[~RasterColorWidget()]";
    #endif

    delete paletteContainer;
    paletteContainer = nullptr;

    delete colorForm;
    colorForm = nullptr;

    delete colorPickerArea;
    colorPickerArea = nullptr;

    delete luminancePicker;
    luminancePicker = nullptr;

    delete contourColorCell;
    contourColorCell = nullptr;

    delete bgColorCell;
    bgColorCell = nullptr;
}

void RasterColorWidget::setupColorDisplay()
{
    QFrame *topPanel = new QFrame(this);
    QBoxLayout *generalLayout = new QBoxLayout(QBoxLayout::TopToBottom);
    // generalLayout->setMargin(0);
    topPanel->setLayout(generalLayout);

    QBoxLayout *mainLayout = new QBoxLayout(QBoxLayout::LeftToRight);
    // mainLayout->setMargin(0);
    mainLayout->setSpacing(1);

    mainLayout->addWidget(new QWidget());

    QBoxLayout *listLayout = new QBoxLayout(QBoxLayout::TopToBottom);
    QLabel *contourLabel = new QLabel(tr("Contour"));

    QSize cellSize(25, 25);
    contourColorCell = new TColorCell(TColorCell::Contour, currentContourBrush, cellSize);
    contourColorCell->setChecked(true);
    connect(contourColorCell, SIGNAL(clicked(TColorCell::FillType)), this, SLOT(updateColorMode(TColorCell::FillType)));

    QBoxLayout *contourLayout = new QBoxLayout(QBoxLayout::LeftToRight);
    contourLayout->addWidget(contourColorCell);
    contourLayout->addSpacing(5);
    contourLayout->addWidget(contourLabel);

    listLayout->addLayout(contourLayout);

    mainLayout->addLayout(listLayout);
    mainLayout->addWidget(new QWidget());

    QLabel *htmlLabel = new QLabel(tr("HTML"), topPanel);
    htmlLabel->setMaximumWidth(50);
    htmlLabel->setAlignment(Qt::AlignHCenter|Qt::AlignVCenter);
    mainLayout->addWidget(htmlLabel);

    htmlField = new QLineEdit(topPanel);
    htmlField->setMaximumWidth(70);
    htmlField->setText(currentContourBrush.color().name());
    connect(htmlField, SIGNAL(editingFinished()), this, SLOT(updateColorFromHTML()));
    mainLayout->addWidget(htmlField);

    mainLayout->addWidget(new QWidget());

    generalLayout->addLayout(mainLayout);
    generalLayout->addWidget(new QWidget());
    generalLayout->setAlignment(mainLayout, Qt::AlignHCenter|Qt::AlignVCenter);

    generalLayout->addWidget(new TSeparator(Qt::Horizontal));    

    QBoxLayout *bgLayout = new QBoxLayout(QBoxLayout::LeftToRight);
    // bgLayout->setMargin(0);
    bgLayout->setSpacing(1);

    bgLayout->addWidget(new QWidget());

    bgColorCell = new TColorCell(TColorCell::Background, bgBrush, cellSize);
    connect(bgColorCell, SIGNAL(clicked(TColorCell::FillType)), this, SLOT(updateColorMode(TColorCell::FillType)));
    bgLayout->addWidget(bgColorCell);

    bgLayout->setSpacing(5);

    QLabel *bgLabel = new QLabel(tr("Background"), topPanel);
    bgLabel->setAlignment(Qt::AlignHCenter|Qt::AlignVCenter);
    bgLayout->addSpacing(5);
    bgLayout->addWidget(bgLabel);

    QWidget *space1 = new QWidget();
    space1->setFixedWidth(10);
    bgLayout->addWidget(space1);

    TImageButton *resetBgButton = new TImageButton(QIcon(QPixmap(THEME_DIR + "icons/reset_bg.png")), 15, this, true);
    resetBgButton->setToolTip(tr("Reset background"));
    connect(resetBgButton, SIGNAL(clicked()), this, SLOT(initBg()));
    bgLayout->addWidget(resetBgButton);

    QWidget *space2 = new QWidget();
    space2->setFixedWidth(10);
    bgLayout->addWidget(space2);

    QLabel *bgHtmlLabel = new QLabel(tr("HTML"), topPanel);
    bgHtmlLabel->setMaximumWidth(50);
    bgHtmlLabel->setAlignment(Qt::AlignHCenter|Qt::AlignVCenter);
    bgLayout->addWidget(bgHtmlLabel);

    bgLayout->setSpacing(5);

    bgHtmlField = new QLineEdit();
    bgHtmlField->setMaximumWidth(70);
    bgHtmlField->setText(bgBrush.color().name());
    connect(bgHtmlField, SIGNAL(editingFinished()), this, SLOT(updateBgColorFromHTML()));
    bgLayout->addWidget(bgHtmlField);

    bgLayout->addWidget(new QWidget());

    generalLayout->addLayout(bgLayout);
    generalLayout->setAlignment(bgLayout, Qt::AlignHCenter);

    generalLayout->addWidget(new QWidget());

    addChild(topPanel);
}

void RasterColorWidget::updateColorMode(TColorCell::FillType type)
{
    #ifdef TUP_DEBUG
        qDebug() << "[RasterColorWidget::updateColorMode()] - type -> " << type;
    #endif

    QBrush brush;
    QColor color;

    if (type == TColorCell::Background) {
        paletteContainer->enableTransparentColor(false);
        currentSpace = TColorCell::Background;
        brush = bgColorCell->brush();
        color = brush.color();
        bgHtmlField->setText(color.name());

        if (contourColorCell->isChecked())
            contourColorCell->setChecked(false);
    } else {
        paletteContainer->enableTransparentColor(true);
        paletteContainer->resetBasicPanel();
        if (bgColorCell->isChecked())
            bgColorCell->setChecked(false);

        if (type == TColorCell::Contour) {
            currentSpace = TColorCell::Contour;
            brush = contourColorCell->brush();
        }
        color = brush.color();
        htmlField->setText(color.name());
    }

    TCONFIG->beginGroup("ColorPalette");
    TCONFIG->setValue("CurrentColorMode", type);
    emit colorSpaceChanged(type);

    if (fgType == Solid && tab->currentIndex() != 0) {
        tab->setCurrentIndex(0);
    } else if (fgType == Gradient && tab->currentIndex() != 1) {
        tab->setCurrentIndex(1);
    }

    updateLuminancePicker(color);
    colorForm->setColor(color);
}

void RasterColorWidget::checkColorButton(TColorCell::FillType type)
{
    #ifdef TUP_DEBUG
        qDebug() << "[RasterColorWidget::checkColorButton()] - type -> " << type;
    #endif

    if (type == TColorCell::Contour) {
        contourColorCell->click();
    } else if (type == TColorCell::Background) {
        bgColorCell->click();
    }
}

void RasterColorWidget::setupMainPalette()
{
    // Palettes
    paletteContainer = new TupViewColorCells(splitter);
    connect(paletteContainer, SIGNAL(colorSelected(QBrush)),
            this, SLOT(updateColorFromPalette(QBrush)));

    splitter->addWidget(paletteContainer);
}

void RasterColorWidget::setupColorChooser()
{
    QFrame *colorMixer = new QFrame;
    colorMixer->setFrameStyle(QFrame::Box | QFrame::Sunken);

    QBoxLayout *mainLayout = new QBoxLayout(QBoxLayout::TopToBottom);
    colorMixer->setLayout(mainLayout);

    colorPickerArea = new TupColorPicker(colorMixer);
    connect(colorPickerArea, SIGNAL(newColor(int,int)), this, SLOT(setHS(int,int)));

    luminancePicker = new TSlider(Qt::Horizontal, TSlider::Color, QColor(0, 0, 0), QColor(255, 255, 255));
    connect(luminancePicker, SIGNAL(colorChanged(QColor)), this, SLOT(syncColor(QColor)));
    luminancePicker->setRange(0, 100);
    luminancePicker->setValue(100);

    colorForm = new TupColorForm;
    connect(colorForm, SIGNAL(brushChanged(QBrush)), this, SLOT(updateColorFromDisplay(QBrush)));

    mainLayout->addWidget(colorPickerArea);
    mainLayout->setAlignment(colorPickerArea, Qt::AlignHCenter);

    mainLayout->addWidget(luminancePicker);
    mainLayout->setAlignment(luminancePicker, Qt::AlignHCenter);

    mainLayout->addWidget(colorForm);
    mainLayout->setAlignment(colorForm, Qt::AlignHCenter);

    mainLayout->addStretch(2);

    tab->addTab(colorMixer, tr("Color Mixer"));
}

void RasterColorWidget::setColorOnAppFromHTML(const QBrush& brush)
{
    QColor color = brush.color();

    if (color.isValid()) {
        colorPickerArea->setColor(color.hue(), color.saturation());
        paletteContainer->setColor(brush);
        colorForm->setColor(color);
    }

    if (currentSpace == TColorCell::Background) {
        bgColorCell->setBrush(brush);

        TupPaintAreaEvent bgEvent(TupPaintAreaEvent::ChangeBgColor, bgColorCell->color());
        emit paintAreaEventTriggered(&bgEvent);
        return;
    }

    if (currentSpace == TColorCell::Contour) {
        contourColorCell->setBrush(brush);

        TupPaintAreaEvent contourEvent(TupPaintAreaEvent::ChangePenColor, contourColorCell->color());
        emit paintAreaEventTriggered(&contourEvent);
        return;
    }
}

void RasterColorWidget::setGlobalColors(const QBrush &brush)
{
    if (currentSpace == TColorCell::Background) {
        bgColorCell->setBrush(brush);
        bgHtmlField->setText(brush.color().name());

        TupPaintAreaEvent event(TupPaintAreaEvent::ChangeBgColor, brush.color());
        emit paintAreaEventTriggered(&event);
    } else {
        if (currentSpace == TColorCell::Contour) {
            contourColorCell->setBrush(brush);
            currentContourBrush = brush;

            TupPaintAreaEvent event(TupPaintAreaEvent::ChangePenColor, brush.color());
            emit paintAreaEventTriggered(&event);
        } else {
            if (brush.color() == Qt::transparent) {
                if (contourColorCell->color() == Qt::transparent) {
                    QBrush black(Qt::black);
                    contourColorCell->setBrush(black);
                    currentContourBrush = black;

                    TupPaintAreaEvent event(TupPaintAreaEvent::ChangePenColor, black);
                    emit paintAreaEventTriggered(&event);
                }
            }

            TupPaintAreaEvent event(TupPaintAreaEvent::ChangeBrush, brush);
            emit paintAreaEventTriggered(&event);
        }
        htmlField->setText(brush.color().name());
    }
}

void RasterColorWidget::updateColorFromPalette(const QBrush &brush)
{
    colorPickerArea->clearSelection();

    setGlobalColors(brush);
    QColor color = brush.color();
    updateLuminancePicker(color);
    colorForm->setColor(color);
}

void RasterColorWidget::updateColorFromDisplay(const QBrush &brush)
{
    setGlobalColors(brush);
    QColor color = brush.color();
    colorPickerArea->setColor(color.hue(), color.saturation());
    updateLuminancePicker(color);
}

void RasterColorWidget::syncColor(const QColor &color)
{
    setGlobalColors(QBrush(color));
    colorForm->setColor(color);
}

void RasterColorWidget::setHS(int hue, int saturation)
{
    paletteContainer->clearSelection();
    paletteContainer->resetBasicPanel();

    int luminance = 255;
    if (hue == 0 && saturation == 0)
        luminance = 0;

    QColor color;
    color.setHsv(hue, saturation, luminance, 255);

    setGlobalColors(QBrush(color));
    updateLuminancePicker(color);
    colorForm->setColor(color);
}

void RasterColorWidget::updateColorFromHTML()
{
    QColor currentColor;
    if (currentSpace == TColorCell::Background) {
        bgColorCell->setChecked(false);
        currentSpace = TColorCell::Contour;
        contourColorCell->setChecked(true);
        currentColor = contourColorCell->color();
    } else if (currentSpace == TColorCell::Contour) {
        currentColor = contourColorCell->color();
    }

    QString colorCode = htmlField->text();
    QColor color(colorCode);

    if (color != currentColor) {
        color.setAlpha(255);
        setColorOnAppFromHTML(color);
    }
}

void RasterColorWidget::updateBgColorFromHTML()
{
    if (currentSpace != TColorCell::Background) {
        if (contourColorCell->isChecked())
            contourColorCell->setChecked(false);

        currentSpace = TColorCell::Background;
        bgColorCell->setChecked(true);
    }

    QColor currentColor = bgColorCell->color();
    QString colorCode = bgHtmlField->text();
    QColor color(colorCode);

    if (color != currentColor) {
        color.setAlpha(255);
        setColorOnAppFromHTML(color);
    }
}

QPair<QColor, QColor> RasterColorWidget::color()
{
    QPair<QColor, QColor> colors;
    colors.first = contourColorCell->color();

    return colors;
}

void RasterColorWidget::parsePaletteFile(const QString &file)
{
    paletteContainer->readPaletteFile(file);
}

void RasterColorWidget::setBgColor(const QColor &color)
{
    QBrush brush(color);
    bgColorCell->setBrush(brush);
}

void RasterColorWidget::initBg()
{
    QBrush brush(Qt::white);
    bgColorCell->setBrush(brush);
    bgColorCell->setChecked(true);
    updateColorMode(TColorCell::Background);

    paletteContainer->clearSelection();

    TupPaintAreaEvent event(TupPaintAreaEvent::ChangeBgColor, brush.color());
    emit paintAreaEventTriggered(&event);
}

void RasterColorWidget::updateColorType(int index)
{
    if (index == RasterColorWidget::Solid) {
        if (currentSpace == TColorCell::Contour)
            fgType = Solid;
        else
            bgType = Solid;
    } else {
        if (currentSpace == TColorCell::Inner)
            fgType = Gradient;
        else
            bgType = Gradient;
    }
}

void RasterColorWidget::updateLuminancePicker(const QColor &color)
{
    if (color == Qt::transparent) {
        luminancePicker->setEnabled(false);
        return;
    } 

    if (!luminancePicker->isEnabled())
        luminancePicker->setEnabled(true);

    if (color != Qt::black) {
        luminancePicker->setColors(Qt::black, color);
        luminancePicker->setValue(100);
    } else {
        luminancePicker->setColors(Qt::black, Qt::white);
        luminancePicker->setValue(0);
    }
}

void RasterColorWidget::updateContourColor(const QColor &color)
{
    if (bgColorCell->isChecked())
        bgColorCell->setChecked(false);

    if (!contourColorCell->isChecked())
        contourColorCell->setChecked(true);

    if (color != contourColorCell->color()) {
        contourColorCell->setBrush(QBrush(color));
        updateColorMode(TColorCell::Contour);
    }
}

void RasterColorWidget::updateBgColor(const QColor &color)
{
    if (contourColorCell->isChecked())
        contourColorCell->setChecked(false);

    if (!bgColorCell->isChecked())
        bgColorCell->setChecked(true);

    if (color != bgColorCell->color()) {
        bgColorCell->setBrush(QBrush(color));
        updateColorMode(TColorCell::Background);
    }
}

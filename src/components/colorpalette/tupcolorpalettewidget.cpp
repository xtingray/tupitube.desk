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

#include "tupcolorpalettewidget.h"

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

TupColorPaletteWidget::TupColorPaletteWidget(QWidget *parent): TupModuleWidgetBase(parent)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupColorPaletteWidget()]";
    #endif

    currentSpace = TColorCell::Contour;
    TCONFIG->beginGroup("ColorPalette");
    TCONFIG->setValue("CurrentColorMode", 0);
    TCONFIG->setValue("TextColor", QColor(Qt::black));

    currentContourBrush = Qt::black;
    currentFillBrush = Qt::transparent;
    flagGradient = true;

    setWindowTitle(tr("Color Palette"));
    setWindowIcon(QPixmap(THEME_DIR + "icons/color_palette.png"));

    splitter = new QSplitter(Qt::Vertical, this);

    tab = new QTabWidget;
    connect(tab, SIGNAL(currentChanged(int)), this, SLOT(updateColorType(int)));

    setupColorDisplay();

    addChild(splitter);

    setupMainPalette();
    setupColorChooser();
    setupGradientManager();

    tab->setPalette(palette());
    tab->setMinimumHeight(320);
    splitter->addWidget(tab);

    setMinimumWidth(316);
    setMaximumWidth(470);
}

TupColorPaletteWidget::~TupColorPaletteWidget()
{
    #ifdef TUP_DEBUG
        qDebug() << "[~TupColorPaletteWidget()]";
    #endif

    delete paletteContainer;
    paletteContainer = nullptr;

    delete colorForm;
    colorForm = nullptr;

    delete colorPickerArea;
    colorPickerArea = nullptr;

    delete luminancePicker;
    luminancePicker = nullptr;

    delete gradientManager;
    gradientManager = nullptr;

    delete contourColorCell;
    contourColorCell = nullptr;

    delete fillColorCell;
    fillColorCell = nullptr;

    delete bgColor;
    bgColor = nullptr;
}

void TupColorPaletteWidget::setupColorDisplay()
{
    QFrame *topPanel = new QFrame(this);
    QBoxLayout *generalLayout = new QBoxLayout(QBoxLayout::TopToBottom);
    generalLayout->setMargin(0);
    topPanel->setLayout(generalLayout);

    QBoxLayout *mainLayout = new QBoxLayout(QBoxLayout::LeftToRight);
    mainLayout->setMargin(0);
    mainLayout->setSpacing(1);

    mainLayout->addWidget(new QWidget());

    TImageButton *changeButton = new TImageButton(QIcon(QPixmap(THEME_DIR + "icons/exchange_colors.png")), 20, this, true);
    changeButton->setToolTip(tr("Exchange colors"));
    connect(changeButton, SIGNAL(clicked()), this, SLOT(switchColors()));
    mainLayout->addWidget(changeButton);
    mainLayout->setSpacing(5);

    QBoxLayout *listLayout = new QBoxLayout(QBoxLayout::TopToBottom);
    QLabel *contourLabel = new QLabel(tr("Contour"));
    QLabel *fillLabel = new QLabel(tr("Fill"));

    QSize cellSize(25, 25);
    // QColor contourColor(0, 0, 0);
    QBrush contourBrush = QBrush(Qt::black);
    contourColorCell = new TColorCell(TColorCell::Contour, contourBrush, cellSize);
    contourColorCell->setChecked(true);
    connect(contourColorCell, SIGNAL(clicked(TColorCell::FillType)), this, SLOT(updateColorMode(TColorCell::FillType)));

    QBrush fillBrush = QBrush(Qt::transparent);
    fillColorCell = new TColorCell(TColorCell::Inner, fillBrush, cellSize);
    connect(fillColorCell, SIGNAL(clicked(TColorCell::FillType)), this, SLOT(updateColorMode(TColorCell::FillType)));

    QBoxLayout *contourLayout = new QBoxLayout(QBoxLayout::LeftToRight);
    contourLayout->addWidget(contourColorCell);
    contourLayout->addWidget(contourLabel);

    QBoxLayout *fillLayout = new QBoxLayout(QBoxLayout::LeftToRight);
    fillLayout->addWidget(fillColorCell);
    fillLayout->addWidget(fillLabel);

    listLayout->addLayout(contourLayout);
    listLayout->setSpacing(5);
    listLayout->addLayout(fillLayout);

    mainLayout->addLayout(listLayout);

    TImageButton *resetButton = new TImageButton(QIcon(QPixmap(THEME_DIR + "icons/reset_colors.png")), 15, this, true);
    resetButton->setToolTip(tr("Reset colors"));
    connect(resetButton, SIGNAL(clicked()), this, SLOT(init()));
    mainLayout->addWidget(resetButton);

    mainLayout->addWidget(new QWidget());

    QLabel *htmlLabel = new QLabel(tr("HTML"), topPanel);
    htmlLabel->setMaximumWidth(50);
    htmlLabel->setAlignment(Qt::AlignHCenter|Qt::AlignVCenter);
    mainLayout->addWidget(htmlLabel);

    htmlField = new QLineEdit(topPanel);
    htmlField->setMaximumWidth(70);
    htmlField->setText("#000000");
    connect(htmlField, SIGNAL(editingFinished()), this, SLOT(updateColorFromHTML()));
    mainLayout->addWidget(htmlField);

    mainLayout->addWidget(new QWidget());

    generalLayout->addLayout(mainLayout);
    generalLayout->addWidget(new QWidget());
    generalLayout->setAlignment(mainLayout, Qt::AlignHCenter|Qt::AlignVCenter);

    generalLayout->addWidget(new TSeparator(Qt::Horizontal));    

    QBoxLayout *bgLayout = new QBoxLayout(QBoxLayout::LeftToRight);
    bgLayout->setMargin(0);
    bgLayout->setSpacing(1);

    bgLayout->addWidget(new QWidget());

    QBrush bgBrush = QBrush(Qt::white);
    bgColor = new TColorCell(TColorCell::Background, bgBrush, cellSize);
    connect(bgColor, SIGNAL(clicked(TColorCell::FillType)), this, SLOT(updateColorMode(TColorCell::FillType)));
    bgLayout->addWidget(bgColor);

    bgLayout->setSpacing(5);

    QLabel *bgLabel = new QLabel(tr("Background"), topPanel);
    bgLabel->setAlignment(Qt::AlignHCenter|Qt::AlignVCenter);
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
    bgHtmlField->setText("#ffffff");
    connect(bgHtmlField, SIGNAL(editingFinished()), this, SLOT(updateBgColorFromHTML()));
    bgLayout->addWidget(bgHtmlField);

    bgLayout->addWidget(new QWidget());

    generalLayout->addLayout(bgLayout);
    generalLayout->setAlignment(bgLayout, Qt::AlignHCenter);

    generalLayout->addWidget(new QWidget());

    addChild(topPanel);
}

void TupColorPaletteWidget::updateColorMode(TColorCell::FillType type)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupColorPaletteWidget::updateColorMode()] - type -> " << type;
    #endif

    QBrush brush;
    QColor color;
    TCONFIG->beginGroup("ColorPalette");
    TCONFIG->setValue("CurrentColorMode", type);

    if (type == TColorCell::Background) {
        paletteContainer->enableTransparentColor(false);
        currentSpace = TColorCell::Background;
        brush = bgColor->brush();
        color = brush.color();
        bgHtmlField->setText(color.name());

        if (contourColorCell->isChecked())
            contourColorCell->setChecked(false);
        if (fillColorCell->isChecked())
            fillColorCell->setChecked(false);
    } else {
        paletteContainer->enableTransparentColor(true);
        paletteContainer->resetBasicPanel();
        if (bgColor->isChecked())
            bgColor->setChecked(false);

        if (type == TColorCell::Contour) {
            currentSpace = TColorCell::Contour;
            brush = contourColorCell->brush();
            if (fillColorCell->isChecked())
                fillColorCell->setChecked(false);                
        } else if (type == TColorCell::Inner) {
            currentSpace = TColorCell::Inner;
            brush = fillColorCell->brush();
            if (contourColorCell->isChecked())
                contourColorCell->setChecked(false);
        }

        TCONFIG->setValue("TextColor", brush.color().name(QColor::HexArgb));
        color = brush.color();
        htmlField->setText(color.name());
    }

    emit colorSpaceChanged(type);

    if (fgType == Solid && tab->currentIndex() != 0) {
        tab->setCurrentIndex(0);
    } else if (fgType == Gradient && tab->currentIndex() != 1) {
        tab->setCurrentIndex(1);
    }

    updateLuminancePicker(color);
    colorForm->setColor(color);
    gradientManager->setCurrentColor(color);
}

void TupColorPaletteWidget::checkColorButton(TColorCell::FillType type)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupColorPaletteWidget::checkColorButton()] - type: " << type;
    #endif

    if (type == TColorCell::Contour) {
        contourColorCell->click();
    } else if (type == TColorCell::Inner) {
        fillColorCell->click();
    } else if (type == TColorCell::Background) {
        bgColor->click();
    }
}

void TupColorPaletteWidget::setupMainPalette()
{
    // Palettes
    paletteContainer = new TupViewColorCells(splitter);
    connect(paletteContainer, SIGNAL(colorSelected(const QBrush&)), this, SLOT(updateColorFromPalette(const QBrush&)));

    splitter->addWidget(paletteContainer);
}

void TupColorPaletteWidget::setupColorChooser()
{
    QFrame *colorMixer = new QFrame;
    colorMixer->setFrameStyle(QFrame::Box|QFrame::Sunken);

    QBoxLayout *mainLayout = new QBoxLayout(QBoxLayout::TopToBottom);
    colorMixer->setLayout(mainLayout);

    colorPickerArea = new TupColorPicker(colorMixer);
    connect(colorPickerArea, SIGNAL(newColor(int, int)), this, SLOT(setHS(int, int)));

    luminancePicker = new TSlider(Qt::Horizontal, TSlider::Color, QColor(0, 0, 0), QColor(255, 255, 255));
    connect(luminancePicker, SIGNAL(colorChanged(const QColor &)), this, SLOT(syncColor(const QColor &)));
    luminancePicker->setRange(0, 100);
    luminancePicker->setValue(100);

    colorForm = new TupColorForm;
    connect(colorForm, SIGNAL(brushChanged(const QBrush&)), this, SLOT(updateColorFromDisplay(const QBrush&)));

    mainLayout->addWidget(colorPickerArea);
    mainLayout->setAlignment(colorPickerArea, Qt::AlignHCenter);

    mainLayout->addWidget(luminancePicker);
    mainLayout->setAlignment(luminancePicker, Qt::AlignHCenter);

    mainLayout->addWidget(colorForm);
    mainLayout->setAlignment(colorForm, Qt::AlignHCenter);

    mainLayout->addStretch(2);

    tab->addTab(colorMixer, tr("Color Mixer"));
}

void TupColorPaletteWidget::setupGradientManager()
{
    gradientManager = new TupGradientCreator(this);
    // connect(gradientManager, SIGNAL(gradientChanged(const QBrush&)), this, SLOT(updateGradientColor(const QBrush &)));

    tab->addTab(gradientManager, tr("Gradients"));
    // SQA: Temporary code
    tab->setTabEnabled(1, false);
}

void TupColorPaletteWidget::setColorOnAppFromHTML(const QBrush& brush)
{
    QColor color = brush.color();

    if (color.isValid()) {
        if (type == Gradient)
            gradientManager->setCurrentColor(color);

        colorPickerArea->setColor(color.hue(), color.saturation());
        paletteContainer->setColor(brush);
        colorForm->setColor(color);

        // if (type == Solid)
        //     outlineAndFillColors->setCurrentColor(color);
    } else if (brush.gradient()) {
          QGradient gradient(*brush.gradient());
          // changeBrushType(tr("Gradient"));

          paletteContainer->setColor(gradient);
          // outlineAndFillColors->setCurrentColor(gradient);
          if (sender() != gradientManager)
              gradientManager->setGradient(gradient);

          // SQA: Gradient issue pending for revision
          // tFatal() << "TupColorPaletteWidget::setColor() - Sending gradient value!";
          // TupPaintAreaEvent event(TupPaintAreaEvent::ChangeBrush, brush);
          // emit paintAreaEventTriggered(&event);
          // return;
    }

    if (currentSpace == TColorCell::Background) {
        bgColor->setBrush(brush);

        TupPaintAreaEvent bgEvent(TupPaintAreaEvent::ChangeBgColor, bgColor->color());
        emit paintAreaEventTriggered(&bgEvent);
        return;
    }

    if (currentSpace == TColorCell::Contour) {
        contourColorCell->setBrush(brush);

        TupPaintAreaEvent contourEvent(TupPaintAreaEvent::ChangePenColor, contourColorCell->color());
        emit paintAreaEventTriggered(&contourEvent);
        return;
    } 

    if (currentSpace == TColorCell::Inner) {
        fillColorCell->setBrush(brush);

        TupPaintAreaEvent fillEvent(TupPaintAreaEvent::ChangeBrush, brush);
        emit paintAreaEventTriggered(&fillEvent);
    }
}

void TupColorPaletteWidget::setGlobalColors(const QBrush &brush)
{
    if (currentSpace == TColorCell::Background) {
        bgColor->setBrush(brush);
        bgHtmlField->setText(brush.color().name());

        TupPaintAreaEvent event(TupPaintAreaEvent::ChangeBgColor, brush.color());
        emit paintAreaEventTriggered(&event);
    } else {
        if (currentSpace == TColorCell::Contour) {
            if (brush.color() == Qt::transparent) {
                if (fillColorCell->color() == Qt::transparent) {
                    QBrush black(Qt::black);
                    fillColorCell->setBrush(black);
                    currentFillBrush = black;

                    TupPaintAreaEvent event(TupPaintAreaEvent::ChangeBrush, black);
                    emit paintAreaEventTriggered(&event);
                }
            }

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

            fillColorCell->setBrush(brush);
            currentFillBrush = brush;

            TupPaintAreaEvent event(TupPaintAreaEvent::ChangeBrush, brush);
            emit paintAreaEventTriggered(&event);
        }
        htmlField->setText(brush.color().name());
    }
}

void TupColorPaletteWidget::updateColorFromPalette(const QBrush &brush)
{
    colorPickerArea->clearSelection();

    setGlobalColors(brush);
    QColor color = brush.color();
    updateLuminancePicker(color);
    colorForm->setColor(color);
    gradientManager->setCurrentColor(color);
}

void TupColorPaletteWidget::updateColorFromDisplay(const QBrush &brush)
{
    setGlobalColors(brush);
    QColor color = brush.color();
    colorPickerArea->setColor(color.hue(), color.saturation());
    updateLuminancePicker(color);
}

void TupColorPaletteWidget::updateGradientColor(const QBrush &brush)
{
    setGlobalColors(brush);
}

void TupColorPaletteWidget::syncColor(const QColor &color)
{
    setGlobalColors(QBrush(color));
    colorForm->setColor(color);
}

void TupColorPaletteWidget::setHS(int hue, int saturation)
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

void TupColorPaletteWidget::updateColorFromHTML()
{
    QColor currentColor;
    if (currentSpace == TColorCell::Background) {
        bgColor->setChecked(false);
        currentSpace = TColorCell::Contour;
        contourColorCell->setChecked(true);
        currentColor = contourColorCell->color();
    } else if (currentSpace == TColorCell::Contour) {
        currentColor = contourColorCell->color();
    } else if (currentSpace == TColorCell::Inner) {
        currentColor = fillColorCell->color();
    }

    QString colorCode = htmlField->text();
    QColor color(colorCode);

    if (color != currentColor) {
        color.setAlpha(255);
        setColorOnAppFromHTML(color);
    }
}

void TupColorPaletteWidget::updateBgColorFromHTML()
{
    if (currentSpace != TColorCell::Background) {
        if (contourColorCell->isChecked())
            contourColorCell->setChecked(false);
        if (fillColorCell->isChecked())
            fillColorCell->setChecked(false);

        currentSpace = TColorCell::Background;
        bgColor->setChecked(true);
    }

    QColor currentColor = bgColor->color();
    QString colorCode = bgHtmlField->text();
    QColor color(colorCode);

    if (color != currentColor) {
        color.setAlpha(255);
        setColorOnAppFromHTML(color);
    }
}

QPair<QColor, QColor> TupColorPaletteWidget::color()
{
    QPair<QColor, QColor> colors;
    colors.first = contourColorCell->color();
    colors.second = fillColorCell->color();

    return colors;
}

void TupColorPaletteWidget::parsePaletteFile(const QString &file)
{
    paletteContainer->readPaletteFile(file);
}


void TupColorPaletteWidget::saveTextColor(const QColor &color)
{
    TCONFIG->beginGroup("ColorPalette");
    TCONFIG->setValue("TextColor", color.name(QColor::HexArgb));
}

void TupColorPaletteWidget::init()
{
    if (bgColor->isChecked())
        bgColor->setChecked(false);

    currentSpace = TColorCell::Contour;

    QColor contourColor = Qt::black;
    saveTextColor(Qt::black);
    currentContourBrush = QBrush(contourColor);
    htmlField->setText("#000000");

    QBrush fillBrush = QBrush(Qt::transparent);
    currentFillBrush = fillBrush;

    contourColorCell->setBrush(currentContourBrush);
    fillColorCell->setBrush(fillBrush);

    if (fillColorCell->isChecked())
        fillColorCell->setChecked(false);
    contourColorCell->setChecked(true);

    paletteContainer->clearSelection();

    colorPickerArea->setColor(contourColor.hue(), contourColor.saturation());

    if (!luminancePicker->isEnabled())
        luminancePicker->setEnabled(true);

    blockSignals(true);
    luminancePicker->setColors(Qt::black, Qt::white);
    luminancePicker->setValue(0);
    colorForm->setColor(contourColor);
    gradientManager->setCurrentColor(Qt::white);
    blockSignals(false);

    emit colorSpaceChanged(TColorCell::Contour);

    TupPaintAreaEvent fillEvent(TupPaintAreaEvent::ChangeBrush, currentFillBrush);
    emit paintAreaEventTriggered(&fillEvent);

    TupPaintAreaEvent event(TupPaintAreaEvent::ChangePenColor, contourColor);
    emit paintAreaEventTriggered(&event);
}

void TupColorPaletteWidget::setBgColor(const QColor &color)
{
    QBrush brush(color);
    bgColor->setBrush(brush);
}

void TupColorPaletteWidget::initBg()
{
    QBrush brush(Qt::white);
    bgColor->setBrush(brush);
    bgColor->setChecked(true);
    updateColorMode(TColorCell::Background);

    paletteContainer->clearSelection();

    TupPaintAreaEvent event(TupPaintAreaEvent::ChangeBgColor, brush.color());
    emit paintAreaEventTriggered(&event);
}

void TupColorPaletteWidget::switchColors()
{
    paletteContainer->clearSelection();

    QBrush tmp = currentContourBrush;
    currentContourBrush = currentFillBrush;
    currentFillBrush = tmp;

    if (bgColor->isChecked())
        bgColor->setChecked(false);
    contourColorCell->setBrush(currentContourBrush);
    fillColorCell->setBrush(currentFillBrush);

    QColor color;
    if (currentSpace == TColorCell::Contour)
        color = contourColorCell->color();
    else
        color = fillColorCell->color();

    blockSignals(true);
    htmlField->setText(color.name());
    colorPickerArea->setColor(color.hue(), color.saturation());
    updateLuminancePicker(color);
    colorForm->setColor(color);
    blockSignals(false);

    TupPaintAreaEvent event = TupPaintAreaEvent(TupPaintAreaEvent::ChangeBrush, currentFillBrush);
    emit paintAreaEventTriggered(&event);

    event = TupPaintAreaEvent(TupPaintAreaEvent::ChangePenColor, currentContourBrush.color());
    emit paintAreaEventTriggered(&event);

    currentSpace = TColorCell::Contour;
}

void TupColorPaletteWidget::updateColorType(int index)
{
    if (index == TupColorPaletteWidget::Solid) {
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

void TupColorPaletteWidget::updateLuminancePicker(const QColor &color)
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

void TupColorPaletteWidget::updateContourColor(const QColor &color)
{
    if (bgColor->isChecked())
        bgColor->setChecked(false);

    if (fillColorCell->isChecked())
        fillColorCell->setChecked(false);

    if (!contourColorCell->isChecked())
        contourColorCell->setChecked(true);

    if (color != contourColorCell->color()) {
        contourColorCell->setBrush(QBrush(color));
        updateColorMode(TColorCell::Contour);
    }

    saveTextColor(color);
}

void TupColorPaletteWidget::updateFillColor(const QColor &color)
{
    if (bgColor->isChecked())
        bgColor->setChecked(false);

    if (contourColorCell->isChecked())
        contourColorCell->setChecked(false);

    if (!fillColorCell->isChecked())
        fillColorCell->setChecked(true);

    if (color != fillColorCell->color()) {
        fillColorCell->setBrush(QBrush(color));
        updateColorMode(TColorCell::Inner);
    }

    saveTextColor(color);
}

void TupColorPaletteWidget::updateBgColor(const QColor &color)
{
    if (contourColorCell->isChecked())
        contourColorCell->setChecked(false);

    if (fillColorCell->isChecked())
        fillColorCell->setChecked(false);

    if (!bgColor->isChecked())
        bgColor->setChecked(true);

    if (color != bgColor->color()) {
        bgColor->setBrush(QBrush(color));
        updateColorMode(TColorCell::Background);
    }
}

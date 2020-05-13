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

#include "tupbrushwidget.h"

TupBrushWidget::TupBrushWidget(QWidget *parent) : TupModuleWidgetBase(parent)
{
    setWindowTitle(tr("Brush Properties"));

    TCONFIG->beginGroup("BrushParameters");
    int thicknessValue = TCONFIG->value("Thickness", 3).toInt();
    if (thicknessValue > 100)
        thicknessValue = 3;

    thickPreview = new TupPenThicknessWidget(this);
    thickPreview->render(thicknessValue);

    thickness = new TEditSpinBox(thicknessValue, 1, 100, 1, tr("Thickness"));
    thickness->setValue(thicknessValue);

    connect(thickness, SIGNAL(valueChanged(int)), this, SLOT(setThickness(int)));
    connect(thickness, SIGNAL(valueChanged(int)), thickPreview, SLOT(render(int)));

    addChild(thickPreview);
    addChild(thickness);

    QWidget *space = new QWidget(this);
    space->setFixedHeight(5);
    addChild(space);

    QLabel *label = new QLabel(tr("Dashes") + ":", this);
    addChild(label);

    QWidget *styleWidget = new QWidget(this);
    QBoxLayout *styleLayout = new QHBoxLayout(styleWidget);

    style = new QComboBox();
    style->setIconSize(QSize(145, 13));
    style->setFixedWidth(180);

    int flag = Qt::SolidLine;
    style->addItem(QIcon(THEME_DIR + "icons/line_style01.png"), "", QVariant(flag));
    flag = Qt::DashLine;
    style->addItem(QIcon(THEME_DIR + "icons/line_style02.png"), "", QVariant(flag));
    flag = Qt::DotLine;
    style->addItem(QIcon(THEME_DIR + "icons/line_style03.png"), "", QVariant(flag));
    flag = Qt::DashDotLine;
    style->addItem(QIcon(THEME_DIR + "icons/line_style04.png"), "", QVariant(flag));
    flag = Qt::DashDotDotLine;
    style->addItem(QIcon(THEME_DIR + "icons/line_style05.png"), "", QVariant(flag));

    styleLayout->addWidget(style);
    connect(style, SIGNAL(currentIndexChanged(int)), this, SLOT(setStyle(int)));

    addChild(styleWidget);

    space = new QWidget(this);
    space->setFixedHeight(5);
    addChild(space);

    label = new QLabel(tr("Cap") + ":", this);
    addChild(label);

    QWidget *capWidget = new QWidget(this);
    QBoxLayout *capLayout = new QHBoxLayout(capWidget);

    roundCapButton = new QPushButton(QIcon(QPixmap(THEME_DIR + "icons/round_cap.png")), "");
    roundCapButton->setToolTip(tr("Round"));
    roundCapButton->setIconSize(QSize(30, 15));
    roundCapButton->setCheckable(true);

    connect(roundCapButton, SIGNAL(clicked()), this, SLOT(enableRoundCapStyle()));

    squareCapButton = new QPushButton(QIcon(QPixmap(THEME_DIR + "icons/square_cap.png")), "");
    squareCapButton->setToolTip(tr("Square"));
    squareCapButton->setIconSize(QSize(33, 15));
    squareCapButton->setCheckable(true);

    connect(squareCapButton, SIGNAL(clicked()), this, SLOT(enableSquareCapStyle()));

    flatCapButton = new QPushButton(QIcon(QPixmap(THEME_DIR + "icons/flat_cap.png")), "");
    flatCapButton->setToolTip(tr("Flat"));
    flatCapButton->setIconSize(QSize(27, 15));
    flatCapButton->setCheckable(true);

    connect(flatCapButton, SIGNAL(clicked()), this, SLOT(enableFlatCapStyle()));

    capLayout->addWidget(roundCapButton);
    capLayout->addWidget(squareCapButton);
    capLayout->addWidget(flatCapButton);
    addChild(capWidget);

    space = new QWidget(this);
    space->setFixedHeight(5);
    addChild(space);

    label = new QLabel(tr("Join") + ":", this);
    addChild(label);

    QWidget *joinWidget = new QWidget(this);
    QBoxLayout *joinLayout = new QHBoxLayout(joinWidget);

    roundJoinButton = new QPushButton(QIcon(QPixmap(THEME_DIR + "icons/round_join.png")), "");
    roundJoinButton->setToolTip(tr("Round"));
    roundJoinButton->setIconSize(QSize(30, 15));
    roundJoinButton->setCheckable(true);

    connect(roundJoinButton, SIGNAL(clicked()), this, SLOT(enableRoundJoinStyle()));

    bevelJoinButton = new QPushButton(QIcon(QPixmap(THEME_DIR + "icons/bevel_join.png")), "");
    bevelJoinButton->setToolTip(tr("Bevel"));
    bevelJoinButton->setIconSize(QSize(33, 15));
    bevelJoinButton->setCheckable(true);

    connect(bevelJoinButton, SIGNAL(clicked()), this, SLOT(enableBevelJoinStyle()));

    miterJoinButton = new QPushButton(QIcon(QPixmap(THEME_DIR + "icons/miter_join.png")), "");
    miterJoinButton->setToolTip(tr("Miter"));
    miterJoinButton->setIconSize(QSize(27, 15));
    miterJoinButton->setCheckable(true);

    connect(miterJoinButton, SIGNAL(clicked()), this, SLOT(enableMiterJoinStyle()));

    joinLayout->addWidget(roundJoinButton);
    joinLayout->addWidget(bevelJoinButton);
    joinLayout->addWidget(miterJoinButton);
    addChild(joinWidget);

    space = new QWidget(this);
    space->setFixedHeight(5);
    addChild(space);

    label = new QLabel(tr("Brush") + ":", this);
    addChild(label);

    addBrushesList();
 
    boxLayout()->addStretch(2);
    
    setWindowIcon(QIcon(THEME_DIR + "icons/brush.png"));
}

TupBrushWidget::~TupBrushWidget()
{
    #ifdef TUP_DEBUG
        qDebug() << "[~TupBrushWidget()]";
    #endif
}

void TupBrushWidget::setThickness(int width)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupBrushWidget::setThickness()]";
        qDebug() << "thickness: " << width;
    #endif

    if (width > 0) {
        pen.setWidth(width);
        TCONFIG->beginGroup("BrushParameters");
        TCONFIG->setValue("Thickness", width);
        updatePenProperties();
    }
}

void TupBrushWidget::setStyle(int styleCode)
{
    pen.setStyle(Qt::PenStyle(style->itemData(styleCode).toInt()));
    updatePenProperties();
}

void TupBrushWidget::setBrushStyle(QListWidgetItem *item)
{
    if (item->toolTip().compare("TexturePattern") == 0) {
        brush = QBrush(QPixmap(THEME_DIR + "icons/brush_15.png"));
        thickPreview->setBrush(24);
    } else {
        int index = brushesList->row(item);
        thickPreview->setBrush(index+1);
        brush.setStyle(Qt::BrushStyle(index+1));
    }

    updatePenProperties();
}

void TupBrushWidget::setPenColor(const QColor color)
{
    brush.setColor(color);
    thickPreview->setColor(color);
}

void TupBrushWidget::setPenThickness(int width)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupBrushWidget::setPenThickness()]";
        qDebug() << "thickness: " << width;
    #endif

    pen.setWidth(width);
    TCONFIG->beginGroup("BrushParameters");
    TCONFIG->setValue("Thickness", width);
    thickPreview->render(width);

    thickness->blockSignals(true);
    thickness->setValue(width);
    thickness->blockSignals(false);
}

void TupBrushWidget::setBrush(const QBrush b)
{
    brush = b;
    thickPreview->setBrush(b);
}

void TupBrushWidget::init(int width)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupBrushWidget::init()]";
        qDebug() << "thickness: " << width;
    #endif

    blockSignals(true);
    setPenColor(QColor(0, 0, 0));

    enableRoundCapStyle();
    enableRoundJoinStyle();
    style->setCurrentIndex(0);

    QListWidgetItem *first = brushesList->item(0);
    brushesList->setCurrentItem(first);
    setBrushStyle(first);
    blockSignals(false);

    setThickness(width);
}

QPen TupBrushWidget::getPen() const
{
    return pen;
}

void TupBrushWidget::updatePenProperties()
{
    pen.setBrush(brush);

    TupPaintAreaEvent event(TupPaintAreaEvent::ChangePen, pen);
    emit paintAreaEventTriggered(&event);
}

void TupBrushWidget::updateBrushProperties()
{
    TupPaintAreaEvent event(TupPaintAreaEvent::ChangeBrush, brush);
    emit paintAreaEventTriggered(&event);
}

void TupBrushWidget::addBrushesList()
{
    brushesList = new QListWidget(this);
    brushesList->setViewMode(QListView::IconMode);
    brushesList->setFlow(QListView::LeftToRight);
    brushesList->setMovement(QListView::Static);

    QListWidgetItem *brushItem = new QListWidgetItem(brushesList);
    brushItem->setIcon(QIcon(THEME_DIR + "icons/brush_01.png"));
    brushItem->setFont(QFont("verdana", 8));
    brushItem->setToolTip("Solid");
    brushItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);

    brushItem = new QListWidgetItem(brushesList);
    brushItem->setIcon(QIcon(THEME_DIR + "icons/brush_02.png"));
    brushItem->setFont(QFont("verdana", 8));
    brushItem->setToolTip("Dense1Pattern");
    brushItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);

    brushItem = new QListWidgetItem(brushesList);
    brushItem->setIcon(QIcon(THEME_DIR + "icons/brush_03.png"));
    brushItem->setFont(QFont("verdana", 8));
    brushItem->setToolTip("Dense2Pattern");
    brushItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);

    brushItem = new QListWidgetItem(brushesList);
    brushItem->setIcon(QIcon(THEME_DIR + "icons/brush_04.png"));
    brushItem->setFont(QFont("verdana", 8));
    brushItem->setToolTip("Dense3Pattern");
    brushItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);

    brushItem = new QListWidgetItem(brushesList);
    brushItem->setIcon(QIcon(THEME_DIR + "icons/brush_05.png"));
    brushItem->setFont(QFont("verdana", 8));
    brushItem->setToolTip("Dense4Pattern");
    brushItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);

    brushItem = new QListWidgetItem(brushesList);
    brushItem->setIcon(QIcon(THEME_DIR + "icons/brush_06.png"));
    brushItem->setFont(QFont("verdana", 8));
    brushItem->setToolTip("Dense5Pattern");
    brushItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);

    brushItem = new QListWidgetItem(brushesList);
    brushItem->setIcon(QIcon(THEME_DIR + "icons/brush_07.png"));
    brushItem->setFont(QFont("verdana", 8));
    brushItem->setToolTip("Dense6Pattern");
    brushItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);

    brushItem = new QListWidgetItem(brushesList);
    brushItem->setIcon(QIcon(THEME_DIR + "icons/brush_08.png"));
    brushItem->setFont(QFont("verdana", 8));
    brushItem->setToolTip("Dense7Pattern");
    brushItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);

    brushItem = new QListWidgetItem(brushesList);
    brushItem->setIcon(QIcon(THEME_DIR + "icons/brush_09.png"));
    brushItem->setFont(QFont("verdana", 8));
    brushItem->setToolTip("HotPattern");
    brushItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);

    brushItem = new QListWidgetItem(brushesList);
    brushItem->setIcon(QIcon(THEME_DIR + "icons/brush_10.png"));
    brushItem->setFont(QFont("verdana", 8));
    brushItem->setToolTip("VerPattern");
    brushItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);

    brushItem = new QListWidgetItem(brushesList);
    brushItem->setIcon(QIcon(THEME_DIR + "icons/brush_11.png"));
    brushItem->setFont(QFont("verdana", 8));
    brushItem->setToolTip("CrossPattern");
    brushItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);

    brushItem = new QListWidgetItem(brushesList);
    brushItem->setIcon(QIcon(THEME_DIR + "icons/brush_12.png"));
    brushItem->setFont(QFont("verdana", 8));
    brushItem->setToolTip("BDiagPattern");
    brushItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);

    brushItem = new QListWidgetItem(brushesList);
    brushItem->setIcon(QIcon(THEME_DIR + "icons/brush_13.png"));
    brushItem->setFont(QFont("verdana", 8));
    brushItem->setToolTip("FDiagPattern");
    brushItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);

    brushItem = new QListWidgetItem(brushesList);
    brushItem->setIcon(QIcon(THEME_DIR + "icons/brush_14.png"));
    brushItem->setFont(QFont("verdana", 8));
    brushItem->setToolTip("DiagCrossPattern");
    brushItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);

    /*
    brushItem = new QListWidgetItem(brushesList);
    brushItem->setIcon(QIcon(THEME_DIR + "icons/brush_15.png"));
    brushItem->setFont(QFont("verdana", 8));
    brushItem->setToolTip("TexturePattern");
    brushItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);

    brushItem = new QListWidgetItem(brushesList);
    brushItem->setIcon(QIcon(THEME_DIR + "icons/brush.png"));
    brushItem->setFont(QFont("verdana", 8));
    brushItem->setToolTip("LinearGradientPattern");
    brushItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);

    brushItem = new QListWidgetItem(brushesList);
    brushItem->setIcon(QIcon(THEME_DIR + "icons/brush.png"));
    brushItem->setFont(QFont("verdana", 8));
    brushItem->setToolTip("ConicalGradientPattern");
    brushItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);

    brushItem = new QListWidgetItem(brushesList);
    brushItem->setIcon(QIcon(THEME_DIR + "icons/brush.png"));
    brushItem->setFont(QFont("verdana", 8));
    brushItem->setToolTip("RadialGradientPattern");
    brushItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
    */

    brushesList->setFixedWidth(255);
    brushesList->setFixedHeight(63);

    addChild(brushesList);
    connect(brushesList, SIGNAL(itemClicked(QListWidgetItem *)), this, SLOT(setBrushStyle(QListWidgetItem *)));
}

void TupBrushWidget::enableRoundCapStyle()
{
    if (!roundCapButton->isDown())
        roundCapButton->setChecked(true);

    if (squareCapButton->isChecked())
        squareCapButton->setChecked(false);

    if (flatCapButton->isChecked())
        flatCapButton->setChecked(false);

    pen.setCapStyle(Qt::RoundCap);

    updatePenProperties();
}

void TupBrushWidget::enableSquareCapStyle()
{
    if (!squareCapButton->isDown())
        squareCapButton->setChecked(true);

    if (roundCapButton->isChecked())
        roundCapButton->setChecked(false);

    if (flatCapButton->isChecked())
        flatCapButton->setChecked(false);

    pen.setCapStyle(Qt::SquareCap);
    updatePenProperties();
}

void TupBrushWidget::enableFlatCapStyle()
{
    if (!flatCapButton->isDown())
        flatCapButton->setChecked(true);

    if (roundCapButton->isChecked())
        roundCapButton->setChecked(false);

    if (squareCapButton->isChecked())
        squareCapButton->setChecked(false);

    pen.setCapStyle(Qt::FlatCap);
    updatePenProperties();
}

void TupBrushWidget::enableRoundJoinStyle()
{
    if (!roundJoinButton->isDown())
        roundJoinButton->setChecked(true);

    if (miterJoinButton->isChecked())
        miterJoinButton->setChecked(false);

    if (bevelJoinButton->isChecked())
        bevelJoinButton->setChecked(false);

    pen.setJoinStyle(Qt::RoundJoin);
    updatePenProperties();
}

void TupBrushWidget::enableMiterJoinStyle()
{
    if (!miterJoinButton->isDown())
        miterJoinButton->setChecked(true);

    if (bevelJoinButton->isChecked())
        bevelJoinButton->setChecked(false);

    if (roundJoinButton->isChecked())
        roundJoinButton->setChecked(false);

    pen.setJoinStyle(Qt::MiterJoin);
    updatePenProperties();
}

void TupBrushWidget::enableBevelJoinStyle()
{
    if (!bevelJoinButton->isDown())
        bevelJoinButton->setChecked(true);

    if (miterJoinButton->isChecked())
        miterJoinButton->setChecked(false);
    
    if (roundJoinButton->isChecked())
        roundJoinButton->setChecked(false);

    pen.setJoinStyle(Qt::BevelJoin);
    updatePenProperties();
}

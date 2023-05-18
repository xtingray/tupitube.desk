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

#include "geometricsettings.h"
#include "tapplicationproperties.h"
#include "tconfig.h"
#include "tseparator.h"

#include <QVBoxLayout>
#include <QGroupBox>

GeometricSettings::GeometricSettings(GeometricSettings::ToolType type, QWidget *parent) : QWidget(parent)
{
    #ifdef TUP_DEBUG
        qDebug() << "[GeometricSettings()]";
    #endif

    QBoxLayout *mainLayout = new QBoxLayout(QBoxLayout::TopToBottom, this);
    QBoxLayout *layout = new QBoxLayout(QBoxLayout::TopToBottom);

    QLabel *toolTitle = new QLabel;
    toolTitle->setAlignment(Qt::AlignHCenter);
    QPixmap pic;

    if (type == GeometricSettings::Rectangle) {
        pic = QPixmap(THEME_DIR + "icons/square.png");
        toolTitle->setToolTip(tr("Rectangle Properties"));
    } else if (type == GeometricSettings::Ellipse) {
        pic = QPixmap(THEME_DIR + "icons/ellipse.png");
        toolTitle->setToolTip(tr("Ellipse Properties"));
    } else if (type == GeometricSettings::Line) {
        pic = QPixmap(THEME_DIR + "icons/line.png");
        toolTitle->setToolTip(tr("Line Properties"));
    }

    toolTitle->setPixmap(pic.scaledToWidth(16, Qt::SmoothTransformation));
    layout->addWidget(toolTitle);
    layout->addWidget(new TSeparator(Qt::Horizontal));

    if (type == GeometricSettings::Line) {
        QGroupBox *groupBox = new QGroupBox(tr("Line Options"));
        QVBoxLayout *lineLayout = new QVBoxLayout(groupBox);
        option1 = new QRadioButton(tr("Bendable"));
        option2 = new QRadioButton(tr("Straight"));

        TCONFIG->beginGroup("GeometricTool");
        int type = TCONFIG->value("LineType", 0).toInt();
        if (type)
            option2->setChecked(true);
        else
            option1->setChecked(true);

        connect(option1, SIGNAL(toggled(bool)), this, SLOT(sendLineState(bool)));
        connect(option2, SIGNAL(toggled(bool)), this, SLOT(sendLineState(bool)));

        lineLayout->addWidget(option1);
        lineLayout->addWidget(option2);
        layout->addWidget(groupBox);
    }

    QLabel *label = new QLabel(tr("Tips"));
    label->setAlignment(Qt::AlignHCenter); 
    layout->addWidget(label);

    mainLayout->addLayout(layout);

    QTextEdit *textArea = new QTextEdit; 

    // SQA: Check this code with several screen resolutions. It must looks good with everyone! 
    // QFont font = this->font();
    // font.setPointSize(8);
    // textArea->setFont(font);
    // textArea->setFont(QFont("Arial", 8, QFont::Normal, false));

    if (type == GeometricSettings::Line) {
        textArea->append("<p><b>" + tr("Mouse Right Click or X Key") + ":</b> " +  tr("Close the line path") + "</p>");
        textArea->append("<p><b>" + tr("Shift") + ":</b> " +  tr("Align line to horizontal/vertical axis") + "</p>"); 
    } else {
        textArea->append("<p><b>" + tr("Ctrl + Left Mouse Button") + ":</b> " +  tr("Set width/height proportional dimensions") + "</p>");
    }

    // QString text = textArea->document()->toPlainText();
    // textArea->setFixedHeight(100);
    // textArea->setFixedHeight(150);

    mainLayout->addWidget(textArea);
    mainLayout->addStretch(2);
}

GeometricSettings::~GeometricSettings()
{
}

void GeometricSettings::sendLineState(bool state)
{
    #ifdef TUP_DEBUG
        qDebug() << "[GeometricSettings::sendLineState()] - state -> " << state;
    #endif

    if (option1->isChecked())
        emit lineTypeChanged(GeometricSettings::Bendable);
    else
        emit lineTypeChanged(GeometricSettings::Straight);
}

void GeometricSettings::updateLineType(int type)
{
    #ifdef TUP_DEBUG
        qDebug() << "[GeometricSettings::updateLineType()] - type -> " << type;
    #endif

    if (type) {
        option2->blockSignals(true);
        option2->setChecked(true);
        option2->blockSignals(false);
    } else {
        option1->blockSignals(true);
        option1->setChecked(true);
        option1->blockSignals(false);
    }
}

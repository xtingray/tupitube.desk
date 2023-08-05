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

#include "tupprojectactionbar.h"
#include "tresponsiveui.h"
#include "talgorithm.h"

TupProjectActionBar::TupProjectActionBar(const QString &tag, QList<Action> actionsList,
                                         Qt::Orientation direction, QWidget *parent) : QWidget(parent)
{
    container = tag;
    orientation = direction;

    QPair<int, int> dimension = TAlgorithm::screenDimension();
    screenWidth = dimension.first;
    screenHeight = dimension.second;

    connect(&actions, SIGNAL(buttonClicked(int)), this, SLOT(emitActionSelected(int)));
    setup(actionsList);
}

TupProjectActionBar::~TupProjectActionBar()
{
}

void TupProjectActionBar::setup(QList<Action> actionsList)
{
    QBoxLayout *mainLayout = nullptr;
    
    switch (orientation) {
        case Qt::Vertical:
        {
            mainLayout = new QBoxLayout(QBoxLayout::LeftToRight, this);
            buttonLayout = new QBoxLayout(QBoxLayout::TopToBottom);
        }
        break;
        case Qt::Horizontal:
        {
            mainLayout = new QBoxLayout(QBoxLayout::TopToBottom, this);
            buttonLayout = new QBoxLayout(QBoxLayout::LeftToRight);
        }
        break;
    }
    
    mainLayout->setSpacing(0);
    mainLayout->setMargin(1);
    
    buttonLayout->setSpacing(1);
    buttonLayout->setMargin(1);
    buttonLayout->addStretch();

    int iconSize = TResponsiveUI::fitActionBarIconSize();

    foreach(Action action, actionsList) {
        if (action == InsertFrame) {
            TImageButton *button = new TImageButton(QIcon(ICONS_DIR + "add_frame.png"), iconSize);
            button->setToolTip(tr("Insert frame"));
            button->setShortcut(QKeySequence(Qt::Key_9));
            actions.addButton(button, InsertFrame);

            buttonLayout->addWidget(button);
            // button->setAnimated(isAnimated);
        }

        if (action == ExtendFrame) {
            TImageButton *button = new TImageButton(QIcon(ICONS_DIR + "extend_frame.png"), iconSize);
            button->setToolTip(tr("Extend frame"));
            // button->setShortcut(QKeySequence());

            actions.addButton(button, ExtendFrame);

            buttonLayout->addWidget(button);
            // button->setAnimated(isAnimated);
        }

        if (action == RemoveFrame) {
            TImageButton *button = new TImageButton(QIcon(ICONS_DIR + "remove_frame.png"), iconSize);
            button->setToolTip(tr("Remove frame"));
            // SQA: This short-cut has been moved to Zoom Out feature
            button->setShortcut(QKeySequence(Qt::Key_0));

            actions.addButton(button, RemoveFrame);

            buttonLayout->addWidget(button);
            // button->setAnimated(isAnimated);
        }

        if (action == MoveFrameBackward) {
            TImageButton *button = new TImageButton();
            if (container.compare("Exposure") == 0) {
                button = new TImageButton(QIcon(ICONS_DIR + "move_frame_up.png"), iconSize);
            } else {
                if (container.compare("TimeLine") == 0)
                    button = new TImageButton(QIcon(ICONS_DIR + "move_frame_backward.png"), iconSize);
            }

            button->setToolTip(tr("Move frame backward"));
            button->setShortcut(QKeySequence(tr("F8")));

            actions.addButton(button, MoveFrameBackward);

            buttonLayout->addWidget(button);
            // button->setAnimated(isAnimated);
        }

        if (action == MoveFrameForward) {
            TImageButton *button = new TImageButton();
            if (container.compare("Exposure") == 0) {
                button = new TImageButton(QIcon(ICONS_DIR + "move_frame_down.png"), iconSize);
            } else {
                if (container.compare("TimeLine") == 0)
                    button = new TImageButton(QIcon(ICONS_DIR + "move_frame_forward.png"), iconSize);
            }

            button->setToolTip(tr("Move frame forward"));
            button->setShortcut(QKeySequence(tr("F9")));

            actions.addButton(button, MoveFrameForward);

            buttonLayout->addWidget(button);
            // button->setAnimated(isAnimated);
        }

        if (action == ReverseFrameSelection) {
            TImageButton *button = new TImageButton();
            if (container.compare("Exposure") == 0) {
                button = new TImageButton(QIcon(ICONS_DIR + "reverse_v.png"), iconSize);
            } else {
                if (container.compare("TimeLine") == 0)
                    button = new TImageButton(QIcon(ICONS_DIR + "reverse_h.png"), iconSize);
            }

            button->setToolTip(tr("Reverse frame selection"));
            // button->setShortcut(QKeySequence(tr("F9")));

            actions.addButton(button, ReverseFrameSelection);

            buttonLayout->addWidget(button);
            // button->setAnimated(isAnimated);
        }

        if (action == CopyFrame) {
            TImageButton *button = new TImageButton(QIcon(ICONS_DIR + "copy.png"), iconSize);
            button->setToolTip(tr("Copy frame"));
            actions.addButton(button, CopyFrame);

            buttonLayout->addWidget(button);
            // button->setAnimated(isAnimated);
        }

        if (action == PasteFrame) {
            TImageButton *button = new TImageButton(QIcon(ICONS_DIR + "paste.png"), iconSize);
            button->setToolTip(tr("Paste frame"));
            actions.addButton(button, PasteFrame);

            buttonLayout->addWidget(button);
            // button->setAnimated(isAnimated);
        }

        if (action == InsertLayer) {
            TImageButton *button = new TImageButton(QIcon(ICONS_DIR + "add_layer.png"), iconSize);
            button->setToolTip(tr("Insert layer"));
            button->setShortcut(QKeySequence(tr("F5")));

            actions.addButton(button, InsertLayer);

            buttonLayout->addWidget(button);
            // button->setAnimated(isAnimated);
        }

        if (action == RemoveLayer) {
            TImageButton *button = new TImageButton(QIcon(ICONS_DIR + "remove_layer.png"), iconSize);
            button->setToolTip(tr("Remove layer"));
            button->setShortcut(QKeySequence(tr("F6")));

            actions.addButton(button, RemoveLayer);

            buttonLayout->addWidget(button);
            // button->setAnimated(isAnimated);
        }

        if (action == InsertScene) {
            TImageButton *button = new TImageButton(QIcon(ICONS_DIR + "add_scene.png"), iconSize);
            button->setToolTip(tr("Insert scene"));

            actions.addButton(button, InsertScene);

            buttonLayout->addWidget(button);
            // button->setAnimated(isAnimated);
        }

        if (action == RemoveScene) {
            TImageButton *button = new TImageButton(QIcon(ICONS_DIR + "remove_scene.png"), iconSize);
            button->setToolTip(tr("Remove scene"));

            actions.addButton(button, RemoveScene);

            buttonLayout->addWidget(button);
            // button->setAnimated(isAnimated);
        }

        if (action == Separator) {
            buttonLayout->addSpacing(3);
            buttonLayout->addWidget(new TSeparator(Qt::Vertical));
            buttonLayout->addSpacing(3);
        }
    }

    buttonLayout->addStretch();
    
    mainLayout->addWidget(new TSeparator(Qt::Horizontal));
    mainLayout->addLayout(buttonLayout);
    mainLayout->addWidget(new TSeparator(Qt::Horizontal));
}

// SQA: Check why this function do nothing :S

void TupProjectActionBar::insertSeparator(int position)
{
    buttonLayout->insertWidget(position + 1, new TSeparator(Qt::Vertical), 1, Qt::AlignCenter);
}

void TupProjectActionBar::insertBlankSpace(int position)
{
    QWidget *widget = new QWidget();
    widget->setFixedSize(5,5);
   
    buttonLayout->insertWidget(position + 1, widget, 1, Qt::AlignCenter);
}

TImageButton *TupProjectActionBar::button(Action action)
{
    return qobject_cast<TImageButton *>(actions.button(action));
}

void TupProjectActionBar::emitActionSelected(int action)
{
    switch(action) {
        case RemoveFrame:
        {
            TCONFIG->beginGroup("General");
            bool ask = TCONFIG->value("ConfirmRemoveFrame", true).toBool();

            if (ask) {
                TOptionalDialog dialog(tr("Do you want to remove current selection?"), tr("Confirmation"), true, false, this);
                dialog.setModal(true);

                dialog.move(static_cast<int> ((screenWidth - dialog.sizeHint().width()) / 2),
                            static_cast<int> ((screenHeight - dialog.sizeHint().height()) / 2));

                if (dialog.exec() == QDialog::Rejected)
                    return;

                TCONFIG->beginGroup("General");
                TCONFIG->setValue("ConfirmRemoveFrame", dialog.shownAgain());
                TCONFIG->sync();
            }
        }
        break;
        case RemoveLayer:
        {
            TCONFIG->beginGroup("General");
            bool ask = TCONFIG->value("ConfirmRemoveLayer", true).toBool();

            if (ask) {
                TOptionalDialog dialog(tr("Do you want to remove this layer?"), tr("Confirmation"), true, false, this);

                dialog.move(static_cast<int> ((screenWidth - dialog.sizeHint().width()) / 2),
                            static_cast<int> ((screenHeight - dialog.sizeHint().height()) / 2));

                if (dialog.exec() == QDialog::Rejected)
                    return;

                TCONFIG->beginGroup("General");
                TCONFIG->setValue("ConfirmRemoveLayer", dialog.shownAgain());
                TCONFIG->sync();
            }
        }
        break;
        case RemoveScene:
        {
            TCONFIG->beginGroup("General");
            bool ask = TCONFIG->value("ConfirmRemoveScene", true).toBool();

            if (ask) {
                TOptionalDialog dialog(tr("Do you want to remove this scene?"), tr("Confirmation"), true, false, this);

                dialog.move(static_cast<int> ((screenWidth - dialog.sizeHint().width()) / 2),
                            static_cast<int> ((screenHeight - dialog.sizeHint().height()) / 2));

                if (dialog.exec() == QDialog::Rejected)
                    return;

                TCONFIG->beginGroup("General");
                TCONFIG->setValue("ConfirmRemoveScene", dialog.shownAgain());
                TCONFIG->sync();
            }
        }
        break;
    }
    
    emit actionSelected(action);
}

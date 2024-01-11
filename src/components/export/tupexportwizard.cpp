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

#include "tupexportwizard.h"

TupExportWizard::TupExportWizard(QWidget *parent) : QDialog(parent)
{
    setModal(true);

    cancelButton = new QPushButton(tr("Cancel"));
    backButton = new QPushButton(tr("Back"));
    nextButton = new QPushButton(tr("Next"));

    connect(cancelButton, SIGNAL(clicked()), this, SLOT(cancel()));
    connect(backButton, SIGNAL(clicked()), this, SLOT(back()));
    connect(nextButton, SIGNAL(clicked()), this, SLOT(next()));

    buttonLayout = new QHBoxLayout;
    buttonLayout->addStretch(1);
    buttonLayout->addWidget(cancelButton);
    buttonLayout->addWidget(backButton);
    buttonLayout->addWidget(nextButton);

    history = new QStackedWidget;

    mainLayout = new QVBoxLayout;
    mainLayout->addWidget(history);
    mainLayout->addLayout(buttonLayout);
    setLayout(mainLayout);
}

TupExportWizard::~TupExportWizard()
{
}

TupExportWizardPage *TupExportWizard::addPage(TupExportWizardPage *newPage)
{
    QString tag = newPage->getTag();
    newPage->setParent(history);
    newPage->show();

    history->addWidget(newPage);

    if (history->count() == 1) { // First Page
        newPage->setFocus();
        backButton->setEnabled(false);
        nextButton->setDefault(true);
    } 

    connect(newPage, SIGNAL(completed()), this, SLOT(pageCompleted()));
    connect(newPage, SIGNAL(emptyField()), this, SLOT(disableNextButton()));

    if (tag.compare("PLUGIN") == 0) {
        // SQA: Pending for implementation
        // connect(newPage, SIGNAL(animatedImageFormatSelected(int, const QString &)),
        //         this, SLOT(setFormat(int, const QString &)));
        connect(newPage, SIGNAL(imagesArrayFormatSelected(TupExportInterface::Format, const QString &)),
                this, SLOT(setFormat(TupExportInterface::Format, const QString &)));
        connect(newPage, SIGNAL(animationFormatSelected(TupExportInterface::Format, const QString &)),
                this, SLOT(setFormat(TupExportInterface::Format, const QString &)));
    }

    if (tag.compare("ANIMATION") == 0 || tag.compare("IMAGES_ARRAY") == 0
        || tag.compare("ANIMATED_IMAGE") == 0 || tag.compare("PROPERTIES") == 0)
        connect(newPage, SIGNAL(isDone()), this, SLOT(closeDialog()));

    return newPage;
}

void TupExportWizard::setButtonLabel(const QString &label)
{
    nextButton->setText(label);
}

void TupExportWizard::showPage(TupExportWizardPage *page)
{
    history->setCurrentWidget(page);
}

void TupExportWizard::showPage(int index)
{
    history->setCurrentIndex(index);
}

void TupExportWizard::cancel()
{       
    close();
}

void TupExportWizard::back()
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupExportWizard::back()]";
    #endif

    TupExportWizardPage *current = qobject_cast<TupExportWizardPage *>(history->currentWidget());
    QString tag = current->getTag();

    if (current)
        current->aboutToBackPage();

    if (tag.compare("ANIMATED_IMAGE") == 0) {
        history->setCurrentIndex(history->currentIndex()-3);
    } else if (tag.compare("IMAGES_ARRAY") == 0) {
        history->setCurrentIndex(history->currentIndex()-2);
    } else if (tag.compare("ANIMATION") == 0 || tag.compare("SCENE") == 0) {
        history->setCurrentIndex(history->currentIndex()-1);
    } else if (tag.compare("PROPERTIES") == 0) {
        backButton->setEnabled(false);
        history->setCurrentIndex(history->currentIndex()-1);
    }

    if (tag.compare("SCENE") == 0)
        backButton->setEnabled(false);

    nextButton->setEnabled(true);

    if (tag.compare("ANIMATION") == 0 || tag.compare("IMAGES_ARRAY") == 0
        || tag.compare("ANIMATED_IMAGE") == 0 || tag.compare("PROPERTIES") == 0)
        nextButton->setText(tr("Next"));
}

void TupExportWizard::next()
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupExportWizard::next()]";
    #endif

    TupExportWizardPage *current = qobject_cast<TupExportWizardPage *>(history->currentWidget());

    if (current) {
        current->aboutToNextPage();

        QString tag = current->getTag();

        #ifdef TUP_DEBUG
            qDebug() << "[TupExportWizard::next()] - tag ->" << tag;
        #endif

        if (tag.compare("PLUGIN") == 0) {                
            backButton->setEnabled(true);
            history->setCurrentIndex(history->currentIndex()+1);
            emit pluginSelected();
        }

        if (tag.compare("ANIMATION") == 0)
            emit animationExported();

        if (tag.compare("ANIMATED_IMAGE") == 0)
            emit animatedImageExported();

        if (tag.compare("IMAGES_ARRAY") == 0)
            emit imagesArrayExported();

        if (tag.compare("PROPERTIES") == 0) {
            current->setTitle(tr("Uploading Source File"));
            emit postProcedureCalled();
        }

        if (tag.compare("SCENE") == 0)  {
            nextButton->setText(tr("Export"));
            backButton->setEnabled(true);

            if (formatCode == TupExportInterface::APNG) { // ANIMATED PNG
                emit animatedImageFileNameChanged();
                history->setCurrentIndex(history->currentIndex() + 3);
            } else if (formatCode == TupExportInterface::JPEG || formatCode == TupExportInterface::PNG
                       || formatCode == TupExportInterface::SVG) { // Images Array
                emit imagesArrayFileNameChanged();
                history->setCurrentIndex(history->currentIndex() + 2);
            } else { // ANIMATION
                emit animationFileNameChanged();
                history->setCurrentIndex(history->currentIndex() + 1);
            }
        }
        pageCompleted();
    } else {
        #ifdef TUP_DEBUG
            qDebug() << "[TupExportWizard::next()] - Fatal Error: Current page is NULL!";
        #endif
    }
}

void TupExportWizard::pageCompleted()
{
    TupExportWizardPage *current = qobject_cast<TupExportWizardPage *>(history->currentWidget());
    QString tag = current->getTag();

    if (tag.compare("SCENE") == 0 || tag.compare("PLUGIN") == 0) {
        nextButton->setEnabled(current->isComplete());
    } else {
        if (tag.compare("IMAGES_ARRAY") == 0 || tag.compare("ANIMATION") == 0
            || tag.compare("ANIMATED_IMAGE") == 0)
            nextButton->setText(tr("Export"));
        if (tag.compare("PROPERTIES") == 0)
            nextButton->setText(tr("Post"));
        nextButton->setEnabled(true);
    }

    if (history->currentIndex() == 1)
        emit scenesUpdated();
}

void TupExportWizard::disableNextButton()
{
    if (nextButton->isEnabled())
        nextButton->setEnabled(false);
}

void TupExportWizard::enableButtonSet(bool enabled)
{
    backButton->setVisible(enabled);
    cancelButton->setVisible(enabled);
    nextButton->setVisible(enabled);
}

void TupExportWizard::closeDialog()
{
    close();
    emit isDone();
}

void TupExportWizard::setFormat(TupExportInterface::Format code, const QString &extension)
{
    formatCode = code;
    format = extension;
}

TupExportWizardPage::TupExportWizardPage(const QString &title, QWidget *parent) : TVHBox(parent)
{
    TVHBox *boxTitle = new TVHBox(this, Qt::Vertical);
    titleLabel = new QLabel(title, boxTitle);
    new TSeparator(boxTitle);
    boxLayout()->setAlignment(boxTitle, Qt::AlignTop);

    container = new QFrame(this);
    layout = new QGridLayout(container);

    image = new QLabel;
    layout->addWidget(image, 0, 0, Qt::AlignLeft);
    image->hide();

    new TSeparator(this);

    hide();
}

TupExportWizardPage::~TupExportWizardPage()
{
}

void TupExportWizardPage::setTitle(const QString &title)
{
    titleLabel->setText(title);
}

void TupExportWizardPage::setPixmap(const QPixmap &pixmap)
{
    image->setPixmap(pixmap);
    image->show();
}

void TupExportWizardPage::setWidget(QWidget *w)
{
    layout->addWidget(w, 0, 1);
}

void TupExportWizardPage::setTag(const QString &label)
{
    tag = label;
}

const QString TupExportWizardPage::getTag()
{
    return tag;
}

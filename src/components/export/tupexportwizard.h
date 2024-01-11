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

#ifndef TUPEXPORTWIZARD_H
#define TUPEXPORTWIZARD_H

#include "tglobal.h"
#include "tvhbox.h"
#include "toolview.h"
#include "tseparator.h"
#include "tupexportinterface.h"

#include <QDialog>
#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QStackedWidget>
#include <QLabel>
#include <QBitmap>

class TupExportWizardPage;

class TUPITUBE_EXPORT TupExportWizard : public QDialog
{
    Q_OBJECT

    public:
        TupExportWizard(QWidget *parent = nullptr);
        ~TupExportWizard();

        TupExportWizardPage *addPage(TupExportWizardPage *page);
        void showPage(int index);
        void showPage(TupExportWizardPage *page);
        void setButtonLabel(const QString &label);

    public slots:
        void enableButtonSet(bool enabled);

    private slots:
        void cancel();
        void back();
        void next();
        void pageCompleted();
        void disableNextButton();
        void closeDialog();
        void setFormat(TupExportInterface::Format format, const QString &extension);

    signals:
        void cancelled();
        void pluginSelected();
        void scenesUpdated();
        void animationExported();
        void animatedImageExported();
        void postProcedureCalled();
        void imagesArrayExported();
        void animationFileNameChanged();
        void animatedImageFileNameChanged();
        void imagesArrayFileNameChanged();

        void isDone();

    private:
        QStackedWidget *history;
        QPushButton *cancelButton;
        QPushButton *backButton;
        QPushButton *nextButton;

        QHBoxLayout *buttonLayout;
        QVBoxLayout *mainLayout;
        QString format;
        TupExportInterface::Format formatCode;
};

#include <QFrame>
#include <QGridLayout>
#include <QLabel>

class TUPITUBE_EXPORT TupExportWizardPage : public TVHBox
{
    Q_OBJECT

    public:
        TupExportWizardPage(const QString &title, QWidget *parent = nullptr);
        virtual ~TupExportWizardPage();

        virtual bool isComplete() const = 0;
        virtual void resetUI() = 0;

        void setTitle(const QString &title);
        void setPixmap(const QPixmap &pixmap);
        void setWidget(QWidget *widget);
        void setTag(const QString &label);
        const QString getTag();

    public slots:
        virtual void aboutToNextPage() {}
        virtual void aboutToBackPage() {}

    signals:
        void completed();
        void emptyField();

    private:
        QFrame *container;
        QGridLayout *layout;
        QLabel *titleLabel;
        QLabel *image;
        QString tag;
};

#endif

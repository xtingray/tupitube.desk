/***************************************************************************
 *   Project TUPITUBE DESK                                                *
 *   Project Contact: info@maefloresta.com                                 *
 *   Project Website: http://www.maefloresta.com                           *
 *   Project Leader: Gustav Gonzalez <info@maefloresta.com>                *
 *                                                                         *
 *   Developers:                                                           *
 *   2010:                                                                 *
 *    Gustavo Gonzalez                                                     *
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

#ifndef TUPSTORYBOARDDIALOG_H
#define TUPSTORYBOARDDIALOG_H

#include "tglobal.h"
#include "tupscene.h"
#include "tupstoryboard.h"
#include "tupexportinterface.h"
#include "tapplicationproperties.h"
#include "tconfig.h"

#include <QDialog>
#include <QColor>
#include <QSize>
#include <QIcon>
#include <QBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QTextEdit>
#include <QListWidgetItem>
#include <QLocale>
#include <QDoubleSpinBox>

class TUPITUBE_EXPORT TupStoryBoardDialog : public QDialog
{
    Q_OBJECT

    public:
        enum DocType { HTML = 1, PDF };
        TupStoryBoardDialog(bool isNetworked, TupExportInterface *imagePlugin, const QColor &color, 
                            const QSize &size, TupScene *scene, int sceneIndex, TupLibrary *library, QWidget *parent);
        ~TupStoryBoardDialog();

    private slots:
        void updateForm(QListWidgetItem *current, QListWidgetItem *previous);
        void updateDuration(double value);
        void exportStoyrboard(const QString &type);
        void postStoryboardAtServer();
        void closeDialog();
        void exportAsHTML();
        void exportAsPDF();

    signals:
        void updateStoryboard(TupStoryboard *, int sceneIndex);
        void postStoryboard(int sceneIndex);

    private:
        void setListComponent();
        void setPreviewScreen();
        void setCoverForm();
        void setSceneForm();

        void thumbnailsGenerator();
        void addScene(const QString &label, const QIcon &icon);

        void saveLastComponent();

        QString getStoryTitle() const;
        QString getStoryAuthor() const;
        QString getStoryTopics() const;
        QString getStorySummary() const;

        QString getSceneDuration() const;
        QFont setFont(double proportion) const;
        QString formatString(QStringList stringWords, int size) const;
        void createHTMLFiles(const QString &path, DocType type);
        void cleanDirectory(const QString &path);
        QPixmap renderCover(const QSize dimension);
 
        bool isNetworked;
        TupExportInterface *imagePlugin;
        QColor bgColor;
        QSize size;
        QSize scaledSize;
        int sceneIndex;
        TupScene *scene;

        TupStoryboard *storyboard;
        int currentIndex;
        QString path;

        QBoxLayout *layout;
        QVBoxLayout *formLayout;

        QWidget *storyPanel;
        QWidget *scenePanel;

        QListWidget *list;
        QLabel *screenLabel;

        QLineEdit *titleEdit;
        QLineEdit *topicsEdit;
        QLineEdit *authorEdit;
        QTextEdit *summaryEdit;

        QLabel *sceneLabel;
        // QLineEdit *sceneDurationEdit;
        QDoubleSpinBox *secSpinBox;

        QLocale utf;
        TupLibrary *library;
};

#endif

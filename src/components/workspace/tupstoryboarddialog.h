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
        TupStoryBoardDialog(bool isNetworked, TupExportInterface *imagePlugin, TupExportInterface *videoPlugin,
                            TupProject *work, int sceneIndex, QWidget *parent);
        ~TupStoryBoardDialog();

    private slots:
        void updateForm(QListWidgetItem *current, QListWidgetItem *previous);
        void updateCoverDuration(double value);
        void updateDuration(double value);
        void postStoryboardAtServer();
        void closeDialog();
        void exportAsPDF();
        void exportAsAnimatic();

    signals:
        void updateStoryboard(TupStoryboard *, int sceneIndex);
        void postStoryboard(int sceneIndex);
        void projectHasChanged();

    private:
        void setListComponent();
        void setPreviewScreen();
        void setCoverForm();
        QWidget * addDurationPanel();
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
        TupExportInterface *videoPlugin;

        TupProject *project;
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

        QTabWidget *sceneTabWidget;
        QTabWidget *coverTabWidget;
        QWidget *storyPanel;
        QWidget *scenePanel;

        QListWidget *list;
        QLabel *screenLabel;

        QLineEdit *titleEdit;
        QLineEdit *topicsEdit;
        QLineEdit *authorEdit;
        QTextEdit *summaryEdit;

        QLabel *sceneLabel;
        QDoubleSpinBox *secSpinBox;
        QDoubleSpinBox *coverSecSpinBox;

        QLocale utf;
};

#endif

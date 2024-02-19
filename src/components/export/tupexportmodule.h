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

#ifndef TUPEXPORTMODULE_H
#define TUPEXPORTMODULE_H

#include "tglobal.h"
#include "tupexportinterface.h"
#include "tupexportwizard.h"
#include "tupproject.h"
// #include "tsizebox.h"

#include <QLineEdit>
#include <QSpinBox>
#include <QCheckBox>
#include <QProgressBar>

class TUPITUBE_EXPORT TupExportModule : public TupExportWizardPage
{
    Q_OBJECT

    public:
        enum OutputFormat { Animation = 0, ImagesArray, AnimatedImage };
        TupExportModule(TupProject *project, OutputFormat output, QString title);
        ~TupExportModule();

        bool isComplete() const;
        void resetUI();

    public slots:
        void exportIt();

    private slots:
        void updateState(const QString &text);
        void chooseFile();
        void chooseDirectory();
        void updateNameField();
        void enableTransparency(bool flag);

    private:
        QList<TupScene *> scenesToExport() const;

    public slots:
        void setScenesIndexes(const QList<int> &indexes);
        void setCurrentExporter(TupExportInterface *currentExporter);
        void setCurrentFormat(TupExportInterface::Format format, const QString &extension);
        void updateProgressMessage(const QString &title);
        void updateProgressLabel(int percent);

    signals:
        void saveFile();
        void exportArray();
        void exportHasStarted();
        void isDone();

    private:
        double calculateProjectDuration(const QList<TupScene *> &scenes, int fps);

        QList<int> sceneIndexes;
        QList<TupScene *> scenes;
        int fps;

        TupExportInterface *m_currentExporter;
        TupExportInterface::Format m_currentFormat;

        TupProject *m_project;        
        QLineEdit *m_filePath;
        QLineEdit *m_prefix;

        QString filename;
        QString path;
        QString extension;
        QCheckBox *bgTransparency;
        bool transparency;
        bool browserWasOpened;
        QSize dimension;

        QWidget *progressWidget;
        QLabel *progressLabel;
        QProgressBar *progressBar;

        OutputFormat outputFormat;
};

#endif

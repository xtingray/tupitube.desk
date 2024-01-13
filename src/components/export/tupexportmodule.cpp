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

#include "tupexportmodule.h"
#include "tapptheme.h"
#include "tconfig.h"
#include "tosd.h"
#include "tupexportpluginobject.h"

#include <QGroupBox>
#include <QFileDialog>
#include <QMessageBox>

TupExportModule::TupExportModule(TupProject *project, OutputFormat output,
                                 QString title) : TupExportWizardPage(title), m_currentExporter(nullptr),
                                 m_currentFormat(TupExportInterface::NONE), m_project(project)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupExportModule::TupExportModule()] - title ->" << title;
    #endif

    outputFormat = output;
    transparency = false;
    browserWasOpened = false;

    if (output == Animation) {
        setTag("ANIMATION");
    } else if (output == ImagesArray) {
        setTag("IMAGES_ARRAY");
    } else if (output == AnimatedImage) {
        setTag("ANIMATED_IMAGE");
    }

    bgTransparency = new QCheckBox(tr("Enable Background Transparency"));

    QWidget *container = new QWidget;
    QVBoxLayout *layout = new QVBoxLayout(container);
    TCONFIG->beginGroup("General");
    path = TCONFIG->value("DefaultPath", QDir::homePath()).toString();

    QHBoxLayout *prefixLayout = new QHBoxLayout;
    prefixLayout->addWidget(new QLabel(tr("Image Name Prefix: ")));
    QHBoxLayout *filePathLayout = new QHBoxLayout;
    if (output == ImagesArray)
        filePathLayout->addWidget(new QLabel(tr("Directory: ")));
    else // Animation or AnimatedImage
        filePathLayout->addWidget(new QLabel(tr("File: ")));

    QString prefix = m_project->getName() + "_img";
    m_prefix = new QLineEdit(prefix);

    m_filePath = new QLineEdit;
    connect(m_filePath, SIGNAL(textChanged(const QString &)), this, SLOT(updateState(const QString &)));

    if (output == ImagesArray)
        connect(m_prefix, SIGNAL(textChanged(const QString &)), this, SLOT(updateState(const QString &)));

    filePathLayout->addWidget(m_filePath);

    QToolButton *button = new QToolButton;
    button->setIcon(QIcon(THEME_DIR + "icons/open.png"));
    button->setToolTip(tr("Choose another path"));

    if (output == ImagesArray)
        connect(button, SIGNAL(clicked()), this, SLOT(chooseDirectory()));
    else
        connect(button, SIGNAL(clicked()), this, SLOT(chooseFile()));

    filePathLayout->addWidget(button);

    if (output == ImagesArray) {
        prefixLayout->addWidget(m_prefix);
        prefixLayout->addWidget(new QLabel(tr("i.e. ") + "<B>" + prefix + "</B>01.png / jpeg / svg"));
        layout->addLayout(prefixLayout);
    }

    layout->addLayout(filePathLayout);

    QWidget *configWidget = new QWidget;
    QHBoxLayout *configureLayout = new QHBoxLayout(configWidget);
    configureLayout->addStretch();

    dimension = m_project->getDimension();
    /*
    int maxDimension = dimension.width();
    if (maxDimension < dimension.height())
        maxDimension = dimension.height();
    */

    if (output == ImagesArray) {
        connect(bgTransparency, SIGNAL(toggled(bool)), this, SLOT(enableTransparency(bool)));
        configureLayout->addWidget(bgTransparency);
    }

    TCONFIG->beginGroup("Theme");
    int uiTheme = TCONFIG->value("UITheme", DARK_THEME).toInt();
    QString style = "QProgressBar { background-color: #DDDDDD; text-align: center; color: #FFFFFF; border-radius: 2px; } ";
    QString color = "#009500";
    if (uiTheme == DARK_THEME)
        color = "#444444";
    style += "QProgressBar::chunk { background-color: " + color + "; border-radius: 2px; }";

    progressBar = new QProgressBar;
    progressBar->setTextVisible(true);
    progressBar->setStyleSheet(style);
    progressBar->setRange(1, 100);

    progressWidget = new QWidget;
    progressLabel = new QLabel("");
    progressLabel->setAlignment(Qt::AlignHCenter);
    QVBoxLayout *progressLayout = new QVBoxLayout(progressWidget);
    progressLayout->addSpacing(50);
    progressLayout->addWidget(progressLabel);
    progressLayout->addWidget(progressBar);
    progressLayout->addSpacing(50);
    progressWidget->setVisible(false);

    configureLayout->addStretch();
    layout->addWidget(configWidget);
    layout->addWidget(progressWidget);
    layout->addStretch();

    setWidget(container);
}

TupExportModule::~TupExportModule()
{
}

bool TupExportModule::isComplete() const
{
    return !m_filePath->text().isEmpty();
}

void TupExportModule::resetUI()
{
}

void TupExportModule::setScenesIndexes(const QList<int> &indexes)
{
    m_indexes = indexes;    
    scenes = scenesToExport();
    #ifdef TUP_DEBUG
        qDebug() << "[TupExportModule::setScenesIndexes()] - Scenes total ->" << scenes.count();
    #endif
    fps = scenes.first()->getFPS();

    if (outputFormat == Animation) {
        double duration = calculateProjectDuration(scenes, fps);
        if (duration < 3) { // Duration is too short to be played
            #ifdef TUP_DEBUG
                qWarning() << "[TupExportModule::setScenesIndexes()] - Fatal Error: The project duration is too short. Aborting export action!";
                qWarning() << "[TupExportModule::setScenesIndexes()] - duration ->" << duration;
            #endif

            QMessageBox msgBox;
            msgBox.setStyleSheet(TAppTheme::themeSettings());
            msgBox.setWindowTitle(tr("Can't export project"));
            msgBox.setIcon(QMessageBox::Critical);
            msgBox.setTextFormat(Qt::RichText);
            msgBox.setText(tr("The duration of the project is TOO SHORT to generate an accurate MP4 file.\n"
                              "Please, try to export the scene several times to extend the animation duration\n"
                              "until reaches at least 3 seconds."));
            msgBox.addButton(QString(tr("Ok")), QMessageBox::AcceptRole);
            msgBox.show();

            if (msgBox.exec() == QMessageBox::AcceptRole) {
                TOsd::self()->display(TOsd::Error, tr("Sorry, export attempt failed!"));
                emit isDone();
            }
        }
    }
}

void TupExportModule::setCurrentExporter(TupExportInterface *currentExporter)
{
    m_currentExporter = currentExporter;

    TupExportPluginObject *plugin = (TupExportPluginObject *) currentExporter;
    connect(plugin, SIGNAL(messageChanged(const QString &)), this, SLOT(updateProgressMessage(const QString &)));
    connect(plugin, SIGNAL(progressChanged(int)), this, SLOT(updateProgressLabel(int)));
}

void TupExportModule::setCurrentFormat(TupExportInterface::Format format, const QString &value)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupExportModule::setCurrentFormat()] - current format ->" << format;
        qDebug() << "[TupExportModule::setCurrentFormat()] - extension ->" << extension;
    #endif

    m_currentFormat = format;
    extension = value;
    filename = path;
    filename = QDir::fromNativeSeparators(filename);

    // Animated Image or Animation
    if (outputFormat == Animation) {
        if (!filename.endsWith("/"))
            filename += "/";

        filename += m_project->getName();
        filename += extension;
    } else { // Images Array
        if (m_currentFormat == TupExportInterface::JPEG) {
            if (bgTransparency->isVisible())
                bgTransparency->setVisible(false);
        } else {
            if (!bgTransparency->isVisible())
                bgTransparency->setVisible(true);
        }
    }

    m_filePath->setText(filename);
}

void TupExportModule::updateNameField()
{
   if (filename.length() > 0) 
       m_filePath->setText(filename);
}

void TupExportModule::enableTransparency(bool flag)
{
   transparency = flag; 
}

void TupExportModule::chooseFile()
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupExportModule::chooseFile()]";
    #endif

    filename = QFileDialog::getSaveFileName(this, tr("Export video as..."), path,
                                            tr("Video File") + " (*" + extension.toLocal8Bit() + ")");
    if (!filename.isEmpty()) {
        browserWasOpened = true;
        QString lower = filename.toLower();
        if (!lower.endsWith(extension))
            filename += extension;

        m_filePath->setText(filename);

        QFileInfo fileInfo(filename);
        QString dir = fileInfo.dir().absolutePath();

        TCONFIG->beginGroup("General");
        TCONFIG->setValue("DefaultPath", dir);
        TCONFIG->sync();
    }
}

void TupExportModule::chooseDirectory()
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupExportModule::chooseDirectory()]";
    #endif

    filename = QFileDialog::getExistingDirectory(this, tr("Choose a directory..."), path,
                                                 QFileDialog::ShowDirsOnly
                                                 | QFileDialog::DontResolveSymlinks);

    if (!filename.isEmpty()) {
        m_filePath->setText(filename);

        TCONFIG->beginGroup("General");
        TCONFIG->setValue("DefaultPath", filename);
        TCONFIG->sync();
    }
}

void TupExportModule::updateState(const QString &name)
{
    if (name.length() > 0) 
        emit completed();
    else
        emit emptyField(); 
}

void TupExportModule::exportIt()
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupExportModule::exportIt()] - current format ->" << m_currentFormat;
    #endif

    bool done = false; 
    bool isArray = false;
    QString name = "";

    QList<TupExportInterface::Format> imageFormats;
    imageFormats << TupExportInterface::JPEG << TupExportInterface::PNG << TupExportInterface::SVG;

    if (outputFormat == ImagesArray) {
        isArray = true;
        name = m_prefix->text();
        path = m_filePath->text();
        path = QDir::fromNativeSeparators(path);

        if (name.length() == 0) {
            TOsd::self()->display(TOsd::Error, tr("Images name prefix is unset! Please, type a prefix."));
            return;
        }

        if (path.length() == 0) {
            TOsd::self()->display(TOsd::Error, tr("Images path can't be unset! Please, choose one."));
            return;
        }

        QDir dir(path);
        if (!dir.exists()) {
            TOsd::self()->display(TOsd::Error, tr("Images path doesn't exist! Please, choose another."));
            return;
        }

        filename = path + "/" + name;

        if (QFile::exists(QString(filename + "0000" + extension))) {
            QMessageBox::StandardButton reply;
            reply = QMessageBox::question(this, tr("Warning!"), tr("Image sequence already exists. Overwrite it?"),
                                          QMessageBox::Yes | QMessageBox::No);

            if (reply == QMessageBox::No)
                return;
        }
    } else { // Animation or Animated Image
        filename = m_filePath->text();
        filename = QDir::fromNativeSeparators(filename);

        if (filename.length() == 0) {
            TOsd::self()->display(TOsd::Error, tr("Animation path is unset! Please, choose one."));
            #ifdef TUP_DEBUG
                qDebug() << "[TupExportModule::exportIt()] - [Tracer 01] Fatal Error: Animation path is unset! ->"
                         << path.toLocal8Bit();
            #endif
            return;
        }

        QFileInfo fileInfo(filename);
        name = fileInfo.completeBaseName();
        path = fileInfo.dir().absolutePath();

        QString lower = name.toLower();
        if (!lower.endsWith(extension))
            name += extension;

        if (path.length() == 0) {
            TOsd::self()->display(TOsd::Error, tr("Animation path can't be unset! Please, choose one."));
            return;
        }

        QDir dir(path);
        if (!dir.exists()) {
            TOsd::self()->display(TOsd::Error, tr("Animation path doesn't exist! Please, choose another."));
            return;
        }

        if (!browserWasOpened) {
            if (QFile::exists(filename)) {
                QMessageBox::StandardButton reply;
                reply = QMessageBox::question(this, tr("Warning!"),
                                              tr("Video file exists. Overwrite it?"),
                                              QMessageBox::Yes | QMessageBox::No);

                if (reply == QMessageBox::No)
                    return;
            } 
        }
    }

    if (outputFormat == ImagesArray) {
        QFileInfo dir(path);
        if (!dir.isReadable() || !dir.isWritable()) {
            TOsd::self()->display(TOsd::Error, tr("Insufficient permissions. Please, choose another directory."));
            return;
        }
    } else {
        QFile file(filename);
        if (!file.open(QIODevice::ReadWrite)) {
            file.remove();
            TOsd::self()->display(TOsd::Error, tr("Insufficient permissions. Please, choose another path."));
            return;
        }
        file.remove();
    }

    emit exportHasStarted();

    progressWidget->setVisible(true);
    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

    if (m_currentExporter) {
        #ifdef TUP_DEBUG
            qWarning() << "[TupExportModule::exportIt()] -  Exporting to file ->" << path.toLocal8Bit();
        #endif

        if (outputFormat == ImagesArray)
            scenes = scenesToExport();

        #ifdef TUP_DEBUG
            qWarning() << "[TupExportModule::exportIt()] - Exporting " << scenes.count() << " scenes";
        #endif

        if (scenes.count() > 0) {
            int width = dimension.width();
            int height = dimension.height();
            // libav requirement: resolution must be a multiple of two
            if ((width % 2) != 0)
                width++;
            if ((height % 2) != 0)
                height++;

            QColor color = m_project->getCurrentBgColor();
            if (m_currentFormat == TupExportInterface::PNG || m_currentFormat == TupExportInterface::SVG) {
                if (transparency)
                    color.setAlpha(0);
                else
                    color.setAlpha(255);
            }

            // SQA: The QSize second parameter will contain the resizing value of the animation
            //      * Pending feature
            done = m_currentExporter->exportToFormat(color, filename, scenes, m_currentFormat, 
                                      QSize(width, height), QSize(width, height), fps /* m_project->getFPS() */,
                                      // QSize(width, height), QSize(newWidth, newHeight), m_fps->value(),
                                      m_project);
        }
    } else {
        TOsd::self()->display(TOsd::Error, tr("Format problem. TupiTube Internal error."));
    }

    QApplication::restoreOverrideCursor();

    if (done) {
        QString message = tr("Video file") + " " + name + " " + tr("was saved successful");
        if (isArray)
            message = tr("Image sequence was saved successful");
        TOsd::self()->display(TOsd::Info, tr(message.toLocal8Bit()));
        emit isDone();
    } else {
        if (m_currentExporter) {
            QString msg = m_currentExporter->getExceptionMsg();
            #ifdef TUP_DEBUG
                qWarning() << "[TupExportModule::exportIt()] -  Error Message ->" << msg;
            #endif

            QMessageBox msgBox;
            msgBox.setStyleSheet(TAppTheme::themeSettings());
            msgBox.setWindowTitle(tr("Fatal Error: Can't export video"));
            msgBox.setIcon(QMessageBox::Critical);
            msgBox.setTextFormat(Qt::RichText);
            msgBox.setText(tr(msg.toLocal8Bit()));
            msgBox.addButton(QString(tr("Ok")), QMessageBox::AcceptRole);
            msgBox.show();

            if (msgBox.exec() == QMessageBox::AcceptRole) {
                msg = tr("Sorry, animation is too short!");
                TOsd::self()->display(TOsd::Error, tr(msg.toLocal8Bit()));
                emit isDone();
            }
        }
    }
}

QList<TupScene *> TupExportModule::scenesToExport() const
{
    QList<TupScene *> scenes;
    foreach (int index, m_indexes)
        scenes << m_project->sceneAt(index);

    return scenes;
}

double TupExportModule::calculateProjectDuration(const QList<TupScene *> &scenes, int fps)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupExportModule::calculateProjectDuration()] - fps ->" << fps;
        qDebug() << "[TupExportModule::calculateProjectDuration()] - scenes.size() ->" << scenes.size();
    #endif

    double durationInSeconds = 0;
    foreach (TupScene *scene, scenes)
        durationInSeconds += static_cast<double>(scene->framesCount()) / static_cast<double>(fps);

    return durationInSeconds;
}

void TupExportModule::updateProgressMessage(const QString &title)
{
    progressLabel->setText(title);
}

void TupExportModule::updateProgressLabel(int percent)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupExportModule::updateProgressLabel()] - percent ->" << percent;
    #endif

    progressBar->setValue(percent);
}

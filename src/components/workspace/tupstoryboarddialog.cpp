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

#include "tupstoryboarddialog.h"
#include "tseparator.h"
#include "talgorithm.h"
#include "tosd.h"

#include <QPainter>
#include <QFileDialog>
#include <QDir>
#include <QDesktopWidget>
#include <QPrintDialog>
#include <QTextBrowser>
#include <QDialogButtonBox>
#include <QPrinter>
#include <QPixmap>
#include <QPushButton>
#include <QListWidget>

TupStoryBoardDialog::TupStoryBoardDialog(bool network, TupExportInterface *plugin, const QColor &color,
                                         const QSize &pSize, TupScene *pScene, int sIndex, TupLibrary *assets,
                                         QWidget *parent) : QDialog(parent)
{
    isNetworked = network;
    imagePlugin = plugin;
    bgColor = color;
    size = pSize;
    scene = pScene;
    sceneIndex = sIndex;
    storyboard = scene->storyboard();
    library = assets;
    utf = QLocale(QLocale::AnyLanguage, QLocale::AnyCountry);

    QDesktopWidget desktop;
    scaledSize = QSize();

    if (pSize.width() > pSize.height()) {
        if (size.width() + 500 > desktop.screenGeometry().width()) {
            int w = desktop.screenGeometry().width() - 500;
            int h = (size.height() * w) / size.width();
            scaledSize.setWidth(w);
            scaledSize.setHeight(h);
        } else {
            scaledSize = size;
        }
    } else {
        if (size.height() + 400 > desktop.screenGeometry().height()) {
            int h = desktop.screenGeometry().height() - 400;
            int w = (size.width() * h) / size.height();
            scaledSize.setWidth(w);
            scaledSize.setHeight(h);
        } else {
            scaledSize = size;
        }
    }

    if (scaledSize.height() + 400 > desktop.screenGeometry().height()) {
        int h = desktop.screenGeometry().height() - 400;
        int w = (size.width() * h) / size.height();
        scaledSize.setWidth(w);
        scaledSize.setHeight(h);
    }

    setModal(true);
    setWindowTitle(tr("Storyboard Settings"));
    setWindowIcon(QIcon(QPixmap(THEME_DIR + "icons/storyboard.png")));

    layout = new QHBoxLayout(this);
    formLayout = new QVBoxLayout;

    setListComponent();
    setPreviewScreen();
    setStoryForm();
    setSceneForm();

    QPushButton *pdfButton = new QPushButton(tr("&PDF"));
    pdfButton->setToolTip(tr("Export as PDF"));
    connect(pdfButton, SIGNAL(clicked()), this, SLOT(exportAsPDF()));

    QPushButton *htmlButton = new QPushButton(tr("&HTML"));
    htmlButton->setToolTip(tr("Export as HTML"));
    connect(htmlButton, SIGNAL(clicked()), this, SLOT(exportAsHTML()));

    QPushButton *closeButton = new QPushButton(QIcon(QPixmap(THEME_DIR + "icons/close.png")), "");
    closeButton->setToolTip(tr("Close"));
    closeButton->setDefault(true);
    connect(closeButton, SIGNAL(clicked()), this, SLOT(closeDialog()));

    QDialogButtonBox *buttonBox = new QDialogButtonBox(Qt::Horizontal, this);
    buttonBox->addButton(pdfButton, QDialogButtonBox::ActionRole);
    buttonBox->addButton(htmlButton, QDialogButtonBox::ActionRole);

    if (isNetworked) {
        QPushButton *postButton = new QPushButton(tr("&Post"));
        connect(postButton, SIGNAL(clicked()), this, SLOT(postStoryboardAtServer()));
        buttonBox->addButton(postButton, QDialogButtonBox::ActionRole);
    }

    buttonBox->addButton(closeButton, QDialogButtonBox::ActionRole);

    formLayout->addWidget(new TSeparator());
    formLayout->addWidget(buttonBox);

    layout->addLayout(formLayout);

    thumbnailGenerator();
}

TupStoryBoardDialog::~TupStoryBoardDialog()
{
}

void TupStoryBoardDialog::setListComponent()
{
    list = new QListWidget(this);
    list->setViewMode(QListView::IconMode);
    list->setWrapping(false);
    list->setFlow(QListView::TopToBottom);
    list->setIconSize(QSize(96, (scaledSize.height() * 96) / scaledSize.width()));
    list->setMovement(QListView::Static);
    list->setFixedWidth(130);
    list->setSpacing(12);

    layout->addWidget(list);

    connect(list, SIGNAL(currentItemChanged(QListWidgetItem *, QListWidgetItem *)),
            this, SLOT(updateForm(QListWidgetItem *, QListWidgetItem*)));
}

void TupStoryBoardDialog::setPreviewScreen()
{
    QPixmap pixmap = QPixmap(scaledSize.width(), scaledSize.height());
    pixmap.fill();

    screenLabel = new QLabel;
    screenLabel->setAlignment(Qt::AlignHCenter);
    screenLabel->setPixmap(pixmap);
    formLayout->addWidget(screenLabel);
}

void TupStoryBoardDialog::setStoryForm()
{
    storyPanel = new QWidget;

    QBoxLayout *sceneLayout = new QBoxLayout(QBoxLayout::TopToBottom, storyPanel);

    QFont font = this->font();
    font.setPointSize(10);
    font.setBold(true);
    QLabel *mainTitle = new QLabel(tr("Storyboard General Information"));
    mainTitle->setFont(font);
    mainTitle->setAlignment(Qt::AlignHCenter);

    QLabel *titleLabel = new QLabel(tr("Title"));
    titleEdit = new QLineEdit("");
    titleEdit->setLocale(utf);

    titleLabel->setBuddy(titleEdit);

    QLabel *authorLabel = new QLabel(tr("Author"));
    authorEdit = new QLineEdit("");
    authorEdit->setLocale(utf);
    authorLabel->setBuddy(authorEdit);

    QLabel *summaryLabel = new QLabel(tr("Summary"));
    summaryEdit = new QTextEdit;
    summaryEdit->setLocale(utf);
    summaryEdit->setAcceptRichText(false);
    summaryEdit->setFixedHeight(80);
    summaryEdit->setText("");

    QHBoxLayout *topLayout = new QHBoxLayout;
    topLayout->addWidget(titleLabel);
    topLayout->addWidget(titleEdit);

    QHBoxLayout *middleLayout = new QHBoxLayout;
    middleLayout->addWidget(authorLabel);
    middleLayout->addWidget(authorEdit);

    sceneLayout->addWidget(mainTitle);
    sceneLayout->addLayout(topLayout);

    if (isNetworked) {
        QLabel *topicsLabel = new QLabel(tr("Topics"));
        topicsEdit = new QLineEdit("");
        topicsEdit->setLocale(utf);
        topicsLabel->setBuddy(topicsEdit);

        QHBoxLayout *topicsLayout = new QHBoxLayout;
        topicsLayout->addWidget(topicsLabel);
        topicsLayout->addWidget(topicsEdit);

        sceneLayout->addLayout(topicsLayout);
    }

    sceneLayout->addLayout(middleLayout);
    sceneLayout->addWidget(summaryLabel);
    sceneLayout->addWidget(summaryEdit);

    formLayout->addWidget(storyPanel);
}

void TupStoryBoardDialog::setSceneForm()
{
    scenePanel = new QWidget;

    QBoxLayout *sceneLayout = new QBoxLayout(QBoxLayout::TopToBottom, scenePanel);
    sceneLayout->setAlignment(Qt::AlignVCenter | Qt::AlignBottom);

    QFont font = this->font();
    font.setPointSize(10);
    font.setBold(true);
    sceneLabel = new QLabel(tr("Scene Information"));
    sceneLabel->setFont(font);
    sceneLabel->setAlignment(Qt::AlignHCenter);

    QLabel *titleLabel = new QLabel(tr("Title"));
    sceneTitleEdit = new QLineEdit("");
    sceneTitleEdit->setLocale(utf);
    titleLabel->setBuddy(sceneTitleEdit);

    QLabel *durationLabel = new QLabel(tr("Duration"));
    sceneDurationEdit = new QLineEdit("");
    sceneDurationEdit->setLocale(utf);
    durationLabel->setBuddy(sceneDurationEdit);

    QLabel *descLabel = new QLabel(tr("Description"));

    sceneDescriptionEdit = new QTextEdit;
    sceneDescriptionEdit->setLocale(utf);
    sceneDescriptionEdit->setAcceptRichText(false);
    sceneDescriptionEdit->setFixedHeight(80);
    sceneDescriptionEdit->setText("");

    QHBoxLayout *topLayout = new QHBoxLayout;
    topLayout->addWidget(titleLabel);
    topLayout->addWidget(sceneTitleEdit);

    QHBoxLayout *middleLayout = new QHBoxLayout;
    middleLayout->addWidget(durationLabel);
    middleLayout->addWidget(sceneDurationEdit);

    sceneLayout->addWidget(sceneLabel);
    sceneLayout->addLayout(topLayout);
    sceneLayout->addLayout(middleLayout);
    sceneLayout->addWidget(descLabel);
    sceneLayout->addWidget(sceneDescriptionEdit);

    formLayout->addWidget(scenePanel);

    scenePanel->hide();
}

void TupStoryBoardDialog::addScene(const QString &label, const QIcon &icon)
{
    QListWidgetItem *sceneItem = new QListWidgetItem(list);
    sceneItem->setIcon(icon);
    sceneItem->setText(label);
    sceneItem->setTextAlignment(Qt::AlignHCenter);
    sceneItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);

    if (label.compare(tr("Cover")) == 0)
        sceneItem->setSelected(true);
}

void TupStoryBoardDialog::thumbnailGenerator()
{
    int height = (scaledSize.height() * 96) / scaledSize.width();
    QPixmap pixmap = QPixmap(96, height); 
    pixmap.fill();

    QPainter painter(&pixmap);
    painter.setPen(Qt::black);
    QFont font = this->font();
    font.setPointSize(8);
    font.setBold(true);
    painter.setFont(font);
    // painter.setFont(QFont("Arial", 8, QFont::Bold));
    QRectF rect(QPointF(0, 0), QSizeF(96, height));
    painter.drawText(rect, Qt::AlignCenter, tr("Storyboard"));
    painter.setPen(QColor(230, 230, 230));
    QRectF rectangle(0, 0, 95, height - 1);
    painter.drawRect(rectangle);

    QIcon icon = QIcon(pixmap); 
    addScene(tr("Cover"), icon);

    int framesCount = scene->framesCount();
    if (storyboard->size() == 0)
        storyboard->init(0, framesCount);

    path = QDir::tempPath() + "/" + TAlgorithm::randomString(8) + "/";
    QDir().mkpath(path);

    for (int i=0; i < framesCount; i++) {
         QString fileName = path + "scene" + QString::number(i);
         bool isOk = imagePlugin->exportFrame(i, bgColor, fileName, scene, size, library);
         fileName += ".png";
         QPixmap resized(fileName);
         resized = resized.scaledToWidth(scaledSize.width(), Qt::SmoothTransformation);
         resized.save(fileName);

         if (isOk) {
             QPixmap pixmap(fileName);
             QPainter painter(&pixmap);
             painter.setPen(Qt::darkGray);
             QRectF rectangle(0, 0, scaledSize.width()-1, scaledSize.height()-1);
             painter.drawRect(rectangle);
             pixmap.scaledToWidth(96, Qt::SmoothTransformation);

             QIcon icon(pixmap);
             QString label = tr("Scene") + " " + QString::number(i);  
             addScene(label, icon);
         }
    }
}

void TupStoryBoardDialog::updateForm(QListWidgetItem *current, QListWidgetItem *previous)
{
    int previousIndex = list->row(previous);
    currentIndex = list->row(current);
    QPixmap pixmap;

    if (currentIndex > 0) {
        int index = currentIndex - 1;
        sceneLabel->setText(tr("Scene No %1 - Information").arg(QString::number(index)));
        QString fileName = path + "scene" + QString::number(index) + ".png";
        pixmap = QPixmap(fileName);

        if (previousIndex == 0) {
            storyPanel->hide();
            scenePanel->show();

            storyboard->setStoryTitle(getStoryTitle());

            if (isNetworked)
                storyboard->setStoryTopics(getStoryTopics());
            else
                storyboard->setStoryTopics("");

            storyboard->setStoryAuthor(getStoryAuthor());
            storyboard->setStorySummary(getStorySummary());
        } else {
            previousIndex--;
            storyboard->setSceneTitle(previousIndex, getSceneTitle());
            storyboard->setSceneDuration(previousIndex, getSceneDuration());
            storyboard->setSceneDescription(previousIndex, getSceneDescription());
        }

        sceneTitleEdit->setText(storyboard->sceneTitle(index));
        sceneDurationEdit->setText(storyboard->sceneDuration(index));
        sceneDescriptionEdit->setPlainText(storyboard->sceneDescription(index));

    } else {
        if (previousIndex != 0) {
            pixmap = QPixmap(scaledSize.width(), scaledSize.height());
            pixmap.fill();

            QPainter painter(&pixmap);
            painter.setPen(Qt::black);
            QFont font = this->font();
            int fontSize = scaledSize.width()*30/520;
            font.setPointSize(fontSize);
            font.setBold(true);
            painter.setFont(font);
            // painter.setFont(QFont("Arial", scaledSize.width()*30/520, QFont::Bold));

            QRectF rect(QPointF(0, (scaledSize.height()-150)/2), QSizeF(scaledSize.width(), 150));
            painter.drawText(rect, Qt::AlignCenter, tr("Storyboard"));
            painter.setPen(Qt::lightGray);
            QRectF rectangle(5, 5, scaledSize.width() - 10, scaledSize.height() - 10);
            painter.drawRect(rectangle);

            scenePanel->hide();
            storyPanel->show();

            if (previousIndex > 0) {
                storyboard->setSceneTitle(previousIndex - 1, getSceneTitle());
                storyboard->setSceneDuration(previousIndex - 1, getSceneDuration());
                storyboard->setSceneDescription(previousIndex - 1, getSceneDescription());
            }

            titleEdit->setText(storyboard->storyTitle());

            if (isNetworked)
                topicsEdit->setText(storyboard->storyTopics());

            authorEdit->setText(storyboard->storyAuthor());
            summaryEdit->setPlainText(storyboard->storySummary());
        }
    }

    screenLabel->setPixmap(pixmap);
}

void TupStoryBoardDialog::createHTMLFiles(const QString &path, DocType type)
{
    if (scaledSize.width() <= 520) {
        QDir directory(path);
        QStringList files = directory.entryList();
        for (int i = 0; i < files.size(); ++i) {
             QString file = files.at(i).toLocal8Bit().constData();
             if (file != "." && file != "..") {
                 QString target = path + "/" + file;
                 if (QFile::exists(target))
                     QFile::remove(target);       
                 QFile::copy(path + file, target);
             }
        }
    } else {
        QDir directory(path);
        QStringList files = directory.entryList();
        for (int i = 0; i < files.size(); ++i) {
             QString file = files.at(i).toLocal8Bit().constData();
             QPixmap pixmap(path + file);
             QString destination = path + "/" + file;
             if (QFile::exists(destination))
                 QFile::remove(destination); 

             QPixmap resized;
             resized = pixmap.scaledToWidth(520, Qt::SmoothTransformation);
             resized.save(destination);
        }
    }

    QString base = kAppProp->shareDir() + "data/storyboard/";

    if (type == HTML) 
        QFile::copy(base + "tupi.html.css", path + "/tupitube.css");
    else
        QFile::copy(base + "tupi.pdf.css", path + "/tupitube.css");

    QString index = path + "/index.html";

    if (QFile::exists(index))
        QFile::remove(index);  

    QFile file(index);
    file.open(QIODevice::WriteOnly | QIODevice::Text);
    QTextStream out(&file);
    out << "<html>\n";
    out << "<head>\n";
    QString record = storyboard->storyTitle();
    if (record.length() == 0)
        record = "&nbsp;";
    out << "<title>" << record << "</title>\n";
    out << "<link rel=\"stylesheet\" type=\"text/css\" href=\"tupitube.css\" media=\"screen\" />\n";
    out << "</head>\n";
    out << "<body>\n";
    out << "<div id=\"header\">\n";
    out << "<div id=\"title\">Storyboard</div>\n";
    out << "<div id=\"item\">\n";
    out << "     <div id=\"item-header\">Title:</div>\n";
    out << "     <div id=\"item-data\">" << record << "</div>\n";
    out << "     </div>\n";
    out << "<div id=\"item\">\n";
    out << "     <div id=\"item-header\">Author:</div>\n";
    record = storyboard->storyAuthor();
    if (record.length() == 0)
        record = "&nbsp;";
    out << "     <div id=\"item-data\">" << record << "</div>\n";
    out << "</div>\n";
    out << "<div id=\"item\">\n";
    out << "     <div id=\"item-header\">Summary:</div>\n";
    record = storyboard->storySummary();
    if (record.length() == 0)
        record = "&nbsp;";
    out << "     <div id=\"item-data\">" << record << "</div>\n";
    out << "</div>\n";
    out << "<div id=\"item\">\n";
    out << "     <div id=\"item-header\">Scenes Total:</div>\n";
    out << "     <div id=\"item-data\">" << QString::number(storyboard->size()) << "</div>\n";
    out << "</div>\n";
    out << "</div>\n";
    if (type == PDF) {
        out << "<div id=\"page-break\">\n";
        out << "</div>\n";
    }

    int scenes = storyboard->size();
    for (int i=0; i < scenes; i++) {
         out << "<div id=\"scene\">\n";
         QString image = "<img src=\"scene" + QString::number(i) + ".png\" />\n";
         out << image;
         out << "<div id=\"paragraph\">\n";
         out << "<div id=\"scene-item\">\n";
         out << " <div id=\"scene-header\">Title:</div>\n";
         record = storyboard->sceneTitle(i);
         if (record.length() == 0)
             record = "&nbsp;";
         out << " <div id=\"scene-data\">" << record << "</div>\n";
         out << "</div>\n";
         out << "<div id=\"scene-item\">\n";
         out << " <div id=\"scene-header\">Duration:</div>\n";
         record = storyboard->sceneDuration(i);
         if (record.length() == 0)
             record = "&nbsp;";
         out << " <div id=\"scene-data\">" << record << "</div>\n";
         out << "</div>\n";
         out << "<div id=\"scene-item\">\n";
         out << " <div id=\"scene-header\">Description:</div>\n";
         record = storyboard->sceneDescription(i);
         if (record.length() == 0)
             record = "&nbsp;";
         out << " <div id=\"scene-data\">" << record << "</div>\n";
         out << "</div>\n";
         out << "</div>\n";
         out << "</div>\n";
         if (type == PDF) {
             if (i < (storyboard->size() - 1)) {
                 out << "<div id=\"page-break\">\n";
                 out << "</div>\n";
             }
         }
    }
    out << "</body>\n";
    out << "</html>";

    file.close(); 
}

void TupStoryBoardDialog::exportAsHTML()
{
    saveLastComponent();

    QString path = QFileDialog::getExistingDirectory(this, tr("Choose a directory..."), QDir::homePath(),
                                                     QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    if (!path.isEmpty()) {
        createHTMLFiles(path, HTML);
        TOsd::self()->display(tr("Info"), tr("Storyboard exported successfully!"), TOsd::Info);
    }
}

void TupStoryBoardDialog::exportAsPDF()
{
    saveLastComponent();

    QString path = QDir::tempPath() + "/" + TAlgorithm::randomString(8) + "/";
    QDir().mkpath(path);
    if (!path.isEmpty())
        createHTMLFiles(path, PDF);

    QString pdfPath = QFileDialog::getSaveFileName(this, tr("Save PDF file"), QDir::homePath(), tr("PDF file (*.pdf)"));

    if (!pdfPath.isEmpty()) {
        if (!pdfPath.toLower().endsWith(".pdf"))
            pdfPath += ".pdf";

        QFile file(path + "index.html");
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
            return;

        QString htmlContent;
        QTextStream in(&file);
        htmlContent = in.readAll();

        QPrinter printer;
        printer.setPageSize(QPrinter::A4);
        printer.setOutputFormat(QPrinter::PdfFormat);
        printer.setOutputFileName(pdfPath);

        QTextBrowser *document = new QTextBrowser;
        QStringList paths;
        paths << path;
        document->setSearchPaths(paths);
        document->setHtml(htmlContent);
        document->print(&printer);
        delete document;
        cleanDirectory(path);

        TOsd::self()->display(tr("Info"), tr("Storyboard exported successfully!"), TOsd::Info);
    }
}

void TupStoryBoardDialog::postStoryboardAtServer()
{
    #ifdef TUP_DEBUG
        QString msg = "TupStoryBoardDialog::postStoryBoardAtServer() - Posting in TupiTube!";
        #ifdef Q_OS_WIN
            qWarning() << msg;
        #else
            tWarning() << msg;
        #endif
    #endif

    saveLastComponent();

    // SQA: This "save call" line should be enhanced
    emit updateStoryboard(storyboard, sceneIndex);

    emit postStoryboard(sceneIndex);
}

void TupStoryBoardDialog::saveLastComponent()
{
    if (currentIndex == 0) {
        storyboard->setStoryTitle(getStoryTitle());
        storyboard->setStoryAuthor(getStoryAuthor());
        if (isNetworked)
            storyboard->setStoryTopics(getStoryTopics());
        else
            storyboard->setStoryTopics("");
        storyboard->setStorySummary(getStorySummary());
    } else {
        storyboard->setSceneTitle(currentIndex - 1, getSceneTitle());
        storyboard->setSceneDuration(currentIndex - 1, getSceneDuration());
        storyboard->setSceneDescription(currentIndex - 1, getSceneDescription());
    }
}

void TupStoryBoardDialog::closeDialog()
{
    saveLastComponent();
    cleanDirectory(path);

    if (isNetworked)
        emit updateStoryboard(storyboard, sceneIndex);

    close();
}

QString TupStoryBoardDialog::getStoryTitle() const
{
    return QString::fromUtf8(titleEdit->text().toUtf8());
}

QString TupStoryBoardDialog::getStoryAuthor() const
{
    return QString::fromUtf8(authorEdit->text().toUtf8());
}

QString TupStoryBoardDialog::getStoryTopics() const
{
    return QString::fromUtf8(topicsEdit->text().toUtf8());
}

QString TupStoryBoardDialog::getStorySummary() const
{
    return QString::fromUtf8(summaryEdit->toPlainText().toUtf8());
}

QString TupStoryBoardDialog::getSceneTitle() const
{
    return QString::fromUtf8(sceneTitleEdit->text().toUtf8());
}

QString TupStoryBoardDialog::getSceneDuration() const
{
    return QString::fromUtf8(sceneDurationEdit->text().toUtf8());
}

QString TupStoryBoardDialog::getSceneDescription() const
{
    return QString::fromUtf8(sceneDescriptionEdit->toPlainText().toUtf8());
}

void TupStoryBoardDialog::exportStoyrboard(const QString &type)
{
    if (type.compare(tr("PDF")) == 0) {
        exportAsPDF();
    } else if (type.compare(tr("Html")) == 0) {
               exportAsHTML();
    }
}

void TupStoryBoardDialog::cleanDirectory(const QString &folder)
{
    QDir dir(folder);
    QStringList files = dir.entryList();
    for (int i = 0; i < files.size(); ++i) {
         QString file = files.at(i).toLocal8Bit().constData();
         if (file != "." && file != "..")
             QFile::remove(folder + file);
    }

    if (!dir.rmdir(folder)) {
        #ifdef TUP_DEBUG
            QString msg = "TupStoryBoardDialog::cleanDirectory() - Can't remove path -> " + folder;
            #ifdef Q_OS_WIN
                qDebug() << msg;
            #else
                tError() << msg;
            #endif
        #endif
    }
}

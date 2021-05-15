/***************************************************************************
 *   Project TUPITUBE DESK                                                *
 *   Project Contact: info@maefloresta.com                                 *
 *   Project Website: http://www.maefloresta.com                           *
 *   Project Leader: Gustav Gonzalez <info@maefloresta.com>                *
 *                                                                         *
 *   Developers:                                                           *
 *   2019:                                                                 *
 *    Alejandro Carrasco Rodríguez                                         *
 *   2010:                                                                 *
 *    Gustavo Gonzalez                                                     *
 *                                                                         *
 *   KTooN's versions:                                                     * 
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
#include <QScreen>
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
    storyboard = scene->getStoryboard();
    library = assets;
    utf = QLocale(QLocale::AnyLanguage, QLocale::AnyCountry);

    QScreen *screen = QGuiApplication::screens().at(0);
    scaledSize = QSize();

    if (pSize.width() > pSize.height()) {
        if (size.width() + 500 > screen->geometry().width()) {
            int w = screen->geometry().width() - 500;
            int h = (size.height() * w) / size.width();
            scaledSize.setWidth(w);
            scaledSize.setHeight(h);
        } else {
            scaledSize = size;
        }
    } else {
        if (size.height() + 400 > screen->geometry().height()) {
            int h = screen->geometry().height() - 400;
            int w = (size.width() * h) / size.height();
            scaledSize.setWidth(w);
            scaledSize.setHeight(h);
        } else {
            scaledSize = size;
        }
    }

    if (scaledSize.height() + 400 > screen->geometry().height()) {
        int h = screen->geometry().height() - 400;
        int w = (size.width() * h) / size.height();
        scaledSize.setWidth(w);
        scaledSize.setHeight(h);
    }

    #ifdef TUP_DEBUG
        qDebug() << "[TupStoryBoardDialog()] - size -> " << size;
        qDebug() << "[TupStoryBoardDialog()] - scaledSize -> " << scaledSize;
    #endif

    setModal(true);
    setWindowTitle(tr("Storyboard Settings"));
    setWindowIcon(QIcon(QPixmap(THEME_DIR + "icons/storyboard.png")));

    layout = new QHBoxLayout(this);
    formLayout = new QVBoxLayout;

    setListComponent();
    setPreviewScreen();
    setCoverForm();
    setSceneForm();

    QPushButton *pdfButton = new QPushButton(tr("&PDF"));
    pdfButton->setToolTip(tr("Export as PDF"));
    pdfButton->setShortcut(QKeySequence("Ctrl+P"));
    connect(pdfButton, SIGNAL(clicked()), this, SLOT(exportAsPDF()));

    QPushButton *htmlButton = new QPushButton(tr("&HTML"));
    htmlButton->setToolTip(tr("Export as HTML"));
    connect(htmlButton, SIGNAL(clicked()), this, SLOT(exportAsHTML()));

    QPushButton *closeButton = new QPushButton(QIcon(QPixmap(THEME_DIR + "icons/apply.png")), "");
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

    thumbnailsGenerator();
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

void TupStoryBoardDialog::setCoverForm()
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
    sceneLayout->setAlignment(Qt::AlignCenter | Qt::AlignBottom);

    QFont font = this->font();
    font.setBold(true);
    sceneLabel = new QLabel("");
    sceneLabel->setFont(font);
    sceneLabel->setAlignment(Qt::AlignHCenter);

    QLabel *durationLabel = new QLabel(tr("Duration"));

    secSpinBox = new QDoubleSpinBox;
    secSpinBox->setDecimals(3);
    secSpinBox->setMinimum(0);
    secSpinBox->setMaximum(20);
    secSpinBox->setValue(0);
    secSpinBox->setSingleStep(0.100);
    connect(secSpinBox, SIGNAL(valueChanged(double)), this, SLOT(updateDuration(double)));

    QLabel *secsLabel = new QLabel(tr("seconds"));

    QWidget *secondsPanel = new QWidget;
    QHBoxLayout *secondsLayout = new QHBoxLayout(secondsPanel);
    secondsLayout->addWidget(durationLabel);
    secondsLayout->addWidget(secSpinBox);
    secondsLayout->addWidget(secsLabel);

    sceneLayout->addWidget(sceneLabel);
    sceneLayout->addWidget(secondsPanel);
    sceneLayout->addStretch();

    formLayout->addWidget(scenePanel, Qt::AlignHCenter);
    formLayout->addStretch();

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

void TupStoryBoardDialog::thumbnailsGenerator()
{
    QPixmap pixmap = QPixmap(size);
    pixmap.fill();

    path = QDir::tempPath() + "/" + TAlgorithm::randomString(8) + "/";
    QDir().mkpath(path + "images");
    QDir().mkpath(path + "display");

    #ifdef TUP_DEBUG
        qDebug() << "[TupStoryBoardDialog::thumbnailGenerator()] - images path -> " << path;
    #endif

    // Creating cover
    QPainter painter(&pixmap);
    painter.setPen(Qt::black);

    QString text = tr("Storyboard\nCover");
    QFont font = setFont(0.5);
    painter.setFont(font);

    QRectF rect(QPointF(0, 0), size);
    painter.drawText(rect, Qt::AlignCenter, text);
    painter.setPen(QColor(230, 230, 230));
    QRectF rectangle(0, 0, size.width() - 1, size.height() - 1);
    painter.drawRect(rectangle);
    painter.end();
    pixmap.save(path + "images/cover.png");

    pixmap = pixmap.scaledToWidth(scaledSize.width(), Qt::SmoothTransformation);
    pixmap.save(path + "display/cover.png");

    pixmap = pixmap.scaledToWidth(96, Qt::SmoothTransformation);
    QIcon icon = QIcon(pixmap);
    addScene(tr("Cover"), icon);

    int framesCount = scene->framesCount();
    if (storyboard->size() == 0)
        storyboard->init(0, framesCount);

    // Creating scenes thumbnails
    for (int i=0; i < framesCount; i++) {
         QString fileName = path + "images/scene" + QString::number(i) + ".png";
         bool isOk = imagePlugin->exportFrame(i, bgColor, fileName, scene, size, library);

         QPixmap displayPic = QPixmap(fileName);
         displayPic = displayPic.scaledToWidth(scaledSize.width(), Qt::SmoothTransformation);
         displayPic.save(path + "display/scene" + QString::number(i) + ".png");

         if (isOk) {
             QPixmap thumbnail = QPixmap(path + "display/scene" + QString::number(i) + ".png");
             QPainter iconPainter(&thumbnail);
             iconPainter.setPen(Qt::darkGray);
             QRectF rectangle(0, 0, scaledSize.width() - 1, scaledSize.height() - 1);
             iconPainter.drawRect(rectangle);
             iconPainter.end();
             thumbnail = thumbnail.scaledToWidth(96, Qt::SmoothTransformation);

             QIcon icon(thumbnail);
             QString label = tr("Scene") + " " + QString::number(i + 1);
             addScene(label, icon);
         }
    }
}

void TupStoryBoardDialog::updateForm(QListWidgetItem *current, QListWidgetItem *previous)
{
    int previousIndex = list->row(previous);
    currentIndex = list->row(current);
    QPixmap pixmap;

    if (currentIndex > 0) { // Scene != Cover
        int index = currentIndex - 1;
        sceneLabel->setText(tr("Scene No %1").arg(QString::number(currentIndex)));
        QString filename = path + "display/scene" + QString::number(index) + ".png";
        if (QFile::exists(filename)) {
            pixmap = QPixmap(filename);

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
                storyboard->setSceneDuration(previousIndex, getSceneDuration());
            }
        } else {
            #ifdef TUP_DEBUG
                qDebug() << "[TupStoryBoardDialog::updateForm()] - Fatal error: image doesn't exist -> " << filename;
            #endif
            return;
        }
        QString duration = storyboard->sceneDuration(index);
        secSpinBox->setValue(duration.toDouble());
    } else { // Cover
        if (previousIndex != 0) {
            pixmap = renderCover(scaledSize);

            scenePanel->hide();
            storyPanel->show();

            if (previousIndex > 0)
                storyboard->setSceneDuration(previousIndex - 1, getSceneDuration());

            titleEdit->setText(storyboard->storyTitle());

            if (isNetworked)
                topicsEdit->setText(storyboard->storyTopics());

            authorEdit->setText(storyboard->storyAuthor());
            summaryEdit->setPlainText(storyboard->storySummary());
        }
    }

    // Display image
    screenLabel->setPixmap(pixmap);
}

QFont TupStoryBoardDialog::setFont(double proportion) const
{
    QFont font = this->font();
    font.setBold(true);

    double percent = 0.1;
    int fontSize;
    while (true) {
        fontSize = scaledSize.width() * percent;
        font.setPointSize(fontSize);
        QFontMetrics fm(font);
        int width = fm.horizontalAdvance(tr("Storyboard"));
        if (width < (scaledSize.width()/proportion))
            break;
        percent -= 0.01;
    }

    return font;
}

QString TupStoryBoardDialog::formatString(QStringList stringWords, int size) const
{
    QString string = "";
    for (int i=0; i<stringWords.size(); i++) {
        string += stringWords.at(i) + " ";
        if (i > 0) {
            if (i % size == 0)
                string += "\n";
        }
    }

    return string;
}

QPixmap TupStoryBoardDialog::renderCover(const QSize dimension)
{
    QPixmap pixmap = QPixmap(dimension.width(), dimension.height());
    pixmap.fill();

    QPainter painter(&pixmap);
    painter.setPen(Qt::black);
    QRectF rectangle(0, 0, dimension.width() - 1, dimension.height() - 1);

    QString storyTitle = storyboard->storyTitle();
    if (storyTitle.isEmpty()) {
        QFont font = setFont(2);
        font.setBold(true);
        painter.setFont(font);
        painter.drawText(rectangle, Qt::AlignCenter, tr("Storyboard\nCover"));
    } else {
        QStringList titleWords = storyTitle.split(" ");
        if (titleWords.size() > 6)
            storyTitle = formatString(titleWords, 6);

        QString author = storyboard->storyAuthor();
        QString summary = storyboard->storySummary();
        QStringList summaryWords = summary.split(" ");
        if (summaryWords.size() > 18)
            summary = formatString(summaryWords, 18);

        int delta = dimension.height() / 2;
        int delta2 = delta * 0.2;
        int delta3 = delta * 0.8;
        QRectF rect1(0, 0, dimension.width(), delta);
        QRectF rect2(0, delta, dimension.width(), delta2);
        QRectF rect3(0, delta + delta2, dimension.width(), delta3);

        painter.setPen(Qt::black);

        QFont font = setFont(2);
        painter.setFont(font);
        painter.drawText(rect1, Qt::AlignCenter, storyTitle);

        font = setFont(3);
        painter.setFont(font);
        painter.drawText(rect2, Qt::AlignCenter, tr("By") + " " + author);

        font = setFont(5);
        painter.setFont(font);
        painter.drawText(rect3, Qt::AlignCenter, summary);
    }

    painter.setPen(Qt::lightGray);
    painter.drawRect(rectangle);

    QString fileName = path + "display/cover.png";
    pixmap.save(fileName);

    return pixmap;
}

void TupStoryBoardDialog::createHTMLFiles(const QString &savePath, DocType type)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupStoryBoardDialog::createHTMLFiles()] - savePath -> " << savePath;
        qDebug() << "[TupStoryBoardDialog::createHTMLFiles()] - path -> " << path;
    #endif

    QPixmap pixmap = renderCover(size);
    QString fileName = path + "images/cover.png";
    pixmap.save(fileName);

    if (scaledSize.width() <= 520) {
        // find all .png files in path (var) directory
        QDir directory(path + "images");
        directory.setNameFilters(QStringList() << "*.png");
        QStringList files = directory.entryList();

        /*
        int scenes = files.size();
        #ifdef TUP_DEBUG
            qDebug() << "[TupStoryBoardDialog::createHTMLFiles()] - files.size() -> " << scenes;
        #endif
        */

        // copy all .png files
        for (int i = 0; i < files.size(); ++i) {
             QString file = files.at(i).toLocal8Bit().constData();
             QString target = savePath + "/" + file;

             if (QFile::exists(target)) QFile::remove(target);
             QFile::copy(path + "images/" + file, target);
        }

    } else {
        // find all .png files in path (var) directory
        QDir directory(path + "images");
        directory.setNameFilters(QStringList() << "*.png");
        QStringList files = directory.entryList();

        /*
        int scenes = files.size();
        #ifdef TUP_DEBUG
            qDebug() << "[TupStoryBoardDialog::createHTMLFiles()] - files.size() -> " << scenes;
        #endif
        */

        // scale and copy all .png files
        for (int i = 0; i < files.size(); ++i) {
             QString file = files.at(i).toLocal8Bit().constData();
             QPixmap pixmap(path + "images/" + file);
             QString destination = savePath + "/" + file;

             if (QFile::exists(destination))
                 QFile::remove(destination);

             pixmap.save(destination);
        }
    }

    QString base = kAppProp->shareDir() + "data/storyboard/";
    if (type == HTML) 
        QFile::copy(base + "tupi.html.css", savePath + "/tupitube.css");
    else
        QFile::copy(base + "tupi.pdf.css", savePath + "/tupitube.css");

    QString indexPath = savePath + "index.html";
    if (QFile::exists(indexPath))
        QFile::remove(indexPath);

    #ifdef TUP_DEBUG
        qDebug() << "[TupStoryBoardDialog::createHTMLFiles()] - indexPath -> " << indexPath;
    #endif

    QFile indexFile(indexPath);
    indexFile.open(QIODevice::WriteOnly | QIODevice::Text);
    QTextStream out(&indexFile);
    out << "<html>\n";
    out << "<head>\n";
    /*
    QString record = storyboard->storyTitle();
    if (record.length() == 0)
        record = "&nbsp;";
    out << "<title>" << record << "</title>\n";
    */

    out << "<link rel=\"stylesheet\" type=\"text/css\" href=\"tupitube.css\" media=\"print\"/>\n";
    out << "</head>\n";
    out << "<body>\n";

    /*
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
    */

    int scenes = storyboard->size();
    #ifdef TUP_DEBUG
        qDebug() << "[TupStoryBoardDialog::createHTMLFiles()] - scenes count -> " << scenes;
    #endif

    // out << "<div id=\"scene\">\n";
    QString image = "<img class=\"printThisFull\" src=\"cover.png\" />\n";
    out << image;
    // out << "</div>\n";
    /*
    if (type == PDF) {
        out << "<div id=\"page-break\">\n";
        out << "</div>\n";
    }
    */

    for (int i=0; i < scenes; i++) {
         // out << "<div id=\"scene\">\n";
         QString image = "<img class=\"printThisFull\" src=\"scene" + QString::number(i) + ".png\" />\n";
         out << image;
         // out << "</div>\n";

         /*
         out << "<div id=\"paragraph\">\n";
         out << "<div id=\"scene-item\">\n";
         out << " <div id=\"scene-header\">Duration:</div>\n";
         record = storyboard->sceneDuration(i);
         if (record.length() == 0)
             record = "&nbsp;";
         out << " <div id=\"scene-data\">" << record << "</div>\n";
         out << "</div>\n";
         out << "</div>\n";
         */

         /*
         if (type == PDF) {
             if (i < (storyboard->size() - 1)) {
                 out << "<div id=\"page-break\">\n";
                 out << "</div>\n";
             }
         }
         */
    }
    out << "</body>\n";
    out << "</html>";

    indexFile.close();
}

void TupStoryBoardDialog::exportAsHTML()
{
    saveLastComponent();
    QString path = QFileDialog::getExistingDirectory(this, tr("Choose a directory..."), QDir::homePath(),
                                                     QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);

    if (!path.isEmpty()) {
        createHTMLFiles(path, HTML);
        TOsd::self()->display(TOsd::Info, tr("Storyboard exported successfully!"));
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
        // printer.setPageSize(QPrinter::A4);
        // QPageSize pageSize(QPageSize::Custom);

        QPageSize pageSize(size, QPageSize::Point);
        printer.setPageSize(pageSize);
        // printer.setPageOrientation(QPageLayout::Landscape);
        printer.setOutputFormat(QPrinter::PdfFormat);
        printer.setOutputFileName(pdfPath);

        QTextBrowser *document = new QTextBrowser;
        QStringList paths;
        paths << path;
        document->setSearchPaths(paths);
        document->setHtml(htmlContent);
        document->print(&printer);
        delete document;

        // cleanDirectory(path);

        TOsd::self()->display(TOsd::Info, tr("Storyboard exported successfully!"));
    }
}

void TupStoryBoardDialog::postStoryboardAtServer()
{
    #ifdef TUP_DEBUG
        qWarning() << "[TupStoryBoardDialog::postStoryBoardAtServer()] - Posting in TupiTube!";
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
        storyboard->setSceneDuration(currentIndex - 1, getSceneDuration());
    }
}

void TupStoryBoardDialog::closeDialog()
{
    saveLastComponent();
    cleanDirectory(path);

    if (isNetworked)
        emit updateStoryboard(storyboard, sceneIndex);

    // close();
    accept();
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

QString TupStoryBoardDialog::getSceneDuration() const
{
    double value = secSpinBox->value();
    return QString::number(value);
}

// SQA ¿quien usa este método? & error de gramática
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
            qDebug() << "[TupStoryBoardDialog::cleanDirectory()] - Can't remove path -> " << folder;
        #endif
    }
}

void TupStoryBoardDialog::updateDuration(double value)
{
    /*
    #ifdef TUP_DEBUG
        qDebug() << "[TupStoryBoardDialog::updateDuration()] - value -> " << value;
    #endif
    */

    int index = list->currentRow() - 1;
    storyboard->setSceneDuration(index, QString::number(value));
}

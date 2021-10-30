/***************************************************************************
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

#include "tuppapagayoapp.h"
#include "tapplicationproperties.h"
#include "tconfig.h"
#include "tosd.h"

#include <QAction>
#include <QToolBar>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QBoxLayout>
#include <QFileInfo>
#include <QSettings>
#include <QMimeData>
#include <QFileDialog>
#include <QScrollArea>
#include <QSpacerItem>
#include <QGroupBox>
#include <QLabel>
#include <QScreen>

TupPapagayoApp::TupPapagayoApp(bool extendedUI, int32 fps, const QString &soundFile, QWidget *parent) : QMainWindow(parent)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupPapagayoApp::TupPapagayoApp()]";
    #endif

    this->extendedUI = extendedUI;
    document = nullptr;
    enableAutoBreakdown = true;
    defaultFps = fps;
    playerStopped = true;

    setUIStyle();

    setupActions();
    setupUI();
    setupMenus();
    setAcceptDrops(true);

    updateActions();

    if (!soundFile.isEmpty())
        openFile(soundFile);
}

TupPapagayoApp::~TupPapagayoApp()
{
    if (document)
        delete document;

    if (waveformView)
        delete waveformView;

    if (mouthView)
        delete mouthView;
}

void TupPapagayoApp::setUIStyle()
{
    QFile file(THEME_DIR + "config/ui.qss");
    if (file.exists()) {
        file.open(QFile::ReadOnly);
        QString uiStyleSheet = QLatin1String(file.readAll());
        if (uiStyleSheet.length() > 0)
            setStyleSheet(uiStyleSheet);
        file.close();
    } else {
        #ifdef TUP_DEBUG
            qWarning() << "[TupPapagayoApp()] - Error: Theme file doesn't exist -> "
                       << QString(THEME_DIR + "config/ui.qss");
        #endif
    }
}

void TupPapagayoApp::setupActions()
{
    // Actions
    actionClose = new QAction(this);
    QIcon closeIcon;
    closeIcon.addFile(THEME_DIR + "icons/close.png", QSize(), QIcon::Normal, QIcon::Off);
    actionClose->setIcon(closeIcon);
    actionClose->setText(tr("Close"));
    actionClose->setShortcut(Qt::Key_Escape);
    connect(actionClose, SIGNAL(triggered()), this, SLOT(close()));

    if (extendedUI) {
        actionOpen = new QAction(this);
        QIcon openIcon;
        openIcon.addFile(THEME_DIR + "icons/open.png", QSize(), QIcon::Normal, QIcon::Off);
        actionOpen->setIcon(openIcon);
        actionOpen->setText(tr("Open"));
        actionOpen->setShortcut(QKeySequence(tr("Ctrl+O")));
        connect(actionOpen, SIGNAL(triggered()), this, SLOT(onFileOpen()));
    }

    actionSave = new QAction(this);
    QIcon saveIcon;
    saveIcon.addFile(THEME_DIR + "icons/save.png", QSize(), QIcon::Normal, QIcon::Off);
    actionSave->setIcon(saveIcon);
    actionSave->setText(tr("Save"));
    actionSave->setShortcut(QKeySequence(tr("Ctrl+S")));
    connect(actionSave, SIGNAL(triggered()), this, SLOT(onFileSave()));

    actionPlay = new QAction(this);
    playIcon.addFile(THEME_DIR + "icons/play.png", QSize(), QIcon::Normal, QIcon::Off);
    actionPlay->setIcon(playIcon);
    actionPlay->setText(tr("Play"));
    actionPlay->setToolTip(tr("Play"));
    connect(actionPlay, SIGNAL(triggered()), this, SLOT(onPlay()));

    pauseIcon.addFile(THEME_DIR + "icons/pause.png", QSize(), QIcon::Normal, QIcon::Off);

    actionStop = new QAction(this);
    QIcon stopIcon;
    stopIcon.addFile(THEME_DIR + "icons/stop.png", QSize(), QIcon::Normal, QIcon::Off);
    actionStop->setIcon(stopIcon);
    actionStop->setText(tr("Stop"));
    actionStop->setToolTip(tr("Stop"));
    connect(actionStop, SIGNAL(triggered()), this, SLOT(onStop()));

    actionZoomIn = new QAction(this);
    QIcon zoomInIcon;
    zoomInIcon.addFile(THEME_DIR + "icons/zoom_in.png", QSize(), QIcon::Normal, QIcon::Off);
    actionZoomIn->setIcon(zoomInIcon);
    actionZoomIn->setText(tr("Zoom In"));
    actionZoomIn->setToolTip(tr("Zoom In"));

    actionZoomOut = new QAction(this);
    QIcon zoomOutIcon;
    zoomOutIcon.addFile(THEME_DIR + "icons/zoom_out.png", QSize(), QIcon::Normal, QIcon::Off);
    actionZoomOut->setIcon(zoomOutIcon);
    actionZoomOut->setText(tr("Zoom Out"));
    actionZoomOut->setToolTip(tr("Zoom Out"));

    actionAutoZoom = new QAction(this);
    QIcon autoZoomIcon;
    autoZoomIcon.addFile(THEME_DIR + "icons/zoom_1.png", QSize(), QIcon::Normal, QIcon::Off);
    actionAutoZoom->setIcon(autoZoomIcon);
    actionAutoZoom->setText(tr("Auto Zoom"));
    actionAutoZoom->setToolTip(tr("Auto Zoom"));

    /*
    actionUndo = new QAction(this);
    actionUndo->setText(tr("Undo"));
    actionUndo->setShortcut(QKeySequence("Ctrl+Z"));

    actionCut = new QAction(this);
    actionCut->setText(tr("Cut"));
    actionCut->setShortcut(tr("Ctrl+X"));

    actionCopy = new QAction(this);
    actionCopy->setText(tr("Copy"));
    actionCopy->setShortcut(tr("Ctrl+C"));

    actionPaste = new QAction(this);
    actionPaste->setText(tr("Paste"));
    actionPaste->setShortcut(tr("Ctrl+V"));
    */
}

void TupPapagayoApp::setupUI()
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupPapagayoApp::setupUI()]";
    #endif

    setWindowTitle(tr("Lip-Sync Manager"));

    // Central Widget
    QWidget *centralWidget = new QWidget(this);

    QHBoxLayout *centralHorizontalLayout = new QHBoxLayout(centralWidget);
    centralHorizontalLayout->setSpacing(6);
    centralHorizontalLayout->setContentsMargins(11, 11, 11, 11);

    QVBoxLayout *innerVerticalLayout = new QVBoxLayout();
    innerVerticalLayout->setSpacing(6);

    QHBoxLayout *firstLineLayout = new QHBoxLayout;

    QScrollArea *scrollArea = new QScrollArea(centralWidget);
    scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    scrollArea->setWidgetResizable(true);

    waveformView = new TupWaveFormView();
    waveformView->setGeometry(QRect(0, 0, 542, 194));
    scrollArea->setWidget(waveformView);
    waveformView->setScrollArea(scrollArea);

    connect(waveformView, SIGNAL(audioStopped()), this, SLOT(updatePauseButton()));
    connect(actionZoomIn, SIGNAL(triggered()), waveformView, SLOT(zoomIn()));
    connect(actionZoomOut, SIGNAL(triggered()), waveformView, SLOT(zoomOut()));
    connect(actionAutoZoom, SIGNAL(triggered()), waveformView, SLOT(autoZoom()));

    // Mouth Component
    QVBoxLayout *verticalLayout = new QVBoxLayout();
    verticalLayout->setSpacing(6);

    mouthsCombo = new QComboBox;
    mouthsCombo->addItem(QIcon(THEME_DIR + "icons/frames_mode.png"), tr("Mouth Sample Pack No 1"));
    mouthsCombo->addItem(QIcon(THEME_DIR + "icons/frames_mode.png"), tr("Mouth Sample Pack No 2"));
    mouthsCombo->addItem(QIcon(THEME_DIR + "icons/frames_mode.png"), tr("Mouth Sample Pack No 3"));
    mouthsCombo->addItem(QIcon(THEME_DIR + "icons/frames_mode.png"), tr("Mouth Sample Pack No 4"));
    mouthsCombo->addItem(QIcon(THEME_DIR + "icons/frames_mode.png"), tr("Mouth Sample Pack No 5"));
    mouthsCombo->addItem(QIcon(THEME_DIR + "icons/frames_mode.png"), tr("Set Mouth Images"));

    verticalLayout->addWidget(mouthsCombo);

    mouthFrame = new QStackedWidget(this);
    QSizePolicy viewSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
    viewSizePolicy.setHorizontalStretch(0);
    viewSizePolicy.setVerticalStretch(0);
    viewSizePolicy.setHeightForWidth(mouthFrame->sizePolicy().hasHeightForWidth());

    mouthFrame->setSizePolicy(viewSizePolicy);
    mouthFrame->setMinimumSize(QSize(280, 200));
    mouthFrame->setMaximumWidth(280);

    QWidget *browserWidget = new QWidget;
    QVBoxLayout *browserMainLayout = new QVBoxLayout(browserWidget);

    QHBoxLayout *browserControlsLayout = new QHBoxLayout;
    QPushButton *mouthsButton = new QPushButton(QIcon(QPixmap(THEME_DIR + "icons/bitmap_array.png")),
                                                " " + tr("Load &Images"));
    connect(mouthsButton, SIGNAL(clicked()), this, SLOT(openImagesDialog()));
    mouthsPath = new QLineEdit("");
    mouthsPath->setReadOnly(true);

    mouthView = new TupMouthView(mouthFrame);
    viewSizePolicy.setHeightForWidth(mouthView->sizePolicy().hasHeightForWidth());
    mouthView->setSizePolicy(viewSizePolicy);
    mouthView->setMinimumSize(QSize(280, 200));
    mouthView->setMaximumWidth(280);

    // SQA: stylesheet pending
    // mouthView->setStyleSheet(QString::fromUtf8(""));

    connect(waveformView, SIGNAL(frameChanged(int)), this, SLOT(updateFrame(int)));
    connect(mouthsCombo, SIGNAL(activated(int)), this, SLOT(updateMouthView(int)));

    mouthFrame->addWidget(mouthView);

    customView = new TupCustomizedMouthView;
    viewSizePolicy.setHeightForWidth(customView->sizePolicy().hasHeightForWidth());
    customView->setSizePolicy(viewSizePolicy);
    customView->setMinimumSize(QSize(280, 200));
    customView->setMaximumWidth(280);

    browserControlsLayout->addWidget(mouthsPath);
    browserControlsLayout->addWidget(mouthsButton);

    browserMainLayout->addLayout(browserControlsLayout);
    browserMainLayout->addWidget(customView);
    mouthFrame->addWidget(browserWidget);

    verticalLayout->addWidget(mouthFrame);
    verticalLayout->addStretch();

    firstLineLayout->addWidget(scrollArea);
    firstLineLayout->addLayout(verticalLayout);

    innerVerticalLayout->addLayout(firstLineLayout);

    // Lateral Panel
    QGroupBox *lateralGroupBox = new QGroupBox(centralWidget);
    lateralGroupBox->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    lateralGroupBox->setTitle(tr("Current Voice"));

    QSizePolicy generalSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    generalSizePolicy.setHorizontalStretch(0);
    generalSizePolicy.setVerticalStretch(0);
    generalSizePolicy.setHeightForWidth(lateralGroupBox->sizePolicy().hasHeightForWidth());

    QVBoxLayout *lateralVerticalLayout = new QVBoxLayout(lateralGroupBox);
    lateralVerticalLayout->setSpacing(6);
    lateralVerticalLayout->setContentsMargins(11, 11, 11, 11);
    QHBoxLayout *voiceHorizontalLayout = new QHBoxLayout();
    voiceHorizontalLayout->setSpacing(6);

    voiceName = new QLineEdit();
    connect(voiceName, SIGNAL(textChanged(QString)), this, SLOT(onVoiceNameChanged()));

    QLabel *voiceLabel = new QLabel(lateralGroupBox);
    voiceLabel->setText(tr("Voice name:"));
    voiceLabel->setBuddy(voiceName);

    voiceHorizontalLayout->addWidget(voiceLabel);
    voiceHorizontalLayout->addWidget(voiceName);

    QSpacerItem *voiceHorizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
    voiceHorizontalLayout->addItem(voiceHorizontalSpacer);
    lateralVerticalLayout->addLayout(voiceHorizontalLayout);

    voiceText = new QPlainTextEdit(lateralGroupBox);
    voiceText->setMaximumHeight(100);
    QSizePolicy voiceSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    voiceSizePolicy.setHorizontalStretch(0);
    voiceSizePolicy.setVerticalStretch(0);
    voiceText->setSizePolicy(voiceSizePolicy);
    connect(voiceText, SIGNAL(textChanged()), this, SLOT(onVoiceTextChanged()));

    QLabel *spokenText = new QLabel(lateralGroupBox);
    spokenText->setText(tr("Spoken text:"));
    spokenText->setBuddy(voiceText);

    lateralVerticalLayout->addWidget(spokenText);
    lateralVerticalLayout->addWidget(voiceText);

    QLabel *phoneticLabel = new QLabel(lateralGroupBox);
    phoneticLabel->setText(tr("Phonetic breakdown:"));

    lateralVerticalLayout->addWidget(phoneticLabel);

    QHBoxLayout *languageHorizontalLayout = new QHBoxLayout;
    languageHorizontalLayout->setSpacing(6);

    languageChoice = new QComboBox();
    languageChoice->addItem(tr("English"));
    languageChoice->addItem(tr("Spanish"));
    languageChoice->addItem(tr("Italian"));

    languageHorizontalLayout->addWidget(languageChoice);

    breakdownButton = new QPushButton(lateralGroupBox);
    breakdownButton->setText(tr("Breakdown"));
    connect(breakdownButton, SIGNAL(clicked()), this, SLOT(runBreakdownProcess()));

    okButton = new QPushButton(lateralGroupBox);
    okButton->setMinimumWidth(60);
    okButton->setIcon(QIcon(THEME_DIR + "icons/apply.png"));
    okButton->setToolTip(tr("Save lip-sync record"));
    connect(okButton, SIGNAL(clicked()), this, SLOT(createLipsyncRecord()));

    QPushButton *cancelButton = new QPushButton(lateralGroupBox);
    cancelButton->setMinimumWidth(60);
    cancelButton->setIcon(QIcon(THEME_DIR + "icons/close.png"));
    cancelButton->setToolTip(tr("Cancel"));
    connect(cancelButton, SIGNAL(clicked()), this, SLOT(close()));

    languageHorizontalLayout->addWidget(breakdownButton);

    QSpacerItem *languageHorizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
    languageHorizontalLayout->addItem(languageHorizontalSpacer);

    languageHorizontalLayout->addWidget(okButton);
    languageHorizontalLayout->addWidget(cancelButton);

    lateralVerticalLayout->addLayout(languageHorizontalLayout);
    innerVerticalLayout->addWidget(lateralGroupBox);
    centralHorizontalLayout->addLayout(innerVerticalLayout);

    setCentralWidget(centralWidget);

    QScreen *screen = QGuiApplication::screens().at(0);
    setMinimumWidth(screen->geometry().width() * 0.7);
}

void TupPapagayoApp::setupMenus()
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupPapagayoApp::setupMenus()]";
    #endif

    QMenuBar *menuBar = new QMenuBar(this);
    menuBar->setGeometry(QRect(0, 0, 872, 22));

    QMenu *menuFile = new QMenu(menuBar);
    menuFile->setTitle(tr("File"));

    if (extendedUI)
        menuFile->addAction(actionOpen);

    menuFile->addAction(actionClose);

    /*
    menuEdit->addAction(actionUndo);
    menuEdit->addAction(actionCut);
    menuEdit->addAction(actionCopy);
    menuEdit->addAction(actionPaste);
    */

    menuBar->addAction(menuFile->menuAction());
    // menuBar->addAction(menuEdit->menuAction());

    setMenuBar(menuBar);

    QToolBar *mainToolBar = new QToolBar(this);
    mainToolBar->setMovable(true);
    mainToolBar->setIconSize(QSize(32, 32));
    mainToolBar->setToolButtonStyle(Qt::ToolButtonIconOnly);
    mainToolBar->setFloatable(false);

    if (extendedUI)
        mainToolBar->addAction(actionOpen);

    mainToolBar->addAction(actionSave);
    mainToolBar->addSeparator();
    mainToolBar->addAction(actionPlay);
    mainToolBar->addAction(actionStop);
    mainToolBar->addSeparator();
    mainToolBar->addAction(actionZoomIn);
    mainToolBar->addAction(actionZoomOut);
    mainToolBar->addAction(actionAutoZoom);

    addToolBar(Qt::TopToolBarArea, mainToolBar);
}

void TupPapagayoApp::openFile(QString filePath)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupPapagayoApp::openFile()] - filePath -> " << filePath;
    #endif

    if (document) {
        delete document;
        document = nullptr;
    }

    document = new TupLipsyncDoc;
    QFileInfo info(filePath);
    if (info.suffix().toLower() == "pgo") {
        document->open(filePath);
    } else {
        document->openAudio(filePath);
        document->setFps(defaultFps);
    }

    if (document->getAudioPlayer() == nullptr) {
        delete document;
        document = nullptr;
        QMessageBox::warning(this, tr("Lip-Sync Manager"),
                             tr("Error opening audio file."),
                             QMessageBox::Ok);
        setWindowTitle(tr("Lip-Sync Manager"));
    } else {
        waveformView->setDocument(document);
        mouthView->setDocument(document);
        customView->setDocument(document);

        document->getAudioPlayer()->setNotifyInterval(17); // 60 fps
        connect(document->getAudioPlayer(), SIGNAL(positionChanged(qint64)), waveformView, SLOT(positionChanged(qint64)));

        if (document->getCurrentVoice()) {
            voiceName->setText(document->getCurrentVoice()->getName());
            enableAutoBreakdown = false;
            voiceText->setPlainText(document->getCurrentVoice()->getText());
            enableAutoBreakdown = true;
        }

        setWindowTitle(tr("Lip-Sync Manager") + " - " + info.fileName());
    }

    // fpsEdit->setValue(document->getFps());
    updateActions();
}

bool TupPapagayoApp::isOKToCloseDocument()
{
    if (document && document->isDirty()) {
        int res = QMessageBox::warning(this, tr("Lip-Sync Manager"),
                  tr("This lip-sync item has been modified.\n"
                  "Do you want to save your changes?"),
                  QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);

        if (res == QMessageBox::Yes) {
            onFileSave();
            if (document->isDirty())
                return false;
            else
                return true;
        } else if (res == QMessageBox::No) {
            return true;
        } else if (res == QMessageBox::Cancel) {
            return false;
        }
    }

    return true;
}

void TupPapagayoApp::closeEvent(QCloseEvent *event)
{
    if (isOKToCloseDocument()) {
        if (document) {
            delete document;
            document = nullptr;
        }
        event->accept();
    } else {
        event->ignore();
    }
}

void TupPapagayoApp::dragEnterEvent(QDragEnterEvent *event)
{
    QList<QUrl> urls = event->mimeData()->urls();
    if (urls.isEmpty())
        return;

    QString filePath = urls.first().toLocalFile();

    if (filePath.isEmpty())
        return;

    QFileInfo info(filePath);
    QString extension = info.suffix().toLower();
    // if (extension == "pgo" || extension == "mp3" || extension == "wav")
    if (extension == "mp3" || extension == "wav")
        event->acceptProposedAction();
}

void TupPapagayoApp::dropEvent(QDropEvent *event)
{
    QList<QUrl> urls = event->mimeData()->urls();
    if (urls.isEmpty())
        return;

    QString filePath = urls.first().toLocalFile();

    if (filePath.isEmpty())
        return;

    QFileInfo info(filePath);
    QString extension = info.suffix().toLower();
    // if (extn == "pgo" || extn == "mp3" || extn == "wav") {
    if (extension == "mp3" || extension == "wav") {
        event->acceptProposedAction();
        if (isOKToCloseDocument())
            openFile(filePath);
    }
}

void TupPapagayoApp::updateActions()
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupPapagayoApp::updateActions()]";
    #endif

    bool flag = false;
    if (document)
        flag = true;

    actionSave->setEnabled(flag);
    actionPlay->setEnabled(flag);
    actionStop->setEnabled(flag);
    actionZoomIn->setEnabled(flag);
    actionZoomOut->setEnabled(flag);
    actionAutoZoom->setEnabled(flag);

    voiceName->setEnabled(flag);
    voiceText->setEnabled(flag);

    languageChoice->setEnabled(flag);
    // fpsEdit->setEnabled(flag);
    mouthsCombo->setEnabled(flag);

    breakdownButton->setEnabled(flag);
    okButton->setEnabled(flag);
}

void TupPapagayoApp::onFileOpen()
{
    if (!isOKToCloseDocument())
        return;

    TCONFIG->beginGroup("General");
    QString path = TCONFIG->value("DefaultPath", QDir::homePath()).toString();
    QString filePath = QFileDialog::getOpenFileName(this,
                                                    tr("Open"), path,
                                                    // tr("Papgayo and Audio files (*.pgo *.mp3 *.wav)"));
                                                    tr("Audio files (*.mp3 *.wav)"));
    if (filePath.isEmpty())
        return;

    openFile(filePath);
}

void TupPapagayoApp::onFileSave()
{
    if (!document)
        return;

    document->save();
    QFileInfo info(document->getFilePath());
    setWindowTitle(tr("Lip-sync Creator") + " - " + info.fileName());
}

void TupPapagayoApp::onPlay()
{
    if (document && document->getAudioPlayer()) {
        if (playerStopped) {
            playerStopped = false;
            actionPlay->setIcon(pauseIcon);
            actionPlay->setText(tr("Pause"));
            actionPlay->setToolTip(tr("Pause"));
            document->playAudio();
        } else {
            updatePauseButton();
            document->pauseAudio();
        }
    }
}

void TupPapagayoApp::onStop()
{
    if (document && document->getAudioPlayer()) {
        playerStopped = true;
        actionPlay->setIcon(playIcon);
        actionPlay->setText(tr("Play"));
        actionPlay->setToolTip(tr("Play"));
        document->stopAudio();
    }
}

void TupPapagayoApp::onPause()
{
    if (document && document->getAudioPlayer()) {
        playerStopped = true;
        document->pauseAudio();
    }
}

void TupPapagayoApp::onFpsChange(int fps)
{
    if (!document)
        return;

    fps = PG_CLAMP(fps, 1, 120);

    if (fps == document->getFps())
        return;

    defaultFps = fps;
    document->setFps(fps);
    waveformView->setDocument(document);
}

void TupPapagayoApp::onVoiceNameChanged()
{
    if (!document || !document->getCurrentVoice())
        return;

    document->getCurrentVoice()->setName(voiceName->text());
}

void TupPapagayoApp::onVoiceTextChanged()
{
    if (!document || !document->getCurrentVoice())
        return;

    document->getCurrentVoice()->setText(voiceText->toPlainText());
    if (enableAutoBreakdown)
        runBreakdownProcess(); // this is cool, but it could slow things down by doing constant breakdowns
    updateActions();
}

void TupPapagayoApp::runBreakdownProcess()
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupPapagayoApp::runBreakdownProcess()]";
    #endif

    if (!document || !document->getCurrentVoice()) {
        #ifdef TUP_DEBUG
            qDebug() << "[TupPapagayoApp::onBreakdownProcess()] - Error: No lip-sync document loaded!";
        #endif
        return;
    }

    TupLipsyncDoc::loadDictionaries();
    document->setDirtyFlag(true);
    int32 duration = document->getFps() * 10;
    if (document->getAudioExtractor()) {
        real time = document->getAudioExtractor()->duration();
        time *= document->getFps();
        duration = PG_ROUND(time);
    }

    document->runBreakdown("EN", duration);
    waveformView->update();
}

void TupPapagayoApp::keyPressEvent(QKeyEvent *event)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupPapagayoApp::keyPressEvent()]";
    #endif

    if (event->key() == Qt::Key_Space) {
        onPlay();
    } else if (event->key() == Qt::Key_Enter || event->key() == Qt::Key_Return) {
        onStop();
    }
}

void TupPapagayoApp::updateMouthView(int index)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupPapagayoApp::updateMouthView()] - index -> " << index;
    #endif

    if (index == 5) {
        if (mouthFrame->currentIndex() == Predefined)
            mouthFrame->setCurrentIndex(Customized);
    } else {
        mouthView->onMouthChanged(index);
        if (mouthFrame->currentIndex() == Customized)
            mouthFrame->setCurrentIndex(Predefined);
    }
}

void TupPapagayoApp::openImagesDialog()
{
    TCONFIG->beginGroup("General");
    QString path = TCONFIG->value("DefaultPath", QDir::homePath()).toString();
    QString dirPath = QFileDialog::getExistingDirectory(this, tr("Choose the images directory..."), path,
                                                    QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    if (!dirPath.isEmpty()) {
        QDir dir(dirPath);
        QStringList imagesList = dir.entryList(QStringList() << "*.png" << "*.jpg" << "*.jpeg");
        if (imagesList.size() > 0) {
            if (imagesList.count() == 10) { // Mouths set always contains 10 figures
                QString firstImage = imagesList.at(0);
                int dot = firstImage.lastIndexOf(".");
                QString extension = firstImage.mid(dot);
                for (int32 i = 0; i < TupLipsyncDoc::phonemesListSize(); i++) {
                    QString image = TupLipsyncDoc::getPhonemeAt(i) + extension;
                    QString path = dirPath + "/" +  image;
                    if (!QFile::exists(path)) {
                        TOsd::self()->display(TOsd::Error, tr("Mouth image is missing!"));
                        #ifdef TUP_DEBUG
                            qDebug() << "[TupPapagayoApp::openImagesDialog()] - Fatal Error: Image file is missing -> " << path;
                        #endif
                        return;
                    }
                }

                mouthsPath->setText(dirPath);
                saveDefaultPath(dirPath);
                customView->loadImages(dirPath);
            } else {
                TOsd::self()->display(TOsd::Error, tr("Mouth images are incomplete!"));
                #ifdef TUP_DEBUG
                    qDebug() << "[TupPapagayoApp::openImagesDialog()] - Fatal Error: Mouth images are incomplete!";
                #endif
            }
        } else {
            TOsd::self()->display(TOsd::Error, tr("Images directory is empty!"));
            #ifdef TUP_DEBUG
                qDebug() << "[TupPapagayoApp::openImagesDialog()] - Fatal Error: Images directory is empty!";
            #endif
        }
    }
}

void TupPapagayoApp::saveDefaultPath(const QString &dir)
{
    TCONFIG->beginGroup("General");
    TCONFIG->setValue("DefaultPath", dir);
    TCONFIG->sync();
}

void TupPapagayoApp::updateFrame(int frame)
{
    if (mouthFrame->currentIndex() == Predefined)
        mouthView->onFrameChanged(frame);
    else
        customView->onFrameChanged(frame);
}

void TupPapagayoApp::updatePauseButton()
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupPapagayoApp::updatePauseButton()]";
    #endif

    playerStopped = true;
    actionPlay->setIcon(playIcon);
    actionPlay->setText(tr("Play"));
    actionPlay->setToolTip(tr("Play"));
}

void TupPapagayoApp::createLipsyncRecord()
{
    QString title = voiceName->text();
    if (title.isEmpty()) {
        TOsd::self()->display(TOsd::Error, tr("Voice name is empty!"));
        return;
    }

    QString words = voiceText->toPlainText();
    if (words.isEmpty()) {
        TOsd::self()->display(TOsd::Error, tr("Voice text is empty!"));
        return;
    }

    int index = mouthsCombo->currentIndex();
    if (index == 5) {
        QString path = mouthsPath->text();
        if (path.isEmpty()) {
            TOsd::self()->display(TOsd::Error, tr("Customized mouths path is unset!"));
            return;
        }
    }

    close();
}

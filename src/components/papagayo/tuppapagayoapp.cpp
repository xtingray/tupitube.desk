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
#include "tapptheme.h"
#include "tosd.h"
#include "toptionaldialog.h"
#include "tupbreakdowndialog.h"
#include "tuppapagayoimporter.h"

#include "tuprequestbuilder.h"
#include "tuplibrary.h"
#include "talgorithm.h"

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

// Constructor to open new lipsync projects
TupPapagayoApp::TupPapagayoApp(PapagayoAppMode mode, TupProject *project, const QString &soundFile,
                               QList<int> indexes, QWidget *parent) : QMainWindow(parent)
{
    #ifdef TUP_DEBUG
        qDebug() << "---";
        qDebug() << "[TupPapagayoApp::TupPapagayoApp()] - Adding new record...";
    #endif

    this->mode = mode;
    tupProject = project;
    document = new TupLipsyncDoc;
    dictionary = document->getDictionary();
    enableAutoBreakdown = true;
    defaultFps = project->getFPS();
    playerStopped = true;
    saveButtonPressed = false;
    pgoFolderPath = project->getDataDir() + "/pgo/";
    soundFilePath = soundFile;

    sceneIndex= indexes.at(0);
    layerIndex = indexes.at(1);
    frameIndex = indexes.at(2);

    setUICore(soundFilePath);
}

// Constructor to open existing lipsync projects
TupPapagayoApp::TupPapagayoApp(PapagayoAppMode mode, TupProject *project, TupLipSync *lipsync,
                               QList<int> indexes, QWidget *parent) : QMainWindow(parent)
{
    #ifdef TUP_DEBUG
        qDebug() << "---";
        qDebug() << "[TupPapagayoApp::TupPapagayoApp()] - Updating existing record...";
    #endif

    Q_UNUSED(lipsync)

    this->mode = mode;
    tupProject = project;
    document = new TupLipsyncDoc;
    dictionary = document->getDictionary();
    enableAutoBreakdown = true;
    defaultFps = project->getFPS();
    playerStopped = true;
    saveButtonPressed = false;

    oldLipsyncName = lipsync->getLipSyncName();
    currentMouthIndex = lipsync->getMouthIndex();

    mouthType = Predefined;
    if (currentMouthIndex == CUSTOMIZED_MOUTH)
        mouthType = Customized;

    pgoFolderPath = project->getDataDir() + "/pgo/";
    pgoFilePath = pgoFolderPath + oldLipsyncName;

    #ifdef TUP_DEBUG
        qDebug() << "[TupPapagayoApp::TupPapagayoApp()] - pgoFilePath -> " << pgoFilePath;
    #endif

    TupLibrary *library = project->getLibrary();
    if (library) {
        soundKey = lipsync->getSoundFile();
        soundFilePath = library->getObjectPath(soundKey);
    } else {
        #ifdef TUP_DEBUG
            qDebug() << "[TupPapagayoApp::TupPapagayoApp()] - "
                        "Fatal Error: Library pointer is NULL!";
        #endif
    }

    sceneIndex= indexes.at(0);
    layerIndex = indexes.at(1);
    frameIndex = indexes.at(2);

    setUICore(pgoFilePath);
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

void TupPapagayoApp::setUICore(const QString &filePath)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupPapagayoApp::setUICore()] - filePath -> " << filePath;
    #endif

    setStyleSheet(TAppTheme::themeSettings());

    setupActions();
    setupUI();
    setupMenus();
    setAcceptDrops(true);

    updateActions();

    if (!filePath.isEmpty())
        openFile(filePath);
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

    if (mode == Insert) {
        actionOpen = new QAction(this);
        QIcon openIcon;
        openIcon.addFile(THEME_DIR + "icons/open.png", QSize(), QIcon::Normal, QIcon::Off);
        actionOpen->setIcon(openIcon);
        actionOpen->setText(tr("Open"));
        actionOpen->setShortcut(QKeySequence(tr("Ctrl+O")));
        connect(actionOpen, SIGNAL(triggered()), this, SLOT(openFile()));
    }

    actionPlay = new QAction(this);
    playIcon.addFile(THEME_DIR + "icons/play.png", QSize(), QIcon::Normal, QIcon::Off);
    actionPlay->setIcon(playIcon);
    actionPlay->setText(tr("Play"));
    actionPlay->setToolTip(tr("Play"));
    connect(actionPlay, SIGNAL(triggered()), this, SLOT(playVoice()));

    pauseIcon.addFile(THEME_DIR + "icons/pause.png", QSize(), QIcon::Normal, QIcon::Off);

    actionStop = new QAction(this);
    QIcon stopIcon;
    stopIcon.addFile(THEME_DIR + "icons/stop.png", QSize(), QIcon::Normal, QIcon::Off);
    actionStop->setIcon(stopIcon);
    actionStop->setText(tr("Stop"));
    actionStop->setToolTip(tr("Stop"));
    connect(actionStop, SIGNAL(triggered()), this, SLOT(stopVoice()));

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

    mouthView = new TupMouthView(dictionary, mouthFrame);
    viewSizePolicy.setHeightForWidth(mouthView->sizePolicy().hasHeightForWidth());
    mouthView->setSizePolicy(viewSizePolicy);
    mouthView->setMinimumSize(QSize(280, 200));
    mouthView->setMaximumWidth(280);

    connect(waveformView, SIGNAL(frameChanged(int)), this, SLOT(updateFrame(int)));
    connect(mouthsCombo, SIGNAL(activated(int)), this, SLOT(updateMouthView(int)));

    mouthFrame->addWidget(mouthView);

    customView = new TupCustomizedMouthView(dictionary);
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

    currentMouthPath = mouthView->getMouthsPath();
    waveformView->setMouthsPath(currentMouthPath);

    // Lateral Panel
    QGroupBox *lateralGroupBox = new QGroupBox(centralWidget);
    lateralGroupBox->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    lateralGroupBox->setTitle(tr("Voice Settings"));

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

    QSpacerItem *voiceHorizontalSpacer = new QSpacerItem(40, 20,
                                                         QSizePolicy::Expanding, QSizePolicy::Minimum);
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
    languageChoice->addItem(tr("Other Language"));

    TCONFIG->beginGroup("General");
    locale = TCONFIG->value("Language", "en").toString();
    if (locale.compare("en") == 0) {
        currentLanguage = English;
    } else {
        currentLanguage = OtherLang;
        languageChoice->setCurrentIndex(OtherLang);
    }

    connect(languageChoice, SIGNAL(activated(int)), this, SLOT(updateLanguage(int)));

    languageHorizontalLayout->addWidget(languageChoice);

    breakdownButton = new QPushButton(lateralGroupBox);
    breakdownButton->setText(tr("Breakdown"));
    connect(breakdownButton, SIGNAL(clicked()), this, SLOT(runManualBreakdownAction()));
    breakdownButton->setEnabled(false);

    okButton = new QPushButton(lateralGroupBox);
    okButton->setMinimumWidth(60);
    okButton->setIcon(QIcon(THEME_DIR + "icons/apply.png"));
    okButton->setToolTip(tr("Save lip-sync record"));

    if (mode == Update) {
        connect(okButton, SIGNAL(clicked()), this, SLOT(callUpdateProcedure()));
    } else { // Insert | VoiceRecorded | AudioFromLibrary
        connect(okButton, SIGNAL(clicked()), this, SLOT(createLipsyncRecord()));
    }

    QPushButton *cancelButton = new QPushButton(lateralGroupBox);
    cancelButton->setMinimumWidth(60);
    cancelButton->setIcon(QIcon(THEME_DIR + "icons/close.png"));
    cancelButton->setToolTip(tr("Cancel"));
    connect(cancelButton, SIGNAL(clicked()), this, SLOT(closeWindow()));

    languageHorizontalLayout->addWidget(breakdownButton);

    QSpacerItem *languageHorizontalSpacer = new QSpacerItem(40, 20,
                                                QSizePolicy::Expanding, QSizePolicy::Minimum);
    languageHorizontalLayout->addItem(languageHorizontalSpacer);

    languageHorizontalLayout->addWidget(okButton);
    languageHorizontalLayout->addWidget(cancelButton);

    lateralVerticalLayout->addLayout(languageHorizontalLayout);
    innerVerticalLayout->addWidget(lateralGroupBox);
    centralHorizontalLayout->addLayout(innerVerticalLayout);

    setCentralWidget(centralWidget);

    QScreen *screen = QGuiApplication::screens().at(0);
    setMinimumWidth(screen->geometry().width() * 0.7);

    if (mode == Update) {
        if (mouthType == Customized) {
            mouthsCombo->setCurrentIndex(CUSTOMIZED_MOUTH);
            currentMouthPath = PROJECT_DIR + "/images/" + oldLipsyncName + "/";
            mouthsPath->setText(currentMouthPath);
            customView->loadImages(currentMouthPath);
            mouthFrame->setCurrentIndex(Customized);
        } else {
            mouthsCombo->setCurrentIndex(currentMouthIndex);
            mouthFrame->setCurrentIndex(mouthType);
            mouthView->onMouthChanged(currentMouthIndex);
            currentMouthPath = mouthView->getMouthsPath();
        }
        waveformView->setMouthsPath(currentMouthPath);
    }
}

void TupPapagayoApp::setupMenus()
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupPapagayoApp::setupMenus()]";
    #endif

    QMenuBar *menuBar = new QMenuBar(this);

    QMenu *menuFile = new QMenu(menuBar);
    menuFile->setTitle(tr("File"));

    if (mode == Insert)
        menuFile->addAction(actionOpen);
    menuFile->addAction(actionClose);

    menuBar->addAction(menuFile->menuAction());
    setMenuBar(menuBar);

    QToolBar *mainToolBar = new QToolBar(this);
    mainToolBar->setMovable(true);
    mainToolBar->setToolButtonStyle(Qt::ToolButtonIconOnly);
    mainToolBar->setFloatable(false);

    if (mode == Insert)
        mainToolBar->addAction(actionOpen);

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

    QFileInfo info(filePath);
    if (info.suffix().toLower() == "pgo") {
        document->openPGOFile(filePath, soundFilePath, defaultFps);
        voiceName->setText(document->getVoiceName());
        voiceText->blockSignals(true);
        voiceText->setPlainText(document->getVoiceText());
        voiceText->blockSignals(false);
    } else { // Loading sound file
        soundFilePath = filePath;
        document->openAudioFile(soundFilePath);
        document->setModifiedFlag(true);
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
        connect(document->getAudioPlayer(), SIGNAL(positionChanged(qint64)),
                waveformView, SLOT(positionChanged(qint64)));
        connect(document->getAudioPlayer(), SIGNAL(mediaStatusChanged(QMediaPlayer::MediaStatus)),
                waveformView, SLOT(updateMediaStatus(QMediaPlayer::MediaStatus)));

        if (document->getVoice()) {
            voiceName->setText(document->getVoiceName());
            enableAutoBreakdown = false;
            voiceText->setPlainText(document->getVoiceText());
            enableAutoBreakdown = true;
        }

        QString title = tr("Lip-Sync Manager") + " - " + info.fileName();
        if (mode == Update) {
            document->setModifiedFlag(false);
            title = tr("Lip-Sync Manager") + " - " + tr("Updating") + " " + info.fileName();
        }

        setWindowTitle(title);
    }

    updateActions();
}

bool TupPapagayoApp::confirmCloseDocument()
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupPapagayoApp::confirmCloseDocument()]";
    #endif

    if (document && document->isModified()) {
        TOptionalDialog dialog(tr("Do you want to save this lip-sync record?"), tr("Confirmation Required"),
                               false, true, this);
        dialog.setModal(true);
        QScreen *screen = QGuiApplication::screens().at(0);
        dialog.move(static_cast<int> ((screen->geometry().width() - dialog.sizeHint().width()) / 2),
                    static_cast<int> ((screen->geometry().height() - dialog.sizeHint().height()) / 2));
        dialog.exec();

        TOptionalDialog::Result result = dialog.getResult();
        if (result == TOptionalDialog::Accepted) {
            if (mode == Insert) { // Adding new record
                if (validateLipsyncForm()) {
                    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
                    if (saveLipsyncRecord()) {
                        #ifdef TUP_DEBUG
                            qDebug() << "[TupPapagayoApp::confirmCloseDocument()] - Lip-sync item saved successfully!";
                        #endif
                        TOsd::self()->display(TOsd::Info, tr("Lip-sync item added!"));
                        QApplication::restoreOverrideCursor();
                        close();
                    } else {
                        QApplication::restoreOverrideCursor();
                        return false;
                    }
                } else {
                    return false;
                }
            } else { // Updating new record
                if (validateLipsyncForm()) {
                    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
                    if (updateLipsyncRecord()) {
                        #ifdef TUP_DEBUG
                            qDebug() << "[TupPapagayoApp::confirmCloseDocument()] - Lip-sync item updated successfully!";
                        #endif
                        TOsd::self()->display(TOsd::Info, tr("Lip-sync item updated!"));
                        QApplication::restoreOverrideCursor();
                        close();
                    } else {
                        QApplication::restoreOverrideCursor();
                        return false;
                    }
                } else {
                    return false;
                }
            }

            return false;
        }

        if (result == TOptionalDialog::Cancelled)
            return false;

        if (result == TOptionalDialog::Discarded)
            return true;
    }

    return true;
}

void TupPapagayoApp::updateLanguage(int index)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupPapagayoApp::updateLanguage()] - index -> " << index;
    #endif

    if (index == English) {
        currentLanguage = English;
    } else {
        currentLanguage = OtherLang;
        buildOtherLanguagePhonemes();
    }
}

void TupPapagayoApp::closeWindow()
{
    if (confirmCloseDocument()) {
        if (document) {
            delete document;
            document = nullptr;
        }
        close();
    }
}

void TupPapagayoApp::closeEvent(QCloseEvent *event)
{
    if (!saveButtonPressed) {
        if (confirmCloseDocument()) {
            if (document) {
                delete document;
                document = nullptr;
            }
            event->accept();
        } else {
            event->ignore();
        }
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
#ifdef Q_OS_WIN
    if (extension == "wav")
        event->acceptProposedAction();
#else
    if (extension == "mp3" || extension == "wav")
        event->acceptProposedAction();
#endif
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
#ifdef Q_OS_WIN
    if (extension == "wav") {
        event->acceptProposedAction();
        if (confirmCloseDocument())
            openFile(filePath);
    }
#else
    if (extension == "mp3" || extension == "wav") {
        event->acceptProposedAction();
        if (confirmCloseDocument())
            openFile(filePath);
    }
#endif
}

void TupPapagayoApp::updateActions()
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupPapagayoApp::updateActions()]";
    #endif

    bool flag = false;
    if (document)
        flag = true;

    actionPlay->setEnabled(flag);
    actionStop->setEnabled(flag);
    actionZoomIn->setEnabled(flag);
    actionZoomOut->setEnabled(flag);
    actionAutoZoom->setEnabled(flag);

    voiceName->setEnabled(flag);
    voiceText->setEnabled(flag);

    languageChoice->setEnabled(flag);
    mouthsCombo->setEnabled(flag);

    okButton->setEnabled(flag);
}

void TupPapagayoApp::openFile()
{
    if (!confirmCloseDocument())
        return;

    TCONFIG->beginGroup("General");
    QString path = TCONFIG->value("DefaultPath", QDir::homePath()).toString();
#ifdef Q_OS_WIN
    QString filter = tr("Audio files (*.wav)");
#else
    QString filter = tr("Audio files (*.mp3 *.wav)");
#endif
    QString filePath = QFileDialog::getOpenFileName(this, tr("Open"), path, filter);
    if (filePath.isEmpty())
        return;

    openFile(filePath);
}

void TupPapagayoApp::playVoice()
{
    if (currentMouthPath.isEmpty()) {
        #ifdef TUP_DEBUG
            qWarning() << "[TupPapagayoApp::playVoice()] - Fatal Error: Mouth images are unset!";
        #endif
        TOsd::self()->display(TOsd::Error, tr("Mouth images are unset!"));
        return;
    }

    if (voiceText->toPlainText().isEmpty()) {
        #ifdef TUP_DEBUG
            qWarning() << "[TupPapagayoApp::playVoice()] - Warning: No voice text to play!";
        #endif
        TOsd::self()->display(TOsd::Warning, tr("No voice text to play!"));
        // return;
    }

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

void TupPapagayoApp::stopVoice()
{
    if (document && document->getAudioPlayer()) {
        playerStopped = true;
        actionPlay->setIcon(playIcon);
        actionPlay->setText(tr("Play"));
        actionPlay->setToolTip(tr("Play"));
        document->stopAudio();
    }
}

void TupPapagayoApp::pauseVoice()
{
    if (document && document->getAudioPlayer()) {
        playerStopped = true;
        document->pauseAudio();
    }
}

void TupPapagayoApp::onVoiceNameChanged()
{
    if (!document || !document->getVoice())
        return;

    document->setVoiceName(voiceName->text());
}

void TupPapagayoApp::loadWordsFromDocument()
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupPapagayoApp::loadWordsFromDocument()]";
    #endif

    wordsList.clear();
    phonemesList.clear();
    QList<LipsyncWord *> words = document->getWords();
    if (!words.isEmpty()) {
        foreach(LipsyncWord *word, words) {
            wordsList << word->getText();
            phonemesList << word->getPhonemesString();
        }
    }
}

void TupPapagayoApp::buildOtherLanguagePhonemes()
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupPapagayoApp::buildOtherLanguagePhonemes()]";
    #endif

    QString newText = voiceText->toPlainText();
    if (newText.isEmpty()) {
        #ifdef TUP_DEBUG
            qDebug() << "[TupPapagayoApp::buildOtherLanguagePhonemes()] - Warning: New voice text is empty!";
        #endif
        return;
    }

    QString currentText = document->getVoiceText();
    phonemesList.clear();

    if (currentText.isEmpty()) {
        #ifdef TUP_DEBUG
            qDebug() << "[TupPapagayoApp::buildOtherLanguagePhonemes()] - Current voice text is empty!";
        #endif
        wordsList = newText.split(" ");
        for(int i=0; i<wordsList.size(); i++)
            phonemesList << "";

        waveformView->update();
    } else {
        if (newText.compare(currentText) != 0) {
            #ifdef TUP_DEBUG
                qDebug() << "[TupPapagayoApp::buildOtherLanguagePhonemes()] - New voice text -> " << newText;
            #endif
            wordsList = newText.split(" ");
            QList<LipsyncWord *> oldWords = document->getWords();
            foreach(QString word, wordsList) {
                if (currentText.contains(word)) {
                    foreach(LipsyncWord *oldWord, oldWords) {
                        QString text = oldWord->getText();
                        if (text.compare(word) == 0)
                            phonemesList << oldWord->getPhonemesString();
                    }
                } else {
                    phonemesList << "";
                }
            }
            waveformView->update();
        } else { // Previous text
            #ifdef TUP_DEBUG
                qDebug() << "[TupPapagayoApp::buildOtherLanguagePhonemes()] - Loading previous voice text...";
            #endif
            loadWordsFromDocument();
        }
    }
}

void TupPapagayoApp::onVoiceTextChanged()
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupPapagayoApp::onVoiceTextChanged()] - currentLanguage -> " << currentLanguage;
        qDebug() << "[TupPapagayoApp::onVoiceTextChanged()] - Locale -> " << locale;
    #endif

    if (!document) {
        #ifdef TUP_DEBUG
            qDebug() << "[TupPapagayoApp::onVoiceTextChanged()] - Warning: Document is NULL!";
        #endif
        return;
    }

    if (!document->getVoice()) {
        #ifdef TUP_DEBUG
            qDebug() << "[TupPapagayoApp::onVoiceTextChanged()] - Warning: Voice is null!";
        #endif
        return;
    }

    QString text = voiceText->toPlainText();
    if (text.isEmpty()) { // voice text is empty
        if (breakdownButton->isEnabled())
            breakdownButton->setEnabled(false);

        wordsList.clear();
        phonemesList.clear();
        document->setVoiceText("");
        waveformView->update();

        #ifdef TUP_DEBUG
            qDebug() << "[TupPapagayoApp::onVoiceTextChanged()] - Warning: Voice text is empty!";
        #endif
        return;
    }

    if (!breakdownButton->isEnabled())
        breakdownButton->setEnabled(true);

    document->setVoiceText(text);
    if (enableAutoBreakdown)
        runBreakdownAction(); // this is cool, but it could slow things down by doing constant breakdowns

    loadWordsFromDocument();
    updateActions();
}

int32 TupPapagayoApp::calculateDuration()
{
    int32 duration = document->getFps() * MOUTHS_PACKAGE_SIZE;
    if (document->getAudioExtractor()) {
        real time = document->getAudioExtractor()->duration();
        time *= document->getFps();
        duration = PG_ROUND(time);
    }

    return duration;
}

void TupPapagayoApp::runBreakdownAction()
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupPapagayoApp::runBreakdownAction()]";
    #endif

    if (!document || !document->getVoice()) {
        #ifdef TUP_DEBUG
            qDebug() << "[TupPapagayoApp::runBreakdownAction()] - "
                        "Error: No lip-sync document loaded!";
        #endif
        return;
    }

    document->setModifiedFlag(true);
    document->runBreakdown(locale, calculateDuration());
    waveformView->update();
}

void TupPapagayoApp::loadDocumentFromScratch(QStringList phonemes)
{
    int32 duration = calculateDuration();
    LipsyncVoice *voice = new LipsyncVoice(voiceName->text());
    LipsyncPhrase *phrase =  new LipsyncPhrase;
    phrase->setText(voiceText->toPlainText());
    phrase->setStartFrame(0);
    phrase->setEndFrame(duration);

    int wordLength = duration / wordsList.size();
    int wordPos = 0;
    for (int i=0; i<wordsList.size(); i++) {
        LipsyncWord *word = new LipsyncWord;
        word->setText(wordsList.at(i));
        word->setStartFrame(wordPos);
        word->setEndFrame(wordPos + wordLength);
        QString phoneme = phonemes.at(i);
        QStringList phonemeParts = phoneme.split(" ");
        int32 phonemeLength = wordLength / phonemeParts.size();
        int32 phonemePos = wordPos;
        for (int j=0; j<phonemeParts.size(); j++) {
            LipsyncPhoneme *ph = new LipsyncPhoneme;
            ph->setText(phonemeParts.at(j));
            ph->setFrame(phonemePos);
            phonemePos += phonemeLength + 1;
            word->addPhoneme(ph);
        }
        phrase->addWord(word);
        wordPos += wordLength + 1;
    }
    voice->setPhrase(phrase);
    document->setVoice(voice);
    document->setVoiceText(voiceText->toPlainText());
}

void TupPapagayoApp::runManualBreakdownAction()
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupPapagayoApp::runManualBreakdownProcess()] - "
                    "currentLanguage -> " << currentLanguage;
        qDebug() << "[TupPapagayoApp::runManualBreakdownProcess()] - "
                    "currentMouthPath -> " << currentMouthPath;
    #endif

    if (wordsList.isEmpty()) {
        phonemesList.clear();
        document->clearVoice();

        waveformView->update();
        #ifdef TUP_DEBUG
            qDebug() << "[TupPapagayoApp::runManualBreakdownProcess()] - "
                        "Fatal Error: Voice text is empty!";
        #endif
        TOsd::self()->display(TOsd::Error, tr("Voice text is empty!"));
        return;
    }

    if (currentLanguage == English) {
        runBreakdownAction();
    } else {
        #ifdef TUP_DEBUG
            qDebug() << "[TupPapagayoApp::runManualBreakdownProcess()] - Calling breakdown dialog...";
            qDebug() << "[TupPapagayoApp::runManualBreakdownProcess()] - wordsList -> " << wordsList;
            qDebug() << "[TupPapagayoApp::runManualBreakdownProcess()] - phonemesList -> " << phonemesList;
        #endif

        openBreakdownDialog(0);
    }
}

void TupPapagayoApp::openBreakdownDialog(int wordIndex)
{
    TupBreakdownDialog *breakdownDialog = new TupBreakdownDialog(wordIndex, wordsList, phonemesList,
                                                                 currentMouthPath, this);
    if (breakdownDialog->exec() == QDialog::Accepted) {
        document->setModifiedFlag(true);

        if (document->voiceTextIsEmpty() == 0) {
            loadDocumentFromScratch(breakdownDialog->phomeneList());
        } else {
            QString newText = voiceText->toPlainText();
            QString currentText = document->getVoiceText();

            if (newText.compare(currentText) != 0) {
                loadDocumentFromScratch(breakdownDialog->phomeneList());
            } else {
                LipsyncPhrase *phrase = document->getPhrase();
                if (phrase) {
                    QList<LipsyncWord *> words = phrase->getWords();
                    QStringList phonemes = breakdownDialog->phomeneList();

                    for (int i = 0; i < words.size(); i++) {
                        LipsyncWord *word = words.at(i);
                        if (word) {
                            QStringList phList = phonemes.at(i).split(' ', Qt::SkipEmptyParts);
                            int32 wordLength = word->getEndFrame() - word->getStartFrame();
                            int32 wordPos = word->getStartFrame();
                            int32 phonemePos = wordPos;
                            int32 phonemeLength = wordLength / phList.size();
                            word->clearPhonemes();

                            for (int i = 0; i < phList.size(); i++) {
                                QString phStr = phList.at(i);
                                if (phStr.isEmpty())
                                    continue;

                                LipsyncPhoneme *phoneme = new LipsyncPhoneme;
                                phoneme->setText(phStr);
                                phoneme->setFrame(phonemePos);
                                phonemePos += phonemeLength + 1;
                                word->addPhoneme(phoneme);
                            }
                        }
                    }
                }
            }
        }
        waveformView->update();
    }
}

void TupPapagayoApp::keyPressEvent(QKeyEvent *event)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupPapagayoApp::keyPressEvent()] - key -> " << event->text();
    #endif

    if (event->key() == Qt::Key_Space) {
        playVoice();
    } else if (event->key() == Qt::Key_Enter || event->key() == Qt::Key_Return) {
        stopVoice();
    }
}

void TupPapagayoApp::updateMouthView(int index)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupPapagayoApp::updateMouthView()] - "
                    "index -> " << index;
        qDebug() << "[TupPapagayoApp::updateMouthView()] - "
                    "currentMouthPath -> " << currentMouthPath;
    #endif

    currentMouthIndex = index;
    if (index == CUSTOMIZED_MOUTH) {
        if (mouthFrame->currentIndex() == Predefined)
            mouthFrame->setCurrentIndex(Customized);

        if (customView->imagesAresLoaded())
            currentMouthPath = customView->getMouthsPath();
        else
            currentMouthPath = "";

        waveformView->setMouthsPath(currentMouthPath);
    } else {
        mouthView->onMouthChanged(index);
        if (mouthFrame->currentIndex() == Customized)
            mouthFrame->setCurrentIndex(Predefined);

        currentMouthPath = mouthView->getMouthsPath();
        waveformView->setMouthsPath(currentMouthPath);
    }
}

void TupPapagayoApp::openImagesDialog()
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupPapagayoApp::openImagesDialog()]";
    #endif

    TCONFIG->beginGroup("General");
    QString path = TCONFIG->value("DefaultPath", QDir::homePath()).toString();
    QString dirPath = QFileDialog::getExistingDirectory(this, tr("Choose the images directory..."), path,
                                                        QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    if (!dirPath.isEmpty()) {
        QDir dir(dirPath);
        QStringList imagesList = dir.entryList(QStringList() << "*.png" << "*.jpg" << "*.jpeg");
        if (imagesList.size() > 0) {
            if (imagesList.count() == MOUTHS_PACKAGE_SIZE) { // Mouths set always contains 10 figures
                QString firstImage = imagesList.at(0);
                int dot = firstImage.lastIndexOf(".");
                QString extension = firstImage.mid(dot);
                for (int32 i = 0; i < dictionary->phonemesListSize(); i++) {
                    QString image = dictionary->getPhonemeAt(i) + extension;
                    QString path = dirPath + "/" + image;
                    if (!QFile::exists(path)) {
                        TOsd::self()->display(TOsd::Error, tr("Mouth image is missing!"));
                        #ifdef TUP_DEBUG
                            qDebug() << "[TupPapagayoApp::openImagesDialog()] - "
                                        "Fatal Error: Image file is missing -> " << path;
                        #endif
                        return;
                    }
                }

                currentMouthPath = dirPath + "/";
                mouthsPath->setText(currentMouthPath);
                saveDefaultPath(currentMouthPath);
                customView->loadImages(currentMouthPath);
                waveformView->setMouthsPath(currentMouthPath);
            } else {
                TOsd::self()->display(TOsd::Error, tr("Mouth images are incomplete!"));
                #ifdef TUP_DEBUG
                    qDebug() << "[TupPapagayoApp::openImagesDialog()] - "
                                "Fatal Error: Mouth images are incomplete!";
                #endif
            }
        } else {
            TOsd::self()->display(TOsd::Error, tr("Images directory is empty!"));
            #ifdef TUP_DEBUG
                qDebug() << "[TupPapagayoApp::openImagesDialog()] - "
                            "Fatal Error: Images directory is empty! -> " << dirPath;
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

bool TupPapagayoApp::validateLipsyncForm()
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupPapagayoApp::validateLipsyncForm()]";
    #endif

    QString title = voiceName->text();
    if (title.isEmpty()) {
        TOsd::self()->display(TOsd::Error, tr("Voice name is empty!"));
        return false;
    }

    QString words = voiceText->toPlainText();
    if (words.isEmpty()) {
        TOsd::self()->display(TOsd::Error, tr("Voice text is empty!"));
        return false;
    }

    int index = mouthsCombo->currentIndex();
    if (index == 5) {
        QString path = mouthsPath->text();
        if (path.isEmpty()) {
            TOsd::self()->display(TOsd::Error, tr("Customized mouths path is unset!"));
            return false;
        }
    }

    if (!document) {
        #ifdef TUP_DEBUG
            qDebug() << "[TupPapagayoApp::validateLipsyncForm()] - "
                        "Warning: No lip-sync document!";
        #endif
        return false;
    }

    loadWordsFromDocument();
    #ifdef TUP_DEBUG
        qDebug() << "[TupPapagayoApp::validateLipsyncForm()] - "
                    "wordsList -> " << wordsList;
        qDebug() << "[TupPapagayoApp::validateLipsyncForm()] - "
                    "phonemesList -> " << phonemesList;
    #endif

    int i = 0;
    foreach (QString phonemes, phonemesList) {
        if (phonemes.isEmpty()) {
            QApplication::restoreOverrideCursor();
            TOsd::self()->display(TOsd::Warning, tr("Some phonemes are missing!"));
            openBreakdownDialog(i);
            return false;
        }
        i++;
    }

    return true;
}

void TupPapagayoApp::createLipsyncRecord()
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupPapagayoApp::createLipsyncRecord()]";
    #endif

    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

    if (validateLipsyncForm()) {
        saveButtonPressed = true;
        if (saveLipsyncRecord()) {
            #ifdef TUP_DEBUG
                qDebug() << "[TupPapagayoApp::createLipsyncRecord()] - "
                            "Lip-sync item saved successfully!";
            #endif
            TOsd::self()->display(TOsd::Info, tr("Lip-sync item added!"));
            QApplication::restoreOverrideCursor();
            close();
        } else {
            #ifdef TUP_DEBUG
                qDebug() << "[TupPapagayoApp::createLipsyncRecord()] - "
                            "Warning: Save procedure has failed!";
            #endif
        }
    } else {
        saveButtonPressed = false;
    }

    QApplication::restoreOverrideCursor();
}

bool TupPapagayoApp::saveLipsyncRecord()
{
    #ifdef TUP_DEBUG
        qDebug() << "---";
        qDebug() << "[TupPapagayoApp::saveLipsyncRecord()] - mode -> " << mode;
    #endif

    if (!QDir(pgoFolderPath).exists()) {
        if (!QDir().mkpath(pgoFolderPath)) {
            #ifdef TUP_DEBUG
                qDebug() << "[TupPapagayoApp::saveLipsyncRecord()] - "
                            "Fatal Error: Can't create folder -> " << pgoFolderPath;
            #endif
            TOsd::self()->display(TOsd::Error, tr("Error while saving lip-sync!"));

            return false;
        }
    }

    QString folderName = QString(voiceName->text() + ".pgo").toLower();
    pgoFilePath = pgoFolderPath + folderName;

    #ifdef TUP_DEBUG
        qDebug() << "[TupPapagayoApp::saveLipsyncRecord()] - pgoFilePath -> " << pgoFilePath;
    #endif

    document->setPGOFilePath(pgoFilePath);
    if (document->save()) {
        #ifdef TUP_DEBUG
            qDebug() << "[TupPapagayoApp::saveLipsyncRecord()] - "
                        "Mouth images path -> " << currentMouthPath;
        #endif

        QFile projectFile(pgoFilePath);
        if (projectFile.exists()) {
            if (projectFile.size() > 0) {
                QDir dir(currentMouthPath);
                QStringList imagesList = dir.entryList(QStringList() << "*.png" << "*.jpg" << "*.jpeg");
                if (imagesList.size() > 0) {
                    if (imagesList.count() == MOUTHS_PACKAGE_SIZE) { // Mouths set always contains 10 figures
                        QString firstImage = imagesList.at(0);
                        int dot = firstImage.lastIndexOf(".");
                        QString extension = firstImage.mid(dot);

                        QSize projectSize = tupProject->getDimension();
                        QPointF projectCenter = QPointF(projectSize.width()/2, projectSize.height()/2);
                        QPixmap pixmap(currentMouthPath + firstImage);
                        QPointF mouthPos = projectCenter - QPointF(pixmap.size().width()/2, pixmap.size().height()/2);

                        TupPapagayoImporter *parser = new TupPapagayoImporter(pgoFilePath, mouthPos, extension, frameIndex);
                        if (parser->fileIsValid()) {
                            // Creating Papagayo folder in the library
                            TupProjectRequest request = TupRequestBuilder::createLibraryRequest(TupProjectRequest::Add, folderName,
                                                                                                TupLibraryObject::Folder);
                            emit requestTriggered(&request);

                            TupLibraryObject::ObjectType type = TupLibraryObject::Image;

                            // Adding mouth images in the library
                            foreach (QString fileName, imagesList) {
                                QString key = fileName.toLower();
                                QString imagePath = currentMouthPath + fileName;
                                QFile imageFile(imagePath);

                                /*
                                #ifdef TUP_DEBUG
                                    qDebug() << "[TupPapagayoApp::saveLipsyncRecord()] - mouth image -> " << fileName;
                                    qDebug() << "[TupPapagayoApp::saveLipsyncRecord()] - key -> " << key;
                                    qDebug() << "[TupPapagayoApp::saveLipsyncRecord()] - mouthPath -> " << currentMouthPath;
                                    qDebug() << "[TupPapagayoApp::saveLipsyncRecord()] - imagePath -> " << imagePath;
                                #endif
                                */

                                // Importing image into Library
                                if (imageFile.open(QIODevice::ReadOnly)) {
                                    QByteArray data = imageFile.readAll();
                                    imageFile.close();

                                    request = TupRequestBuilder::createLibraryRequest(TupProjectRequest::Add, key, type, TupProject::FRAMES_MODE, data, folderName,
                                                                                      sceneIndex, layerIndex, frameIndex);
                                    emit requestTriggered(&request);
                                } else {
                                    #ifdef TUP_DEBUG
                                        qDebug() << "[TupPapagayoApp::saveLipsyncRecord()] - "
                                                    "Fatal Error: Can't open image file -> " << imagePath;
                                    #endif
                                    TOsd::self()->display(TOsd::Error, tr("Can't load mouth image!"));

                                    return false;
                                }
                            }

                            // Adding lip-sync sound file
                            #ifdef TUP_DEBUG
                                qDebug() << "[TupPapagayoApp::saveLipsyncRecord()] - "
                                            "Processing audio file -> " << soundFilePath;
                            #endif

                            document->resetDocument();

                            QFile soundFile(soundFilePath);
                            QFileInfo info(soundFilePath);
                            QString soundKey = info.fileName().toLower();
                            if (soundFile.open(QIODevice::ReadOnly)) {
                                QByteArray data = soundFile.readAll();
                                soundFile.close();
                                if (mode == AudioFromLibrary) {
                                    emit soundRemoved(PapagayoApp, soundKey); // Releasing audio from library and player
                                    #ifdef TUP_DEBUG
                                        qDebug() << "[TupPapagayoApp::saveLipsyncRecord()] - "
                                                    "Removing existing audio record from library -> " << soundKey;
                                    #endif
                                    TupProjectRequest request = TupRequestBuilder::createLibraryRequest(TupProjectRequest::Remove,
                                                                                                        soundKey, TupLibraryObject::Audio);
                                    emit requestTriggered(&request);
                                } else {
                                    if (mode == VoiceRecorded || mode == Update) {
                                        if (!QFile::remove(soundFilePath)) {
                                            #ifdef TUP_DEBUG
                                                qDebug() << "[TupPapagayoApp::saveLipsyncRecord()] - "
                                                            "Fatal Error: Can't remove sound file -> " << soundFilePath;
                                            #endif
                                            TOsd::self()->display(TOsd::Error, tr("Can't remove temporary voice sound!"));

                                            return false;
                                        } else {
                                            #ifdef TUP_DEBUG
                                                qDebug() << "[TupPapagayoApp::saveLipsyncRecord()] - "
                                                            "Removing temp sound file successfully -> " << soundFilePath;
                                            #endif
                                        }
                                    }
                                }

                                #ifdef TUP_DEBUG
                                    qDebug() << "[TupPapagayoApp::saveLipsyncRecord()] - "
                                                "Adding sound file into library again -> " << soundKey << " | folder -> " << folderName;
                                #endif

                                request = TupRequestBuilder::createLibraryRequest(TupProjectRequest::Add, soundKey, TupLibraryObject::Audio, TupProject::FRAMES_MODE,
                                                                                  data, folderName, sceneIndex, layerIndex, frameIndex);
                                emit requestTriggered(&request);
                            } else {
                                #ifdef TUP_DEBUG
                                    qDebug() << "[TupPapagayoApp::saveLipsyncRecord()] - "
                                                "Fatal Error: Can't open sound file -> " << soundFilePath;
                                #endif
                                TOsd::self()->display(TOsd::Error, tr("Can't load voice sound!"));

                                return false;
                            }

                            // Adding Papagayo project
                            currentMouthIndex = mouthsCombo->currentIndex();
                            parser->setMouthIndex(currentMouthIndex);
                            parser->setSoundFile(soundKey);
                            QString xml = parser->toString();

                            request = TupRequestBuilder::createLayerRequest(sceneIndex, layerIndex,
                                                                            TupProjectRequest::AddLipSync, xml);
                            emit requestTriggered(&request);

                            // Adding frames if they are required
                            TupScene *scene = tupProject->sceneAt(sceneIndex);
                            if (scene) {
                                int sceneFrames = scene->framesCount();
                                int lipSyncFrames = frameIndex + parser->getFrameCount();

                                #ifdef TUP_DEBUG
                                    qDebug() << "[TupPapagayoApp::saveLipsyncRecord()] - scenesFrames -> " << sceneFrames;
                                    qDebug() << "[TupPapagayoApp::saveLipsyncRecord()] - lipSyncFrames -> " << lipSyncFrames;
                                #endif

                                if (lipSyncFrames > sceneFrames) {
                                    int layersCount = scene->layersCount();
                                    for (int i = sceneFrames; i < lipSyncFrames; i++) {
                                         for (int j = 0; j < layersCount; j++) {
                                              request = TupRequestBuilder::createFrameRequest(sceneIndex, j, i, TupProjectRequest::Add, tr("Frame"));
                                              emit requestTriggered(&request);
                                         }
                                    }

                                    QString selection = QString::number(layerIndex) + "," + QString::number(layerIndex) + ","
                                                        + QString::number(frameIndex) + "," + QString::number(frameIndex);

                                    request = TupRequestBuilder::createFrameRequest(sceneIndex, layerIndex, frameIndex, TupProjectRequest::Select, selection);
                                    emit requestTriggered(&request);
                                }
                            }

                            TOsd::self()->display(TOsd::Info, tr("Papagayo file has been imported successfully"));
                        } else {
                            TOsd::self()->display(TOsd::Error, tr("Papagayo file is invalid!"));
                            #ifdef TUP_DEBUG
                                qDebug() << "[TupPapagayoApp::saveLipsyncRecord()] - Fatal Error: Papagayo file is invalid!";
                            #endif

                            return false;
                        }
                    } else {
                        TOsd::self()->display(TOsd::Error, tr("Mouth images are incomplete!"));
                        #ifdef TUP_DEBUG
                            qDebug() << "[TupPapagayoApp::saveLipsyncRecord()] - "
                                        "Fatal Error: Mouth images are incomplete!";
                        #endif

                        return false;
                    }
                } else {
                    TOsd::self()->display(TOsd::Error, tr("Images directory is empty!"));
                    #ifdef TUP_DEBUG
                        qDebug() << "[TupPapagayoApp::saveLipsyncRecord()] - "
                                    "Fatal Error: Images directory is empty! -> " << currentMouthPath;
                    #endif

                    return false;
                }
            } else {
                TOsd::self()->display(TOsd::Error, tr("Papagayo project is invalid!"));
                #ifdef TUP_DEBUG
                    qDebug() << "[TupPapagayoApp::saveLipsyncRecord()] - "
                                "Fatal Error: Papagayo file is invalid!";
                #endif

                return false;
            }
        } else {
            TOsd::self()->display(TOsd::Error, tr("Papagayo project is invalid!"));
            #ifdef TUP_DEBUG
                qDebug() << "[TupPapagayoApp::saveLipsyncRecord()] - "
                            "Fatal Error: Papagayo file doesn't exist!";
            #endif

            return false;
        }
    } else {
        #ifdef TUP_DEBUG
            qDebug() << "[TupPapagayoApp::saveLipsyncRecord()] - "
                        "Fatal Error: Can't save lip-sync record!";
        #endif
        TOsd::self()->display(TOsd::Error, tr("Error while saving lip-sync record!"));

        return false;
    }

    return true;
}

bool TupPapagayoApp::updateLipsyncRecord()
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupPapagayoApp::updateLipsyncRecord()]";
    #endif

    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

    tempMouthPath = CACHE_DIR + TAlgorithm::randomString(5);
    QDir tempDir(tempMouthPath);
    if (tempDir.exists()) {
        if (!tempDir.removeRecursively()) {
            #ifdef TUP_DEBUG
                qWarning() << "[TupPapagayoApp::updateLipsyncRecord()] - "
                            "Fatal Error: Can't remove temp folder -> " << tempMouthPath;
            #endif
            return false;
        }
    }

    if (!tempDir.mkdir(tempMouthPath)) {
        #ifdef TUP_DEBUG
            qWarning() << "[TupPapagayoApp::updateLipsyncRecord()] - "
                        "Fatal Error: Can't create temp folder -> " << tempMouthPath;
        #endif
        return false;
    }

    if (!TAlgorithm::copyFolder(currentMouthPath, tempMouthPath)) {
        #ifdef TUP_DEBUG
            qWarning() << "[TupPapagayoApp::updateLipsyncRecord()] - "
                        "Fatal Error: Can't copy mouths folder -> " << tempMouthPath;
        #endif
        return false;
    }
    currentMouthPath = tempMouthPath + "/";

    tempSoundFile = CACHE_DIR + soundKey;
    if (QFile::exists(tempSoundFile)) {
        if (QFile::remove(tempSoundFile)) {
            #ifdef TUP_DEBUG
                qDebug() << "[TupPapagayoApp::updateLipsyncRecord()] - "
                            "Success. Temp sound file removed! -> " << tempSoundFile;
            #endif
        } else {
            #ifdef TUP_DEBUG
                qWarning() << "[TupPapagayoApp::updateLipsyncRecord()] - "
                            "Fatal Error: Can't remove previous temp sound file -> " << tempSoundFile;
            #endif
            return false;
        }
    }

    if (QFile::copy(soundFilePath, tempSoundFile)) {
        #ifdef TUP_DEBUG
            qDebug() << "[TupPapagayoApp::updateLipsyncRecord()] - "
                        "Success copy! Source -> " << soundFilePath << " - Destination -> " << tempSoundFile;
        #endif
    } else {
        #ifdef TUP_DEBUG
            qWarning() << "[TupPapagayoApp::updateLipsyncRecord()] - "
                        "Fatal Error: Can't store temp sound file -> " << soundFilePath;
        #endif
        return false;
    }

    #ifdef TUP_DEBUG
        qDebug() << "[TupPapagayoApp::updateLipsyncRecord()] - "
                    "Removing lipsync item -> " << oldLipsyncName;
    #endif

    #ifdef TUP_DEBUG
        qDebug() << "[TupPapagayoApp::updateLipsyncRecord()] - Releasing audio object -> " << soundKey;
    #endif
    emit soundRemoved(PapagayoApp, soundKey); // Releasing audio from library and player

    // Removing lip sync record
    TupProjectRequest request = TupRequestBuilder::createLayerRequest(sceneIndex, layerIndex,
                                                                      TupProjectRequest::RemoveLipSync,
                                                                      oldLipsyncName);
    emit requestTriggered(&request);                

    #ifdef TUP_DEBUG
        qDebug() << "[TupPapagayoApp::updateLipsyncRecord()] - "
                    "Removing lipsync folder -> " << oldLipsyncName;
    #endif

    // Removing lip sync folder
    request = TupRequestBuilder::createLibraryRequest(TupProjectRequest::Remove,
                                                      oldLipsyncName, TupLibraryObject::Folder);
    emit requestTriggered(&request);

    soundFilePath = tempSoundFile;

    return saveLipsyncRecord();
}

void TupPapagayoApp::callUpdateProcedure()
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupPapagayoApp::callUpdateProcedure()]";
    #endif

    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

    if (validateLipsyncForm()) {
        saveButtonPressed = true;
        if (updateLipsyncRecord()) {
            // Remove tmp mouth folder
            QDir tempDir(tempMouthPath);
            if (tempDir.exists()) {
                if (!tempDir.removeRecursively()) {
                    #ifdef TUP_DEBUG
                        qDebug() << "[TupPapagayoApp::callUpdateProcedured()] - "
                                    "Fatal Error: Can't remove temp folder -> " << tempMouthPath;
                    #endif
                    return;
                } else {
                    #ifdef TUP_DEBUG
                        qDebug() << "[TupPapagayoApp::callUpdateProcedure()] - "
                                    "Mouth folder removed successfully -> " << tempMouthPath;
                    #endif
                }
            }

            #ifdef TUP_DEBUG
                qDebug() << "[TupPapagayoApp::callUpdateRecord()] - Lip-sync item saved successfully!";
            #endif
            TOsd::self()->display(TOsd::Info, tr("Lip-sync item updated!"));
            QApplication::restoreOverrideCursor();
            close();
        } else {
            #ifdef TUP_DEBUG
                qDebug() << "[TupPapagayoApp::callUpdateRecord()] - Warning: Save procedure has failed!";
            #endif
            TOsd::self()->display(TOsd::Error, tr("Error while updating lip-sync!"));
        }
    } else {
        saveButtonPressed = false;
    }

    QApplication::restoreOverrideCursor();
}

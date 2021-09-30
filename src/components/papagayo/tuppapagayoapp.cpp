#include "tuppapagayoapp.h"
#include "tapplicationproperties.h"

#include <QAction>
#include <QToolBar>
#include <QMenu>
#include <QMenuBar>
#include <QStatusBar>
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

TupPapagayoApp::TupPapagayoApp(QWidget *parent) : QMainWindow(parent)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupPapagayoApp::TupPapagayoApp()]";
    #endif

    QFile file(THEME_DIR + "config/ui.qss");
    if (file.exists()) {
        file.open(QFile::ReadOnly);
        QString uiStyleSheet = QLatin1String(file.readAll());
        if (uiStyleSheet.length() > 0)
            setStyleSheet(uiStyleSheet);
        file.close();
    } else {
        #ifdef TUP_DEBUG
            qWarning() << "[TupPapagayoApp()] - Error: Theme file doesn't exist -> " << QString(THEME_DIR + "config/ui.qss");
        #endif
    }

    document = nullptr;
    enableAutoBreakdown = true;
    defaultFps = 24;
    playerStopped = true;

    setupActions();
    setupUI();
    setupConnections();
    setupMenus();
    setAcceptDrops(true);

    restoreSettings();
    updateActions();
}

TupPapagayoApp::~TupPapagayoApp()
{
    if (document)
        delete document;
}

void TupPapagayoApp::setupActions()
{
    // Actions
    actionClose = new QAction(this);
    actionClose->setObjectName(QString::fromUtf8("actionClose"));
    QIcon closeIcon;
    closeIcon.addFile(QString::fromUtf8(":/images/images/close.png"), QSize(), QIcon::Normal, QIcon::Off);
    actionClose->setIcon(closeIcon);
    actionClose->setText(tr("Close"));
    actionClose->setShortcut(Qt::Key_Escape);

    actionOpen = new QAction(this);
    actionOpen->setObjectName(QString::fromUtf8("actionOpen"));
    QIcon openIcon;
    openIcon.addFile(QString::fromUtf8(":/images/images/open.png"), QSize(), QIcon::Normal, QIcon::Off);
    actionOpen->setIcon(openIcon);
    actionOpen->setText(tr("Open"));
    actionOpen->setShortcut(QKeySequence(tr("Ctrl+O")));

    actionSave = new QAction(this);
    actionSave->setObjectName(QString::fromUtf8("actionSave"));
    QIcon saveIcon;
    saveIcon.addFile(QString::fromUtf8(":/images/images/save.png"), QSize(), QIcon::Normal, QIcon::Off);
    actionSave->setIcon(saveIcon);
    actionSave->setText(tr("Save"));
    actionSave->setShortcut(QKeySequence(tr("Ctrl+S")));

    actionSaveAs = new QAction(this);
    actionSaveAs->setObjectName(QString::fromUtf8("actionSave_As"));
    actionSaveAs->setText(tr("Save As"));

    actionPlay = new QAction(this);
    actionPlay->setObjectName(QString::fromUtf8("actionPlay"));
    QIcon playIcon;
    playIcon.addFile(QString::fromUtf8(":/images/images/play.png"), QSize(), QIcon::Normal, QIcon::Off);
    actionPlay->setIcon(playIcon);
    actionPlay->setText(tr("Play"));
    actionPlay->setToolTip(tr("Play"));

    actionStop = new QAction(this);

    actionStop->setObjectName(QString::fromUtf8("actionStop"));
    QIcon stopIcon;
    stopIcon.addFile(QString::fromUtf8(":/images/images/stop.png"), QSize(), QIcon::Normal, QIcon::Off);
    actionStop->setIcon(stopIcon);
    actionStop->setText(tr("Stop"));
    actionStop->setToolTip(tr("Stop"));

    actionZoomIn = new QAction(this);
    actionZoomIn->setObjectName(QString::fromUtf8("actionZoomIn"));
    QIcon zoomInIcon;
    zoomInIcon.addFile(QString::fromUtf8(":/images/images/zoom_in.png"), QSize(), QIcon::Normal, QIcon::Off);
    actionZoomIn->setIcon(zoomInIcon);
    actionZoomIn->setText(tr("Zoom In"));
    actionZoomIn->setToolTip(tr("Zoom In"));

    actionZoomOut = new QAction(this);
    actionZoomOut->setObjectName(QString::fromUtf8("actionZoomOut"));
    QIcon zoomOutIcon;
    zoomOutIcon.addFile(QString::fromUtf8(":/images/images/zoom_out.png"), QSize(), QIcon::Normal, QIcon::Off);
    actionZoomOut->setIcon(zoomOutIcon);
    actionZoomOut->setText(tr("Zoom Out"));
    actionZoomOut->setToolTip(tr("Zoom Out"));

    actionAutoZoom = new QAction(this);
    actionAutoZoom->setObjectName(QString::fromUtf8("actionAutoZoom"));
    QIcon autoZoomIcon;
    autoZoomIcon.addFile(QString::fromUtf8(":/images/images/zoom_1.png"), QSize(), QIcon::Normal, QIcon::Off);
    actionAutoZoom->setIcon(autoZoomIcon);
    actionAutoZoom->setText(tr("Auto Zoom"));
    actionAutoZoom->setToolTip(tr("Auto Zoom"));

    actionUndo = new QAction(this);
    actionUndo->setObjectName(QString::fromUtf8("actionUndo"));
    actionUndo->setText(tr("Undo"));
    actionUndo->setShortcut(QKeySequence("Ctrl+Z"));

    actionCut = new QAction(this);
    actionCut->setObjectName(QString::fromUtf8("actionCut"));
    actionCut->setText(tr("Cut"));
    actionCut->setShortcut(tr("Ctrl+X"));

    actionCopy = new QAction(this);
    actionCopy->setObjectName(QString::fromUtf8("actionCopy"));
    actionCopy->setText(tr("Copy"));
    actionCopy->setShortcut(tr("Ctrl+C"));

    actionPaste = new QAction(this);
    actionPaste->setObjectName(QString::fromUtf8("actionPaste"));
    actionPaste->setText(tr("Paste"));
    actionPaste->setShortcut(tr("Ctrl+V"));   
}

void TupPapagayoApp::setupUI()
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupPapagayoApp::setupUI()]";
    #endif

    setWindowTitle(tr("Lip-Sync Manager"));

    // Central Widget
    QWidget *centralWidget = new QWidget(this);
    centralWidget->setObjectName(QString::fromUtf8("centralWidget"));

    QHBoxLayout *centralHorizontalLayout = new QHBoxLayout(centralWidget);
    centralHorizontalLayout->setSpacing(6);
    centralHorizontalLayout->setContentsMargins(11, 11, 11, 11);
    centralHorizontalLayout->setObjectName(QString::fromUtf8("centralHorizontalLayout"));

    QVBoxLayout *innerVerticalLayout = new QVBoxLayout();
    innerVerticalLayout->setSpacing(6);
    innerVerticalLayout->setObjectName(QString::fromUtf8("innerVerticalLayout"));

    QScrollArea *scrollArea = new QScrollArea(centralWidget);
    scrollArea->setObjectName(QString::fromUtf8("scrollArea"));
    scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    scrollArea->setWidgetResizable(true);

    waveformView = new TupWaveFormView();
    waveformView->setObjectName(QString::fromUtf8("waveformView"));
    waveformView->setGeometry(QRect(0, 0, 542, 194));
    scrollArea->setWidget(waveformView);
    waveformView->setScrollArea(scrollArea);

    innerVerticalLayout->addWidget(scrollArea);

    // Lateral Panel
    QGroupBox *lateralGroupBox = new QGroupBox(centralWidget);
    lateralGroupBox->setObjectName(QString::fromUtf8("lateralGroupBox"));
    lateralGroupBox->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    lateralGroupBox->setTitle(tr("Current Voice"));

    QSizePolicy generalSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    generalSizePolicy.setHorizontalStretch(0);
    generalSizePolicy.setVerticalStretch(0);
    generalSizePolicy.setHeightForWidth(lateralGroupBox->sizePolicy().hasHeightForWidth());

    QVBoxLayout *lateralVerticalLayout = new QVBoxLayout(lateralGroupBox);
    lateralVerticalLayout->setSpacing(6);
    lateralVerticalLayout->setContentsMargins(11, 11, 11, 11);
    lateralVerticalLayout->setObjectName(QString::fromUtf8("lateralVerticalLayout"));
    QHBoxLayout *voiceHorizontalLayout = new QHBoxLayout();
    voiceHorizontalLayout->setSpacing(6);
    voiceHorizontalLayout->setObjectName(QString::fromUtf8("voiceHorizontalLayout"));

    voiceName = new QLineEdit();

    QLabel *voiceLabel = new QLabel(lateralGroupBox);
    voiceLabel->setText(tr("Voice name:"));
    voiceLabel->setBuddy(voiceName);

    voiceHorizontalLayout->addWidget(voiceLabel);
    voiceHorizontalLayout->addWidget(voiceName);

    QSpacerItem *voiceHorizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
    voiceHorizontalLayout->addItem(voiceHorizontalSpacer);
    lateralVerticalLayout->addLayout(voiceHorizontalLayout);

    voiceText = new QPlainTextEdit(lateralGroupBox);
    voiceText->setObjectName(QString::fromUtf8("voiceText"));
    QSizePolicy voiceSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    voiceSizePolicy.setHorizontalStretch(0);
    voiceSizePolicy.setVerticalStretch(0);
    voiceSizePolicy.setHeightForWidth(voiceText->sizePolicy().hasHeightForWidth());
    voiceText->setSizePolicy(voiceSizePolicy);

    QLabel *spokenText = new QLabel(lateralGroupBox);
    spokenText->setText(tr("Spoken text:"));
    spokenText->setBuddy(voiceText);

    lateralVerticalLayout->addWidget(spokenText);
    lateralVerticalLayout->addWidget(voiceText);

    QLabel *phoneticLabel = new QLabel(lateralGroupBox);
    phoneticLabel->setText(tr("Phonetic breakdown:"));
    phoneticLabel->setObjectName(QString::fromUtf8("phoneticLabel"));

    lateralVerticalLayout->addWidget(phoneticLabel);

    QHBoxLayout *languageLayout = new QHBoxLayout;
    languageLayout->setObjectName(QString::fromUtf8("languageLayout"));

    QHBoxLayout *languageHorizontalLayout = new QHBoxLayout();
    languageHorizontalLayout->setSpacing(6);
    languageHorizontalLayout->setObjectName(QString::fromUtf8("languageHorizontalLayout"));

    languageChoice = new QComboBox();
    languageChoice->addItem(tr("English"));
    languageChoice->addItem(tr("Spanish"));
    languageChoice->addItem(tr("Italian"));
    languageChoice->setObjectName(QString::fromUtf8("languageChoice"));

    languageHorizontalLayout->addWidget(languageChoice);

    breakdownButton = new QPushButton(lateralGroupBox);
    breakdownButton->setText(tr("Breakdown"));
    breakdownButton->setObjectName(QString::fromUtf8("breakdownButton"));

    languageHorizontalLayout->addWidget(breakdownButton);

    QSpacerItem *languageHorizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
    languageHorizontalLayout->addItem(languageHorizontalSpacer);

    exportChoice = new QComboBox;
    exportChoice->addItem(tr("Anime Studio"));

    languageHorizontalLayout->addWidget(exportChoice);

    exportButton = new QPushButton(lateralGroupBox);
    exportButton->setText(tr("Export..."));
    exportButton->setObjectName(QString::fromUtf8("exportButton"));

    languageHorizontalLayout->addWidget(exportButton);
    lateralVerticalLayout->addLayout(languageHorizontalLayout);
    innerVerticalLayout->addWidget(lateralGroupBox);
    centralHorizontalLayout->addLayout(innerVerticalLayout);

    QVBoxLayout *verticalLayout = new QVBoxLayout();
    verticalLayout->setSpacing(6);
    verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
    QHBoxLayout *horizontalLayout = new QHBoxLayout();
    horizontalLayout->setSpacing(6);
    horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));

    QLabel *fpsLabel = new QLabel(centralWidget);
    fpsLabel->setObjectName(QString::fromUtf8("label"));
    fpsLabel->setText(tr("FPS:"));
    horizontalLayout->addWidget(fpsLabel);

    fpsEdit = new QLineEdit(centralWidget);
    fpsEdit->setObjectName(QString::fromUtf8("fpsEdit"));
    QSizePolicy fpsSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    fpsSizePolicy.setHorizontalStretch(1);
    fpsSizePolicy.setVerticalStretch(0);
    fpsSizePolicy.setHeightForWidth(fpsEdit->sizePolicy().hasHeightForWidth());
    fpsEdit->setSizePolicy(fpsSizePolicy);
    fpsEdit->setValidator(new QIntValidator(1, 120));

    horizontalLayout->addWidget(fpsEdit);
    verticalLayout->addLayout(horizontalLayout);

    mouthsCombo = new QComboBox(centralWidget);
    mouthsCombo->addItem(tr("Mouth 1"));
    mouthsCombo->addItem(tr("Mouth 2"));
    mouthsCombo->addItem(tr("Gary C Martin"));
    mouthsCombo->addItem(tr("Preston Blair"));
    mouthsCombo->setObjectName(QString::fromUtf8("comboBox"));
    generalSizePolicy.setHeightForWidth(mouthsCombo->sizePolicy().hasHeightForWidth());
    mouthsCombo->setSizePolicy(generalSizePolicy);
    mouthsCombo->setMinimumSize(QSize(0, 0));
    mouthsCombo->setMaximumSize(QSize(16777215, 16777215));

    verticalLayout->addWidget(mouthsCombo);

    QFrame *frame = new QFrame();
    frame->setObjectName(QString::fromUtf8("frame"));

    QSizePolicy frameSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
    frameSizePolicy.setHorizontalStretch(0);
    frameSizePolicy.setVerticalStretch(0);
    frameSizePolicy.setHeightForWidth(frame->sizePolicy().hasHeightForWidth());

    frame->setSizePolicy(frameSizePolicy);
    frame->setFrameShape(QFrame::StyledPanel);
    frame->setFrameShadow(QFrame::Raised);

    QHBoxLayout *mouthHorizontalLayout = new QHBoxLayout(frame);
    mouthHorizontalLayout->setSpacing(6);
    mouthHorizontalLayout->setContentsMargins(11, 11, 11, 11);
    mouthHorizontalLayout->setObjectName(QString::fromUtf8("mouthHorizontalLayout"));
    mouthHorizontalLayout->setContentsMargins(0, 0, 0, 0);

    mouthView = new TupMouthView(frame);
    mouthView->setObjectName(QString::fromUtf8("mouthView"));
    frameSizePolicy.setHeightForWidth(mouthView->sizePolicy().hasHeightForWidth());
    mouthView->setSizePolicy(frameSizePolicy);
    mouthView->setMinimumSize(QSize(200, 200));
    mouthView->setMaximumSize(QSize(16777215, 16777215));
    // SQA: stylesheet pending
    mouthView->setStyleSheet(QString::fromUtf8(""));
    mouthHorizontalLayout->addWidget(mouthView);
    mouthHorizontalLayout->addWidget(mouthView);

    mouthHorizontalLayout->addWidget(mouthView);
    verticalLayout->addWidget(frame);

    QGroupBox *voiceGroupBox = new QGroupBox();
    voiceGroupBox->setObjectName(QString::fromUtf8("voiceGroupBox"));
    voiceGroupBox->setTitle(tr("Voice List"));

    QSizePolicy voiceGroupSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    voiceGroupSizePolicy.setHorizontalStretch(0);
    voiceGroupSizePolicy.setVerticalStretch(0);
    voiceGroupSizePolicy.setHeightForWidth(voiceGroupBox->sizePolicy().hasHeightForWidth());
    voiceGroupBox->setSizePolicy(voiceGroupSizePolicy);
    QVBoxLayout *voiceVerticalLayout = new QVBoxLayout(voiceGroupBox);
    voiceVerticalLayout->setSpacing(6);
    voiceVerticalLayout->setContentsMargins(11, 11, 11, 11);
    voiceVerticalLayout->setObjectName(QString::fromUtf8("voiceVerticalLayout"));

    voiceList = new QListWidget(voiceGroupBox);
    voiceList->setObjectName(QString::fromUtf8("voiceList"));
    QSizePolicy voiceListSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
    voiceListSizePolicy.setHorizontalStretch(0);
    voiceListSizePolicy.setVerticalStretch(0);
    voiceListSizePolicy.setHeightForWidth(voiceList->sizePolicy().hasHeightForWidth());
    voiceList->setSizePolicy(voiceListSizePolicy);
    voiceList->setMaximumSize(QSize(16777215, 16777215));
    voiceList->setEditTriggers(QAbstractItemView::DoubleClicked|QAbstractItemView::EditKeyPressed);
    voiceVerticalLayout->addWidget(voiceList);

    QHBoxLayout *voiceHLayout = new QHBoxLayout();
    voiceHLayout->setSpacing(6);
    voiceHLayout->setObjectName(QString::fromUtf8("voiceHLayout"));

    newVoiceButton = new QPushButton(voiceGroupBox);
    newVoiceButton->setObjectName(QString::fromUtf8("newVoiceButton"));
    newVoiceButton->setText(tr("New"));

    voiceHLayout->addWidget(newVoiceButton);

    deleteVoiceButton = new QPushButton(voiceGroupBox);
    deleteVoiceButton->setObjectName(QString::fromUtf8("deleteVoiceButton"));
    deleteVoiceButton->setText(tr("Delete"));
    voiceHLayout->addWidget(deleteVoiceButton);
    voiceVerticalLayout->addLayout(voiceHLayout);

    verticalLayout->addWidget(voiceGroupBox);
    centralHorizontalLayout->addLayout(verticalLayout);

    setCentralWidget(centralWidget);

    QStatusBar *statusBar = new QStatusBar(this);
    statusBar->setObjectName(QString::fromUtf8("statusBar"));
    setStatusBar(statusBar);
}

void TupPapagayoApp::setupMenus()
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupPapagayoApp::setupMenus()]";
    #endif

    QMenuBar *menuBar = new QMenuBar(this);
    menuBar->setObjectName(QString::fromUtf8("menuBar"));
    menuBar->setGeometry(QRect(0, 0, 872, 22));

    QMenu *menuFile = new QMenu(menuBar);
    menuFile->setObjectName(QString::fromUtf8("menuFile"));
    menuFile->setTitle(tr("File"));

    QMenu *menuEdit = new QMenu(menuBar);
    menuEdit->setObjectName(QString::fromUtf8("menuEdit"));
    menuEdit->setTitle(QCoreApplication::translate("TupPapagayoApp", "Edit", nullptr));
    menuFile->setTitle(tr("Edit"));

    menuFile->addAction(actionOpen);
    menuFile->addAction(actionSave);
    menuFile->addAction(actionSaveAs);
    menuFile->addAction(actionClose);

    menuEdit->addAction(actionUndo);
    menuEdit->addAction(actionCut);
    menuEdit->addAction(actionCopy);
    menuEdit->addAction(actionPaste);

    menuBar->addAction(menuFile->menuAction());
    menuBar->addAction(menuEdit->menuAction());

    setMenuBar(menuBar);

    QToolBar *mainToolBar = new QToolBar(this);
    mainToolBar->setObjectName(QString::fromUtf8("mainToolBar"));
    mainToolBar->setMovable(true);
    mainToolBar->setIconSize(QSize(32, 32));
    mainToolBar->setToolButtonStyle(Qt::ToolButtonIconOnly);
    mainToolBar->setFloatable(false);

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

void TupPapagayoApp::setupConnections()
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupPapagayoApp::setupConnections()]";
    #endif

    connect(actionOpen, SIGNAL(triggered()), this, SLOT(onFileOpen()));
    connect(actionPlay, SIGNAL(triggered()), this, SLOT(onPlay()));
    connect(actionStop, SIGNAL(triggered()), this, SLOT(onStop()));
    connect(newVoiceButton, SIGNAL(clicked()), this, SLOT(onNewVoice()));
    connect(deleteVoiceButton, SIGNAL(clicked()), this, SLOT(onDeleteVoice()));
    connect(voiceName, SIGNAL(textChanged(QString)), this, SLOT(onVoiceNameChanged()));
    connect(voiceText, SIGNAL(textChanged()), this, SLOT(onVoiceTextChanged()));
    connect(voiceList, SIGNAL(itemClicked(QListWidgetItem*)), this, SLOT(onVoiceSelected(QListWidgetItem*)));
    connect(voiceList, SIGNAL(itemChanged(QListWidgetItem*)), this, SLOT(onVoiceItemChanged(QListWidgetItem*)));
    connect(breakdownButton, SIGNAL(clicked()), this, SLOT(onBreakdown()));
    connect(mouthsCombo, SIGNAL(activated(int)), mouthView, SLOT(onMouthChanged(int)));
    connect(actionSave, SIGNAL(triggered()), this, SLOT(onFileSave()));
    connect(actionSaveAs, SIGNAL(triggered()), this, SLOT(onFileSaveAs()));
    connect(exportButton, SIGNAL(clicked()), this, SLOT(onExport()));
    connect(actionClose, SIGNAL(triggered()), this, SLOT(close()));

    connect(actionZoomIn, SIGNAL(triggered()), waveformView, SLOT(onZoomIn()));
    connect(actionZoomOut, SIGNAL(triggered()), waveformView, SLOT(onZoomOut()));
    connect(actionAutoZoom, SIGNAL(triggered()), waveformView, SLOT(onAutoZoom()));
    connect(fpsEdit, SIGNAL(textChanged(QString)), this, SLOT(onFpsChange(QString)));
    connect(waveformView, SIGNAL(frameChanged(int)), mouthView, SLOT(onFrameChanged(int)));
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
        QMessageBox::warning(this, tr("Papagayo"),
                             tr("Error opening audio file."),
                             QMessageBox::Ok);
        setWindowTitle(tr("Papagayo"));
    } else {
        waveformView->setDocument(document);
        mouthView->setDocument(document);
        document->getAudioPlayer()->setNotifyInterval(17); // 60 fps
        connect(document->getAudioPlayer(), SIGNAL(positionChanged(qint64)), waveformView, SLOT(positionChanged(qint64)));

        rebuildVoiceList();
        if (document->getCurrentVoice()) {
            voiceName->setText(document->getCurrentVoice()->getName());
            enableAutoBreakdown = false;
            voiceText->setPlainText(document->getCurrentVoice()->getText());
            enableAutoBreakdown = true;
        }
        setWindowTitle(tr("Papagayo") + " - " + info.fileName());
    }

    fpsEdit->setText(QString::number(document->getFps()));
    updateActions();
}

bool TupPapagayoApp::isOKToCloseDocument()
{
    if (document && document->isDirty()) {
        int res = QMessageBox::warning(this, tr("Papagayo"),
                  tr("The document has been modified.\n"
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

void TupPapagayoApp::restoreSettings()
{
    QSettings settings;
    restoreGeometry(settings.value("TupPapagayoApp/geometry").toByteArray());
    restoreState(settings.value("TupPapagayoApp/windowState").toByteArray());

    defaultFps = settings.value("defaultFps").toInt();
    if (defaultFps < 1)
        defaultFps = 24;
    defaultFps = PG_CLAMP(defaultFps, 1, 120);
}

void TupPapagayoApp::saveSettings()
{
    QSettings settings;

    settings.setValue("TupPapagayoApp/geometry", saveGeometry());
    settings.setValue("TupPapagayoApp/windowState", saveState());
    settings.setValue("defaultFps", defaultFps);
}

void TupPapagayoApp::closeEvent(QCloseEvent *event)
{
    if (isOKToCloseDocument()) {
        if (document) {
            delete document;
            document = nullptr;
        }
        saveSettings();
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
    QString extn = info.suffix().toLower();
    if (extn == "pgo" || extn == "mp3" || extn == "wav")
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
    QString extn = info.suffix().toLower();
    if (extn == "pgo" || extn == "mp3" || extn == "wav") {
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
    actionSaveAs->setEnabled(flag);
    actionPlay->setEnabled(flag);
    actionStop->setEnabled(flag);
    actionZoomIn->setEnabled(flag);
    actionZoomOut->setEnabled(flag);
    actionAutoZoom->setEnabled(flag);

    voiceName->setEnabled(flag);
    voiceText->setEnabled(flag);

    voiceName->setEnabled(true);
    voiceText->setEnabled(true);

    languageChoice->setEnabled(false);

    exportChoice->setEnabled(flag);
    fpsEdit->setEnabled(flag);
    voiceList->setEnabled(flag);
    newVoiceButton->setEnabled(flag);

    if (flag) {
        breakdownButton->setEnabled(document->getCurrentVoice() && !document->getCurrentVoice()->getText().isEmpty());
        exportButton->setEnabled(document->getCurrentVoice() && !document->getCurrentVoice()->getText().isEmpty());
        deleteVoiceButton->setEnabled(document->getCurrentVoice() && document->getVoices().size() > 1);
    } else {        
        breakdownButton->setEnabled(false);
        exportButton->setEnabled(false);
        deleteVoiceButton->setEnabled(false);
    }
}

void TupPapagayoApp::onFileOpen()
{
    if (!isOKToCloseDocument())
        return;

    QSettings settings;
    QString filePath = QFileDialog::getOpenFileName(this,
                                                    tr("Open"), settings.value("default_dir", "").toString(),
                                                    tr("Papgayo and Audio files (*.pgo *.mp3 *.wav)"));
    if (filePath.isEmpty())
        return;

    QFileInfo info(filePath);
    settings.setValue("default_dir", info.dir().absolutePath());

    openFile(filePath);
}

void TupPapagayoApp::onFileSave()
{
    if (!document)
        return;

    if (document->getFilePath().isEmpty()) {
        onFileSaveAs();
        return;
    }

    document->save();
    QFileInfo info(document->getFilePath());
    setWindowTitle(tr("Lip-sync Creator") + " - " + info.fileName());
}

void TupPapagayoApp::onFileSaveAs()
{
    if (!document)
        return;

    QSettings settings;
    QString name = tr("Untitled.pgo");
    if (!document->getFilePath().isEmpty()) {
        name = document->getFilePath();
    } else {
        QDir dir(settings.value("default_dir", "").toString());
        name = dir.absoluteFilePath(name);
    }
    QString filePath = QFileDialog::getSaveFileName(this,
                                                    tr("Save"), name,
                                                    tr("Papgayo files (*.pgo)"));
    if (filePath.isEmpty())
        return;

    QFileInfo info(filePath);
    settings.setValue("default_dir", info.dir().absolutePath());

    document->setFilePath(filePath);
    onFileSave();
}

void TupPapagayoApp::onPlay()
{
    if (document && document->getAudioPlayer()) {
        playerStopped = false;
        document->getAudioPlayer()->play();
    }
}

void TupPapagayoApp::onStop()
{
    if (document && document->getAudioPlayer()) {
        playerStopped = true;
        document->getAudioPlayer()->stop();
    }
}

void TupPapagayoApp::onFpsChange(QString text)
{
    if (!document)
        return;

    int32 fps = text.toInt();
    fps = PG_CLAMP(fps, 1, 120);

    if (fps == document->getFps())
        return;

    defaultFps = fps;
    document->setFps(fps);
    waveformView->setDocument(nullptr);
    waveformView->setDocument(document);
}

void TupPapagayoApp::onNewVoice()
{
    if (!document)
        return;

    QString newVoiceName = tr("Voice");
    newVoiceName += " ";
    newVoiceName += QString::number(document->getVoices().size() + 1);
    document->setCurrentVoice(new LipsyncVoice(newVoiceName));
    document->appendVoice(document->getCurrentVoice());
    rebuildVoiceList();

    if (document->getCurrentVoice()) {
        voiceName->setText(document->getCurrentVoice()->getName());
        voiceText->setPlainText(document->getCurrentVoice()->getText());
    }
}

void TupPapagayoApp::onDeleteVoice()
{
    if (!document || document->getVoices().size() < 2 || document->getCurrentVoice() == nullptr)
        return;

    int id = document->getVoices().indexOf(document->getCurrentVoice());
    document->removeVoiceAt(id);
    delete document->getCurrentVoice();
    if (id > 0)
        id--;
    document->setCurrentVoice(document->getVoiceAt(id));
    rebuildVoiceList();

    if (document->getCurrentVoice()) {
        voiceName->setText(document->getCurrentVoice()->getName());
        enableAutoBreakdown = false;
        voiceText->setPlainText(document->getCurrentVoice()->getText());
        enableAutoBreakdown = true;
    }
    updateActions();
}

void TupPapagayoApp::onVoiceSelected(QListWidgetItem *item)
{
    if (rebuildingList || !document)
        return;

    int id = voiceList->row(item);
    if (id >= 0 && id < document->getVoices().size()) {
        document->setCurrentVoice(document->getVoiceAt(id));
        if (document->getCurrentVoice()) {
            voiceName->setText(document->getCurrentVoice()->getName());
            enableAutoBreakdown = false;
            voiceText->setPlainText(document->getCurrentVoice()->getText());
            enableAutoBreakdown = true;
        }
    }
    waveformView->update();
    updateActions();
}

void TupPapagayoApp::onVoiceItemChanged(QListWidgetItem *item)
{
    if (rebuildingList || !document)
        return;

    int id = voiceList->row(item);
    if (id >= 0 && id < document->getVoices().size()) {
        document->setCurrentVoice(document->getVoiceAt(id));
        if (document->getCurrentVoice()) {
            document->getCurrentVoice()->setName(item->text());
            voiceName->setText(document->getCurrentVoice()->getName());
        }
    }
}

void TupPapagayoApp::onVoiceNameChanged()
{
    if (!document || !document->getCurrentVoice())
        return;

    document->getCurrentVoice()->setName(voiceName->text());
    rebuildVoiceList();
}

void TupPapagayoApp::onVoiceTextChanged()
{
    if (!document || !document->getCurrentVoice())
        return;

    document->getCurrentVoice()->setText(voiceText->toPlainText());
    if (enableAutoBreakdown)
        onBreakdown(); // this is cool, but it could slow things down by doing constant breakdowns
    updateActions();
}

void TupPapagayoApp::onBreakdown()
{
    if (!document || !document->getCurrentVoice())
        return;

    TupLipsyncDoc::loadDictionaries();
    document->setDirtyFlag(true);
    int32 duration = document->getFps() * 10;
    if (document->getAudioExtractor()) {
        real time = document->getAudioExtractor()->duration();
        time *= document->getFps();
        duration = PG_ROUND(time);
    }

    document->getCurrentVoice()->runBreakdown("EN", duration);
    waveformView->update();
}

void TupPapagayoApp::onExport()
{
    if (!document || !document->getCurrentVoice())
        return;

    QSettings settings;
    QString name = document->getCurrentVoice()->getName() + tr(".dat");
    QDir dir(settings.value("default_dir", "").toString());
    name = dir.absoluteFilePath(name);
    QString filePath = QFileDialog::getSaveFileName(this, tr("Export"), name, tr("DAT files (*.dat)"));
    if (filePath.isEmpty())
        return;

    QFileInfo info(filePath);
    settings.setValue("default_dir", info.dir().absolutePath());

    document->getCurrentVoice()->exportVoice(filePath);
}

void TupPapagayoApp::rebuildVoiceList()
{
    if (rebuildingList)
        return;

    rebuildingList = true;
    voiceList->clear();

    if (document) {
        for (int i = 0; i < document->getVoices().size(); i++) {
            voiceList->addItem(document->getVoiceAt(i)->getName());
            QListWidgetItem *item = voiceList->item(i);
            item->setFlags(item->flags() | Qt::ItemIsEditable);
        }

        if (document->getCurrentVoice()) {
            voiceList->setCurrentItem(voiceList->item(document->getVoices().indexOf(document->getCurrentVoice())));
        }
    }
    rebuildingList = false;
}

void TupPapagayoApp::keyPressEvent(QKeyEvent *event)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupPapagayoApp::keyPressEvent()]";
    #endif

    if (event->key() == Qt::Key_Space) {
        if (playerStopped)
            onPlay();
        else
            onStop();
    }
}

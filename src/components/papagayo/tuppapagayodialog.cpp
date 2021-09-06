#include <QtWidgets>

#include "tuppapagayodialog.h"
#include "ui_tuppapagayodialog.h"

TupPapagayoDialog::TupPapagayoDialog(QWidget *parent) : QMainWindow(parent), ui(new Ui::TupPapagayoDialog)
{
    document = nullptr;
    enableAutoBreakdown = true;
    defaultFps = 24;

	ui->setupUi(this);

	setAcceptDrops(true);

    ui->waveformView->setScrollArea(ui->scrollArea);
	ui->fpsEdit->setValidator(new QIntValidator(1, 120));

	connect(ui->actionZoomIn, SIGNAL(triggered()), ui->waveformView, SLOT(onZoomIn()));
	connect(ui->actionZoomOut, SIGNAL(triggered()), ui->waveformView, SLOT(onZoomOut()));
	connect(ui->actionAutoZoom, SIGNAL(triggered()), ui->waveformView, SLOT(onAutoZoom()));
	connect(ui->fpsEdit, SIGNAL(textChanged(QString)), this, SLOT(onFpsChange(QString)));
	connect(ui->waveformView, SIGNAL(frameChanged(int)), ui->mouthView, SLOT(onFrameChanged(int)));

    restoreSettings();
	updateActions();
}

TupPapagayoDialog::~TupPapagayoDialog()
{
    if (document) {
        delete document;
    }

	delete ui;
}

void TupPapagayoDialog::openFile(QString filePath)
{
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
        ui->waveformView->setDocument(document);
        ui->mouthView->setDocument(document);
        document->getAudioPlayer()->setNotifyInterval(17); // 60 fps
        connect(document->getAudioPlayer(), SIGNAL(positionChanged(qint64)), ui->waveformView, SLOT(positionChanged(qint64)));

        rebuildVoiceList();
        if (document->getCurrentVoice()) {
            ui->voiceName->setText(document->getCurrentVoice()->getName());
            enableAutoBreakdown = false;
            ui->voiceText->setPlainText(document->getCurrentVoice()->getText());
            enableAutoBreakdown = true;
		}
		setWindowTitle(tr("Papagayo") + " - " + info.fileName());
	}

    ui->fpsEdit->setText(QString::number(document->getFps()));
	updateActions();
}

bool TupPapagayoDialog::isOKToCloseDocument()
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

void TupPapagayoDialog::restoreSettings()
{
	QSettings settings;
	restoreGeometry(settings.value("TupPapagayoDialog/geometry").toByteArray());
	restoreState(settings.value("TupPapagayoDialog/windowState").toByteArray());

    defaultFps = settings.value("defaultFps").toInt();
    if (defaultFps < 1)
        defaultFps = 24;
    defaultFps = PG_CLAMP(defaultFps, 1, 120);
}

void TupPapagayoDialog::saveSettings()
{
	QSettings settings;

	settings.setValue("TupPapagayoDialog/geometry", saveGeometry());
	settings.setValue("TupPapagayoDialog/windowState", saveState());
    settings.setValue("defaultFps", defaultFps);
}

void TupPapagayoDialog::closeEvent(QCloseEvent *event)
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

void TupPapagayoDialog::dragEnterEvent(QDragEnterEvent *event)
{
	QList<QUrl>	urls = event->mimeData()->urls();
	if (urls.isEmpty())
		return;

	QString filePath = urls.first().toLocalFile();

	if (filePath.isEmpty())
		return;

	QFileInfo info(filePath);
	QString extn = info.suffix().toLower();
	if (extn == "wav" || extn == "pgo" || extn == "aif" || extn == "aiff")
		event->acceptProposedAction();
}

void TupPapagayoDialog::dropEvent(QDropEvent *event)
{
	QList<QUrl>	urls = event->mimeData()->urls();
	if (urls.isEmpty())
		return;

	QString filePath = urls.first().toLocalFile();

	if (filePath.isEmpty())
		return;

	QFileInfo info(filePath);
	QString extn = info.suffix().toLower();
    if (extn == "wav" || extn == "pgo" || extn == "aif" || extn == "aiff") {
		event->acceptProposedAction();
        if (isOKToCloseDocument())
            openFile(filePath);
	}
}

void TupPapagayoDialog::updateActions()
{
    if (document) {
		ui->actionSave->setEnabled(true);
		ui->actionSave_As->setEnabled(true);
		ui->actionPlay->setEnabled(true);
		ui->actionStop->setEnabled(true);
		ui->actionZoomIn->setEnabled(true);
		ui->actionZoomOut->setEnabled(true);
		ui->actionAutoZoom->setEnabled(true);

		ui->voiceName->setEnabled(true);
		ui->voiceText->setEnabled(true);
		ui->languageChoice->setEnabled(false);
        ui->breakdownButton->setEnabled(document->getCurrentVoice() && !document->getCurrentVoice()->getText().isEmpty());
		ui->exportChoice->setEnabled(true);
        ui->exportButton->setEnabled(document->getCurrentVoice() && !document->getCurrentVoice()->getText().isEmpty());
		ui->fpsEdit->setEnabled(true);
		ui->voiceList->setEnabled(true);
		ui->newVoiceButton->setEnabled(true);
        ui->deleteVoiceButton->setEnabled(document->getCurrentVoice() && document->getVoices().size() > 1);
    } else {
		ui->actionSave->setEnabled(false);
		ui->actionSave_As->setEnabled(false);
		ui->actionPlay->setEnabled(false);
		ui->actionStop->setEnabled(false);
		ui->actionZoomIn->setEnabled(false);
		ui->actionZoomOut->setEnabled(false);
		ui->actionAutoZoom->setEnabled(false);

		ui->voiceName->setEnabled(false);
		ui->voiceText->setEnabled(false);
		ui->languageChoice->setEnabled(false);
		ui->breakdownButton->setEnabled(false);
		ui->exportChoice->setEnabled(false);
		ui->exportButton->setEnabled(false);
		ui->fpsEdit->setEnabled(false);
		ui->voiceList->setEnabled(false);
		ui->newVoiceButton->setEnabled(false);
		ui->deleteVoiceButton->setEnabled(false);
	}
}

void TupPapagayoDialog::onHelpAboutPapagayo()
{
	QString msg = "\
                    <center>\
                    <font size=+3><b>Papagayo</b></font>\
                    <br><font size=-1><i>Version: 2.0b1</i></font>\
                    <br><font size=-1>&copy; 2005-2014 Mike Clifton</font>\
                    <br><font size=-1><a href=\"http://www.gnu.org/licenses/gpl.html\">License: GPLv3</a></font>\
                    <br><br><img src=\":/images/images/papagayo.png\"><br>\
                    <br>A lip-sync tool for use with <a href=\"http://anime.smithmicro.com/\">Anime Studio Pro</a>\
                    <br>(formerly <a href=\"http://www.lostmarble.com\">Lost Marble's Moho</a>)\
                    <br><font size=-1>This software uses the Carnegie Mellon <a href=\"http://www.speech.cs.cmu.edu/cgi-bin/cmudict\">Pronouncing Dictionary</a></font>\
                    <br><font size=-1>Spanish pronunciation code provided by <a href=\"http://www-personal.monash.edu.au/~myless/catnap/index.html\">Myles Strous</a></font>\
                    <br><font size=-1>3D mouth shapes provided by <a href=\"http://www.garycmartin.com\">Gary C. Martin</a></font>\
                    <br><font size=-1>Some icons provided by <a href=\"http://glyphicons.com\">GLYPHICONS.com</a></font>\
                    </center>\
                    ";

	QMessageBox::about(this, tr("About Papagayo"), msg);
}

void TupPapagayoDialog::onFileOpen()
{
    if (!isOKToCloseDocument())
		return;

	QSettings settings;
	QString filePath = QFileDialog::getOpenFileName(this,
													tr("Open"), settings.value("default_dir", "").toString(),
													tr("Papgayo and Audio files (*.pgo;*.wav;*.aif;*.aiff)"));
	if (filePath.isEmpty())
		return;

	QFileInfo info(filePath);
	settings.setValue("default_dir", info.dir().absolutePath());

    openFile(filePath);
}

void TupPapagayoDialog::onFileSave()
{
    if (!document)
		return;

    if (document->getFilePath().isEmpty()) {
		onFileSaveAs();
		return;
	}

    document->save();
    QFileInfo info(document->getFilePath());
	setWindowTitle(tr("Papagayo") + " - " + info.fileName());
}

void TupPapagayoDialog::onFileSaveAs()
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

void TupPapagayoDialog::onPlay()
{
    if (document && document->getAudioPlayer())
        document->getAudioPlayer()->play();
}

void TupPapagayoDialog::onStop()
{
    if (document && document->getAudioPlayer())
        document->getAudioPlayer()->stop();
}

void TupPapagayoDialog::onFpsChange(QString text)
{
    if (!document)
		return;

	int32 fps = text.toInt();
	fps = PG_CLAMP(fps, 1, 120);

    if (fps == document->getFps())
		return;

    defaultFps = fps;
    document->setFps(fps);
    ui->waveformView->setDocument(nullptr);
    ui->waveformView->setDocument(document);
}

void TupPapagayoDialog::onNewVoice()
{
    if (!document)
		return;

    QString	newVoiceName = tr("Voice");
	newVoiceName += " ";
    newVoiceName += QString::number(document->getVoices().size() + 1);
    document->setCurrentVoice(new LipsyncVoice(newVoiceName));
    document->appendVoice(document->getCurrentVoice());
    rebuildVoiceList();

    if (document->getCurrentVoice()) {
        ui->voiceName->setText(document->getCurrentVoice()->getName());
        ui->voiceText->setPlainText(document->getCurrentVoice()->getText());
	}
}

void TupPapagayoDialog::onDeleteVoice()
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
        ui->voiceName->setText(document->getCurrentVoice()->getName());
        enableAutoBreakdown = false;
        ui->voiceText->setPlainText(document->getCurrentVoice()->getText());
        enableAutoBreakdown = true;
	}
	updateActions();
}

void TupPapagayoDialog::onVoiceSelected(QListWidgetItem *item)
{
    if (rebuildingList || !document)
		return;

	int id = ui->voiceList->row(item);
    if (id >= 0 && id < document->getVoices().size()) {
        document->setCurrentVoice(document->getVoiceAt(id));
        if (document->getCurrentVoice()) {
            ui->voiceName->setText(document->getCurrentVoice()->getName());
            enableAutoBreakdown = false;
            ui->voiceText->setPlainText(document->getCurrentVoice()->getText());
            enableAutoBreakdown = true;
		}
	}
	ui->waveformView->update();
	updateActions();
}

void TupPapagayoDialog::onVoiceItemChanged(QListWidgetItem *item)
{
    if (rebuildingList || !document)
		return;

	int id = ui->voiceList->row(item);
    if (id >= 0 && id < document->getVoices().size()) {
        document->setCurrentVoice(document->getVoiceAt(id));
        if (document->getCurrentVoice()) {
            document->getCurrentVoice()->setName(item->text());
            ui->voiceName->setText(document->getCurrentVoice()->getName());
		}
	}
}

void TupPapagayoDialog::onVoiceNameChanged()
{
    if (!document || !document->getCurrentVoice())
		return;

    document->getCurrentVoice()->setName(ui->voiceName->text());
    rebuildVoiceList();
}

void TupPapagayoDialog::onVoiceTextChanged()
{
    if (!document || !document->getCurrentVoice())
		return;

    document->getCurrentVoice()->setText(ui->voiceText->toPlainText());
    if (enableAutoBreakdown)
		onBreakdown(); // this is cool, but it could slow things down by doing constant breakdowns
	updateActions();
}

void TupPapagayoDialog::onBreakdown()
{
    if (!document || !document->getCurrentVoice())
		return;

    TupLipsyncDoc::loadDictionaries();
    document->setDirtyFlag(true);
    int32 duration = document->getFps() * 10;
    if (document->getAudioExtractor()) {
        real f = document->getAudioExtractor()->duration();
        f *= document->getFps();
		duration = PG_ROUND(f);
	}

    document->getCurrentVoice()->runBreakdown("EN", duration);
	ui->waveformView->update();
}

void TupPapagayoDialog::onExport()
{
    if (!document || !document->getCurrentVoice())
		return;

	QSettings settings;
    QString name = document->getCurrentVoice()->getName() + tr(".dat");
	QDir dir(settings.value("default_dir", "").toString());
	name = dir.absoluteFilePath(name);
	QString filePath = QFileDialog::getSaveFileName(this,
													tr("Export"), name,
													tr("DAT files (*.dat)"));
	if (filePath.isEmpty())
		return;

	QFileInfo info(filePath);
	settings.setValue("default_dir", info.dir().absolutePath());

    document->getCurrentVoice()->exportVoice(filePath);
}

void TupPapagayoDialog::rebuildVoiceList()
{
    if (rebuildingList)
		return;

    rebuildingList = true;
	ui->voiceList->clear();

    if (document) {
        for (int i = 0; i < document->getVoices().size(); i++) {
            ui->voiceList->addItem(document->getVoiceAt(i)->getName());
			QListWidgetItem *item = ui->voiceList->item(i);
			item->setFlags(item->flags() | Qt::ItemIsEditable);
		}

        if (document->getCurrentVoice()) {
            ui->voiceList->setCurrentItem(ui->voiceList->item(document->getVoices().indexOf(document->getCurrentVoice())));
		}
	}
    rebuildingList = false;
}

#include "tupbreakdowndialog.h"
#include "ui_tupbreakdowndialog.h"
#include "tuplipsyncdoc.h"

TupBreakdownDialog::TupBreakdownDialog(LipsyncWord *word, QWidget *parent) : QDialog(parent), ui(new Ui::TupBreakdownDialog)
{
	ui->setupUi(this);
    ui->wordLabel->setText(tr("Break down the word:") + " " + word->getText());

	connect(ui->aiBut, SIGNAL(clicked()), this, SLOT(on_aiBut()));
	connect(ui->oBut, SIGNAL(clicked()), this, SLOT(on_oBut()));
	connect(ui->eBut, SIGNAL(clicked()), this, SLOT(on_eBut()));
	connect(ui->uBut, SIGNAL(clicked()), this, SLOT(on_uBut()));
	connect(ui->lBut, SIGNAL(clicked()), this, SLOT(on_lBut()));
	connect(ui->wqBut, SIGNAL(clicked()), this, SLOT(on_wqBut()));
	connect(ui->mbpBut, SIGNAL(clicked()), this, SLOT(on_mbpBut()));
	connect(ui->fvBut, SIGNAL(clicked()), this, SLOT(on_fvBut()));
	connect(ui->etcBut, SIGNAL(clicked()), this, SLOT(on_etcBut()));
	connect(ui->restBut, SIGNAL(clicked()), this, SLOT(on_restBut()));

	QString str;
    for (int i = 0; i < word->phonemesSize(); i++) {
		str += " ";
        str += word->getPhonemeAt(i)->getText();
	}
	ui->breakdownEdit->setText(str.trimmed());
}

TupBreakdownDialog::~TupBreakdownDialog()
{
	delete ui;
}

QString TupBreakdownDialog::phonemeString()
{
	return ui->breakdownEdit->text().trimmed();
}

void TupBreakdownDialog::on_aiBut()
{
    addPhoneme("AI");
}

void TupBreakdownDialog::on_oBut()
{
    addPhoneme("O");
}

void TupBreakdownDialog::on_eBut()
{
    addPhoneme("E");
}

void TupBreakdownDialog::on_uBut()
{
    addPhoneme("U");
}

void TupBreakdownDialog::on_lBut()
{
    addPhoneme("L");
}

void TupBreakdownDialog::on_wqBut()
{
    addPhoneme("WQ");
}

void TupBreakdownDialog::on_mbpBut()
{
    addPhoneme("MBP");
}

void TupBreakdownDialog::on_fvBut()
{
    addPhoneme("FV");
}

void TupBreakdownDialog::on_etcBut()
{
    addPhoneme("etc");
}

void TupBreakdownDialog::on_restBut()
{
    addPhoneme("rest");
}

void TupBreakdownDialog::addPhoneme(QString phoneme)
{
	QString str = ui->breakdownEdit->text().trimmed();
	str += " ";
	str += phoneme;
	ui->breakdownEdit->setText(str.trimmed());
}

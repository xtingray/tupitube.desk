#include "tupbreakdowndialog.h"
// #include "ui_tupbreakdowndialog.h"
#include "tuplipsyncdoc.h"

#include <QPushButton>

TupBreakdownDialog::TupBreakdownDialog(LipsyncWord *word, QWidget *parent) : QDialog(parent)  // , ui(new Ui::TupBreakdownDialog)
{
    // ui->setupUi(this);
    // ui->wordLabel->setText(tr("Break down the word:") + " " + word->getText());
    title = tr("Break down the word:") + " " + word->getText();

    setupUI();
    connect(aiBut, SIGNAL(clicked()), this, SLOT(on_aiBut()));
    connect(oBut, SIGNAL(clicked()), this, SLOT(on_oBut()));
    connect(eBut, SIGNAL(clicked()), this, SLOT(on_eBut()));
    connect(uBut, SIGNAL(clicked()), this, SLOT(on_uBut()));
    connect(lBut, SIGNAL(clicked()), this, SLOT(on_lBut()));
    connect(wqBut, SIGNAL(clicked()), this, SLOT(on_wqBut()));
    connect(mbpBut, SIGNAL(clicked()), this, SLOT(on_mbpBut()));
    connect(fvBut, SIGNAL(clicked()), this, SLOT(on_fvBut()));
    connect(etcBut, SIGNAL(clicked()), this, SLOT(on_etcBut()));
    connect(restBut, SIGNAL(clicked()), this, SLOT(on_restBut()));

	QString str;
    for (int i = 0; i < word->phonemesSize(); i++) {
		str += " ";
        str += word->getPhonemeAt(i)->getText();
	}
    breakdownEdit->setText(str.trimmed());
}

TupBreakdownDialog::~TupBreakdownDialog()
{
    // delete ui;
}

void TupBreakdownDialog::setupUI()
{
    if (breakdownDialog->objectName().isEmpty())
        breakdownDialog->setObjectName(QString::fromUtf8("breakdownDialog"));

    breakdownDialog->setWindowModality(Qt::NonModal);
    breakdownDialog->resize(364, 207);
    QSizePolicy sizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    sizePolicy.setHorizontalStretch(0);
    sizePolicy.setVerticalStretch(0);
    sizePolicy.setHeightForWidth(breakdownDialog->sizePolicy().hasHeightForWidth());
    breakdownDialog->setSizePolicy(sizePolicy);
    breakdownDialog->setSizeGripEnabled(false);
    breakdownDialog->setModal(false);
    verticalLayout = new QVBoxLayout(breakdownDialog);
    verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
    verticalLayout->setSizeConstraint(QLayout::SetFixedSize);

    wordLabel = new QLabel(breakdownDialog);
    wordLabel->setObjectName(QString::fromUtf8("wordLabel"));
    wordLabel->setText(title);
    verticalLayout->addWidget(wordLabel);

    gridLayout = new QGridLayout();
    gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
    aiBut = new QPushButton(breakdownDialog);
    aiBut->setObjectName(QString::fromUtf8("aiBut"));

    gridLayout->addWidget(aiBut, 0, 0, 1, 1);

    oBut = new QPushButton(breakdownDialog);
    oBut->setObjectName(QString::fromUtf8("oBut"));

    gridLayout->addWidget(oBut, 0, 1, 1, 1);

    eBut = new QPushButton(breakdownDialog);
    eBut->setObjectName(QString::fromUtf8("eBut"));

    gridLayout->addWidget(eBut, 0, 2, 1, 1);

    uBut = new QPushButton(breakdownDialog);
    uBut->setObjectName(QString::fromUtf8("uBut"));

    gridLayout->addWidget(uBut, 0, 3, 1, 1);

    lBut = new QPushButton(breakdownDialog);
    lBut->setObjectName(QString::fromUtf8("lBut"));

    gridLayout->addWidget(lBut, 1, 0, 1, 1);

    wqBut = new QPushButton(breakdownDialog);
    wqBut->setObjectName(QString::fromUtf8("wqBut"));

    gridLayout->addWidget(wqBut, 1, 1, 1, 1);

    mbpBut = new QPushButton(breakdownDialog);
    mbpBut->setObjectName(QString::fromUtf8("mbpBut"));

    gridLayout->addWidget(mbpBut, 1, 2, 1, 1);

    fvBut = new QPushButton(breakdownDialog);
    fvBut->setObjectName(QString::fromUtf8("fvBut"));

    gridLayout->addWidget(fvBut, 1, 3, 1, 1);

    etcBut = new QPushButton(breakdownDialog);
    etcBut->setObjectName(QString::fromUtf8("etcBut"));

    gridLayout->addWidget(etcBut, 2, 1, 1, 1);

    restBut = new QPushButton(breakdownDialog);
    restBut->setObjectName(QString::fromUtf8("restBut"));

    gridLayout->addWidget(restBut, 2, 2, 1, 1);

    verticalLayout->addLayout(gridLayout);

    breakdownEdit = new QLineEdit(breakdownDialog);
    breakdownEdit->setObjectName(QString::fromUtf8("breakdownEdit"));

    verticalLayout->addWidget(breakdownEdit);

    buttonBox = new QDialogButtonBox(breakdownDialog);
    buttonBox->setObjectName(QString::fromUtf8("buttonBox"));
    buttonBox->setOrientation(Qt::Horizontal);
    buttonBox->setStandardButtons(QDialogButtonBox::Cancel|QDialogButtonBox::Ok);

    verticalLayout->addWidget(buttonBox);

    breakdownDialog->setWindowTitle(QCoreApplication::translate("TupBreakdownDialog", "Unknown Word", nullptr));
    wordLabel->setText(QCoreApplication::translate("TupBreakdownDialog", "Break down the word: BIGPLACEHOLDERWORD", nullptr));
    aiBut->setText(QCoreApplication::translate("TupBreakdownDialog", "AI", nullptr));
    oBut->setText(QCoreApplication::translate("TupBreakdownDialog", "O", nullptr));
    eBut->setText(QCoreApplication::translate("TupBreakdownDialog", "E", nullptr));
    uBut->setText(QCoreApplication::translate("TupBreakdownDialog", "U", nullptr));
    lBut->setText(QCoreApplication::translate("TupBreakdownDialog", "L", nullptr));
    wqBut->setText(QCoreApplication::translate("TupBreakdownDialog", "WQ", nullptr));
    mbpBut->setText(QCoreApplication::translate("TupBreakdownDialog", "MBP", nullptr));
    fvBut->setText(QCoreApplication::translate("TupBreakdownDialog", "FV", nullptr));
    etcBut->setText(QCoreApplication::translate("TupBreakdownDialog", "etc", nullptr));
    restBut->setText(QCoreApplication::translate("TupBreakdownDialog", "rest", nullptr));

    connect(buttonBox, SIGNAL(accepted()), breakdownDialog, SLOT(accept()));
    connect(buttonBox, SIGNAL(rejected()), breakdownDialog, SLOT(reject()));
}

QString TupBreakdownDialog::phonemeString()
{
    return breakdownEdit->text().trimmed();
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
    QString str = breakdownEdit->text().trimmed();
	str += " ";
	str += phoneme;
    breakdownEdit->setText(str.trimmed());
}

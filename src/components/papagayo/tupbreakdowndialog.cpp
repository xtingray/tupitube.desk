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

#include "tupbreakdowndialog.h"
#include "tapplicationproperties.h"
#include "tbutton.h"
#include "timagelabel.h"

#include <QComboBox>
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>

TupBreakdownDialog::TupBreakdownDialog(LipsyncWord *word, const QString &mouthsPath, QWidget *parent) : QDialog(parent)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupBreakdownDialog()] - mouthsPath -> " << mouthsPath;
    #endif

    setWindowTitle(tr("Word:") + " " + word->getText());
    setWindowIcon(QIcon(QPixmap(THEME_DIR + "icons/papagayo.png")));

    QFile file(THEME_DIR + "config/ui.qss");
    if (file.exists()) {
        file.open(QFile::ReadOnly);
        QString styleSheet = QLatin1String(file.readAll());
        if (styleSheet.length() > 0)
            setStyleSheet(styleSheet);
        file.close();
    } else {
        #ifdef TUP_DEBUG
            qWarning() << "[TupBreakdownDialog()] - theme file doesn't exist -> " << (THEME_DIR + "config/ui.qss");
        #endif
    }

    mouthLabels << "AI" << "E" << "etc" << "FV" << "L" << "MBP" << "O" << "rest" << "U" << "WQ";
    folder = mouthsPath;

    QDir directory(folder);
    QStringList mouthsList = directory.entryList(QStringList(), QDir::Files);
    QFileInfo info(mouthsList.at(0));
    extension = info.suffix();

    QVBoxLayout *layout = new QVBoxLayout(this);

    QLabel *wordLabel = new QLabel(this);
    wordLabel->setAlignment(Qt::AlignCenter);
    wordLabel->setText(tr("Break down the word:") + " <b>" + word->getText() + "</b>");
    layout->addWidget(wordLabel);

    stackedWidget = new QStackedWidget;
    stackedWidget->addWidget(createMouthsCollection());

    layout->addWidget(stackedWidget, Qt::AlignCenter);

    QHBoxLayout *phonemesLayout = new QHBoxLayout;

    QLabel *phonemesLabel = new QLabel(tr("Phonemes:"));
    breakdownEdit = new QLineEdit;

    QPushButton *clearButton = new QPushButton(this);
    clearButton->setMinimumWidth(60);
    clearButton->setIcon(QIcon(THEME_DIR + "icons/clear.png"));
    clearButton->setToolTip(tr("Clear phonemes"));
    connect(clearButton, SIGNAL(clicked()), this, SLOT(clearPhonemes()));

    phonemesLayout->addWidget(phonemesLabel);
    phonemesLayout->addWidget(breakdownEdit);
    phonemesLayout->addWidget(clearButton);

    layout->addLayout(phonemesLayout);

    QHBoxLayout *buttonsLayout = new QHBoxLayout;

    QPushButton *okButton = new QPushButton(this);
    okButton->setMinimumWidth(60);
    okButton->setIcon(QIcon(THEME_DIR + "icons/apply.png"));
    okButton->setToolTip(tr("Save word"));
    connect(okButton, SIGNAL(clicked()), this, SLOT(accept()));

    QPushButton *cancelButton = new QPushButton(this);
    cancelButton->setMinimumWidth(60);
    cancelButton->setIcon(QIcon(THEME_DIR + "icons/close.png"));
    cancelButton->setToolTip(tr("Close"));
    connect(cancelButton, SIGNAL(clicked()), this, SLOT(reject()));

    buttonsLayout->addStretch(1);
    buttonsLayout->addWidget(okButton);
    buttonsLayout->addWidget(cancelButton);

    layout->addLayout(buttonsLayout);
}

TupBreakdownDialog::~TupBreakdownDialog()
{
}

QWidget * TupBreakdownDialog::createMouthsCollection()
{
    QWidget *collection = new QWidget;
    QGridLayout *mouthsLayout = new QGridLayout(collection);
    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < 5; j++)
            mouthsLayout->addWidget(createMouthPanel(i, j), i, j, Qt::AlignCenter);
    }

    return collection;
}

QWidget * TupBreakdownDialog::createMouthPanel(int row, int column)
{
    int labelIndex = column;
    if (row == 1)
        labelIndex = 5 + column;
    QString text = mouthLabels.at(labelIndex);

    QWidget *panel = new QWidget;
    QVBoxLayout *panelLayout = new QVBoxLayout(panel);

    TButton *phonemeButton = new TButton(text);
    connect(phonemeButton, &TButton::clicked, this, &TupBreakdownDialog::addPhoneme);
    panelLayout->addWidget(phonemeButton);

    QString imgPath = folder + "/" + text + "." + extension;
    #ifdef TUP_DEBUG
        qDebug() << "[TupBreakdownDialog::createMouthPanel()] - imgPath -> " << imgPath;
    #endif

    TImageLabel *mouthImage = new TImageLabel(text, QColor(200, 255, 200));
    connect(mouthImage, &TImageLabel::clicked, this, &TupBreakdownDialog::addPhoneme);
    connect(phonemeButton, SIGNAL(clicked(QString)), mouthImage, SLOT(activateMark()));
    mouthImage->setAlignment(Qt::AlignCenter);
    mouthImage->setPixmap(QPixmap(imgPath));
    mouthImage->setStyleSheet("QWidget { border: 1px solid #cccccc; border-radius: 3px; }");
    panelLayout->addWidget(mouthImage, Qt::AlignCenter);

    return panel;
}

QString TupBreakdownDialog::phonemeString()
{
    return breakdownEdit->text().trimmed();
}

void TupBreakdownDialog::addPhoneme(const QString &phoneme)
{
    QString str = breakdownEdit->text().trimmed();
    str += " ";
    str += phoneme;
    breakdownEdit->setText(str.trimmed());
}

void TupBreakdownDialog::clearPhonemes()
{
    breakdownEdit->clear();
}

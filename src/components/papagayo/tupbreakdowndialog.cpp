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
#include "tseparator.h"
#include "tosd.h"

#include <QComboBox>
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>

TupBreakdownDialog::TupBreakdownDialog(const QString &word, const QString &phonemes,
                                       const QString &mouthsPath, QWidget *parent) : QDialog(parent)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupBreakdownDialog()] - word -> " << word;
        qDebug() << "[TupBreakdownDialog()] - phonemes -> " << phonemes;
        qDebug() << "[TupBreakdownDialog()] - mouthsPath -> " << mouthsPath;
    #endif

    isPhrase = false;
    currentIndex = 0;

    setInitVars(word, mouthsPath);
    setUIStyle();
    setUI(word, phonemes);
}

TupBreakdownDialog::TupBreakdownDialog(QStringList wordsList, QStringList phonemesList, const QString &mouthsPath,
                                       QWidget *parent) : QDialog(parent)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupBreakdownDialog()] - mouthsPath -> " << mouthsPath;
    #endif

    this->wordsList = wordsList;
    this->phonemesList = phonemesList;
    isPhrase = true;
    currentIndex = 0;

    setInitVars(wordsList.at(0), mouthsPath);
    setUIStyle();
    setUI(wordsList.at(0), phonemesList.at(0));
}

TupBreakdownDialog::~TupBreakdownDialog()
{
}

void TupBreakdownDialog::setInitVars(const QString &word, const QString &mouthsPath)
{
    setWindowTitle(tr("Word:") + " " + word);
    setWindowIcon(QIcon(QPixmap(THEME_DIR + "icons/papagayo.png")));
    mouthLabels << "AI" << "E" << "etc" << "FV" << "L" << "MBP" << "O" << "rest" << "U" << "WQ";
    folder = mouthsPath;

    QDir directory(folder);
    QStringList mouthsList = directory.entryList(QStringList(), QDir::Files);
    QFileInfo info(mouthsList.at(0));
    extension = info.suffix();
}

void TupBreakdownDialog::setUIStyle()
{
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
}

void TupBreakdownDialog::setUI(const QString &word, const QString &phonemes)
{
    mainLayout = new QVBoxLayout(this);

    wordLabel = new QLabel(this);
    wordLabel->setAlignment(Qt::AlignCenter);
    wordLabel->setText(tr("Break down the word:") + " <b>" + word + "</b>");
    mainLayout->addWidget(wordLabel);

    stackedWidget = new QStackedWidget;
    stackedWidget->addWidget(createMouthsCollection());

    mainLayout->addWidget(stackedWidget, Qt::AlignCenter);

    QHBoxLayout *phonemesLayout = new QHBoxLayout;

    QLabel *phonemesLabel = new QLabel(tr("Phonemes:"));
    breakdownEdit = new QLineEdit(phonemes);

    QPushButton *clearButton = new QPushButton(this);
    clearButton->setMinimumWidth(60);
    clearButton->setIcon(QIcon(THEME_DIR + "icons/clear.png"));
    clearButton->setToolTip(tr("Clear phonemes"));
    connect(clearButton, SIGNAL(clicked()), this, SLOT(clearPhonemes()));

    phonemesLayout->addWidget(phonemesLabel);
    phonemesLayout->addWidget(breakdownEdit);
    phonemesLayout->addWidget(clearButton);

    mainLayout->addLayout(phonemesLayout);
    mainLayout->addWidget(new TSeparator(Qt::Horizontal));

    setButtonsPanel();
}

void TupBreakdownDialog::setButtonsPanel()
{
    QHBoxLayout *buttonsLayout = new QHBoxLayout;
    okButton = new QPushButton(this);
    okButton->setMinimumWidth(60);

    if (isPhrase) {
        previousButton = new QPushButton(this);
        previousButton->setMinimumWidth(60);
        previousButton->setIcon(QIcon(THEME_DIR + "icons/previous.png"));
        previousButton->setToolTip(tr("Next word"));
        connect(previousButton, SIGNAL(clicked()), this, SLOT(previousWord()));
        previousButton->setEnabled(false);

        okButton->setIcon(QIcon(THEME_DIR + "icons/next.png"));
        okButton->setToolTip(tr("Next word"));
        connect(okButton, SIGNAL(clicked()), this, SLOT(nextWord()));
    } else {
        okButton->setIcon(QIcon(THEME_DIR + "icons/apply.png"));
        okButton->setToolTip(tr("Save word"));
        connect(okButton, SIGNAL(clicked()), this, SLOT(savePhonemes()));

        if (breakdownEdit->text().isEmpty())
            okButton->setEnabled(false);
    }

    QPushButton *cancelButton = new QPushButton(this);
    cancelButton->setMinimumWidth(60);
    cancelButton->setIcon(QIcon(THEME_DIR + "icons/close.png"));
    cancelButton->setToolTip(tr("Close"));
    connect(cancelButton, SIGNAL(clicked()), this, SLOT(reject()));

    buttonsLayout->addStretch(1);
    if (isPhrase)
        buttonsLayout->addWidget(previousButton);

    buttonsLayout->addWidget(okButton);
    buttonsLayout->addWidget(cancelButton);

    mainLayout->addLayout(buttonsLayout);
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

    QString imgPath = folder + text + "." + extension;
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

    if  (!okButton->isEnabled())
        okButton->setEnabled(true);
}

void TupBreakdownDialog::clearPhonemes()
{
    breakdownEdit->clear();
}

void TupBreakdownDialog::previousWord()
{
    if (currentIndex > 0) {
        currentIndex--;

        QString word = wordsList.at(currentIndex);
        setWindowTitle(tr("Word:") + " " + word);
        wordLabel->setText(tr("Break down the word:") + " <b>" + word + "</b>");
        breakdownEdit->setText(phonemesList.at(currentIndex));

        if (currentIndex == wordsList.size() - 2) {
            okButton->setIcon(QIcon(THEME_DIR + "icons/next.png"));
            okButton->setToolTip(tr("Next word"));
            disconnect(okButton, SIGNAL(clicked()), this, SLOT(savePhonemes()));
            connect(okButton, SIGNAL(clicked()), this, SLOT(nextWord()));
        }

        if (currentIndex == 0) {
            if (previousButton->isEnabled())
                previousButton->setEnabled(false);
        }
    }
}

void TupBreakdownDialog::nextWord()
{
    QString wordPhonemes = breakdownEdit->text().trimmed();
    if (wordPhonemes.isEmpty()) {
        notifyMissingPhonemes();
        return;
    }

    phonemesList[currentIndex] = wordPhonemes;
    currentIndex++;
    if (currentIndex < wordsList.size()) {
        QString word = wordsList.at(currentIndex);
        setWindowTitle(tr("Word:") + " " + word);
        wordLabel->setText(tr("Break down the word:") + " <b>" + word + "</b>");
        breakdownEdit->setText(phonemesList.at(currentIndex));

        if (currentIndex == wordsList.size() - 1) {
            okButton->setIcon(QIcon(THEME_DIR + "icons/apply.png"));
            okButton->setToolTip(tr("Save sentence"));
            disconnect(okButton, SIGNAL(clicked()), this, SLOT(nextWord()));
            connect(okButton, SIGNAL(clicked()), this, SLOT(savePhonemes()));
        }

        if (!previousButton->isEnabled())
            previousButton->setEnabled(true);
    }
}

QStringList TupBreakdownDialog::phomeneList()
{
    return phonemesList;
}

void TupBreakdownDialog::savePhonemes()
{
    QString field = breakdownEdit->text();
    if (field.isEmpty()) {
        notifyMissingPhonemes();
        return;
    } else {
        if (isPhrase)
            phonemesList[currentIndex] = field.trimmed();
        accept();
    }
}

void TupBreakdownDialog::notifyMissingPhonemes()
{
    TOsd::self()->display(TOsd::Warning, tr("Word phonemes are missing!"));
    #ifdef TUP_DEBUG
        qDebug() << "[TupBreakdownDialog::notifyMissingPhonemes()] - Fatal Error: Word phonemes are missing!!!";
    #endif
    breakdownEdit->setFocus();
}

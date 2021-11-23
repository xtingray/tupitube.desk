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

#include <QFile>
#include <QFileInfo>
#include <QDir>

#include "tuplipsyncdoc.h"
#include "tapplicationproperties.h"

LipsyncPhoneme::LipsyncPhoneme()
{
    text = "";
    frame = 0;
    top = 0;
    bottom = 0;
}

LipsyncPhoneme::~LipsyncPhoneme()
{
}

void LipsyncPhoneme::setText(const QString string)
{
    text = string;
}

QString LipsyncPhoneme::getText() const
{
    return text;
}

void LipsyncPhoneme::setFrame(int frameIndex)
{
    frame = frameIndex;
}

int LipsyncPhoneme::getFrame() const
{
    return frame;
}

int LipsyncPhoneme::getTop()
{
    return top;
}

int LipsyncPhoneme::getBottom()
{
    return bottom;
}

void LipsyncPhoneme::setTop(int32 index)
{
    top = index;
}

void LipsyncPhoneme::setBottom(int32 index)
{
    bottom = index;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

LipsyncWord::LipsyncWord()
{
    text = "";
    startFrame = 0;
    endFrame = 0;
    top = bottom = 0;
}

LipsyncWord::~LipsyncWord()
{
    while (!phonemes.isEmpty())
        delete phonemes.takeFirst();
}

void LipsyncWord::runBreakdown(QString language)
{
    while (!phonemes.isEmpty())
        delete phonemes.takeFirst();

    QString msg = text;
    msg.remove(QRegExp("[.,!?;-/()¿]"));
	QStringList	pronunciation;
    if (language == "EN") {
        pronunciation = TupLipsyncDoc::getDictionaryValue(msg.toUpper());
        if (pronunciation.size() > 1) {
            for (int32 i = 1; i < pronunciation.size(); i++) {
				QString p = pronunciation.at(i);
				if (p.length() == 0)
					continue;
				LipsyncPhoneme *phoneme = new LipsyncPhoneme;
                phoneme->setText(TupLipsyncDoc::getPhonemeFromDictionary(p, "etc"));
                phonemes << phoneme;
			}
		}
	}
}

void LipsyncWord::repositionPhoneme(LipsyncPhoneme *phoneme)
{
    int id = phonemes.indexOf(phoneme);

    if ((id > 0) && (phoneme->getFrame() < phonemes[id - 1]->getFrame() + 1))
        phoneme->setFrame(phonemes[id - 1]->getFrame() + 1);

    if ((id < phonemes.size() - 1) && (phoneme->getFrame() > phonemes[id + 1]->getFrame() - 1))
        phoneme->setFrame(phonemes[id + 1]->getFrame() - 1);

    if (phoneme->getFrame() < startFrame)
        phoneme->setFrame(startFrame);

    if (phoneme->getFrame() > endFrame)
        phoneme->setFrame(endFrame);
}

void LipsyncWord::setText(const QString string)
{
    text = string;
}

QString LipsyncWord::getText() const
{
    return text;
}

void LipsyncWord::setStartFrame(int32 frameIndex)
{
    startFrame = frameIndex;
}

int LipsyncWord::getStartFrame() const
{
    return startFrame;
}

void LipsyncWord::setEndFrame(int32 frameIndex)
{
    endFrame = frameIndex;
}

int LipsyncWord::getEndFrame() const
{
    return endFrame;
}

QList<LipsyncPhoneme *>	LipsyncWord::getPhonemes()
{
    return phonemes;
}

void LipsyncWord::addPhoneme(LipsyncPhoneme *phoneme)
{
    phonemes << phoneme;
}

int LipsyncWord::phonemesSize()
{
    return phonemes.size();
}

LipsyncPhoneme* LipsyncWord::getPhonemeAt(int32 index)
{
    if (phonemes.size() > index)
        return phonemes.at(index);

    return nullptr;
}

LipsyncPhoneme * LipsyncWord::getLastPhoneme()
{
    return phonemes.last();
}

void LipsyncWord::removeFirstPhoneme()
{
    delete phonemes.takeFirst();
}

void LipsyncWord::removePhonemes()
{
    while (!phonemes.isEmpty())
        delete phonemes.takeFirst();
}

int LipsyncWord::getTop()
{
    return top;
}

int LipsyncWord::getBottom()
{
    return bottom;
}

void LipsyncWord::setTop(int32 index)
{
    top = index;
}

void LipsyncWord::setBottom(int32 index)
{
    bottom = index;
}

int LipsyncWord::getFrameFromPhonemeAt(int32 index)
{
    LipsyncPhoneme *phoneme = getPhonemeAt(index);
    if (phoneme)
        return phoneme->getFrame();

    return 0;
}

QString LipsyncWord::getPhonemesString() const
{
    QString string = "";
    foreach(LipsyncPhoneme *phoneme, phonemes)
        string += phoneme->getText() + " ";

    return string.trimmed();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

LipsyncPhrase::LipsyncPhrase()
{
    text = "";
    startFrame = 0;
    endFrame = 0;
    top = bottom = 0;
}

LipsyncPhrase::~LipsyncPhrase()
{
    cleanWords();
}

void LipsyncPhrase::runBreakdown(QString language)
{
	// break phrase into words
    cleanWords();

    QStringList strList = text.split(' ', Qt::SkipEmptyParts);
    for (int32 i = 0; i < strList.size(); i++) {
		if (strList.at(i).length() == 0)
			continue;
		LipsyncWord *word = new LipsyncWord;
        word->setText(strList.at(i));
        words << word;
	}

	// now break down the words
    for (int32 i = 0; i < words.size(); i++)
        words[i]->runBreakdown(language);
}

void LipsyncPhrase::repositionWord(LipsyncWord *word)
{
    int id = words.indexOf(word);

    if ((id > 0) && (word->getStartFrame() < words[id - 1]->getEndFrame() + 1)) {
        word->setStartFrame(words[id - 1]->getEndFrame() + 1);
        if (word->getEndFrame() < word->getStartFrame() + 1)
            word->setEndFrame(word->getStartFrame() + 1);
	}

    if ((id < words.size() - 1) && (word->getEndFrame() > words[id + 1]->getStartFrame() - 1)) {
        word->setEndFrame(words[id + 1]->getStartFrame() - 1);
        if (word->getStartFrame() > word->getEndFrame() - 1)
            word->setStartFrame(word->getEndFrame() - 1);
	}

    if (word->getStartFrame() < startFrame)
        word->setStartFrame(startFrame);

    if (word->getEndFrame() > endFrame)
        word->setEndFrame(endFrame);

    if (word->getEndFrame() < word->getStartFrame())
        word->setEndFrame(word->getStartFrame());

	// now divide up the total time by phonemes
    int32 frameDuration = word->getEndFrame() - word->getStartFrame() + 1;
    int32 phonemeCount = word->getPhonemes().size();
	float framesPerPhoneme = 1.0f;
    if (frameDuration > 0 && phonemeCount > 0) {
		framesPerPhoneme = (float)frameDuration / (float)phonemeCount;
		if (framesPerPhoneme < 1.0f)
			framesPerPhoneme = 1.0f;
	}

	// finally, assign frames based on phoneme durations
    float curFrame = word->getStartFrame();
    QList<LipsyncPhoneme *> phonemes = word->getPhonemes();
    for (int32 i = 0; i < phonemes.size(); i++) {
        phonemes[i]->setFrame(PG_ROUND(curFrame));
		curFrame = curFrame + framesPerPhoneme;
	}

    for (int32 i = 0; i < phonemes.size(); i++)
        word->repositionPhoneme(phonemes[i]);
}

void LipsyncPhrase::setText(const QString string)
{
    text = string;
}

QString LipsyncPhrase::getText() const
{
    return text;
}

void LipsyncPhrase::setStartFrame(int frameIndex)
{
    startFrame = frameIndex;
}

int LipsyncPhrase::getStartFrame() const
{
    return startFrame;
}

void LipsyncPhrase::setEndFrame(int frameIndex)
{
    endFrame = frameIndex;
}

int LipsyncPhrase::getEndFrame() const
{
    return endFrame;
}

int32 LipsyncPhrase::getTop()
{
    return top;
}

int32 LipsyncPhrase::getBottom()
{
    return bottom;
}

void LipsyncPhrase::setTop(int32 index)
{
    top = index;
}

void LipsyncPhrase::setBottom(int32 index)
{
    bottom = index;
}

LipsyncWord * LipsyncPhrase::getWordAt(int index)
{
    if (words.size() > index)
        return words.at(index);

    return nullptr;
}

void LipsyncPhrase::addWord(LipsyncWord* word)
{
    words.append(word);
}

int LipsyncPhrase::wordsSize()
{
    return words.size();
}

QList<LipsyncWord *> LipsyncPhrase::getWords()
{
    return words;
}

LipsyncWord* LipsyncPhrase::getLastWord()
{
    return words.last();
}

int LipsyncPhrase::getStartFrameFromWordAt(int index)
{
    LipsyncWord *word = getWordAt(index);
    if (word)
        return word->getStartFrame();

    return 0;
}

int LipsyncPhrase::getEndFrameFromWordAt(int index)
{
    LipsyncWord *word = getWordAt(index);
    if (word)
        return word->getEndFrame();

    return 0;
}

void LipsyncPhrase::cleanWords()
{
    while (!words.isEmpty())
        delete words.takeFirst();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

LipsyncVoice::LipsyncVoice(const QString &name)
{
    this->name = name;
}

LipsyncVoice::~LipsyncVoice()
{
    cleanPhrases();
}

void LipsyncVoice::setName(const QString &name)
{
    this->name = name;
}

QString LipsyncVoice::getName() const
{
    return name;
}

void LipsyncVoice::setText(const QString &text)
{
    this->text = text;
}

QString LipsyncVoice::getText() const
{
    return text;
}

void LipsyncVoice::addPhrase(LipsyncPhrase *phrase)
{
    phrases << phrase;
}

QList<LipsyncPhrase *> LipsyncVoice::getPhrases()
{
    return phrases;
}

LipsyncPhrase * LipsyncVoice::getFirstPhrase()
{
    if (!phrases.isEmpty())
        return phrases.first();

    return nullptr;
}

LipsyncPhrase* LipsyncVoice::getPhraseAt(int index)
{
    if (phrases.count() > index)
        return phrases.at(index);

    return nullptr;
}

int LipsyncVoice::getPhrasesTotal()
{
    return phrases.size();
}

int LipsyncVoice::getPhraseStartFrame(int index)
{
    LipsyncPhrase *phrase = getPhraseAt(index);
    if (phrase)
        return phrase->getStartFrame();

    return 0;
}

int LipsyncVoice::getPhraseEndFrame(int index)
{
    LipsyncPhrase *phrase = getPhraseAt(index);
    if (phrase)
        return phrase->getEndFrame();

    return 0;
}

void LipsyncVoice::open(QTextStream &in)
{
    int32 numPhrases, numWords;
    int32 numPhonemes = 0;
    QString str;

    name = in.readLine().trimmed();
    text = in.readLine().trimmed();
    text = text.split('|').join('\n');

	numPhrases = in.readLine().toInt();
    for (int p = 0; p < numPhrases; p++) {
		LipsyncPhrase *phrase = new LipsyncPhrase;
        phrase->setText(in.readLine().trimmed());
        phrase->setStartFrame(in.readLine().toInt());
        phrase->setEndFrame(in.readLine().toInt());
		numWords = in.readLine().toInt();
        for (int w = 0; w < numWords; w++) {
			LipsyncWord *word = new LipsyncWord;
			str = in.readLine().trimmed();
			QStringList strList = str.split(' ', Qt::SkipEmptyParts);
            if (strList.size() >= 4) {
                word->setText(strList.at(0));
                word->setStartFrame(strList.at(1).toInt());
                word->setEndFrame(strList.at(2).toInt());
				numPhonemes = strList.at(3).toInt();
			}

            for (int ph = 0; ph < numPhonemes; ph++) {
				LipsyncPhoneme *phoneme = new LipsyncPhoneme;
				str = in.readLine().trimmed();
				QStringList strList = str.split(' ', Qt::SkipEmptyParts);
                if (strList.size() >= 2) {
                    phoneme->setFrame(strList.at(0).toInt());
                    phoneme->setText(strList.at(1));
				}
                word->addPhoneme(phoneme);
			} // for ph
            phrase->addWord(word);
		} // for w
        phrases << phrase;
	} // for p
}

void LipsyncVoice::save(QTextStream &out)
{
    out << '\t' << name << Qt::endl;
    out << '\t' << text.split('\n').join('|') << Qt::endl;
    out << '\t' << phrases.size() << Qt::endl;

    for (int p = 0; p < phrases.size(); p++) {
        LipsyncPhrase *phrase = phrases[p];
        out << "\t\t" << phrase->getText() << Qt::endl;
        out << "\t\t" << phrase->getStartFrame() << Qt::endl;
        out << "\t\t" << phrase->getEndFrame() << Qt::endl;
        out << "\t\t" << phrase->wordsSize() << Qt::endl;

        for (int w = 0; w < phrase->wordsSize(); w++) {
            LipsyncWord *word = phrase->getWordAt(w);
            out << "\t\t\t" << word->getText()
                << ' ' << word->getStartFrame()
                << ' ' << word->getEndFrame()
                << ' ' << word->getPhonemes().size()
				<< Qt::endl;
            for (int ph = 0; ph < word->phonemesSize(); ph++) {
                LipsyncPhoneme *phoneme = word->getPhonemeAt(ph);
                out << "\t\t\t\t" << phoneme->getFrame() << ' ' << phoneme->getText() << Qt::endl;
			} // for ph
		} // for w
	} // for p
}

void LipsyncVoice::exportVoice(QString path)
{
    QFile file(path);

    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
		return;

    QTextStream out(&file);
	out << "MohoSwitch1" << Qt::endl;

    int startFrame = 0;
    int endFrame = 1;
	QString	phoneme, nextPhoneme;

    if (phrases.size() > 0) {
        startFrame = phrases[0]->getStartFrame();
        endFrame = phrases.last()->getEndFrame();
	}

    if (startFrame > 1) {
		phoneme = "rest";
		out << 1 << ' ' << "rest" << Qt::endl;
	}

    for (int frame = startFrame; frame <= endFrame; frame++) {
        nextPhoneme = getPhonemeAtFrame(frame);
        if (nextPhoneme != phoneme) {
            if (phoneme == "rest") { // export an extra "rest" phoneme at the end of a pause between words or phrases
				out << frame << ' ' << phoneme << Qt::endl;
			}
			phoneme = nextPhoneme;
			out << frame - 1 << ' ' << phoneme << Qt::endl;
		}
	}
	out << endFrame + 2 << ' ' << "rest" << Qt::endl;
}

void LipsyncVoice::runBreakdown(QString language, int32 audioDuration)
{
	// make sure there is a space after all punctuation marks
	QString punctuation = ".,!?;";
	bool repeatLoop = true;
    while (repeatLoop) {
		repeatLoop = false;
        int32 n = text.length();
        for (int32 i = 0; i < n - 1; i++) {
            if (punctuation.contains(text[i]) && !text[i + 1].isSpace()) {
                text.insert(i + 1,	' ');
				repeatLoop = true;
				break;
			}
		}
	}

	// break text into phrases
    cleanPhrases();

    QStringList strList = text.split('\n', Qt::SkipEmptyParts);
    for (int32 i = 0; i < strList.size(); i++) {
		if (strList.at(i).length() == 0)
			continue;

		LipsyncPhrase *phrase = new LipsyncPhrase;
        phrase->setText(strList.at(i));
        phrases << phrase;
	}

	// now break down the phrases
    for (int32 i = 0; i < phrases.size(); i++) {
        phrases[i]->runBreakdown(language);
    }

	// for first-guess frame alignment, count how many phonemes we have
	int32 phonemeCount = 0;
    for (int32 i = 0; i < phrases.size(); i++) {
        LipsyncPhrase *phrase = phrases[i];
        for (int32 j = 0; j < phrase->wordsSize(); j++) {
            if (phrase->getWordAt(j)->phonemesSize() == 0) // deal with unknown words
				phonemeCount += 4;
			else
                phonemeCount += phrase->getWordAt(j)->phonemesSize();
		}
	}
	// now divide up the total time by phonemes
	int32 framesPerPhoneme = 1;
    if (audioDuration > 0 && phonemeCount > 0) {
		framesPerPhoneme = PG_ROUND((float)audioDuration / (float)phonemeCount);
		if (framesPerPhoneme < 1)
			framesPerPhoneme = 1;
	}

	// finally, assign frames based on phoneme durations
	int32 curFrame = 0;
    for (int32 i = 0; i < phrases.size(); i++) {
        LipsyncPhrase *phrase = phrases[i];
        for (int32 j = 0; j < phrase->wordsSize(); j++) {
            LipsyncWord *word = phrase->getWordAt(j);
            for (int32 k = 0; k < word->phonemesSize(); k++) {
                LipsyncPhoneme *phoneme = word->getPhonemeAt(k);
                phoneme->setFrame(curFrame);
				curFrame += framesPerPhoneme;
			} // for k
            if (word->phonemesSize() == 0) { // deal with unknown words
                word->setStartFrame(curFrame);
                word->setEndFrame(curFrame + 3);
				curFrame += 4;
            } else {
                word->setStartFrame(word->getPhonemeAt(0)->getFrame());
                word->setEndFrame(word->getLastPhoneme()->getFrame() + framesPerPhoneme - 1);
			}
		} // for j

        phrase->setStartFrame(phrase->getWordAt(0)->getStartFrame());
        phrase->setEndFrame(phrase->getLastWord()->getEndFrame());
	} // for i
}

void LipsyncVoice::repositionPhrase(LipsyncPhrase *phrase, int32 audioDuration)
{
    int id = phrases.indexOf(phrase);

    if ((id > 0) && (phrase->getStartFrame() < phrases[id - 1]->getEndFrame() + 1)) {
        phrase->setStartFrame(phrases[id - 1]->getEndFrame() + 1);
        if (phrase->getEndFrame() < phrase->getStartFrame() + 1)
            phrase->setEndFrame(phrase->getStartFrame() + 1);
	}

    if ((id < phrases.size() - 1) && (phrase->getEndFrame() > phrases[id + 1]->getStartFrame() - 1)) {
        phrase->setEndFrame(phrases[id + 1]->getStartFrame() - 1);
        if (phrase->getStartFrame() > phrase->getEndFrame() - 1)
            phrase->setStartFrame(phrase->getEndFrame() - 1);
	}

    if (phrase->getStartFrame() < 0)
        phrase->setStartFrame(0);

    if (phrase->getEndFrame() > audioDuration)
        phrase->setEndFrame(audioDuration);

    if (phrase->getStartFrame() > phrase->getEndFrame() - 1)
        phrase->setStartFrame(phrase->getEndFrame() - 1);

	// for first-guess frame alignment, count how many phonemes we have
    int32 frameDuration = phrase->getEndFrame() - phrase->getStartFrame() + 1;
	int32 phonemeCount = 0;
    for (int32 i = 0; i < phrase->wordsSize(); i++) {
        LipsyncWord *word = phrase->getWordAt(i);
        if (word->phonemesSize() == 0) // deal with unknown words
			phonemeCount += 4;
		else
            phonemeCount += word->phonemesSize();
	}

	// now divide up the total time by phonemes
	float framesPerPhoneme = 1.0f;
    if (frameDuration > 0 && phonemeCount > 0) {
		framesPerPhoneme = (float)frameDuration / (float)phonemeCount;
		if (framesPerPhoneme < 1.0f)
			framesPerPhoneme = 1.0f;
	}

	// finally, assign frames based on phoneme durations
    float curFrame = phrase->getStartFrame();
    for (int32 i = 0; i < phrase->wordsSize(); i++) {
        LipsyncWord *word = phrase->getWordAt(i);
        for (int32 j = 0; j < word->phonemesSize(); j++) {
            word->getPhonemeAt(j)->setFrame(PG_ROUND(curFrame));
			curFrame += framesPerPhoneme;
		}

        if (word->phonemesSize() == 0) { // deal with unknown words
            word->setStartFrame(PG_ROUND(curFrame));
            word->setEndFrame(word->getStartFrame() + 3);
			curFrame += 4.0f;
        } else {
            word->setStartFrame(word->getPhonemeAt(0)->getFrame());
            word->setEndFrame(word->getLastPhoneme()->getFrame() + PG_ROUND(framesPerPhoneme) - 1);
		}
		phrase->repositionWord(word);
	}
}

QString LipsyncVoice::getPhonemeAtFrame(int32 frame)
{
    for (int32 i = 0; i < phrases.size(); i++) {
        LipsyncPhrase *phrase = phrases[i];
        if (frame >= phrase->getStartFrame() && frame <= phrase->getEndFrame()) {
            // we found the phrase that contains this frame
            for (int32 j = 0; j < phrase->wordsSize(); j++) {
                LipsyncWord *word = phrase->getWordAt(j);
                if (frame >= word->getStartFrame() && frame <= word->getEndFrame()) { // we found the word that contains this frame
                    if (word->phonemesSize() > 0) {
                        for (int32 k = word->phonemesSize() - 1; k >= 0; k--) {
                            if (frame >= word->getPhonemeAt(k)->getFrame()) {
                                return word->getPhonemeAt(k)->getText();
							}
						}
                    } else { // volume-based breakdown
						return "";
					}
				}
			}
		}
	}

	return "rest";
}

bool LipsyncVoice::textIsEmpty()
{
    return text.isEmpty();
}

void LipsyncVoice::cleanPhrases()
{
    while (!phrases.isEmpty())
        delete phrases.takeFirst();

    phrases = QList<LipsyncPhrase *>();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

QList<QString> TupLipsyncDoc::phonemesList;
QHash<QString, QString> TupLipsyncDoc::dictionaryToPhonemeMap;
QHash<QString, QStringList>	TupLipsyncDoc::phonemeDictionary;

TupLipsyncDoc::TupLipsyncDoc()
{
    dirty = false;
    fps = 24;
    audioDuration = 0;
    audioPlayer = nullptr;
    audioExtractor = nullptr;
    maxAmplitude = 1.0f;
    currentVoice = nullptr;
}

TupLipsyncDoc::~TupLipsyncDoc()
{
    if (audioPlayer) {
        audioPlayer->stop();
        delete audioPlayer;
        audioPlayer = nullptr;
	}

    if (audioExtractor) {
        delete audioExtractor;
        audioExtractor = nullptr;
	}

    cleanVoices();
}

void TupLipsyncDoc::loadDictionaries()
{
	if (phonemeDictionary.size() > 0)
		return;

    QFile *file;
    file = new QFile(SHARE_DIR + "data/dictionaries/standard_dictionary");
    if (file->open(QIODevice::ReadOnly | QIODevice::Text)) {
        loadDictionary(file);
        file->close();
	}
    delete file;

    file = new QFile(SHARE_DIR + "data/dictionaries/extended_dictionary");
    if (file->open(QIODevice::ReadOnly | QIODevice::Text)) {
        loadDictionary(file);
        file->close();
	}
    delete file;

    file = new QFile(SHARE_DIR + "data/dictionaries/user_dictionary");
    if (file->open(QIODevice::ReadOnly | QIODevice::Text)) {
        loadDictionary(file);
        file->close();
	}
    delete file;

    file = new QFile(SHARE_DIR + "data/dictionaries/phoneme_mapping");
    if (file->open(QIODevice::ReadOnly | QIODevice::Text)) {
        while (!file->atEnd()) {
            QString line = file->readLine();
			line = line.trimmed();

            if (line.isEmpty()) {
                continue; // skip comments
            } else if (line.at(0) == "#" || line.length() == 0) {
				continue; // skip comments
            }

			QStringList strList = line.split(' ', Qt::SkipEmptyParts);
            if (strList.size() > 1) {
				if (strList[0] == ".")
					phonemesList << strList.at(1);
				else
					dictionaryToPhonemeMap.insert(strList.at(0), strList.at(1));
			}
		}
        file->close();
	}
    delete file;
}

void TupLipsyncDoc::loadDictionary(QFile *file)
{
    while (!file->atEnd()) {
        QString line = file->readLine();
		line = line.trimmed();
        if (line.isEmpty()) {
            continue;
        } else if (line.at(0) == "#") {
            continue; // skip comments
        }

		QStringList strList = line.split(' ', Qt::SkipEmptyParts);
        if (strList.size() > 1) {
			if (!phonemeDictionary.contains(strList.at(0)))
				phonemeDictionary.insert(strList.at(0), strList);
		}
	}
}

void TupLipsyncDoc::open(const QString &path)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupLipsyncDoc::open()]";
    #endif

    QFile *file;
    QString str;
    QString tempPath;
    int32 numVoices;

    file = new QFile(path);
    if (!file->open(QIODevice::ReadOnly | QIODevice::Text)) {
        #ifdef TUP_DEBUG
            qDebug() << "[TupLipsyncDoc::open()] - Fatal Error: can't open file -> " << path;
        #endif
        file->close();
        delete file;
		return;
	}

    if (audioPlayer) {
        audioPlayer->stop();
        delete audioPlayer;
        audioPlayer = nullptr;
	}

    if (audioExtractor) {
        delete audioExtractor;
        audioExtractor = nullptr;
	}

    cleanVoices();
    currentVoice = nullptr;

    QTextStream in(file);
    filePath = path;
	str = in.readLine(); // discard the header
	tempPath = in.readLine().trimmed();
	QFileInfo audioFileInfo(tempPath);
    if (!audioFileInfo.isAbsolute()) {
		QFileInfo fileInfo(path);
		QDir dir = fileInfo.absoluteDir();
		tempPath = dir.absoluteFilePath(tempPath);
	}
    audioPath = tempPath;

    fps = in.readLine().toInt();
    fps = PG_CLAMP(fps, 1, 120);
    audioDuration = in.readLine().toInt();

	numVoices = in.readLine().toInt();
    for (int i = 0; i < numVoices; i++) {
        LipsyncVoice *voice = new LipsyncVoice("");
		voice->open(in);
        voices << voice;
	}

    file->close();
    delete file;
    openAudio(audioPath);
    if (voices.size() > 0)
        currentVoice = voices[0];

    dirty = false;
}

void TupLipsyncDoc::openAudio(const QString &path)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupLipsyncDoc::openAudio()] - Loading audio file -> " << path;
    #endif

    dirty = true;
    maxAmplitude = 1.0f;

    if (audioPlayer) {
        audioPlayer->stop();
        delete audioPlayer;
        audioPlayer = nullptr;
	}

    if (audioExtractor) {
        delete audioExtractor;
        audioExtractor = nullptr;
	}

    audioPath = path;
    audioPlayer = new QMediaPlayer;
    audioPlayer->setMedia(QUrl::fromLocalFile(audioPath));
    if (audioPlayer->error()) {
        #ifdef TUP_DEBUG
            qDebug() << "[TupLipsyncDoc::openAudio()] - Fatal Error: Can't open audio -> " << path;
            qDebug() << "[TupLipsyncDoc::openAudio()] - Error Output -> " << audioPlayer->errorString();
        #endif
        delete audioPlayer;
        audioPlayer = nullptr;
    } else {
        #ifdef TUP_DEBUG
            qDebug() << "[TupLipsyncDoc::openAudio()] - Audio file loaded successful!";
        #endif
        fps = 24;
        audioExtractor = new TupAudioExtractor(path.toUtf8().data());
        if (audioExtractor->isValid()) {
            real frames = audioExtractor->duration() * fps;
            audioDuration = PG_ROUND(frames);
            maxAmplitude = 0.001f;
			real time = 0.0f, sampleDur = 1.0f / 24.0f;
            while (time < audioExtractor->duration()) {
                real amp = audioExtractor->getRMSAmplitude(time, sampleDur);
                if (amp > maxAmplitude)
                    maxAmplitude = amp;
				time += sampleDur;
			}
        } else {
            #ifdef TUP_DEBUG
                qDebug() << "[TupLipsyncDoc::openAudio()] - Fatal Error: Audio extractor failed!";
            #endif
            delete audioExtractor;
            audioExtractor = nullptr;
		}
	}

    if (voices.size() == 0) {
        currentVoice = new LipsyncVoice(tr("Voice 1"));
        voices << currentVoice;
	}
}

bool TupLipsyncDoc::save()
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupLipsyncDoc::save()] - filePath -> " << filePath;
    #endif

    if (filePath.isEmpty()) {
        #ifdef TUP_DEBUG
            qDebug() << "[TupLipsyncDoc::save()] - Fatal Error: filePath is unset!";
        #endif
        return false;
    }

    QFile *file;
    file = new QFile(filePath);
    if (!file->open(QIODevice::WriteOnly | QIODevice::Text)) {
        #ifdef TUP_DEBUG
            qDebug() << "[TupLipsyncDoc::save()] - Fatal Error: Can't write PGO file!";
        #endif

        file->close();
        delete file;
        return false;
	}

    if (audioExtractor && audioExtractor->isValid()) {
        real frames = audioExtractor->duration() * fps;
        audioDuration = PG_ROUND(frames);
	}

    QTextStream out(file);
	out << "lipsync version 1" << Qt::endl;

    QFileInfo docInfo(filePath);
    QFileInfo audioInfo(audioPath);
    QString path = audioPath;
    if (audioInfo.absoluteDir() == docInfo.absoluteDir())
        path = audioInfo.fileName();

    out << path << Qt::endl;
    out << fps << Qt::endl;
    out << audioDuration << Qt::endl;

    out << voices.size() << Qt::endl;
    for (int i = 0; i < voices.size(); i++)
        voices[i]->save(out);

    file->close();
    delete file;
    dirty = false;

    return true;
}

void TupLipsyncDoc::setFps(int32 fps)
{
    this->fps = fps;
    dirty = true;

    if (audioExtractor && audioExtractor->isValid()) {
        real duration = audioExtractor->duration() * fps;
        audioDuration = PG_ROUND(duration);
	}
}

QMediaPlayer *TupLipsyncDoc::getAudioPlayer()
{
    return audioPlayer;
}

TupAudioExtractor* TupLipsyncDoc::getAudioExtractor()
{
    return audioExtractor;
}

QString TupLipsyncDoc::getVolumePhonemeAtFrame(int32 frame)
{
    if (!audioExtractor)
		return "rest";

    real amp = audioExtractor->getRMSAmplitude((real)frame / (real)fps, 1.0f / (real)fps);
    amp /= maxAmplitude;
	amp *= 4.0f;
	int32 volID = PG_ROUND(amp);
	volID = PG_CLAMP(volID, 0, 4);

	// new method - use a fixed set of phonemes for this method:
	// rest, etc, E, L, AI
	// presumably, these will vary from more closed to more open
	// the benefit of this is that the same mouths can be used for amplitude-based lipsync as well as proper lipsync
    switch (volID) {
		case 0:
			return "rest";
		case 1:
			return "etc";
		case 2:
			return "E";
		case 3:
			return "L";
		case 4:
			return "AI";
	}
	return "rest";
}

LipsyncVoice* TupLipsyncDoc::getCurrentVoice()
{
    return currentVoice;
}

void TupLipsyncDoc::setCurrentVoice(LipsyncVoice *voice)
{
    currentVoice = voice;
}

QList<LipsyncVoice *> TupLipsyncDoc::getVoices()
{
    return voices;
}

LipsyncVoice* TupLipsyncDoc::getVoiceAt(int index)
{
    return voices.at(index);
}

void TupLipsyncDoc::appendVoice(LipsyncVoice *voice)
{
    voices.append(voice);
}

void TupLipsyncDoc::cleanVoices()
{
    while (!voices.isEmpty())
        delete voices.takeFirst();
}

void TupLipsyncDoc::setFilePath(const QString &path)
{
    filePath = path;
}

QString TupLipsyncDoc::getFilePath() const
{
    return filePath;
}

bool TupLipsyncDoc::isModified()
{
    return dirty;
}

void TupLipsyncDoc::setModifiedFlag(bool flag)
{
    dirty = flag;
}

void TupLipsyncDoc::removeVoiceAt(int index)
{
    voices.removeAt(index);
}

QString TupLipsyncDoc::getPhonemeFromDictionary(const QString &key, const QString &defaultValue)
{
    return dictionaryToPhonemeMap.value(key, defaultValue);
}

QStringList TupLipsyncDoc::getDictionaryValue(const QString &key)
{
    return phonemeDictionary.value(key);
}

int TupLipsyncDoc::phonemesListSize()
{
    return phonemesList.size();
}

QString TupLipsyncDoc::getPhonemeAt(int index)
{
    return phonemesList.at(index);
}

void TupLipsyncDoc::playAudio()
{
    audioPlayer->play();
}

void TupLipsyncDoc::pauseAudio()
{
    audioPlayer->pause();
}

void TupLipsyncDoc::stopAudio()
{
    audioPlayer->stop();
}

void TupLipsyncDoc::runBreakdown(const QString &lang, int32 duration)
{
    if (currentVoice)
        currentVoice->runBreakdown(lang, duration);
}

bool TupLipsyncDoc::voiceTextIsEmpty()
{
    if (currentVoice)
        return currentVoice->textIsEmpty();

    return true;
}

void TupLipsyncDoc::setVoiceText(const QString &text)
{
    if (currentVoice)
        currentVoice->setText(text);
}

QString TupLipsyncDoc::getVoiceText() const
{
    if (currentVoice)
        return currentVoice->getText();

    return "";
}

QString TupLipsyncDoc::getPhonemeAtFrame(int frame) const
{
    if (currentVoice)
        return currentVoice->getPhonemeAtFrame(frame);

    return "";
}

LipsyncPhrase* TupLipsyncDoc::getPhraseAt(int index)
{
    if (currentVoice)
        return currentVoice->getPhraseAt(index);

    return nullptr;
}

LipsyncPhrase * TupLipsyncDoc::getFirstPhrase()
{
    if (currentVoice)
        return currentVoice->getFirstPhrase();

    return nullptr;
}

int TupLipsyncDoc::getPhrasesTotal()
{
    if (currentVoice)
        return currentVoice->getPhrasesTotal();

    return 0;
}

int TupLipsyncDoc::getStartFrameFromPhraseAt(int index)
{
    if (currentVoice)
        return currentVoice->getPhraseStartFrame(index);

    return 0;
}

int TupLipsyncDoc::getEndFrameFromPhraseAt(int index)
{
    if (currentVoice)
        return currentVoice->getPhraseEndFrame(index);

    return 0;
}

void TupLipsyncDoc::repositionPhrase(LipsyncPhrase *phrase)
{
    if (currentVoice)
        currentVoice->repositionPhrase(phrase, audioDuration);
}

QList<LipsyncWord *> TupLipsyncDoc::getWords()
{
    QList<LipsyncWord *> words;
    if (currentVoice) {
        LipsyncPhrase *phrase = getFirstPhrase();
        if (phrase)
            words = phrase->getWords();
    }

    return words;
}

void TupLipsyncDoc::cleanPhrases()
{
    if (currentVoice)
        currentVoice->cleanPhrases();
}

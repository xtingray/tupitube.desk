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

#include "tuplipsyncdoc.h"

#include <QFile>
#include <QFileInfo>
#include <QDir>

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

/***/

LipsyncWord::LipsyncWord()
{
    text = "";
    startFrame = 0;
    endFrame = 0;
    top = 0;
    bottom = 0;

    phonemes = QList<LipsyncPhoneme *>();
}

LipsyncWord::~LipsyncWord()
{
    clearPhonemes();
}

void LipsyncWord::runBreakdown(const QString &lang, TupLipsyncDictionary *lipsyncDictionary)
{
   #ifdef TUP_DEBUG
       qDebug() << "[LipsyncWord::runBreakdown()] - lang -> " << lang;
   #endif

    clearPhonemes();

    QString msg = text;
    msg.remove(QRegExp("[.,!?;-/()¿]"));
    if (lang == "en") {
        QStringList	pronunciation;
        pronunciation << lipsyncDictionary->getDictionaryValue(msg.toUpper());
        if (pronunciation.size() > 1) {
            for (int32 i = 1; i < pronunciation.size(); i++) {
				QString p = pronunciation.at(i);
                if (p.length() == 0)
					continue;
				LipsyncPhoneme *phoneme = new LipsyncPhoneme;
                phoneme->setText(lipsyncDictionary->getPhonemeFromDictionary(p, "etc"));
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
    if (!phonemes.isEmpty() && index >= 0) {
        if (phonemes.size() > index)
            return phonemes.at(index);
    }

    return nullptr;
}

LipsyncPhoneme * LipsyncWord::getLastPhoneme()
{
    if (!phonemes.isEmpty())
        return phonemes.last();

    return nullptr;
}

void LipsyncWord::removeFirstPhoneme()
{
    if (!phonemes.isEmpty())
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

void LipsyncWord::clearPhonemes()
{
    while (!phonemes.isEmpty())
        delete phonemes.takeFirst();
}

int LipsyncWord::length()
{
    return text.length();
}

/***/

LipsyncPhrase::LipsyncPhrase()
{
    text = "";
    startFrame = 0;
    endFrame = 0;
    top = bottom = 0;

    words = QList<LipsyncWord *>();
}

LipsyncPhrase::~LipsyncPhrase()
{
    clearWords();
}

void LipsyncPhrase::runBreakdown(QString lang, TupLipsyncDictionary *lipsyncDictionary)
{
    #ifdef TUP_DEBUG
        qDebug() << "[LipsyncPhrase::runBreakdown()] - lang -> " << lang;
    #endif

	// break phrase into words
    clearWords();

    QStringList strList = text.split(' ', Qt::SkipEmptyParts);
    for (int32 i = 0; i < strList.size(); i++) {
        QString input = strList.at(i);
        input.remove(QRegExp("[.,!?;-/()¿]"));
        if (input.length() == 0)
			continue;

		LipsyncWord *word = new LipsyncWord;
        word->setText(input);
        words << word;
	}

	// now break down the words
    for (int32 i = 0; i < words.size(); i++)
        words[i]->runBreakdown(lang, lipsyncDictionary);
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

bool LipsyncPhrase::isTextEmpty()
{
    return text.isEmpty();
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
    if (!words.isEmpty() && index >=0) {
        if (words.size() > index)
            return words.at(index);
    }

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

void LipsyncPhrase::clearWords()
{
    while (!words.isEmpty())
        delete words.takeFirst();
}

/***/

LipsyncVoice::LipsyncVoice(const QString &name)
{
    this->name = name;
    phrase = new LipsyncPhrase;
}

LipsyncVoice::~LipsyncVoice()
{
    clearPhrase();
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
    if (phrase) {
        this->text = text;
        phrase->setText(text);
    }
}

QString LipsyncVoice::getText() const
{
    if (phrase)
        return phrase->getText();

    return "";
}

bool LipsyncVoice::isTextEmpty()
{
    if (phrase)
        return phrase->isTextEmpty();

    return true;
}

void LipsyncVoice::setPhrase(LipsyncPhrase *phrase)
{
    this->phrase = phrase;
}

LipsyncPhrase * LipsyncVoice::getPhrase()
{
    return phrase;
}

int LipsyncVoice::getPhraseStartFrame()
{
    if (phrase)
        return phrase->getStartFrame();

    return 0;
}

int LipsyncVoice::getPhraseEndFrame()
{
    if (phrase)
        return phrase->getEndFrame();

    return 0;
}

void LipsyncVoice::open(QTextStream &in)
{    
    int32 numWords;
    int32 numPhonemes = 0;
    QString str;

    name = in.readLine().trimmed();
    text = in.readLine().trimmed();
    text = text.split('|').join('\n');

    #ifdef TUP_DEBUG
        qDebug() << "[LipsyncVoice::open()] - voice name -> " << name;
        qDebug() << "[LipsyncVoice::open()] - voice text -> " << text;
    #endif

    LipsyncPhrase *phrase = new LipsyncPhrase;
    phrase->setText(in.readLine().trimmed());
    phrase->setStartFrame(in.readLine().toInt());
    phrase->setEndFrame(in.readLine().toInt());
    numWords = in.readLine().toInt();

    #ifdef TUP_DEBUG
        qDebug() << "[LipsyncVoice::open()] - numWords -> " << numWords;
    #endif

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

    this->phrase = phrase;
}

void LipsyncVoice::save(QTextStream &out)
{
    #ifdef TUP_DEBUG
        qDebug() << "[LipsyncVoice::save()]";
    #endif

    out << '\t' << name << Qt::endl;
    out << '\t' << text.split('\n').join('|') << Qt::endl;
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
}

void LipsyncVoice::runBreakdown(QString lang, TupLipsyncDictionary *lipsyncDictionary, int32 audioDuration)
{
    #ifdef TUP_DEBUG
        qDebug() << "[LipsyncVoice::runBreakdown()] - text -> " << text;
    #endif

    if (text.isEmpty()) {
        return;
    }

	// make sure there is a space after all punctuation marks
	QString punctuation = ".,!?;";
	bool repeatLoop = true;
    while (repeatLoop) {
		repeatLoop = false;
        int32 n = text.length();
        for (int32 i = 0; i < n - 1; i++) {
            if (punctuation.contains(text[i]) && !text[i + 1].isSpace()) {
                text.insert(i + 1, ' ');
				repeatLoop = true;
				break;
			}
		}
	}

	// break text into phrases
    clearPhrase();

    QStringList strList = text.split('\n', Qt::SkipEmptyParts);
    for (int32 i = 0; i < strList.size(); i++) {
		if (strList.at(i).length() == 0)
			continue;

        phrase = new LipsyncPhrase;
        phrase->setText(strList.at(i));
	}

    phrase->runBreakdown(lang, lipsyncDictionary);

	// for first-guess frame alignment, count how many phonemes we have
	int32 phonemeCount = 0;
    for (int32 j = 0; j < phrase->wordsSize(); j++) {
        LipsyncWord *word = phrase->getWordAt(j);
        if (word->phonemesSize() == 0) // deal with unknown words
            phonemeCount += (word->length()/2);
        else
            phonemeCount += word->phonemesSize();
    }

	// now divide up the total time by phonemes
	int32 framesPerPhoneme = 1;
    if (audioDuration > 0 && phonemeCount > 0) {
		framesPerPhoneme = PG_ROUND((float)audioDuration / (float)phonemeCount);
		if (framesPerPhoneme < 1)
			framesPerPhoneme = 1;
	}

	// finally, assign frames based on phoneme durations
    int32 currentFrame = 0;
    for (int32 j = 0; j < phrase->wordsSize(); j++) {
        LipsyncWord *word = phrase->getWordAt(j);
        for (int32 k = 0; k < word->phonemesSize(); k++) {
            LipsyncPhoneme *phoneme = word->getPhonemeAt(k);
            phoneme->setFrame(currentFrame);
            currentFrame += framesPerPhoneme;
        } // for k

        if (word->phonemesSize() == 0) { // deal with unknown words
            word->setStartFrame(currentFrame);
            word->setEndFrame(currentFrame + word->length());
            currentFrame += word->length();
        } else {
            word->setStartFrame(word->getPhonemeAt(0)->getFrame());
            word->setEndFrame(word->getLastPhoneme()->getFrame() + framesPerPhoneme - 1);
        }
    } // for j

    phrase->setStartFrame(phrase->getWordAt(0)->getStartFrame());
    phrase->setEndFrame(phrase->getLastWord()->getEndFrame());
}

void LipsyncVoice::repositionPhrase(LipsyncPhrase *phrase, int32 audioDuration)
{
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
        if (word->phonemesSize() == 0) { // deal with unknown words
            phonemeCount += (word->length()/2);
        } else {
            phonemeCount += word->phonemesSize();
        }
	}

	// now divide up the total time by phonemes
	float framesPerPhoneme = 1.0f;
    if (frameDuration > 0 && phonemeCount > 0) {
		framesPerPhoneme = (float)frameDuration / (float)phonemeCount;
		if (framesPerPhoneme < 1.0f)
			framesPerPhoneme = 1.0f;
	}

	// finally, assign frames based on phoneme durations
    float currentFrame = phrase->getStartFrame();
    for (int32 i = 0; i < phrase->wordsSize(); i++) {
        LipsyncWord *word = phrase->getWordAt(i);
        for (int32 j = 0; j < word->phonemesSize(); j++) {
            word->getPhonemeAt(j)->setFrame(PG_ROUND(currentFrame));
            currentFrame += framesPerPhoneme;
		}

        if (word->phonemesSize() == 0) { // deal with unknown words
            word->setStartFrame(PG_ROUND(currentFrame));
            word->setEndFrame(word->getStartFrame() + word->length());
            currentFrame += word->length();
        } else {
            word->setStartFrame(word->getPhonemeAt(0)->getFrame());
            word->setEndFrame(word->getLastPhoneme()->getFrame() + PG_ROUND(framesPerPhoneme) - 1);
		}
		phrase->repositionWord(word);
	}
}

QString LipsyncVoice::getPhonemeAtFrame(int32 frame)
{
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

	return "rest";
}

bool LipsyncVoice::textIsEmpty()
{
    return text.isEmpty();
}

void LipsyncVoice::clearPhrase()
{
    phrase->setText("");
    phrase->clearWords();
}

/***/

TupLipsyncDictionary::TupLipsyncDictionary()
{
    loadDictionaries();
}

TupLipsyncDictionary::~TupLipsyncDictionary()
{
}

void TupLipsyncDictionary::loadDictionaries()
{
    if (phonemeDictionary.size() > 0)
        return;

#ifdef Q_OS_WIN
    QString dictPath = SHARE_DIR + "dictionaries/";
#else
    QString dictPath = SHARE_DIR + "data/dictionaries/";
#endif

    QFile *file = new QFile(dictPath + "standard_dictionary");
    if (file->open(QIODevice::ReadOnly | QIODevice::Text)) {
        loadDictionary(file);
        file->close();
    }
    delete file;

    file = new QFile(dictPath + "extended_dictionary");
    if (file->open(QIODevice::ReadOnly | QIODevice::Text)) {
        loadDictionary(file);
        file->close();
    }
    delete file;

    file = new QFile(dictPath + "user_dictionary");
    if (file->open(QIODevice::ReadOnly | QIODevice::Text)) {
        loadDictionary(file);
        file->close();
    }
    delete file;

    file = new QFile(dictPath + "phoneme_mapping");
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

void TupLipsyncDictionary::loadDictionary(QFile *file)
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

QString TupLipsyncDictionary::getPhonemeFromDictionary(const QString &key, const QString &defaultValue)
{
    return dictionaryToPhonemeMap.value(key, defaultValue);
}

QStringList TupLipsyncDictionary::getDictionaryValue(const QString &key)
{
    return phonemeDictionary.value(key);
}

int TupLipsyncDictionary::phonemesListSize()
{
    return phonemesList.size();
}

QString TupLipsyncDictionary::getPhonemeAt(int index)
{
    return phonemesList.at(index);
}

/***/

TupLipsyncDoc::TupLipsyncDoc()
{
    projectHasChanged = false;
    fps = 24;
    audioDuration = 0;
    audioPlayer = nullptr;
    audioExtractor = nullptr;
    maxAmplitude = 1.0f;
    voice = nullptr;

    lipsyncDictionary = new TupLipsyncDictionary;
}

TupLipsyncDoc::~TupLipsyncDoc()
{
    releaseAudioPlayer();

    if (audioExtractor) {
        delete audioExtractor;
        audioExtractor = nullptr;
	}

    clearVoice();
}

void TupLipsyncDoc::releaseAudioPlayer()
{
    if (audioPlayer) {
        audioPlayer->stop();
        audioPlayer->setMedia(QMediaContent());

        delete audioPlayer;
        audioPlayer = nullptr;
    }
}

TupLipsyncDictionary * TupLipsyncDoc::getDictionary()
{
    return lipsyncDictionary;
}

void TupLipsyncDoc::openPGOFile(const QString &pgoPath, const QString &audioPath, int fps)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupLipsyncDoc::openPGOFile()] - pgoPath -> " << pgoPath;
        qDebug() << "[TupLipsyncDoc::openPGOFile()] - audioPath -> " << audioPath;
        qDebug() << "[TupLipsyncDoc::openPGOFile()] - fps -> " << fps;
    #endif

    QFile *pgoFile;
    QString str;

    pgoFile = new QFile(pgoPath);
    if (!pgoFile->open(QIODevice::ReadOnly | QIODevice::Text)) {
        #ifdef TUP_DEBUG
            qDebug() << "[TupLipsyncDoc::openPGOFile()] - Fatal Error: can't open file -> " << pgoPath;
        #endif
        pgoFile->close();
        delete pgoFile;

		return;
	}
    pgoFilePath = pgoPath;

    if (audioPlayer) {
        audioPlayer->stop();
        delete audioPlayer;
        audioPlayer = nullptr;
	}

    if (audioExtractor) {
        delete audioExtractor;
        audioExtractor = nullptr;
	}

    clearVoice();
    voice = nullptr;

    QTextStream in(pgoFile);
	str = in.readLine(); // discard the header

    this->audioPath = audioPath;
    this->fps = fps;
    this->fps = PG_CLAMP(this->fps, 1, 120);

    audioDuration = in.readLine().toInt();

    voice = new LipsyncVoice("");
    voice->open(in);

    pgoFile->close();
    delete pgoFile;
    openAudioFile(audioPath);

    projectHasChanged = false;
}

void TupLipsyncDoc::openAudioFile(const QString &path)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupLipsyncDoc::openAudioFile()] - Loading audio file -> " << path;
    #endif

    maxAmplitude = 1.0f;

    if (audioPlayer) {
        audioPlayer->stop();
        audioPlayer->setMedia(QMediaContent());

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
            qDebug() << "[TupLipsyncDoc::openAudioFile()] - Fatal Error: Can't open audio -> " << path;
            qDebug() << "[TupLipsyncDoc::openAudioFile()] - Error Output -> " << audioPlayer->errorString();
        #endif
        delete audioPlayer;
        audioPlayer = nullptr;
    } else {
        #ifdef TUP_DEBUG
            qDebug() << "[TupLipsyncDoc::openAudioFile()] - Audio file loaded successful!";
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
                qDebug() << "[TupLipsyncDoc::openAudioFile()] - Fatal Error: Audio extractor failed!";
            #endif
            delete audioExtractor;
            audioExtractor = nullptr;
		}
	}

    if (!voice)
        voice = new LipsyncVoice(tr("Voice 1"));
}

bool TupLipsyncDoc::save()
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupLipsyncDoc::save()] - filePath -> " << pgoFilePath;
    #endif

    if (pgoFilePath.isEmpty()) {
        #ifdef TUP_DEBUG
            qDebug() << "[TupLipsyncDoc::save()] - Fatal Error: filePath is unset!";
        #endif

        return false;
    }

    QFile *file;
    file = new QFile(pgoFilePath);
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
    out << audioDuration << Qt::endl;

    voice->save(out);

    file->close();
    delete file;
    projectHasChanged = false;

    #ifdef TUP_DEBUG
        qDebug() << "[TupLipsyncDoc::save()] - File saved successfully! - pgoFilePath -> " << pgoFilePath;
    #endif

    return true;
}

void TupLipsyncDoc::setFps(int32 fps)
{
    this->fps = fps;
    projectHasChanged = true;

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

LipsyncVoice* TupLipsyncDoc::getVoice()
{
    return voice;
}

void TupLipsyncDoc::setVoice(LipsyncVoice *voice)
{
    this->voice = voice;
}

void TupLipsyncDoc::clearVoice()
{
    if (voice)
        voice->clearPhrase();

    delete voice;
}

void TupLipsyncDoc::setPGOFilePath(const QString &path)
{
    pgoFilePath = path;
}

QString TupLipsyncDoc::getPGOFilePath() const
{
    return pgoFilePath;
}

bool TupLipsyncDoc::isModified()
{
    return projectHasChanged;
}

void TupLipsyncDoc::setModifiedFlag(bool flag)
{
    projectHasChanged = flag;
}

QString TupLipsyncDoc::getPhonemeFromDictionary(const QString &key, const QString &defaultValue)
{
    return lipsyncDictionary->getPhonemeFromDictionary(key, defaultValue);
}

QStringList TupLipsyncDoc::getDictionaryValue(const QString &key)
{
    return lipsyncDictionary->getDictionaryValue(key);
}

int TupLipsyncDoc::phonemesListSize()
{
    return lipsyncDictionary->phonemesListSize();
}

QString TupLipsyncDoc::getPhonemeAt(int index)
{
    return lipsyncDictionary->getPhonemeAt(index);
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
    if (voice)
        voice->runBreakdown(lang, lipsyncDictionary, duration);
}

bool TupLipsyncDoc::voiceTextIsEmpty()
{
    if (voice)
        return voice->textIsEmpty();

    return true;
}

void TupLipsyncDoc::setVoiceName(const QString &name)
{
    if (voice)
        voice->setName(name);
}

QString TupLipsyncDoc::getVoiceName() const
{
    if (voice)
        return voice->getName();

    return "";
}

void TupLipsyncDoc::setVoiceText(const QString &text)
{
    if (voice)
        voice->setText(text);
}

QString TupLipsyncDoc::getVoiceText() const
{
    if (voice)
        return voice->getText();

    return "";
}

QString TupLipsyncDoc::getPhonemeAtFrame(int frame) const
{
    if (voice)
        return voice->getPhonemeAtFrame(frame);

    return "";
}

LipsyncPhrase * TupLipsyncDoc::getPhrase()
{
    if (voice)
        return voice->getPhrase();

    return nullptr;
}

int TupLipsyncDoc::getStartFrameFromPhrase()
{
    if (voice)
        return voice->getPhraseStartFrame();

    return 0;
}

int TupLipsyncDoc::getEndFrameFromPhrase()
{
    if (voice)
        return voice->getPhraseEndFrame();

    return 0;
}

void TupLipsyncDoc::repositionPhrase(LipsyncPhrase *phrase)
{
    if (voice)
        voice->repositionPhrase(phrase, audioDuration);
}

QList<LipsyncWord *> TupLipsyncDoc::getWords()
{
    QList<LipsyncWord *> words;
    if (voice) {
        LipsyncPhrase *phrase = getPhrase();
        if (phrase)
            words = phrase->getWords();
    }

    return words;
}

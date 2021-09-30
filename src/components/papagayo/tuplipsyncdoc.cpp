#include <QFile>
#include <QFileInfo>
#include <QDir>

#include "tuplipsyncdoc.h"

LipsyncPhoneme::LipsyncPhoneme()
{
    text = "";
    frame = 0;
    top = bottom = 0;
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

void LipsyncWord::setStartFrame(int frameIndex)
{
    startFrame = frameIndex;
}

int LipsyncWord::getStartFrame() const
{
    return startFrame;
}

void LipsyncWord::setEndFrame(int frameIndex)
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

LipsyncPhoneme* LipsyncWord::getPhonemeAt(int index)
{
    return phonemes.at(index);
}

LipsyncPhoneme * LipsyncWord::getLastPhoneme()
{
    return phonemes.last();
}

void LipsyncWord::removeFirstPhoneme()
{
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
    while (!words.isEmpty())
        delete words.takeFirst();
}

void LipsyncPhrase::runBreakdown(QString language)
{
	// break phrase into words
    while (!words.isEmpty())
        delete words.takeFirst();

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

    for (int32 i = 0; i < phonemes.size(); i++) {
        word->repositionPhoneme(phonemes[i]);
	}
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
    return words.at(index);
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

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

LipsyncVoice::LipsyncVoice(const QString &name)
{
    this->name = name;
}

LipsyncVoice::~LipsyncVoice()
{
    while (!phrases.isEmpty())
        delete phrases.takeFirst();
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

QList<LipsyncPhrase *> LipsyncVoice::getPhrases()
{
    return phrases;
}

LipsyncPhrase* LipsyncVoice::getPhraseAt(int index)
{
    return phrases.at(index);
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
    while (!phrases.isEmpty())
        delete phrases.takeFirst();

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

    while (!voices.isEmpty())
        delete voices.takeFirst();
}

void TupLipsyncDoc::loadDictionaries()
{
	if (phonemeDictionary.size() > 0)
		return;

    QFile *file;
    file = new QFile(":/dictionaries/dictionaries/standard_dictionary");
    if (file->open(QIODevice::ReadOnly | QIODevice::Text)) {
        loadDictionary(file);
        file->close();
	}
    delete file;

    file = new QFile(":/dictionaries/dictionaries/extended_dictionary");
    if (file->open(QIODevice::ReadOnly | QIODevice::Text)) {
        loadDictionary(file);
        file->close();
	}
    delete file;

    file = new QFile(":/dictionaries/dictionaries/user_dictionary");
    if (file->open(QIODevice::ReadOnly | QIODevice::Text)) {
        loadDictionary(file);
        file->close();
	}
    delete file;

    file = new QFile(":/dictionaries/dictionaries/phoneme_mapping");
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

    while (!voices.isEmpty())
        delete voices.takeFirst();
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
    // connect(player, SIGNAL(positionChanged(qint64)), this, SLOT(positionChanged(qint64)));
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

void TupLipsyncDoc::save()
{
    if (filePath.isEmpty())
		return;

    QFile *file;
    file = new QFile(filePath);
    if (!file->open(QIODevice::WriteOnly | QIODevice::Text)) {
        file->close();
        delete file;
		return;
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
    if (audioInfo.absoluteDir() == docInfo.absoluteDir()) {
        path = audioInfo.fileName();
	}

    out << path << Qt::endl;
    out << fps << Qt::endl;
    out << audioDuration << Qt::endl;

    out << voices.size() << Qt::endl;
    for (int i = 0; i < voices.size(); i++) {
        voices[i]->save(out);
	}

    file->close();
    delete file;
    dirty = false;
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

void TupLipsyncDoc::setFilePath(const QString &path)
{
    filePath = path;
}

QString TupLipsyncDoc::getFilePath() const
{
    return filePath;
}

bool TupLipsyncDoc::isDirty()
{
    return dirty;
}

void TupLipsyncDoc::setDirtyFlag(bool flag)
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

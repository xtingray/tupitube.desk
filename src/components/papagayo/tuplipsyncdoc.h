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

#ifndef TUPLIPSYNCDOC_H
#define TUPLIPSYNCDOC_H

#include <QMediaPlayer>
#include <QFile>

#include "tglobal.h"
#include "tupaudioextractor.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class LipsyncPhoneme
{
    public:
        LipsyncPhoneme();
        ~LipsyncPhoneme();

        void setText(const QString string);
        QString getText() const;
        void setFrame(int frameIndex);
        int getFrame() const;

        int getTop();
        int getBottom();
        void setTop(int32 index);
        void setBottom(int32 index);

    private:
        QString text;
        int32 frame;
        int32 top, bottom;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class LipsyncWord
{
    public:
        LipsyncWord();
        ~LipsyncWord();

        void runBreakdown(QString language);
        void repositionPhoneme(LipsyncPhoneme *phoneme);

        void setText(const QString string);
        QString getText() const;

        void setStartFrame(int32 frameIndex);
        int getStartFrame() const;
        void setEndFrame(int32 frameIndex);
        int getEndFrame() const;

        QList<LipsyncPhoneme *>	getPhonemes();
        void addPhoneme(LipsyncPhoneme *phoneme);
        int phonemesSize();
        LipsyncPhoneme * getPhonemeAt(int32 index);
        LipsyncPhoneme * getLastPhoneme();
        void removeFirstPhoneme();
        void removePhonemes();
        int getFrameFromPhonemeAt(int32 index);
        QString getPhonemesString() const;

        int32 getTop();
        int32 getBottom();
        void setTop(int32 index);
        void setBottom(int32 index);

    private:
        QString text;
        int32 startFrame, endFrame;
        int32 top, bottom;
        QList<LipsyncPhoneme *>	phonemes;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class LipsyncPhrase
{
    public:
        LipsyncPhrase();
        ~LipsyncPhrase();

        void runBreakdown(QString language);
        void repositionWord(LipsyncWord *word);

        void setText(const QString string);
        QString getText() const;

        void setStartFrame(int frameIndex);
        int getStartFrame() const;
        void setEndFrame(int frameIndex);
        int getEndFrame() const;

        int32 getTop();
        int32 getBottom();
        void setTop(int32 index);
        void setBottom(int32 index);

        LipsyncWord * getWordAt(int index);
        void addWord(LipsyncWord* word);
        int wordsSize();
        QList<LipsyncWord *> getWords();
        LipsyncWord* getLastWord();
        int getStartFrameFromWordAt(int index);
        int getEndFrameFromWordAt(int index);

    private:
        void cleanWords();

        QString text;
        int32 startFrame, endFrame;
        int32 top, bottom;
        QList<LipsyncWord *> words;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class LipsyncVoice
{
    public:
        LipsyncVoice(const QString &name);
        ~LipsyncVoice();

        void open(QTextStream &in);
        void save(QTextStream &out);
        void exportVoice(QString path);
        void runBreakdown(QString language, int32 audioDuration);
        void repositionPhrase(LipsyncPhrase *phrase, int32 audioDuration);
        QString getPhonemeAtFrame(int32 frame);

        void setName(const QString &name);
        QString getName() const;
        void setText(const QString &text);
        QString getText() const;
        void addPhrase(LipsyncPhrase *phrase);
        QList<LipsyncPhrase *> getPhrases();
        LipsyncPhrase * getFirstPhrase();
        int getPhrasesTotal();
        LipsyncPhrase* getPhraseAt(int index);
        int getPhraseStartFrame(int index);
        int getPhraseEndFrame(int index);
        bool textIsEmpty();
        void cleanPhrases();

    private:
        QString name;
        QString	text;
        QList<LipsyncPhrase *> phrases;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class TUPITUBE_EXPORT TupLipsyncDoc : public QObject
{
	Q_OBJECT

    public:
        TupLipsyncDoc();
        ~TupLipsyncDoc();

        static void loadDictionaries();

        void open(const QString &path);
        void openAudio(const QString &path);
        bool save();
        void rebuildAudioSamples();

        void setFilePath(const QString &path);
        QString getFilePath() const;
        int32 getFps() { return fps; }
        void setFps(int32 fps);
        QMediaPlayer *getAudioPlayer();
        TupAudioExtractor *getAudioExtractor();
        int32 getDuration() { return audioDuration; }
        QString getVolumePhonemeAtFrame(int32 frame);
        LipsyncVoice *  getCurrentVoice();
        void setCurrentVoice(LipsyncVoice *voice);
        QList<LipsyncVoice *> getVoices();
        LipsyncVoice* getVoiceAt(int index);
        LipsyncPhrase * getFirstPhrase();
        void appendVoice(LipsyncVoice *voice);
        bool isModified();
        void setModifiedFlag(bool flag);
        void removeVoiceAt(int index);
        void runBreakdown(const QString &lang, int32 duration);
        bool voiceTextIsEmpty();
        void setVoiceText(const QString &text);
        QString getVoiceText() const;
        QList<LipsyncWord *> getWords();

        static QString getPhonemeFromDictionary(const QString &key, const QString &defaultValue);
        static QStringList getDictionaryValue(const QString &key);
        static int phonemesListSize();
        static QString getPhonemeAt(int index);
        QString getPhonemeAtFrame(int frame) const;
        LipsyncPhrase* getPhraseAt(int index);
        int getPhrasesTotal();
        int getStartFrameFromPhraseAt(int index);
        int getEndFrameFromPhraseAt(int index);
        void repositionPhrase(LipsyncPhrase *phrase);

        void playAudio();
        void pauseAudio();
        void stopAudio();
        void cleanVoices();
        void cleanPhrases();

    private:
        static void loadDictionary(QFile *file);

        int32 fps;
        int32 audioDuration;
        QString audioPath;
        QMediaPlayer *audioPlayer;
        TupAudioExtractor *audioExtractor;
        real maxAmplitude;

        QString filePath;
        bool dirty;
        QList<LipsyncVoice *> voices;
        LipsyncVoice *currentVoice;

        static QList<QString> phonemesList;
        static QHash<QString, QString> dictionaryToPhonemeMap;
        static QHash<QString, QStringList> phonemeDictionary;

	/*
	 * I would have preferred to use a QAudioDecoder object, but it doesn't seem to actually be implemented (at least on Mac).
	*/
};

#endif // LIPSYNCDOC_H

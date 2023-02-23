/***************************************************************************
 *   Project TUPITUBE DESK                                                 *
 *   Project Contact: info@maefloresta.com                                 *
 *   Project Website: http://www.maefloresta.com                           *
 *   Project Leader: Gustav Gonzalez <info@maefloresta.com>                *
 *                                                                         *
 *   Developers:                                                           *
 *   2010:                                                                 *
 *    Gustavo Gonzalez                                                     *
 *                                                                         *
 *   KTooN's versions:                                                     *
 *                                                                         *
 *   2006:                                                                 *
 *    David Cuadrado                                                       *
 *    Jorge Cuadrado                                                       *
 *   2003:                                                                 *
 *    Fernado Roldan                                                       *
 *    Simena Dinas                                                         *
 *                                                                         *
 *   Copyright (C) 2010 Gustav Gonzalez - http://www.maefloresta.com       *
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

#include "tuplipsync.h" 
#include "tupsvg2qt.h"

// Transformation

TupTransformation::TupTransformation()
{
}

TupTransformation::TupTransformation(const QDomElement &e)
{
    setTransformationDom(e);
}

TupTransformation::~TupTransformation()
{
}

void TupTransformation::setTransformationDom(const QDomElement &e)
{
    transDom = e;

    QPointF pos;
    TupSvg2Qt::parsePointF(e.attribute("pos"), pos);
    transStructure.pos = pos;
    transStructure.rotationAngle = e.attribute("rotation").toInt();
    transStructure.scaleFactor.setX(e.attribute("scale_x").toDouble());
    transStructure.scaleFactor.setY(e.attribute("scale_y").toDouble());
}

QDomElement TupTransformation::getTransformationDom() const
{
    return transDom;
}

TupTransformation::Parameters TupTransformation::getTransformationParams()
{
    return transStructure;
}

QPointF TupTransformation::getPosition()
{
    return transStructure.pos;
}

int TupTransformation::getRotationAngle()
{
    return transStructure.rotationAngle;
}

QPointF TupTransformation::getScaleFactor()
{
    return transStructure.scaleFactor;
}

void TupTransformation::fromXml(const QString &xml)
{
    QDomDocument document;
    if (document.setContent(xml))
        transDom = document.documentElement();
}

QDomElement TupTransformation::toXml(QDomDocument &doc) const
{
    /*
    #ifdef TUP_DEBUG
        qDebug() << "[TupTransformation::toXml()]";
    #endif
    */

    QDomElement element = doc.createElement("properties");
    element.setAttribute("pos", "(" + QString::number(transStructure.pos.x()) + "," +
                                       QString::number(transStructure.pos.y()) + ")");
    element.setAttribute("scale_x", transStructure.scaleFactor.x());
    element.setAttribute("scale_y", transStructure.scaleFactor.y());
    element.setAttribute("transform", transDom.attribute("transform"));
    element.setAttribute("flags", transDom.attribute("flags"));
    element.setAttribute("rotation", transStructure.rotationAngle);
    element.setAttribute("enabled", transDom.attribute("enabled"));

    return element;
}

// Phoneme

TupPhoneme::TupPhoneme() : QObject()
{
}

TupPhoneme::TupPhoneme(const QString &value, const QDomElement &doc) : QObject()
{
    phoneme = value;
    setTransformationDom(doc);
}

TupPhoneme::~TupPhoneme()
{
}

void TupPhoneme::setValue(const QString &value)
{
    phoneme = value;
}

QString TupPhoneme::value() const
{
    return phoneme;
}

void TupPhoneme::setTransformationDom(const QDomElement &e)
{
    transformation = new TupTransformation(e);
}

QDomElement TupPhoneme::getTransformationDom() const
{
    if (transformation)
        return transformation->getTransformationDom();

    return QDomElement();
}

TupTransformation::Parameters TupPhoneme::getTransformationParams() const
{
    if (transformation)
        return transformation->getTransformationParams();

    TupTransformation::Parameters params;

    return params;
}

void TupPhoneme::fromXml(const QString &xml)
{
    /*
    #ifdef TUP_DEBUG
        qDebug() << "[TupPhoneme::fromXml()] - xml:";
        qDebug() << xml;
    #endif
    */

    QDomDocument document;
    if (document.setContent(xml)) {
        QDomElement element = document.documentElement();
        if (!element.isNull()) {
            if (element.tagName() == "phoneme") {
                phoneme = element.attribute("value");
                QDomNode node = element.firstChild();
                transformation = new TupTransformation(node.toElement());
            }
        }
    }
}

QDomElement TupPhoneme::toXml(QDomDocument &doc) const
{
    /*
    #ifdef TUP_DEBUG
        qDebug() << "[TupPhoneme::toXml()]";
    #endif
    */

    QDomElement root = doc.createElement("phoneme");
    root.setAttribute("value", phoneme);
    root.appendChild(transformation->toXml(doc));

    return root;
}

// Word

TupWord::TupWord()
{
}

TupWord::TupWord(int index)
{
    initIndex = index;
}

TupWord::~TupWord()
{
}

void TupWord::setInitFrame(int index)
{
    initIndex = index;
}

int TupWord::initFrame()
{
    return initIndex;
}

void TupWord::setEndFrame(int index)
{
    endIndex = index;
}

int TupWord::endFrame()
{
    return endIndex;
}

void TupWord::addPhoneme(TupPhoneme *phoneme)
{
    if (phoneme)
        phonemes << phoneme;
}

QList<TupPhoneme *> TupWord::phonemesList()
{
    return phonemes;
}

int TupWord::phonemesTotal()
{
    return phonemes.size();
}

bool TupWord::hasPhonemes()
{
    return !phonemes.isEmpty();
}

TupPhoneme * TupWord::firstPhoneme()
{
    if (!phonemes.isEmpty())
        return phonemes.first();

    return nullptr;
}

TupPhoneme * TupWord::lastPhoneme()
{
    if (!phonemes.isEmpty())
        return phonemes.last();

    return nullptr;
}

TupPhoneme * TupWord::phonemeAt(int frame)
{
    if (frame >= initIndex && frame <= endIndex)
        return phonemes.at(frame);

    return nullptr;
}

bool TupWord::contains(int frame)
{
    if (frame >= initIndex && frame <= endIndex)
        return true;

    return false;
}

QDomElement TupWord::getTransformationDom(int frame)
{
    TupPhoneme *phoneme = phonemeAt(frame);
    if (phoneme)
        return  phoneme->getTransformationDom();

    return QDomElement();
}

TupTransformation::Parameters TupWord::getTransformationParams(int frame)
{
    TupPhoneme *phoneme = phonemeAt(frame);
    if (phoneme)
        return  phoneme->getTransformationParams();

    TupTransformation::Parameters params;

    return params;
}

QList<QDomElement> TupWord::getWordTransformations()
{
    QList<QDomElement> transformations;
    for(int i=0; i<phonemes.size(); i++) {
        TupPhoneme *phoneme = phonemes.at(i);
        transformations << phoneme->getTransformationDom();
    }

    return transformations;
}

void TupWord::setWordTransformations(QList<QDomElement> transformations)
{
    int newLength = phonemesTotal();
    int oldLength = transformations.size();

    if (newLength > oldLength) {
        // Updating the first phonemes of the new word with
        // the few phonemes transformations of the old word
        for(int i=0; i<oldLength; i++) {
            TupPhoneme *phoneme = phonemes.at(i);
            phoneme->setTransformationDom(transformations.at(i));
        }

        // Updating the last phonemes of the new word with
        // the last phoneme transformations of the old word
        int index = oldLength - 1;
        for(int i=oldLength; i<newLength; i++) {
            TupPhoneme *phoneme = phonemes.at(i);
            phoneme->setTransformationDom(transformations.at(index));
        }
    } else { // if (newLength <= oldLength)
        // Updating all the phonemes of the new word with
        // the first phoneme transformations of the old world
        for(int i=0; i<newLength; i++) {
            TupPhoneme *phoneme = phonemes.at(i);
            phoneme->setTransformationDom(transformations.at(i));
        }
    }
}

void TupWord::fromXml(const QString &xml)
{
    /*
    #ifdef TUP_DEBUG
        qDebug() << "[TupWord::fromXml()] - xml:";
        qDebug() << xml;
    #endif
    */

    QDomDocument document;
    if (document.setContent(xml)) {
        QDomElement root = document.documentElement();
        initIndex = root.attribute("initFrame").toInt();
        endIndex = initIndex - 1;

        QDomNode node = root.firstChild();
        while (!node.isNull()) {
            QDomElement element = node.toElement();
            if (!element.isNull()) {
                if (element.tagName() == "phoneme") {
                    TupPhoneme *phoneme = new TupPhoneme();
                    QString input;
                    {
                        QTextStream ts(&input);
                        ts << node;
                    }

                    phoneme->fromXml(input);
                    phonemes << phoneme;
                    endIndex++;
                }
            }

            node = node.nextSibling();
        }
    }
}

QDomElement TupWord::toXml(QDomDocument &doc) const
{
    /*
    #ifdef TUP_DEBUG
        qDebug() << "[TupWord::toXml()]";
    #endif
    */

    QDomElement root = doc.createElement("word");
    root.setAttribute("initFrame", initIndex);

    int total = phonemes.size();
    for(int i=0; i<total; i++) {
        TupPhoneme *phoneme = phonemes.at(i);
        root.appendChild(phoneme->toXml(doc));
    }

    return root;
}

// Phrase

TupPhrase::TupPhrase()
{
}

TupPhrase::TupPhrase(int index) : QObject()
{
    initIndex = index;
}

TupPhrase::~TupPhrase()
{
}

void TupPhrase::setInitFrame(int index)
{
    initIndex = index;
}

int TupPhrase::initFrame()
{
    return initIndex;
}

void TupPhrase::setEndFrame(int index)
{
    endIndex = index;
}

int TupPhrase::endFrame()
{
    return endIndex;
}

void TupPhrase::addWord(TupWord *word)
{
    if (word)
        words << word;
}

void TupPhrase::insertWord(int index, TupWord *word)
{
    if (word)
        words.insert(index, word);
}

QList<TupWord *> TupPhrase::getWords()
{
    return words;
}

TupWord * TupPhrase::wordAt(int index)
{
    if (!words.isEmpty()) {
        if (index >= 0 && index < words.size())
            return words.at(index);
    }

    return nullptr;
}

bool TupPhrase::contains(int frame)
{
    if (frame >= initIndex && frame <= endIndex) 
        return true;

    return false;
}

QDomElement TupPhrase::getTransformationDom(int frame)
{
    TupWord *word = wordAt(frame);
    if (word)
        return word->getTransformationDom(frame);

    return QDomElement();
}

TupTransformation::Parameters TupPhrase::getTransformationParams(int frame)
{
    TupWord *word = wordAt(frame);
    if (word)
        return word->getTransformationParams(frame);

    TupTransformation::Parameters params;

    return params;
}

void TupPhrase::fromXml(const QString &xml)
{
    /*
    #ifdef TUP_DEBUG
        qDebug() << "[TupPhrase::fromXml()] - xml:";
        qDebug() << xml;
    #endif
    */

    QDomDocument document;
    if (document.setContent(xml)) {
        QDomElement root = document.documentElement();
        initIndex = root.attribute("initFrame").toInt();
        QDomNode node = root.firstChild();

        while (!node.isNull()) {
               QDomElement element = node.toElement();
               if (!element.isNull()) {
                   if (element.tagName() == "word") {
                       TupWord *word = new TupWord();
                       QString input;
                       {
                           QTextStream ts(&input);
                           ts << node;
                       }

                       word->fromXml(input);
                       words << word;
                   }
               }

               node = node.nextSibling();
        }

        TupWord *last = words.last();
        endIndex = last->endFrame();
    }
}

QDomElement TupPhrase::toXml(QDomDocument &doc) const
{
    /*
    #ifdef TUP_DEBUG
        qDebug() << "[TupPhrase::toXml()]";
    #endif
    */

    QDomElement root = doc.createElement("phrase");
    root.setAttribute("initFrame", initIndex);

    int total = words.size();
    for(int i=0; i<total; i++) {
        TupWord *word = words.at(i);
        root.appendChild(word->toXml(doc));
    }

    return root;
}

void TupPhrase::updateWordsTransformations(QList<TupWord *> oldWords)
{
    int oldSize = oldWords.size();
    int newSize = words.size();

    if (newSize <= oldSize) {
        for (int i=0; i < newSize; i++) {
            TupWord *word = words.at(i);
            QList<QDomElement> transformations = oldWords.at(i)->getWordTransformations();
            word->setWordTransformations(transformations);
        }
    } else { // if (newSize > oldSize)
        QDomElement transformation;
        // Updating the transformations using all the old words
        for (int i=0; i < oldSize; i++) {
            TupWord *word = words.at(i);
            QList<QDomElement> transformations = oldWords.at(i)->getWordTransformations();
            word->setWordTransformations(transformations);
            if (i == oldSize - 1)
                transformation = transformations.at(transformations.size() - 1);
        }

        // Updating the pending new words using the last transformation of the last old word
        for (int i=oldSize; i < newSize; i++) {
            TupWord *word = words.at(i);
            int wordSize = word->phonemesTotal();
            QList<QDomElement> transformations;
            for (int j=0; j < wordSize; j++)
                transformations << transformation;
            word->setWordTransformations(transformations);
        }
    }
}

// Voice

TupVoice::TupVoice()
{
    phrase = new TupPhrase;
}

TupVoice::TupVoice(const QString &label)
{
    title = label;
    phrase = new TupPhrase;
}

TupVoice::~TupVoice()
{
}

void TupVoice::setVoiceTitle(const QString &label)
{
    title = label;
}

QString TupVoice::voiceTitle() const
{
    return title;
}

QDomElement TupVoice::setDefaultTransformation(int x, int y)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupVoice::setDefaultTransformation()] - point -> " << QPointF(x, y);
    #endif

    QDomDocument doc;
    transformation = doc.createElement("properties");
    transformation.setAttribute("pos", "(" + QString::number(x) + "," + QString::number(y) + ")");
    transformation.setAttribute("scale_x", "1");
    transformation.setAttribute("scale_y", "1");
    transformation.setAttribute("transform", "matrix(1,0,0,1,0,0)");
    transformation.setAttribute("flags", "0");
    transformation.setAttribute("rotation", "0");
    transformation.setAttribute("enabled", "1");

    return transformation;
}

QDomElement TupVoice::getDefaultTransformation() const
{
    return transformation;
}

QDomElement TupVoice::getTransformationDomAt(int frame) const
{
    return phrase->getTransformationDom(frame);
}

void TupVoice::updateMouthTransformation(const QDomElement &doc, int frame)
{
    /*
    #ifdef TUP_DEBUG
        qDebug() << "[TupVoice::updateMouthTransformation()] - frame -> " << frame;
    #endif
    */

    int index = frame - initIndex;
    // Look for phoneme for this frame index
    int i = 0;
    if (phrase->contains(index)) {
        int j = 0;
        QList<TupWord *> wordList = phrase->getWords();
        foreach (TupWord *word, wordList ) {
            int initFrame = word->initFrame();
            if (word->contains(index)) {
                QList<TupPhoneme *> phonemeList = word->phonemesList();
                int position = index - initFrame;
                TupPhoneme *phoneme = phonemeList.at(position);
                phoneme->setTransformationDom(doc);

                for (int n=position+1; n<phonemeList.count(); n++) {
                    TupPhoneme *p = phonemeList.at(n);
                    p->setTransformationDom(doc);
                }

                for (int n=j+1; n<wordList.count(); n++) {
                    TupWord *w = wordList.at(n);
                    foreach (TupPhoneme *p, w->phonemesList())
                        p->setTransformationDom(doc);
                }
                return;
            }
            j++;
        }
    }
    i++;
}

void TupVoice::setText(const QString &content)
{
    script = content;
}

QString TupVoice::text() const
{
    return script;
}

int TupVoice::initFrame()
{
    return initIndex;
}

int TupVoice::endFrame()
{
    return endIndex;
}

void TupVoice::setPhrase(TupPhrase *phrase)
{
    if (phrase)
        this->phrase = phrase;
}

TupPhrase * TupVoice::getPhrase()
{
    return phrase;
}

TupPhoneme * TupVoice::getPhonemeAt(int frame)
{
    if (phrase->contains(frame)) {
        foreach (TupWord *word, phrase->getWords()) {
            int initFrame = word->initFrame();
            int index = frame - initFrame;
            if (initFrame <= frame) {
                if (word->contains(frame)) {
                    TupPhoneme *phoneme = word->phonemesList().at(index);
                    return phoneme;
                }
            }
        }
    }

    return nullptr;
}

bool TupVoice::contains(int frame)
{
    if (frame >= initIndex && frame <= endIndex)
        return true;

    return false;
}

void TupVoice::fromXml(const QString &xml)
{
    /*
    #ifdef TUP_DEBUG
        qDebug() << "[TupVoice::fromXml()] - xml:";
        qDebug() << xml;
    #endif
    */

    QDomDocument document;
    if (document.setContent(xml)) {
        QDomElement root = document.documentElement();
        script = root.attribute("text");

        QDomNode n = root.firstChild();
        while (!n.isNull()) {
               QDomElement e = n.toElement();
               if (!e.isNull()) {
                   if (e.tagName() == "phrase") {
                       phrase = new TupPhrase();
                       QString input;
                       {
                           QTextStream ts(&input);
                           ts << n;
                       }
                       phrase->fromXml(input);
                   }
               }
               n = n.nextSibling();
        }

        initIndex = phrase->initFrame();
        endIndex = phrase->endFrame();
    }
}

QDomElement TupVoice::toXml(QDomDocument &doc) const
{
    /*
    #ifdef TUP_DEBUG
        qDebug() << "[TupVoice::toXml()]";
    #endif
    */

    QDomElement root = doc.createElement("voice");
    root.setAttribute("name", title);
    root.setAttribute("text", script);
    root.appendChild(phrase->toXml(doc));

    return root;
}

void TupVoice::updateWordsTransformations(QList<TupWord *> words)
{
    if (phrase)
        phrase->updateWordsTransformations(words);
}

QList<TupWord *> TupVoice::getPhraseWords()
{
    QList<TupWord *> words;
    if (phrase)
        words = phrase->getWords();

    return words;
}

// LipSync

TupLipSync::TupLipSync() : QObject()
{
}

TupLipSync::TupLipSync(const QString &name, const QString &sound, int init) : QObject()
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupLipSync::TupLipSync()] - lipSyncName -> " << name;
        qDebug() << "[TupLipSync::TupLipSync()] - soundFile -> " << sound;
        qDebug() << "[TupLipSync::TupLipSync()] - initFrame -> " << init;
    #endif

    lipSyncName = name;
    soundFile = sound;
    initFrame = init;
}

TupLipSync::~TupLipSync()
{
}

QString TupLipSync::getLipSyncName() const
{
    return lipSyncName;
}

void TupLipSync::setLipSyncName(const QString &title)
{
    lipSyncName = title;
}

void TupLipSync::setPicsExtension(const QString &format)
{
    extension = format;
}

QString TupLipSync::getPicExtension() const
{
    return extension;
}

QString TupLipSync::getSoundFile() const
{
    return soundFile;
}

void TupLipSync::setSoundFile(const QString &file)
{
    soundFile = file;
}

void TupLipSync::setMouthIndex(int index)
{
    mouthIndex = index;
}

int TupLipSync::getMouthIndex()
{
    return mouthIndex;
}

int TupLipSync::getInitFrame()
{
    return initFrame;
}

void TupLipSync::setInitFrame(int frame)
{
    initFrame = frame;
}

int TupLipSync::getFramesTotal()
{
    return framesTotal;
}

void TupLipSync::setFramesTotal(int total)
{
    framesTotal = total;
}

void TupLipSync::setVoice(TupVoice *voice)
{
    if (voice)
        this->voice = voice;
}

TupVoice * TupLipSync::getVoice()
{
    return voice;
}

void TupLipSync::fromXml(const QString &xml)
{
    QDomDocument document;   
    if (!document.setContent(xml)) {
        #ifdef TUP_DEBUG
            qWarning() << "[TupLipSync::fromXml()] - Fatal Error: XML input is corrupted!";
            qWarning() << xml;
        #endif
        return;
    }

    QDomElement root = document.documentElement();
    lipSyncName = root.attribute("name");
    soundFile = root.attribute("soundFile");
    initFrame = root.attribute("initFrame", "1").toInt();
    framesTotal = root.attribute("framesTotal").toInt();
    extension = root.attribute("extension");
    mouthIndex = root.attribute("mouthIndex").toInt();

    QDomNode n = root.firstChild();
    while (!n.isNull()) {
        QDomElement e = n.toElement();
        if (!e.isNull()) {
            if (e.tagName() == "voice") {
                QString name = e.attribute("name");
                voice = new TupVoice(name);
                QString input;
                {
                    QTextStream ts(&input);
                    ts << n;
                }
                voice->fromXml(input);
            }
        }
        n = n.nextSibling();
    }
}

QDomElement TupLipSync::toXml(QDomDocument &doc) const
{
    /*
    #ifdef TUP_DEBUG
        qDebug() << "[TupLipSync::toXml()]";
    #endif
    */

    QDomElement root = doc.createElement("lipsync");
    root.setAttribute("name", lipSyncName);
    root.setAttribute("soundFile", soundFile);
    root.setAttribute("initFrame", initFrame);
    root.setAttribute("framesTotal", framesTotal);
    root.setAttribute("extension", extension);
    root.setAttribute("mouthIndex", mouthIndex);

    root.appendChild(voice->toXml(doc));

    return root;
}

void TupLipSync::updateMouthTransformation(const QDomElement &doc, int frame)
{
    if (voice)
        voice->updateMouthTransformation(doc, frame);
}

// This method looks for "holes" inside the lipsync record, and fill them with "rest words"

void TupLipSync::verifyStructure()
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupLipSync::verifyStructure()] - framesTotal -> " << framesTotal;
    #endif

    for (int frame=0; frame < framesTotal; frame++) {
         TupPhrase *phrase = voice->getPhrase();
         if (phrase->contains(frame)) {
             int i = -1;
             foreach (TupWord *word, phrase->getWords()) {
                 i++;
                 int initFrame = word->initFrame();
                 if (initFrame <= frame) {
                     if (word->contains(frame)) // Frame is part of a word
                         break;
                 } else { // Frame is not part of any word, the "hole" has to be filled
                     int init = 0;
                     int endFrame = word->initFrame() - 1;
                     int total; // = word->initFrame();
                     QDomElement transformation = voice->getDefaultTransformation();
                     if (i > 0) { // The word is not the first one
                         TupWord *prevWord = phrase->wordAt(i - 1);
                         init = prevWord->endFrame() + 1;

                         if (prevWord->hasPhonemes()) {
                             TupPhoneme *phoneme = prevWord->lastPhoneme();
                             transformation = phoneme->getTransformationDom();
                         } else {
                             #ifdef TUP_DEBUG
                                 qDebug() << "[TupLipSync::verifyStructure()] - "
                                             "Warning: Word(" << (i-1) << ") has NO phonemes!";
                             #endif
                         }

                         total = (endFrame - init) + 1;
                     } else {
                         #ifdef TUP_DEBUG
                             qDebug() << "[TupLipSync::verifyStructure()] - "
                                         "Warning! There is a hole in the very beginning of the lipsync!";
                         #endif
                         init = 0;
                         total = word->initFrame();
                     }

                     // Create a word with "rest" phonemes and insert it in the hole
                     TupWord *w = new TupWord(init);
                     for (int j=0; j<total; j++) {
                          TupPhoneme *phoneme = new TupPhoneme("rest", transformation);
                          w->addPhoneme(phoneme);
                     }
                     w->setEndFrame(endFrame);
                     phrase->insertWord(i, w);
                     if (init < phrase->initFrame())
                         phrase->setInitFrame(init);

                     break; // Hole filled, test next frame
                 }
             }
         }
    }
}

QString TupLipSync::toString() const
{
    QDomDocument document;
    QDomElement root = this->toXml(document);

    QString xml;
    {
      QTextStream ts(&xml);
      ts << root;
    }

    return xml;
}

void TupLipSync::updateWordTransformations(QList<TupWord *> words)
{
    if (voice)
        voice->updateWordsTransformations(words);
}

QList<TupWord *> TupLipSync::getVoiceWords()
{
    QList<TupWord *> words;
    if (voice)
        words = voice->getPhraseWords();

    return words;
}

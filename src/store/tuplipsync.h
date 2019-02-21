/***************************************************************************
 *   Project TUPITUBE DESK                                                *
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

#ifndef TUPLIPSYNC_H
#define TUPLIPSYNC_H

#include "tupabstractserializable.h"

#include <QDomDocument>
#include <QDomElement>
#include <QStringList>
#include <QPoint>
#include <QTextStream>
#include <QMap>

class TUPITUBE_EXPORT TupPhoneme : public QObject, public TupAbstractSerializable
{
    public:
        TupPhoneme();
        TupPhoneme(const QString &value, QPointF point);
        ~TupPhoneme();

        void setValue(const QString &value);
        QString value() const;
        void setPos(QPointF point);
        QPointF position();

        virtual void fromXml(const QString &xml);
        virtual QDomElement toXml(QDomDocument &doc) const;

    private:
        QString phoneme;
        QPointF pos;
};

class TUPITUBE_EXPORT TupWord : public QObject, public TupAbstractSerializable
{
    public:
        TupWord();
        TupWord(int index);
        ~TupWord();

        void setInitFrame(int index);
        int initFrame();
        void setEndFrame(int index);
        int endFrame();
        void addPhoneme(TupPhoneme *phoneme);
        QList<TupPhoneme *> phonemesList();
        bool contains(int frame);

        virtual void fromXml(const QString &xml);
        virtual QDomElement toXml(QDomDocument &doc) const;

    private:
        int initIndex;
        int endIndex;
        
        QList<TupPhoneme *> phonemes;
};

class TUPITUBE_EXPORT TupPhrase : public QObject, public TupAbstractSerializable
{
    public:
        TupPhrase();
        TupPhrase(int index);
        ~TupPhrase();

        void setInitFrame(int index);
        int initFrame();
        void setEndFrame(int index);
        int endFrame();
        void addWord(TupWord *word);
        void insertWord(int index, TupWord *word);
        QList<TupWord *> wordsList();
        bool contains(int frame);

        virtual void fromXml(const QString &xml);
        virtual QDomElement toXml(QDomDocument &doc) const;

    private:
        int initIndex; 
        int endIndex;
        QList<TupWord *> words;
};

class TUPITUBE_EXPORT TupVoice : public QObject, public TupAbstractSerializable
{
    public:
        TupVoice();
        TupVoice(const QString &label, QPointF pos);
        ~TupVoice();

        void setVoiceTitle(const QString &label);
        QString voiceTitle() const;
        void setMouthPos(QPointF pos);
        void updateMouthPos(QPointF pos, int frame);
        QPointF mouthPos();
        void setText(const QString &content);
        QString text() const;
        int initFrame();
        int endFrame();
        void addPhrase(TupPhrase *phrase);
        QList<TupPhrase *> getPhrases();
        TupPhoneme * getPhonemeAt(int frame);
        bool contains(int frame);

        virtual void fromXml(const QString &xml);
        virtual QDomElement toXml(QDomDocument &doc) const;

    private:
        QString title;
        QPointF point;
        QString script;
        int initIndex;
        int endIndex;
        QList<TupPhrase *> phrases;
};

class TUPITUBE_EXPORT TupLipSync : public QObject, public TupAbstractSerializable
{
    public:
        TupLipSync();
        TupLipSync(const QString &getLipSyncName, const QString &getSoundFile, int getInitFrame);
        ~TupLipSync();

        void addPhrase(TupPhrase *phrase);
        QString getLipSyncName() const;
        void setLipSyncName(const QString &title);
        void setPicsExtension(const QString &extension);
        QString getPicExtension() const;
        QString getSoundFile() const;
        void setSoundFile(const QString &file);
        int getFPS();
        void setFPS(int fps);
        int getInitFrame();
        void setInitFrame(int frame);
        int getFramesCount();
        void setFramesCount(int getFramesCount);
        void addVoice(TupVoice *voice);
        TupVoice *voiceAt(int index);
        QList<TupVoice *> getVoices();
        void updateMouthPosition(int mouthIndex, QPointF point, int frame);
        void verifyStructure();

        virtual void fromXml(const QString &xml);
        virtual QDomElement toXml(QDomDocument &doc) const;

    private:        
        QString lipSyncName;
        QString soundFile;
        QString extension;
        int fps;
        int initFrame;
        int framesCount;
        QList<TupVoice *> voices;
};

#endif

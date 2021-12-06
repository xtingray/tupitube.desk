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

class TUPITUBE_EXPORT TupTransformation : public QObject, public TupAbstractSerializable
{
    public:
        TupTransformation();
        TupTransformation(const QDomElement &doc);
        ~TupTransformation();

        void setTransformationDom(const QDomElement &e);
        QDomElement getTransformationDom() const;

        struct Parameters
        {
            QPointF pos = QPointF(0, 0);
            int rotationAngle = 0;
            QPointF scaleFactor = QPointF(1, 1);
        };

        TupTransformation::Parameters getTransformationParams();
        QPointF getPosition();
        int getRotationAngle();
        QPointF getScaleFactor();

        virtual void fromXml(const QString &xml);
        virtual QDomElement toXml(QDomDocument &doc) const;

    private:
        QDomElement transDom;
        Parameters transStructure;
};

class TUPITUBE_EXPORT TupPhoneme : public QObject, public TupAbstractSerializable
{
    public:
        TupPhoneme();
        TupPhoneme(const QString &value, const QDomElement &doc);
        ~TupPhoneme();

        void setValue(const QString &value);
        QString value() const;

        void setTransformationDom(const QDomElement &doc);
        TupTransformation getTransformation();
        QDomElement getTransformationDom() const;
        TupTransformation::Parameters getTransformationParams() const;

        virtual void fromXml(const QString &xml);
        virtual QDomElement toXml(QDomDocument &doc) const;

    private:
        QString phoneme;
        TupTransformation *transformation;
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
        bool hasPhonemes();
        TupPhoneme * firstPhoneme();
        TupPhoneme * phonemeAt(int frame);
        TupPhoneme * lastPhoneme();
        bool contains(int frame);

        QDomElement getTransformationDom(int frame);
        TupTransformation::Parameters getTransformationParams(int frame);

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
        TupWord * wordAt(int index);
        bool contains(int frame);

        QDomElement getTransformationDom(int frame);
        TupTransformation::Parameters getTransformationParams(int frame);

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
        TupVoice(const QString &label);
        ~TupVoice();

        void setVoiceTitle(const QString &label);
        QString voiceTitle() const;

        QDomElement setDefaultTransformation(int x, int yl) const;
        QDomElement getDefaultTransformation() const;

        QDomElement getTransformationDomAt(int frame) const;
        TupTransformation::Parameters getTransformationParamsAt(int frame) const;
        void updateMouthTransformation(const QDomElement &doc, int frame);

        void setText(const QString &content);
        QString text() const;
        int initFrame();
        int endFrame();
        void setPhrase(TupPhrase *phrase);
        TupPhrase * getPhrase();
        TupPhoneme * getPhonemeAt(int frame);
        bool contains(int frame);

        virtual void fromXml(const QString &xml);
        virtual QDomElement toXml(QDomDocument &doc) const;

    private:
        QString title;
        QDomElement transformation;
        QString script;
        int initIndex;
        int endIndex;
        TupPhrase *phrase;
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
        bool hasNativeMouths();
        QString getSoundFile() const;
        void setSoundFile(const QString &file);
        int getInitFrame();
        void setInitFrame(int frame);
        int getFramesTotal();
        void setFramesTotal(int total);

        void setVoice(TupVoice *voice);
        TupVoice * getVoice();

        void updateMouthTransformation(const QDomElement &doc, int frame);

        void verifyStructure();
        QString toString() const;

        virtual void fromXml(const QString &xml);
        virtual QDomElement toXml(QDomDocument &doc) const;

    private:        
        QString lipSyncName;
        QString soundFile;
        QString extension;
        int initFrame;
        int framesTotal;
        TupVoice *voice;
};

#endif

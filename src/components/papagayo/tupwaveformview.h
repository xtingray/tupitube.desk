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

#ifndef TUPWAVEFORMVIEW_H
#define TUPWAVEFORMVIEW_H

#include "tglobal.h"
#include "tuplipsyncdoc.h"

#include <QWidget>
#include <QScrollArea>
#include <QMouseEvent>

class Q_DECL_EXPORT TupWaveFormView : public QWidget
{
	Q_OBJECT

    public:
        explicit TupWaveFormView(QWidget *parent = nullptr);
        ~TupWaveFormView();

        QSize sizeHint() const;
        void setScrollArea(QScrollArea *scrollArea);
        void setDocument(TupLipsyncDoc *doc);

    signals:
        void frameChanged(int);
        void audioStopped();

    public slots:
        void onZoomIn();
        void onZoomOut();
        void onAutoZoom();
        void positionChanged(qint64 milliseconds);

    protected:
        void mousePressEvent(QMouseEvent *event);
        void mouseDoubleClickEvent(QMouseEvent *event);
        void mouseMoveEvent(QMouseEvent *event);
        void mouseReleaseEvent(QMouseEvent *event);
        void paintEvent(QPaintEvent *event);

    private:
        QScrollArea	*scrollArea;
        TupLipsyncDoc *document;
        int32 numSamples;
        real *amp;
        bool dragging, doubleClick;
        int32 draggingEnd;
        int32 currentFrame;
        int32 oldFrame;
        int32 scrubFrame;
        int32 audioStopFrame;
        int32 sampleWidth;
        int32 samplesPerFrame;
        int32 samplesPerSec;
        int32 frameWidth;
        int32 phraseBottom;
        int32 wordBottom;
        int32 phonemeTop;
        bool onlySilent;

        LipsyncPhrase *selectedPhrase, *parentPhrase;
        LipsyncWord *selectedWord, *parentWord;
        LipsyncPhoneme *selectedPhoneme;
};

#endif // TUPWAVEFORMVIEW_H

#ifndef TUPWAVEFORMVIEW_H
#define TUPWAVEFORMVIEW_H

#include <QWidget>

#include "tuplipsyncdoc.h"

class QScrollArea;

class TupWaveFormView : public QWidget
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

        LipsyncPhrase *selectedPhrase, *parentPhrase;
        LipsyncWord *selectedWord, *parentWord;
        LipsyncPhoneme *selectedPhoneme;
};

#endif // TUPWAVEFORMVIEW_H

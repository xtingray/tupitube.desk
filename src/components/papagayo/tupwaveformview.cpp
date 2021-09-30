#include <QPainter>
#include <QMouseEvent>
#include <QScrollArea>
#include <QScrollBar>

#include "tupwaveformview.h"
#include "tupbreakdowndialog.h"

#define DEFAULT_SAMPLE_WIDTH 4
#define DEFAULT_SAMPLES_PER_FRAME 2

TupWaveFormView::TupWaveFormView(QWidget *parent) : QWidget(parent)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupWaveFormView::TupWaveFormView()]";
    #endif

    // setAttribute(Qt::WA_StaticContents);
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Minimum); // QSizePolicy::Fixed ?

    scrollArea = nullptr;
    document = nullptr;
    numSamples = 0;
    amp = nullptr;
    dragging = false;
    doubleClick = false;
    currentFrame = 0;
    oldFrame = 0;
    scrubFrame = 0;
    audioStopFrame = -1;
    sampleWidth = DEFAULT_SAMPLE_WIDTH;
    samplesPerFrame = DEFAULT_SAMPLES_PER_FRAME;
    samplesPerSec = 24 * samplesPerFrame;
    frameWidth = sampleWidth * samplesPerFrame;
    phraseBottom = 16;
    wordBottom = 32;
    phonemeTop = 128;
    selectedPhrase = nullptr;
    selectedWord = nullptr;
    selectedPhoneme = nullptr;
}

TupWaveFormView::~TupWaveFormView()
{
    if (amp) {
        delete [] amp;
        amp = nullptr;
	}
}

QSize TupWaveFormView::sizeHint() const
{
    if (document && numSamples > 0)
        return QSize(numSamples * sampleWidth, 100);
	else
		return QSize(400, 100);
}

void TupWaveFormView::setScrollArea(QScrollArea *scrollArea)
{
    this->scrollArea = scrollArea;
}

void TupWaveFormView::setDocument(TupLipsyncDoc *doc)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupWaveFormView::setDocument()]";
    #endif

    if (document == nullptr && doc) {
        sampleWidth = DEFAULT_SAMPLE_WIDTH;
        samplesPerFrame = DEFAULT_SAMPLES_PER_FRAME;
        samplesPerSec = doc->getFps() * samplesPerFrame;
        frameWidth = sampleWidth * samplesPerFrame;
	}

    document = doc;
    numSamples = 0;

    if (amp) {
        delete [] amp;
        amp = nullptr;
	}

    if (document && document->getAudioExtractor()) {
        TupAudioExtractor *extractor = document->getAudioExtractor();

        frameWidth = sampleWidth * samplesPerFrame;
		real duration = extractor->duration();
		real time = 0.0f;
        real sampleDur = 1.0f / samplesPerSec;
		real maxAmp = 0.0f;
        while (time < duration) {
            numSamples++;
			time += sampleDur;
		}

        if (numSamples < 1)
            numSamples = 1;
        amp = new real[numSamples];
		time = 0.0f;
		int32 i = 0;

        while (time < duration) {
            amp[i] = extractor->getRMSAmplitude(time, sampleDur);
            if (amp[i] > maxAmp)
                maxAmp = amp[i];
			time += sampleDur;
			i++;
		}
		// normalize amplitudes
		maxAmp = 0.95f / maxAmp;
        for (i = 0; i < numSamples; i++)
            amp[i] *= maxAmp;
	}

	updateGeometry();
	update();
}

void TupWaveFormView::onZoomIn()
{
    if (document && samplesPerFrame < 16) {
        if (sampleWidth < 2) {
            sampleWidth = 2;
            samplesPerFrame = 1;
            samplesPerSec = document->getFps() * samplesPerFrame;
            frameWidth = sampleWidth * samplesPerFrame;
            setDocument(document);
        } else {
            samplesPerFrame *= 2;
            samplesPerSec = document->getFps() * samplesPerFrame;
            frameWidth = sampleWidth * samplesPerFrame;
            setDocument(document);
		}
	}
}

void TupWaveFormView::onZoomOut()
{
    if (document) {
        if (samplesPerFrame > 1) {
            samplesPerFrame /= 2;
            if (samplesPerFrame < 1)
                samplesPerFrame = 1;

            samplesPerSec = document->getFps() * samplesPerFrame;
            frameWidth = sampleWidth * samplesPerFrame;
            setDocument(document);
        } else if (sampleWidth > 2) {
            sampleWidth /= 2;
            if (sampleWidth < 1)
                sampleWidth = 1;
            samplesPerSec = document->getFps() * samplesPerFrame;
            frameWidth = sampleWidth * samplesPerFrame;
            setDocument(document);
		}
	}
}

void TupWaveFormView::onAutoZoom()
{
    if (document) {
        sampleWidth = DEFAULT_SAMPLE_WIDTH;
        samplesPerFrame = DEFAULT_SAMPLES_PER_FRAME;
        samplesPerSec = document->getFps() * samplesPerFrame;
        frameWidth = sampleWidth * samplesPerFrame;
        setDocument(document);
	}
}

void TupWaveFormView::positionChanged(qint64 milliseconds)
{
    if (document) {
        real f = ((real)milliseconds / 1000.0f) * document->getFps();
		int32 frame = PG_FLOOR(f);
        if (frame != currentFrame) {
            if (audioStopFrame >= 0) {
                if (frame > audioStopFrame) {
                    if (document->getAudioPlayer())
                        document->getAudioPlayer()->stop();
                    audioStopFrame = -1;
                } else {
                    currentFrame = frame;
                    emit frameChanged(currentFrame);
				}
				update();
            } else if (dragging) {
                if (frame > currentFrame + 1) {
                    if (document->getAudioPlayer())
                        document->getAudioPlayer()->stop();
				}
            } else {
                currentFrame = frame;
                emit frameChanged(currentFrame);
				update();
			}

            QMediaPlayer *audioPlayer = document->getAudioPlayer();
            if (!dragging && audioPlayer && audioPlayer->state() == QMediaPlayer::PlayingState) {
                if (scrollArea) {
                    QScrollBar *scrollBar = scrollArea->horizontalScrollBar();
                    if (scrollBar) {
                        int frameX = currentFrame * frameWidth;
						int scrollX = scrollBar->value();
                        int scrollW = scrollArea->width();
                        if (frameX - scrollX > scrollW) {
							scrollBar->setValue(frameX - scrollW / 6);
                        } else if (frameX - scrollX < 0) {
							scrollBar->setValue(frameX);
                        }
					}
				}
			}
		}
	}
}

void TupWaveFormView::mousePressEvent(QMouseEvent *event)
{
    int32 mouseY = event->y();
    real f = (real)event->x() / (real)frameWidth;
    int32 frame = PG_FLOOR(f), frameDist;

    scrubFrame = -1;
    currentFrame = -1;
    oldFrame = frame;
    dragging = false;
    draggingEnd = -1;
    selectedPhrase = parentPhrase = nullptr;
    selectedWord = parentWord = nullptr;
    selectedPhoneme = nullptr;

    if (document && document->getAudioPlayer()) {
        dragging = true;

        if (document->getCurrentVoice()) {
            // test to see if the user clicked on a phrase, word, or phoneme
			// first, find the phrase that was clicked on
            for (int32 i = 0; i < document->getCurrentVoice()->getPhrases().size(); i++) {
                if (frame >= document->getCurrentVoice()->getPhraseAt(i)->getStartFrame()
                    && frame <= document->getCurrentVoice()->getPhraseAt(i)->getEndFrame()) {
                    selectedPhrase = document->getCurrentVoice()->getPhraseAt(i);
					break;
				}
			}
			// next, find the word that was clicked on
            if (selectedPhrase) {
                for (int32 i = 0; i < selectedPhrase->wordsSize(); i++) {
                    if (frame >= selectedPhrase->getWordAt(i)->getStartFrame() && frame <= selectedPhrase->getWordAt(i)->getEndFrame()) {
                        selectedWord = selectedPhrase->getWordAt(i);
						break;
					}
				}
			}
			// finally, find the phoneme that was clicked on
            if (selectedWord) {
                for (int32 i = 0; i < selectedWord->phonemesSize(); i++) {
                    if (frame == selectedWord->getPhonemeAt(i)->getFrame()) {
                        selectedPhoneme = selectedWord->getPhonemeAt(i);
						break;
					}
				}
			}

            parentPhrase = selectedPhrase;
            parentWord = selectedWord;

			// now, test if the click was within the vertical range of one of these objects
            if (selectedPhrase && mouseY >= selectedPhrase->getTop() && mouseY <= selectedPhrase->getBottom()) {
                selectedWord = nullptr;
                selectedPhoneme = nullptr;
                draggingEnd = 0; // beginning of phrase
                frameDist = frame - selectedPhrase->getStartFrame();
                if ((selectedPhrase->getEndFrame() - frame) < frameDist) {
                    draggingEnd = 1; // end of phrase
                    frameDist = selectedPhrase->getEndFrame() - frame;
				}
                if ((selectedPhrase->getEndFrame() - selectedPhrase->getStartFrame() > 1) &&
                        (PG_FABS((selectedPhrase->getEndFrame() + selectedPhrase->getStartFrame()) / 2 - frame) < frameDist)) {
                    draggingEnd = 2; // middle of phrase
				}
            } else if (selectedWord && mouseY >= selectedWord->getTop() && mouseY <= selectedWord->getBottom()) {
                selectedPhrase = nullptr;
                selectedPhoneme = nullptr;
                draggingEnd = 0; // beginning of word
                frameDist = frame - selectedWord->getStartFrame();
                if ((selectedWord->getEndFrame() - frame) < frameDist) {
                    draggingEnd = 1; // end of word
                    frameDist = selectedWord->getEndFrame() - frame;
				}

                if ((selectedWord->getEndFrame() - selectedWord->getStartFrame() > 1) &&
                        (PG_FABS((selectedWord->getEndFrame() + selectedWord->getStartFrame()) / 2 - frame) < frameDist)) {
                    draggingEnd = 2; // middle of phrase
				}
            } else if (selectedPhoneme && mouseY >= selectedPhoneme->getTop() && mouseY <= selectedPhoneme->getBottom()) {
                selectedPhrase = nullptr;
                selectedWord = nullptr;
                draggingEnd = 0;
            } else {
                selectedPhrase = parentPhrase = nullptr;
                selectedWord = parentWord = nullptr;
                selectedPhoneme = nullptr;
			}

            if (selectedPhrase == nullptr && selectedWord == nullptr && selectedPhoneme == nullptr) {
				mouseMoveEvent(event);
			}

            if (event->button() == Qt::RightButton && selectedWord) {
                dragging = false;
            } else if (doubleClick) {
				bool playSegment = false;
                QMediaPlayer *audioPlayer = document->getAudioPlayer();
				int32 startFrame;
                audioStopFrame = -1;
                if (audioPlayer) {
                    if (selectedPhrase) {
						playSegment = true;
                        startFrame = selectedPhrase->getStartFrame();
                        audioStopFrame = selectedPhrase->getEndFrame() + 1;
                    } else if (selectedWord) {
						playSegment = true;
                        startFrame = selectedWord->getStartFrame();
                        audioStopFrame = selectedWord->getEndFrame() + 1;
                    } else if (selectedPhoneme) {
						playSegment = true;
                        startFrame = selectedPhoneme->getFrame();
                        audioStopFrame = startFrame + 1;
					}

                    if (playSegment) {
                        float pos = ((real)startFrame / (real)document->getFps()) * 1000.0f;
                        audioPlayer->setPosition(PG_ROUND(pos));
						audioPlayer->play();
                        emit frameChanged(scrubFrame);
					}
				}

                dragging = false;
                draggingEnd = -1;
                selectedPhrase = nullptr;
                selectedWord = nullptr;
                selectedPhoneme = nullptr;
			}
		}
	}
}

void TupWaveFormView::mouseDoubleClickEvent(QMouseEvent *event)
{
    doubleClick = true;
	mousePressEvent(event);
    doubleClick = false;
}

void TupWaveFormView::mouseMoveEvent(QMouseEvent *event)
{
    if (!dragging || !document)
		return;

    bool needUpdate = false;
    real f = (real)event->x() / (real)frameWidth;
    int32 frame = PG_FLOOR(f);

    if (selectedPhrase) {
        if (draggingEnd == 0) {
            if (frame != selectedPhrase->getStartFrame()) {
                document->setDirtyFlag(true);
                selectedPhrase->setStartFrame(frame);
                if (selectedPhrase->getStartFrame() > selectedPhrase->getEndFrame() - 1)
                    selectedPhrase->setStartFrame(selectedPhrase->getEndFrame() - 1);

                document->getCurrentVoice()->repositionPhrase(selectedPhrase, document->getDuration());
				needUpdate = true;
			}
        } else if (draggingEnd == 1) {
            if (frame != selectedPhrase->getEndFrame()) {
                document->setDirtyFlag(true);
                selectedPhrase->setEndFrame(frame);
                if (selectedPhrase->getEndFrame() < selectedPhrase->getStartFrame() + 1)
                    selectedPhrase->setEndFrame(selectedPhrase->getStartFrame() + 1);

                document->getCurrentVoice()->repositionPhrase(selectedPhrase, document->getDuration());
				needUpdate = true;
			}
        } else if (draggingEnd == 2) {
            if (frame != oldFrame) {
                document->setDirtyFlag(true);
                selectedPhrase->setStartFrame(selectedPhrase->getStartFrame() + (frame - oldFrame));
                selectedPhrase->setEndFrame(selectedPhrase->getEndFrame() + (frame - oldFrame));
                if (selectedPhrase->getEndFrame() < selectedPhrase->getStartFrame() + 1)
                    selectedPhrase->setEndFrame(selectedPhrase->getStartFrame() + 1);

                document->getCurrentVoice()->repositionPhrase(selectedPhrase, document->getDuration());
				needUpdate = true;
			}
		}
    } else if (selectedWord) {
        if (draggingEnd == 0) {
            if (frame != selectedWord->getStartFrame()) {
                document->setDirtyFlag(true);
                selectedWord->setStartFrame(frame);
                if (selectedWord->getStartFrame() > selectedWord->getEndFrame() - 1)
                    selectedWord->setStartFrame(selectedWord->getEndFrame() - 1);

                parentPhrase->repositionWord(selectedWord);
				needUpdate = true;
			}
        } else if (draggingEnd == 1) {
            if (frame != selectedWord->getEndFrame()) {
                document->setDirtyFlag(true);
                selectedWord->setEndFrame(frame);
                if (selectedWord->getEndFrame() < selectedWord->getStartFrame() + 1)
                    selectedWord->setEndFrame(selectedWord->getStartFrame() + 1);

                parentPhrase->repositionWord(selectedWord);
				needUpdate = true;
			}
        } else if (draggingEnd == 2) {
            if (frame != oldFrame) {
                document->setDirtyFlag(true);
                selectedWord->setStartFrame(selectedWord->getStartFrame() + (frame - oldFrame));
                selectedWord->setEndFrame(selectedWord->getEndFrame() + (frame - oldFrame));
                if (selectedWord->getEndFrame() < selectedWord->getStartFrame() + 1)
                    selectedWord->setEndFrame(selectedWord->getStartFrame() + 1);

                parentPhrase->repositionWord(selectedWord);
				needUpdate = true;
			}
		}
    } else if (selectedPhoneme) {
        if (draggingEnd == 0) {
            if (frame != selectedPhoneme->getFrame()) {
                document->setDirtyFlag(true);
                selectedPhoneme->setFrame(frame);
                parentWord->repositionPhoneme(selectedPhoneme);
				needUpdate = true;
			}
		}
	}

    oldFrame = frame;

    QMediaPlayer *audioPlayer = document->getAudioPlayer();
    if (frame != scrubFrame) {
        scrubFrame = frame;
        currentFrame = scrubFrame;
        f = ((real)scrubFrame / (real)document->getFps()) * 1000.0f;
		audioPlayer->setPosition(PG_FLOOR(f));
		audioPlayer->play();
        emit frameChanged(scrubFrame);
		needUpdate = true;
	}

	if (needUpdate)
		update();
}

void TupWaveFormView::mouseReleaseEvent(QMouseEvent *event)
{
    if (document && document->getAudioPlayer() && audioStopFrame < 0)
        document->getAudioPlayer()->stop();

    if (event->button() == Qt::RightButton && selectedWord) {
		// manually enter the pronunciation for this word
        TupBreakdownDialog *dlog = new TupBreakdownDialog(selectedWord, this);
        if (dlog->exec() == QDialog::Accepted) {
            document->setDirtyFlag(true);
            while (!selectedWord->getPhonemes().isEmpty())
                selectedWord->removeFirstPhoneme();
                // delete selectedWord->getPhonemes().takeFirst();

            QStringList phList = dlog->phonemeString().split(' ', Qt::SkipEmptyParts);
            for (int i = 0; i < phList.size(); i++) {
				QString phStr = phList.at(i);
				if (phStr.isEmpty())
					continue;

				LipsyncPhoneme *phoneme = new LipsyncPhoneme;
                phoneme->setText(phStr);
                selectedWord->addPhoneme(phoneme);
			}

            if (parentPhrase)
                parentPhrase->repositionWord(selectedWord);
			update();
		}

		delete dlog;
	}

    scrubFrame = -1;
    currentFrame = -1;
    dragging = false;
    draggingEnd = -1;
    selectedPhrase = nullptr;
    selectedWord = nullptr;
    selectedPhoneme = nullptr;

    emit frameChanged(0);
	update();
}

void TupWaveFormView::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)

    QPainter dc(this);

    int32 clientWidth = width();
    int32 clientHeight = height();

    if (document == nullptr) {
        dc.drawText(QRect(0, 0, clientWidth, clientHeight), Qt::AlignHCenter | Qt::AlignVCenter, tr("Drop audio file here"));
		return;
	}

    int32 topBorder = 16; // should be the height of frame label text
    int32 halfClientHeight;
    int32 sampleHeight, halfSampleHeight, textHeight;
    int32 fps = document->getFps();
    int32 x = 0;
    int32 frameX;
    int32 sample = 0;
    int32 frame = 0;
    bool drawPlayMarker = false;
    QRect rect;
    QColor textCol(64, 64, 64);
    QColor sampleFillCol(162, 205, 242);
    QColor sampleOutlineCol(30, 121, 198);
    QColor playBackCol(255, 127, 127);
    QColor playForeCol(209, 102, 121, 128);
    QColor playOutlineCol(128, 0, 0);
    QColor frameCol(192, 192, 192);
    QColor phraseFillCol(205, 242, 162);
    QColor phraseOutlineCol(121, 198, 30);
    QColor wordFillCol(242, 205, 162);
    QColor wordOutlineCol(198, 121, 30);
    QColor wordMissingFillCol(255, 127, 127);
    QColor wordMissingOutlineCol(255, 0, 0);
    QColor phonemeFillCol(231, 185, 210);
    QColor phonemeOutlineCol(173, 114, 146);
    QMediaPlayer *audioPlayer = document->getAudioPlayer();

	textHeight = dc.fontMetrics().height() + 4;
	topBorder = textHeight;
	halfClientHeight = (clientHeight - textHeight) / 2;

    if (audioPlayer && audioPlayer->state() == QMediaPlayer::PlayingState) {
		drawPlayMarker = true;
        x = currentFrame * frameWidth;
        dc.fillRect(QRect(x, 0, frameWidth, clientHeight), playBackCol);
	}

	x = 0;
    for (int32 i = 0; i < numSamples; i++) {
        if (((sample + 1) % samplesPerFrame) == 0) {
			dc.setPen(frameCol);
			// draw frame marker
            frameX = (frame + 1) * frameWidth;
            if (sampleWidth >= 2 && ((frameWidth > 2) || ((frame + 2) % fps == 0))) {
				dc.drawLine(frameX, topBorder, frameX, clientHeight);
			}

			// draw frame label
            if ((frameWidth > 30) || ((frame + 2) % fps == 0)) {
				dc.drawLine(frameX, 0, frameX, topBorder);
				dc.drawText(frameX + 2, textHeight - 4, QString::number(frame + 2));
			}
		}

        sampleHeight = PG_ROUND(amp[i] * (real)(clientHeight - topBorder));
		halfSampleHeight = sampleHeight / 2;
        rect.setRect(x, topBorder + halfClientHeight - halfSampleHeight, sampleWidth + 1, sampleHeight);
        dc.fillRect(rect, sampleFillCol);
		dc.setPen(sampleOutlineCol);
        dc.drawLine(rect.topLeft(), rect.topRight());
        dc.drawLine(rect.bottomLeft(), rect.bottomRight());
        dc.drawLine(rect.topRight(), rect.bottomRight());

        if (i == 0) {
            dc.drawLine(rect.topLeft(), rect.bottomLeft());
        } else if (amp[i] > amp[i - 1]) {
            sampleHeight = PG_ROUND(amp[i - 1] * (real)(clientHeight - topBorder));
			halfSampleHeight = sampleHeight / 2;
            dc.drawLine(rect.topLeft(), QPoint(rect.left(), topBorder + halfClientHeight - halfSampleHeight));
            dc.drawLine(rect.bottomLeft(), QPoint(rect.left(), topBorder + halfClientHeight - halfSampleHeight + sampleHeight - 1));
		}

        x += sampleWidth;
		sample++;
        if ((sample % samplesPerFrame) == 0)
			frame++;
	}

    if (document->getCurrentVoice()) {
		topBorder += 4;
        for (int32 p = 0; p < document->getCurrentVoice()->getPhrases().size(); p++) {
            LipsyncPhrase *phrase = document->getCurrentVoice()->getPhraseAt(p);
            rect = QRect(phrase->getStartFrame() * frameWidth, topBorder,
                         (phrase->getEndFrame() - phrase->getStartFrame() + 1) * frameWidth, textHeight);
            phrase->setTop(rect.top());
            phrase->setBottom(rect.bottom());
            dc.fillRect(rect, phraseFillCol);
			dc.setPen(phraseOutlineCol);
            dc.drawRect(rect);
            dc.setClipRect(rect);
			dc.setPen(textCol);
            rect = rect.marginsRemoved(QMargins(2, 2, 2, 2));
            dc.drawText(QPoint(rect.left(), rect.bottom() - 2), phrase->getText());
			dc.setClipping(false);

            for (int32 w = 0; w < phrase->wordsSize(); w++) {
                LipsyncWord *word = phrase->getWordAt(w);
                rect = QRect(word->getStartFrame() * frameWidth, topBorder + 4 + textHeight,
                             (word->getEndFrame() - word->getStartFrame() + 1) * frameWidth, textHeight);
				if (w & 1)
                    rect.translate(0, textHeight - textHeight / 4);
                word->setTop(rect.top());
                word->setBottom(rect.bottom());
                if (word->phonemesSize() == 0) {
                    dc.fillRect(rect, wordMissingFillCol);
					dc.setPen(wordMissingOutlineCol);
                } else {
                    dc.fillRect(rect, wordFillCol);
					dc.setPen(wordOutlineCol);
				}

                dc.drawRect(rect);
                dc.setClipRect(rect);
				dc.setPen(textCol);
                rect = rect.marginsRemoved(QMargins(2, 2, 2, 2));
                dc.drawText(QPoint(rect.left(), rect.bottom() - 2), word->getText());
				dc.setClipping(false);

                for (int32 i = 0; i < word->phonemesSize(); i++) {
                    LipsyncPhoneme *phoneme = word->getPhonemeAt(i);
                    rect = QRect(phoneme->getFrame() * frameWidth, clientHeight - 4 - textHeight, frameWidth, textHeight);
					if (i & 1)
                        rect.translate(0, -(textHeight - textHeight / 4));
                    phoneme->setTop(rect.top());
                    phoneme->setBottom(rect.bottom());
                    dc.fillRect(rect, phonemeFillCol);
					dc.setPen(phonemeOutlineCol);
                    dc.drawRect(rect);
					dc.setPen(textCol);
                    rect = rect.marginsRemoved(QMargins(2, 2, 2, 2));
                    dc.drawText(QPoint(rect.left(), rect.bottom() - 2), phoneme->getText());
				} // for i
			} // for w
		} // for p
	}

    if (drawPlayMarker) {
        x = currentFrame * frameWidth;
        dc.fillRect(QRect(x, 0, frameWidth, clientHeight), playForeCol);
		dc.setPen(playOutlineCol);
        dc.drawRect(QRect(x, 0, frameWidth, clientHeight));
	}
}

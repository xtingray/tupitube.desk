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

#include "tupwaveformview.h"
#include "tupbreakdowndialog.h"
#include "tosd.h"

#include <QPainter>
#include <QScrollArea>
#include <QScrollBar>

#define DEFAULT_SAMPLE_WIDTH 4
#define DEFAULT_SAMPLES_PER_FRAME 2

TupWaveFormView::TupWaveFormView(QWidget *parent) : QWidget(parent)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupWaveFormView::TupWaveFormView()]";
    #endif

    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Minimum);

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

    markText = Qt::black;
    textCol = QColor(64, 64, 64);
    sampleFillCol = QColor(162, 205, 242);
    sampleOutlineCol = QColor(30, 121, 198);
    playBackCol = QColor(255, 127, 127);
    playForeCol = QColor(209, 102, 121, 128);
    playOutlineCol = QColor(128, 0, 0);
    frameCol = QColor(192, 192, 192);
    phraseFillCol = QColor(205, 242, 162);
    phraseOutlineCol = QColor(121, 198, 30);
    wordFillCol = QColor(242, 205, 162);
    wordOutlineCol = QColor(198, 121, 30);
    wordMissingFillCol = QColor(255, 127, 127);
    wordMissingOutlineCol = QColor(255, 0, 0);
    phonemeFillCol = QColor(231, 185, 210);
    phonemeOutlineCol = QColor(173, 114, 146);

    setToolTip(tr("Drop audio file here"));
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
        return QSize(numSamples * sampleWidth, height());
	else
        return QSize(width(), height());
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

    setToolTip(tr(""));

    if (document == nullptr && doc) {
        sampleWidth = DEFAULT_SAMPLE_WIDTH;
        samplesPerFrame = DEFAULT_SAMPLES_PER_FRAME;
        samplesPerSec = doc->getFps() * samplesPerFrame;
        frameWidth = sampleWidth * samplesPerFrame;
	}

    document = doc;
    audioPlayer = document->getAudioPlayer();
    numSamples = 0;

    if (amp) {
        delete [] amp;
        amp = nullptr;
	}

    TupAudioExtractor *extractor = document->getAudioExtractor();
    if (document && extractor) {
        #ifdef TUP_DEBUG
            qDebug() << "[TupWaveFormView::setDocument()] - Processing audio...";
        #endif
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

        onlySilent = true;
        while (time < duration) {
            amp[i] = extractor->getRMSAmplitude(time, sampleDur);
            if (amp[i] > 0)
                onlySilent = false;

            if (amp[i] > maxAmp)
                maxAmp = amp[i];
			time += sampleDur;
			i++;
		}

		// normalize amplitudes
		maxAmp = 0.95f / maxAmp;
        for (i = 0; i < numSamples; i++)
            amp[i] *= maxAmp;

        if (!onlySilent) {
            updateGeometry();
            update();
        } else {
            #ifdef TUP_DEBUG
                qDebug() << "[TupWaveFormView::setDocument()] - Fatal Error: Invalid sound file!";
            #endif
            TOsd::self()->display(TOsd::Error, tr("Sound file has no voices!"), 3000);
            close();
        }

    } else {
        #ifdef TUP_DEBUG
            qDebug() << "[TupWaveFormView::setDocument()] - Fatal Error: Can't process audio input!";
        #endif
    }
}

void TupWaveFormView::setMouthsPath(const QString &path)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupWaveFormView::setMouthsPath()] - path -> " << path;
    #endif

    mouthsPath = path;
}

void TupWaveFormView::zoomIn()
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

void TupWaveFormView::zoomOut()
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

void TupWaveFormView::autoZoom()
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
    /*
    #ifdef TUP_DEBUG
        qDebug() << "[TupWaveFormView::positionChanged()] - milliseconds -> " << milliseconds;
    #endif
    */

    if (document) {
        real f = ((real)milliseconds / 1000.0f) * document->getFps();
		int32 frame = PG_FLOOR(f);

        /*
        #ifdef TUP_DEBUG
            qDebug() << "[TupWaveFormView::positionChanged()] - frame -> " << frame;
            qDebug() << "[TupWaveFormView::positionChanged()] - duration -> " << document->getDuration();
        #endif
        */

        if (frame == document->getDuration())
            emit audioStopped();

        if (frame != currentFrame) {
            if (audioStopFrame >= 0) {
                if (frame > audioStopFrame) {
                    if (document->getAudioPlayer())
                        document->stopAudio();

                    audioStopFrame = -1;
                } else {
                    currentFrame = frame;
                    emit frameChanged(currentFrame);
				}
				update();
            } else if (dragging) {
                if (frame > currentFrame + 1) {
                    if (document->getAudioPlayer())
                        document->stopAudio();
				}
            } else {
                currentFrame = frame;
                emit frameChanged(currentFrame);
				update();
			}

            QMediaPlayer *audioPlayer = document->getAudioPlayer();
            if (audioPlayer) {
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
    selectedPhrase = nullptr;
    parentPhrase = nullptr;
    selectedWord = nullptr;
    parentWord = nullptr;
    selectedPhoneme = nullptr;

    if (document && document->getAudioPlayer()) {
        dragging = true;

        if (document->getVoice()) {
            // test to see if the user clicked on a phrase, word, or phoneme
			// first, find the phrase that was clicked on
            // for (int32 i = 0; i < document->getPhrasesTotal(); i++) {
                if (frame >= document->getStartFrameFromPhrase()
                    && frame <= document->getEndFrameFromPhrase()) {
                    selectedPhrase = document->getPhrase();
                    // break;
				}
            // }

			// next, find the word that was clicked on
            if (selectedPhrase) {
                for (int32 i = 0; i < selectedPhrase->wordsSize(); i++) {
                    if (frame >= selectedPhrase->getStartFrameFromWordAt(i) && frame <= selectedPhrase->getEndFrameFromWordAt(i)) {
                        selectedWord = selectedPhrase->getWordAt(i);
						break;
					}
				}
            } else {
                #ifdef TUP_DEBUG
                    qDebug() << "[TupWaveFormView::mousePressEvent()] - Warning: NO phrase found!";
                #endif
            }

			// finally, find the phoneme that was clicked on
            if (selectedWord) {
                for (int32 i = 0; i < selectedWord->phonemesSize(); i++) {
                    if (frame == selectedWord->getFrameFromPhonemeAt(i)) {
                        selectedPhoneme = selectedWord->getPhonemeAt(i);
						break;
					}
				}
            } else {
                #ifdef TUP_DEBUG
                    qDebug() << "[TupWaveFormView::mousePressEvent()] - Warning: NO word found!";
                #endif
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
                document->setModifiedFlag(true);
                selectedPhrase->setStartFrame(frame);
                if (selectedPhrase->getStartFrame() > selectedPhrase->getEndFrame() - 1)
                    selectedPhrase->setStartFrame(selectedPhrase->getEndFrame() - 1);

                document->repositionPhrase(selectedPhrase);
				needUpdate = true;
			}
        } else if (draggingEnd == 1) {
            if (frame != selectedPhrase->getEndFrame()) {
                document->setModifiedFlag(true);
                selectedPhrase->setEndFrame(frame);
                if (selectedPhrase->getEndFrame() < selectedPhrase->getStartFrame() + 1)
                    selectedPhrase->setEndFrame(selectedPhrase->getStartFrame() + 1);

                document->repositionPhrase(selectedPhrase);
                needUpdate = true;
			}
        } else if (draggingEnd == 2) {
            if (frame != oldFrame) {
                document->setModifiedFlag(true);
                selectedPhrase->setStartFrame(selectedPhrase->getStartFrame() + (frame - oldFrame));
                selectedPhrase->setEndFrame(selectedPhrase->getEndFrame() + (frame - oldFrame));
                if (selectedPhrase->getEndFrame() < selectedPhrase->getStartFrame() + 1)
                    selectedPhrase->setEndFrame(selectedPhrase->getStartFrame() + 1);

                document->repositionPhrase(selectedPhrase);
                needUpdate = true;
			}
		}
    } else if (selectedWord) {
        if (draggingEnd == 0) {
            if (frame != selectedWord->getStartFrame()) {
                document->setModifiedFlag(true);
                selectedWord->setStartFrame(frame);
                if (selectedWord->getStartFrame() > selectedWord->getEndFrame() - 1)
                    selectedWord->setStartFrame(selectedWord->getEndFrame() - 1);

                parentPhrase->repositionWord(selectedWord);
				needUpdate = true;
			}
        } else if (draggingEnd == 1) {
            if (frame != selectedWord->getEndFrame()) {
                document->setModifiedFlag(true);
                selectedWord->setEndFrame(frame);
                if (selectedWord->getEndFrame() < selectedWord->getStartFrame() + 1)
                    selectedWord->setEndFrame(selectedWord->getStartFrame() + 1);

                parentPhrase->repositionWord(selectedWord);
				needUpdate = true;
			}
        } else if (draggingEnd == 2) {
            if (frame != oldFrame) {
                document->setModifiedFlag(true);
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
                document->setModifiedFlag(true);
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
    #ifdef TUP_DEBUG
        qDebug() << "[TupWaveFormView::mouseReleaseEvent()]";
    #endif

    if (document) {
        if (document->getAudioPlayer() && audioStopFrame < 0)
            document->stopAudio();

        if (event->button() == Qt::RightButton && selectedWord) {
            if (mouthsPath.isEmpty()) {
                #ifdef TUP_DEBUG
                    qDebug() << "[TupWaveFormView::mouseReleaseEvent()] - Warning: No mouth images path is set!";
                #endif
                TOsd::self()->display(TOsd::Error, tr("Mouth images are unset!"), 3000);
                return;
            }

            // manually enter the pronunciation for this word
            TupBreakdownDialog *breakdownDialog = new TupBreakdownDialog(selectedWord->getText(), selectedWord->getPhonemesString(),
                                                                         mouthsPath, this);
            if (breakdownDialog->exec() == QDialog::Accepted) {
                document->setModifiedFlag(true);
                selectedWord->clearPhonemes();

                QStringList phList = breakdownDialog->phonemeString().split(' ', Qt::SkipEmptyParts);
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

            delete breakdownDialog;
        } else {
            #ifdef TUP_DEBUG
                qDebug() << "[TupWaveFormView::mouseReleaseEvent()] - Warning: NO word selected!";
            #endif
        }
    } else {
        #ifdef TUP_DEBUG
            qDebug() << "[TupWaveFormView::mouseReleaseEvent()] - Fatal Error: document is NULL!";
        #endif
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
    /*
    #ifdef TUP_DEBUG
        qDebug() << "[TupWaveFormView::paintEvent()]";
    #endif
    */

    Q_UNUSED(event)

    QPainter dc(this);
    int32 clientHeight = height();
    if (onlySilent) {
        #ifdef TUP_DEBUG
            qDebug() << "[TupWaveFormView::paintEvent()] - Fatal Error: Invalid sound file!";
        #endif
        return;
    }

    if (document == nullptr) {
        #ifdef TUP_DEBUG
            qDebug() << "[TupWaveFormView::paintEvent()] - Fatal Error: Document is NULL!";
        #endif
        return;
    }

    int32 topBorder = 16; // should be the height of frame label text
    int32 halfClientHeight;
    int32 sampleHeight;
    int32 halfSampleHeight;
    int32 textHeight;
    int32 fps = document->getFps();
    int32 x = 0;
    int32 frameX;
    int32 sample = 0;
    int32 frame = 0;
    bool drawPlayMarker = false;
    QRect rect;

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
                dc.setPen(markText);
                dc.drawText(frameX + 4, textHeight - 4, QString::number(frame + 2));
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

    if (document->getVoice()) {
		topBorder += 4;
        LipsyncPhrase *phrase = document->getPhrase();
        if (phrase) {
            if (!document->voiceTextIsEmpty()) {
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
            } else {
                #ifdef TUP_DEBUG
                    qDebug() << "[TupWaveFormView::paintEvent()] - Warning: Phrase is empty!";
                #endif
            }
        } else {
            #ifdef TUP_DEBUG
                qDebug() << "[TupWaveFormView::paintEvent()] - Fatal Error: Phrase is NULL!";
            #endif
        }
    } else {
        #ifdef TUP_DEBUG
            qDebug() << "[TupWaveFormView::paintEvent()] - Fatal Error: Voice is NULL!";
        #endif
    }

    if (drawPlayMarker) {
        x = currentFrame * frameWidth;
        dc.fillRect(QRect(x, 0, frameWidth, clientHeight), playForeCol);
		dc.setPen(playOutlineCol);
        dc.drawRect(QRect(x, 0, frameWidth, clientHeight));
	}
}

void TupWaveFormView::updateMediaStatus(QMediaPlayer::MediaStatus status)
{
    if (status == QMediaPlayer::EndOfMedia)
        emit audioStopped();
}

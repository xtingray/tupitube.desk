#include <QPainter>

#include "tupmouthview.h"

TupMouthView::TupMouthView(QWidget *parent) : QWidget(parent)
{
    document = nullptr;
    mouthID = 0;
    frame = 0;

	TupLipsyncDoc::loadDictionaries();

    for (int32 mouth = 0; mouth < 4; mouth++) {
        QString basePath;

        switch (mouth) {
			case 0:
				basePath = ":/mouths/mouths/1_Mouth_1/";
				break;
			case 1:
				basePath = ":/mouths/mouths/2_Mouth_2/";
				break;
			case 2:
				basePath = ":/mouths/mouths/3_Gary_C_Martin/";
				break;
			case 3:
				basePath = ":/mouths/mouths/4_Preston_Blair/";
				break;
		}

        for (int32 i = 0; i < TupLipsyncDoc::phonemesListSize(); i++) {
            mouths[mouth].insert(TupLipsyncDoc::getPhonemeAt(i), new QImage(basePath + TupLipsyncDoc::getPhonemeAt(i) + ".jpg"));
		}
	}
}

TupMouthView::~TupMouthView()
{
}

void TupMouthView::setDocument(TupLipsyncDoc *doc)
{
    document = doc;
	update();
}

void TupMouthView::setMouth(int32 id)
{
    mouthID = PG_CLAMP(id, 0, 3);
}

void TupMouthView::onMouthChanged(int id)
{
    setMouth(id);
	update();
}

void TupMouthView::onFrameChanged(int frameIndex)
{
    frame = frameIndex;
	update();
}

void TupMouthView::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)

    QString phoneme;
    QImage *img = nullptr;
    QPainter dc(this);

    if (document && document->getCurrentVoice())
        phoneme = document->getCurrentVoice()->getPhonemeAtFrame(frame);
	else
		phoneme = "etc";

    if (phoneme.isEmpty() && document)
        phoneme = document->getVolumePhonemeAtFrame(frame);

    img = mouths[mouthID].value(phoneme);
    if (img) {
        int32 x = 0, y = 0;
        int32 w = width();
        int32 h = height();
        QColor backCol(255, 255, 255);
        if (w > h) {
			dc.fillRect(QRect(x, y, w, h), backCol);
			x = (w - h) / 2;
			w = h;
        } else if (h > w) {
			dc.fillRect(QRect(x, y, w, h), backCol);
			y = (h - w) / 2;
			h = w;
		}
		dc.drawImage(QRect(x, y, w, h), *img);
    } else {
		dc.eraseRect(0, 0, width(), height());
    }
}

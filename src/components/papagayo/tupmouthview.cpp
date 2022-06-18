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

#include "tupmouthview.h"
#include <QPainter>

TupMouthView::TupMouthView(TupLipsyncDictionary *dictionary, QWidget *parent) : QWidget(parent)
{
    document = nullptr;
    mouthID = 0;
    frame = 0;

    for (int32 mouth = 0; mouth < 5; mouth++) {
        #ifdef Q_OS_UNIX
            QString basePath = SHARE_DIR + "data/mouths/" + QString::number(mouth + 1) + "/";
        #else
            QString basePath = SHARE_DIR + "mouths/" + QString::number(mouth + 1) + "/";
        #endif

        #ifdef TUP_DEBUG
            qDebug() << "[TupMouthView()] - basePath -> " << basePath;
        #endif
        mouthsPath << basePath;
        for (int32 i = 0; i < dictionary->phonemesListSize(); i++) {
            QString path = basePath + dictionary->getPhonemeAt(i) + ".png";
            if  (!QFile::exists(path)) {
                #ifdef TUP_DEBUG
                    qWarning() << "[TupMouthView()] - Warning: Mouth path doesn't exist! -> " << path;
                #endif
            } else {
                #ifdef TUP_DEBUG
                    qDebug() << "[TupMouthView()] - Mouth image loaded successfully! -> " << path;
                    qDebug() << "[TupMouthView()] - Phoneme -> " << dictionary->getPhonemeAt(i);
                #endif
            }

            mouths[mouth].insert(dictionary->getPhonemeAt(i), new QImage(path));
        }
	}

    currentPath = mouthsPath.at(0);

    #ifdef TUP_DEBUG
        qDebug() << "[TupMouthView()] - Current mouth image -> " << currentPath;
    #endif
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
    mouthID = PG_CLAMP(id, 0, 4);
}

QString TupMouthView::getMouthsPath() const
{
    return currentPath;
}

void TupMouthView::onMouthChanged(int id)
{
    setMouth(id);
    currentPath = mouthsPath.at(id);

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

    if (document && document->getVoice())
        phoneme = document->getPhonemeAtFrame(frame);
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

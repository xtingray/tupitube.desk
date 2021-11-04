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

#include <QPainter>

#include "tupcustomizedmouthview.h"
#include "tapplicationproperties.h"
#include "tosd.h"

TupCustomizedMouthView::TupCustomizedMouthView(QWidget *parent) : QWidget(parent)
{
    document = nullptr;
    frame = 0;
    assetsLoaded = false;
    imagesPath = "";

	TupLipsyncDoc::loadDictionaries();
}

TupCustomizedMouthView::~TupCustomizedMouthView()
{
}

void TupCustomizedMouthView::setDocument(TupLipsyncDoc *doc)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupCustomizedMouthView::setDoccument()]";
    #endif

    document = doc;
	update();
}

void TupCustomizedMouthView::loadImages(const QString &folderPath)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupCustomizedMouthView::loadImages()] - folderPath -> " << folderPath;
    #endif

    QDir dir(folderPath);
    QStringList imagesList = dir.entryList(QStringList() << "*.png" << "*.jpg" << "*.jpeg");
    if (imagesList.size() > 0) {
        if (imagesList.count() == 10) { // Mouths set always contains 10 figures
            QString firstImage = imagesList.at(0);
            int dot = firstImage.lastIndexOf(".");
            QString extension = firstImage.mid(dot);

            for (int32 i = 0; i < TupLipsyncDoc::phonemesListSize(); i++) {
                QString path = folderPath + "/" + TupLipsyncDoc::getPhonemeAt(i) + extension;
                if (QFile::exists(path)) {
                    mouths.insert(TupLipsyncDoc::getPhonemeAt(i), new QImage(path));
                } else {
                    TOsd::self()->display(TOsd::Error, tr("Mouth images are missing!"));
                    #ifdef TUP_DEBUG
                        qDebug() << "[TupCustomizedMouthView::loadImages()] - Fatal Error: Mouth image is missing -> " << path;
                    #endif
                }
            }
            assetsLoaded = true;
            imagesPath = folderPath;
            update();
        } else {
            TOsd::self()->display(TOsd::Error, tr("Mouth images are incomplete!"));
            #ifdef TUP_DEBUG
                qDebug() << "[TupCustomizedMouthView::loadImages()] - Fatal Error: Mouth images are incomplete!";
            #endif
        }
    } else {
        TOsd::self()->display(TOsd::Error, tr("Images directory is empty!"));
        #ifdef TUP_DEBUG
            qDebug() << "[TupCustomizedMouthView::loadImages()] - Fatal Error: Images directory is empty!";
        #endif
    }
}

void TupCustomizedMouthView::onFrameChanged(int frameIndex)
{
    /*
    #ifdef TUP_DEBUG
        qDebug() << "[TupCustomizedMouthView::onFrameChanged()] - frameIndex -> " << frameIndex;
    #endif
    */

    frame = frameIndex;
	update();
}

void TupCustomizedMouthView::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)

    if (!assetsLoaded)
        return;

    QString phoneme;
    QImage *img = nullptr;
    QPainter dc(this);

    if (document && document->getCurrentVoice())
        phoneme = document->getPhonemeAtFrame(frame);
    else
		phoneme = "etc";

    if (phoneme.isEmpty() && document)
        phoneme = document->getVolumePhonemeAtFrame(frame);

    img = mouths.value(phoneme);
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

bool TupCustomizedMouthView::imagesAresLoaded()
{
    return !imagesPath.isEmpty();
}

QString TupCustomizedMouthView::getMouthsPath() const
{
    return imagesPath;
}

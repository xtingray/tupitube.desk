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

#include "tupcustomizedmouthview.h"
#include "tosd.h"

#include <QPainter>

TupCustomizedMouthView::TupCustomizedMouthView(TupLipsyncDictionary *lipsyncDictionary, QWidget *parent) : QWidget(parent)
{
    document = nullptr;
    frame = 0;
    assetsLoaded = false;
    imagesPath = "";
    dictionary = lipsyncDictionary;
}

TupCustomizedMouthView::~TupCustomizedMouthView()
{
}

void TupCustomizedMouthView::setDocument(TupLipsyncDoc *doc)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupCustomizedMouthView::setDocument()]";
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
        if (imagesList.count() == MOUTHS_PACKAGE_SIZE) { // Mouths set always contains 10 figures
            for (int32 i = 0; i < MOUTHS_PACKAGE_SIZE; i++) {
                QString filename = imagesList.at(i);
                int dot = filename.lastIndexOf(".");
                QString extension = filename.mid(dot);
                QString name = filename.left(dot);
                QString nameLower = filename.left(dot).toLower();
                bool found = false;
                for (int32 i = 0; i < dictionary->phonemesListSize(); i++) {
                     QString phoneme = dictionary->getPhonemeAt(i);
                     QString phonemeLower = phoneme.toLower();
                     if (nameLower.compare(phonemeLower) == 0) {
                         QString path = folderPath + "/" + name + extension;
                         QImage img(path);
                         int width = img.width();
                         int height = img.height();
                         if (width > MOUTH_WIDTH || height > MOUTH_HEIGHT) {
                             if (width > height)
                                 img = QImage(img.scaledToWidth(MOUTH_WIDTH, Qt::SmoothTransformation));
                             else
                                 img = QImage(img.scaledToHeight(MOUTH_HEIGHT, Qt::SmoothTransformation));
                         }

                         mouths.insert(phoneme, img);
                         found = true;
                         break;
                     }
                }
                if (!found) {
                    TOsd::self()->display(TOsd::Error, tr("Mouth images are missing!"));
                    #ifdef TUP_DEBUG
                        qWarning() << "[TupCustomizedMouthView::loadImages()] - "
                                    "Fatal Error: Image file is missing -> " << filename;
                    #endif

                    return;
                }
            }

            TOsd::self()->display(TOsd::Info, tr("Customized mouths are loaded!"));
            #ifdef TUP_DEBUG
                qDebug() << "[TupCustomizedMouthView::loadImages()] - "
                            "Customized mouths were loaded successfully!";
            #endif

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

    if (!assetsLoaded) {
        #ifdef TUP_DEBUG
            qDebug() << "[TupCustomizedMouthView::paintEvent()] - Fatal Error: Assets are not loaded!";
        #endif

        return;
    }

    QString phoneme;
    QImage img;
    QPainter dc(this);

    if (document && document->getVoice())
        phoneme = document->getPhonemeAtFrame(frame);
    else
		phoneme = "etc";

    if (phoneme.isEmpty() && document)
        phoneme = document->getVolumePhonemeAtFrame(frame);

    img = mouths.value(phoneme);
    if (!img.isNull()) {
        int32 x = 0, y = 0;
        int32 w = width();
        int32 h = height();
        QColor backCol(Qt::white);
        dc.fillRect(QRect(x, y, w, h), backCol);
        x = (w - (img.width()))/2;
        y = (h - (img.height()))/2;

        dc.drawImage(QPoint(x, y), img);
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

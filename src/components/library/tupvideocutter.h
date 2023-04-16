/***************************************************************************
 *   Project TUPITUBE DESK                                                *
 *   Project Contact: info@maefloresta.com                                 *
 *   Project Website: http://www.maefloresta.com                           *
 *   Project Leader: Gustav Gonzalez <info@maefloresta.com>                *
 *                                                                         *
 *   Developers:                                                           *
 *   2010:                                                                 *
 *    Gustavo Gonzalez / xtingray                                          *
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

#ifndef TUPVIDEOCUTTER_H
#define TUPVIDEOCUTTER_H

#include "tglobal.h"

#include <QObject>
#include <QDebug>

#ifdef __cplusplus
extern "C" {
// Libav libraries
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>

// Required to create the PNG files
#include <png.h>
}
#endif

class TUPITUBE_EXPORT TupVideoCutter : public QObject
{
    Q_OBJECT

    public:
        TupVideoCutter();
        ~TupVideoCutter();

        bool loadFile(const QString &videoFile, const QString &outputPath);
        void setPhotogramsTotal(int frames);
        bool startExtraction();
        QSize getVideoSize() const;
        void releaseResources();

    signals:
        void msgSent(const QString &msg);
        void imageExtracted(int index);

    private:
        // Decode packets into frames
        int decodePacket(AVPacket *pPacket, AVCodecContext *pCodecContext, AVFrame *pFrame);
        // Save a frame into a .png file
        int saveFrameToPng(AVFrame *frame, const QString &filename);

        QString filename;
        QString outputFolder;
        int imagesTotal;

        AVFormatContext *formatContext;
        AVCodecContext *inputCodecContext;
        int videoStreamIndex;
        AVFrame *inputFrame;
        AVPacket *inputPacket;
        QSize videoSize;
};

#endif

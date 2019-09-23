/***************************************************************************
 *   Project TUPITUBE DESK                                                *
 *   Project Contact: info@maefloresta.com                                 *
 *   Project Website: http://www.maefloresta.com                           *
 *   Project Leader: Gustav Gonzalez <info@maefloresta.com>                *
 *                                                                         *
 *   Developers:                                                           *
 *   2010:                                                                 *
 *    Gustav Gonzalez / xtingray                                           *
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

#ifndef TFFMPEGMOVIEGENERATOR_H
#define TFFMPEGMOVIEGENERATOR_H

#include "tglobal.h"
#include "tmoviegenerator.h"

#ifdef __cplusplus
extern "C" {
#include "libavutil/channel_layout.h"
#include "libavutil/mathematics.h"
#include "libavutil/opt.h"
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
// #include "libavresample/avresample.h"
// #include "libswscale/swscale.h"
}
#endif

#if LIBAVCODEC_VERSION_INT < AV_VERSION_INT(55,28,1)
#define av_frame_alloc  avcodec_alloc_frame
#endif

#define STREAM_DURATION 4.73684

class TUPITUBE_PLUGIN TFFmpegMovieGenerator : public TMovieGenerator
{
    public:
        TFFmpegMovieGenerator(TMovieGeneratorInterface::Format format, const QSize &size, int fps = 24,
                             double duration = 0);
        ~TFFmpegMovieGenerator();

        virtual bool validMovieHeader();
        virtual QString getErrorMsg() const;
        void saveMovie(const QString &filename);

    protected:
        void createMovieFile(const QString &fileName);
        virtual void handle(const QImage &image);
        virtual bool beginVideo();
        virtual void endVideo();

    private:
        bool openVideo(AVCodec *codec, AVStream *st);
        AVStream * addVideoStream(AVFormatContext *oc, AVCodec **codec, enum AVCodecID codec_id,
                                  const QString &movieFile, int width, int height, int fps);
        bool writeVideoFrame(const QString &movieFile, const QImage &image);
        void RGBtoYUV420P(const uint8_t *bufferRGB, uint8_t *bufferYUV, uint iRGBIncrement,
                          bool bSwapRGB, int width, int height);
        void chooseFileExtension(int format);
        void closeVideo(AVStream *st);

        AVFrame *videoFrame;
        AVStream *video_st;
        AVFormatContext *oc;
        AVOutputFormat *fmt;

        QString errorMsg;
        int frameCount;
        QString movieFile;
        int fps;
        double streamDuration;
        bool exception;

        bool hasSounds;
        int64_t next_pts;

        /*
        AVFrame *audioFrame;
        AVFrame *tmpAudioFrame;
        float t;
        float tincr;
        float tincr2;
        */
};

#endif

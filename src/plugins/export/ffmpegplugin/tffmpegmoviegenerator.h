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

#include <QObject>

#ifdef __cplusplus
extern "C" {
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libswresample/swresample.h"
#include "libswscale/swscale.h"
#include "libavutil/avutil.h"
#include "libavutil/channel_layout.h"
#include "libavutil/mathematics.h"
#include "libavutil/opt.h"
#include "libavutil/imgutils.h"

#include "libavutil/frame.h"
#include "libavutil/samplefmt.h"
#include "libavutil/timestamp.h"

// #include <stdio.h>
#include "libavformat/avio.h"
#include "libavutil/audio_fifo.h"
#include "libavutil/avassert.h"
#include "libavutil/avstring.h"
}
#endif

#if LIBAVCODEC_VERSION_INT < AV_VERSION_INT(55,28,1)
#define av_frame_alloc  avcodec_alloc_frame
#endif

#define STREAM_DURATION 4.73684

class TUPITUBE_PLUGIN TFFmpegMovieGenerator : public TMovieGenerator
{
    public:
        TFFmpegMovieGenerator(TMovieGeneratorInterface::Format format, const QSize &size,
                              int fps = 24, double duration = 0, const QString &audioPath = QString());
        ~TFFmpegMovieGenerator();

        virtual bool validMovieHeader();
        virtual QString getErrorMsg() const;
        bool createVideoFrame(const QImage &image);
        bool writeAudioStream();
        void saveMovie(const QString &filename);

    protected:
        void copyMovieFile(const QString &fileName);
        void handle(const QImage &image);
        void endVideoFile();

    private:
        bool initVideoFile();
        void setFileExtension(int format);
        bool openVideoStream();
        bool openAudioInputStream();
        bool openAudioOutputStream();
        bool openAudioOutputCodec();

        AVStream * addVideoStream();
        bool loadInputAudio(const QString &soundPath);
        int writeVideoFrame(AVPacket *pkt);
        void RGBtoYUV420P(const uint8_t *bufferRGB, uint8_t *bufferYUV, uint iRGBIncrement, bool bSwapRGB);

        void logPacket(MediaType type, AVRational timeBase, const AVPacket *pkt, const QString &direction);
        double av_q2d(AVRational a);
        QString formatTS(int64_t ts, AVRational tb);
        QString rationalToString(AVRational a);

        // Global Variables
        int videoW;
        int videoH;
        AVFormatContext *formatContext;
        AVOutputFormat *outputFormat;

        AVCodecContext *videoCodecContext;
        enum AVCodecID videoCodecID;
        const AVCodec *videoCodec;
        AVFrame *videoFrame;
        AVStream *videoStream;

        // Audio stuff
        bool hasSound;
        QString inputAudioPath;
        enum AVCodecID audioOutputCodecID;
        const AVCodec *audioOutputCodec;
        AVCodecContext *audioOutputCodecContext;
        AVStream *audioOutputStream;

        AVFormatContext *audioInputFormatContext;
        AVCodecContext *audioInputCodecContext;
        AVCodec *audioInputCodec;
        AVStream *audioInputStream;
        SwrContext *resampleContext;

        int videoPktCounter;
        int audioPktCounter;
        int samples_count;

        QString errorMsg;
        int framesCount;
        QString movieFile;
        int fps;
        double mp4Duration;
        bool exception;

        int realFrames;
};

#endif

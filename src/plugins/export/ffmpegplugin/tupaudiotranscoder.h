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

#ifndef TUPAUDIOTRANSCODER_H
#define TUPAUDIOTRANSCODER_H

#include "tglobal.h"
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

#include "libavformat/avio.h"
#include "libavutil/audio_fifo.h"
#include "libavutil/avassert.h"
#include "libavutil/avstring.h"
}
#endif

class TUPITUBE_PLUGIN TupAudioTranscoder : public QObject
{
    public:
        TupAudioTranscoder(const QString &input, const QString &output);
        ~TupAudioTranscoder();

        QString getErrorMsg() const;
        int transcodeAudio();

    private:
        void logAudioPacket(AVRational timeBase, const AVPacket *pkt, const QString &direction);
        double av_q2d(AVRational a);
        QString formatTS(int64_t ts, AVRational tb);
        QString rationalToString(AVRational a);

        // Audio methods
        int openInputFile(const char *filename,
                           AVFormatContext **inputFormatContext,
                           AVCodecContext **inputCodecContext);
        int openOutputFile(const char *filename,
                            AVCodecContext *inputCodecContext,
                            AVFormatContext **outputFormatContext,
                            AVCodecContext **outputCodecContext);

        int initPacket(AVPacket **packet);
        int initInputFrame(AVFrame **frame);
        int initResampler(AVCodecContext *inputCodecContext,
                          AVCodecContext *outputCodecContext,
                          SwrContext **resampleContext);
        int initFifo(AVAudioFifo **fifo, AVCodecContext *outputCodecContext);
        int writeOutputFileHeader(AVFormatContext *outputFormatContext);
        int decodeAudioFrame(AVFrame *frame,
                              AVFormatContext *inputFormatContext,
                              AVCodecContext *inputCodecContext,
                              int *dataPresent, int *finished);
        int initConvertedSamples(uint8_t ***convertedInputSamples,
                                  AVCodecContext *outputCodecContext,
                                  int frameSize);
        int convertSamples(const uint8_t **inputData,
                           uint8_t **convertedData, const int frameSize,
                           SwrContext *resampleContext);
        int addSamplesToFifo(AVAudioFifo *fifo,
                               uint8_t **convertedInputSamples,
                               const int frameSize);
        int readDecodeConvertAndStore(AVAudioFifo *fifo,
                                         AVFormatContext *inputFormatContext,
                                         AVCodecContext *inputCodecContext,
                                         AVCodecContext *outputCodecContext,
                                         SwrContext *resamplerContext,
                                         int *finished);
        int initOutputFrame(AVFrame **frame,
                             AVCodecContext *outputCodecContext,
                             int frameSize);

        int encodeAudioFrame(AVFrame *frame,
                                      AVFormatContext *outputFormatContext,
                                      AVCodecContext *outputCodecContext,
                                      int *dataPresent);

        int loadEncodeAndWrite(AVAudioFifo *fifo,
                                         AVFormatContext *outputFormatContext,
                                         AVCodecContext *outputCodecContext);

        int writeOutputFileTrailer(AVFormatContext *outputFormatContext);

        // Global Variables
        AVFormatContext *audioOutputFormatContext;
        AVOutputFormat *audioOutputFormat;

        QString audioInputFile;
        QString audioOutputFile;

        enum AVCodecID audioOutputCodecID;
        const AVCodec *audioOutputCodec;
        AVCodecContext *audioOutputCodecContext;
        AVStream *audioOutputStream;

        AVFormatContext *audioInputFormatContext;
        AVCodecContext *audioInputCodecContext;
        AVCodec *audioInputCodec;
        AVStream *audioInputStream;
        SwrContext *resampleContext;

        int audioPktCounter;
        int samples_count;

        QString errorMsg;
        int framesCount;
        bool exception;
};

#endif

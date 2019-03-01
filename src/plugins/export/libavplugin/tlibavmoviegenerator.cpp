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

#define INT64_C
#define __STDC_CONSTANT_MACROS
#include <stdint.h>

#include "tlibavmoviegenerator.h"
#include "talgorithm.h"
#include <QDir>

#ifdef __cplusplus
extern "C" {
#include "libavutil/channel_layout.h"
#include "libavutil/mathematics.h"
#include "libavutil/opt.h"
#include "libavformat/avformat.h"
#include "libavresample/avresample.h"
// #include "libswscale/swscale.h"
}
#endif

#if LIBAVCODEC_VERSION_INT < AV_VERSION_INT(55,28,1)
#define av_frame_alloc  avcodec_alloc_frame
#endif

#define STREAM_DURATION 4.73684

struct TLibavMovieGenerator::Private
{
    AVFrame *videoFrame;

    bool hasSounds;
    AVFrame *audioFrame;
    AVFrame *tmpAudioFrame;

    // pts of the next frame that will be generated
    int64_t next_pts;

    float t;
    float tincr;
    float tincr2;

    QString movieFile;
    int fps;
    int frameCount;
    double streamDuration;
    bool exception;
    QString errorMsg;

    AVStream *video_st;
    // AVStream *audio_st;
    // AVAudioResampleContext *avAudioR;

    AVFormatContext *oc;
    AVOutputFormat *fmt;

    void chooseFileExtension(int format);

    AVStream * addVideoStream(AVFormatContext *oc, AVCodec **codec, enum AVCodecID codec_id, 
                              const QString &movieFile, int width, int height, int fps);
	/*
    AVStream * addAudioStream(AVFormatContext *oc, AVCodec **codec, enum AVCodecID codec_id, 
                              AVAudioResampleContext *avAudioR);
    */

    bool openVideo(AVCodec *codec, AVStream *st);
    // bool openAudio(AVCodec *codec, AVStream *st);

    // Video methods
    void RGBtoYUV420P(const uint8_t *bufferRGB, uint8_t *bufferYUV, uint iRGBIncrement, bool bSwapRGB, int width, int height);
    bool writeVideoFrame(const QString &movieFile, const QImage &image);

    // Audio methods
	/*
    AVFrame *allocAudioFrame(enum AVSampleFormat sample_fmt, uint64_t channel_layout,
                             int sample_rate, int nb_samples);
    AVFrame * getAudioFrame();
    int encodeAudioFrame();
    bool writeAudioFrame();
	*/

    void closeVideo(AVStream *st);
    // void closeAudio();
};

/*
TLibavMovieGenerator::TLibavMovieGenerator(TMovieGeneratorInterface::Format format, int width, int height, 
                      int fps, double duration) : TMovieGenerator(width, height), k(new Private)
{
    k->movieFile = QDir::tempPath() + "/tupitube_video_" + TAlgorithm::randomString(12);
    k->chooseFileExtension(format);
    k->fps = fps;
    k->streamDuration = duration;
    k->next_pts = 0;
    k->hasSounds = false;

    k->exception = beginVideo();
}
*/

TLibavMovieGenerator::TLibavMovieGenerator(TMovieGeneratorInterface::Format format, const QSize &size, 
                      int fps, double duration) : TMovieGenerator(size.width(), size.height()), k(new Private)
{
    k->movieFile = QDir::tempPath() + "/tupitube_video_" + TAlgorithm::randomString(12);
    k->chooseFileExtension(format);
    k->fps = fps;
    k->streamDuration = duration;
    k->next_pts = 0;
    k->hasSounds = false;

    k->exception = beginVideo();
}

TLibavMovieGenerator::~TLibavMovieGenerator()
{
    delete k;
}

bool TLibavMovieGenerator::beginVideo()
{
    int ret;
    AVCodec *video_codec = 0;
    // AVCodec *audio_codec = 0;
    QString errorDetail = "This is not a problem directly related to <b>TupiTube Desk</b>. "
                          "Please, check your libav installation and codec support. "
                          "More info: <b>http://libav.org</b>";

    av_register_all();

    k->fmt = av_guess_format(NULL, k->movieFile.toLocal8Bit().data(), NULL);
    if (!k->fmt) {
        #ifdef TUP_DEBUG
            QString msg = QString("") + "TLibavMovieGenerator::beginVideo() - Can't guess format. Selecting MPEG by default...";
            #ifdef Q_OS_WIN
                qDebug() << msg;
            #else
                tError() << msg;
            #endif
        #endif

        k->fmt = av_guess_format("mpeg", NULL, NULL);
    }

    if (!k->fmt) {
        k->errorMsg = "libav error: Error while doing export. " + errorDetail; 
        return false;
    }

    k->oc = avformat_alloc_context();
    if (!k->oc) {
        fprintf(stderr, "Memory error\n");
        return false;
    }
    k->oc->oformat = k->fmt;

    if (!k->oc) {
        k->errorMsg = "libav error: Error while doing export. " + errorDetail;
        #ifdef TUP_DEBUG
            QString msg = QString("") + "TLibavMovieGenerator::beginVideo() - " + k->errorMsg;
            #ifdef Q_OS_WIN
                qDebug() << msg;
            #else
                tError() << msg;
            #endif
        #endif
        return false;
    }

    k->video_st = NULL;
    if (k->fmt->video_codec != AV_CODEC_ID_NONE)
        k->video_st = k->addVideoStream(k->oc, &video_codec, k->fmt->video_codec, k->movieFile, width(), height(), k->fps);

	/*
    if (k->hasSounds) {
        k->audio_st = NULL;
        if (k->fmt->audio_codec != AV_CODEC_ID_NONE)
            k->audio_st = k->addAudioStream(k->oc, &audio_codec, k->fmt->audio_codec, k->avAudioR);
    }
    */
	
    av_dump_format(k->oc, 0, k->movieFile.toLocal8Bit().data(), 1); 
	
    if (k->video_st) {
        bool success = k->openVideo(video_codec, k->video_st);
        if (!success) {
            k->errorMsg = "<b>libav error:</b> Could not initialize video codec.";
            #ifdef TUP_DEBUG
                QString msg = "TLibavMovieGenerator::beginVideo() - " + k->errorMsg;
                #ifdef Q_OS_WIN
                    qDebug() << msg;
                #else
                    tError() << msg;
                #endif
            #endif

            return false;
        }
    } else {
        k->errorMsg = "<b>libav error:</b> Video codec required is not installed. " + errorDetail;
        #ifdef TUP_DEBUG
            QString msg = "TLibavMovieGenerator::beginVideo() - " + k->errorMsg;
            #ifdef Q_OS_WIN
                qDebug() << msg;
            #else 
                tError() << msg;
            #endif
        #endif

        return false;
    }

	/*
    if (k->hasSounds) {
        if (k->audio_st) {
            bool success = k->openAudio(audio_codec, k->audio_st);  
            if (!success) {
                k->errorMsg = "<b>libav error:</b> Could not initialize audio codec.";
                #ifdef TUP_DEBUG
                    QString msg = "TLibavMovieGenerator::beginVideo() - " + k->errorMsg;
                    #ifdef Q_OS_WIN
                        qDebug() << msg;
                    #else
                        tError() << msg;
                    #endif
                #endif

                return false;
            }
        } else {
            k->errorMsg = "<b>libav error:</b> Audio codec required is not installed. " + errorDetail;
            #ifdef TUP_DEBUG
                QString msg = "TLibavMovieGenerator::beginVideo() - " + k->errorMsg;
                #ifdef Q_OS_WIN
                    qDebug() << msg;
                #else
                    tError() << msg;
                #endif
            #endif

            return false;
        }
    }
	*/

    if (!(k->fmt->flags & AVFMT_NOFILE)) {
        ret = avio_open(&k->oc->pb, k->movieFile.toLocal8Bit().data(), AVIO_FLAG_WRITE);
        if (ret < 0) {
            k->errorMsg = "libav error: could not open video file";
            #ifdef TUP_DEBUG
                QString msg = QString("") + "TLibavMovieGenerator::beginVideo() - " + k->errorMsg;
                #ifdef Q_OS_WIN
                    qDebug() << msg;
                #else
                    tError() << msg;
                #endif
            #endif
            return false;
        }
    }

    avformat_write_header(k->oc, NULL);

    if (k->videoFrame)
        k->videoFrame->pts = 0;

    k->frameCount = 0;

    return true;
}

AVStream * TLibavMovieGenerator::Private::addVideoStream(AVFormatContext *oc, AVCodec **codec, enum AVCodecID codec_id, 
                                 const QString &movieFile, int width, int height, int fps)
{
    #ifdef TUP_DEBUG
        QString msg = "TLibavMovieGenerator::addVideoStream() - codec_id: " + QString::number(codec_id);
        #ifdef Q_OS_WIN
            qWarning() << msg;
        #else
            tWarning() << msg;
        #endif
    #endif

    AVCodecContext *c;
    AVStream *st;
    QString errorMsg = "";

    // Find the video encoder
    *codec = avcodec_find_encoder(codec_id);
    if (!(*codec)) {
        errorMsg = "libav error: Could not find video encoder.";
        #ifdef TUP_DEBUG
            QString msg1 = "TLibavMovieGenerator::addVideoStream() - " + errorMsg;
            QString msg2 = "TLibavMovieGenerator::addVideoStream() - Unavailable Codec ID: " + QString::number(codec_id);
            #ifdef Q_OS_WIN
                qDebug() << msg1;
                qDebug() << msg2;
            #else
                tError() << msg1;
                tError() << msg2;
            #endif
        #endif

        return NULL;
    }

    st = avformat_new_stream(oc, *codec);
    if (!st) {
        errorMsg = "libav error: Could not video alloc stream."; 
        #ifdef TUP_DEBUG
            QString msg = QString("") + "TLibavMovieGenerator::addVideoStream() - " + errorMsg;
            #ifdef Q_OS_WIN
                qDebug() << msg;
            #else
                tError() << msg;
            #endif
        #endif

        return NULL;
    }
    st->id = oc->nb_streams-1;

    c = st->codec;
    c->codec_id = codec_id;

    // Put sample parameters
    c->bit_rate = 6000000;
    if (fps == 1)
        c->bit_rate = 4000000;

    // Resolution must be a multiple of two
    c->width = width;  
    c->height = height; 

    c->gop_size = 12;
    c->max_b_frames = 0;

    c->time_base.num = 1;
    c->time_base.den = fps;

    if (movieFile.endsWith("gif", Qt::CaseInsensitive)) {
        st->time_base.num = 1;
        st->time_base.den = fps;
        c->pix_fmt = AV_PIX_FMT_RGB24;
    } else {
        c->pix_fmt = AV_PIX_FMT_YUV420P;
    }

    if (c->codec_id == AV_CODEC_ID_MPEG2VIDEO) {
	// Just for testing, we also add B frames
	c->max_b_frames = 2;
    }

    if (c->codec_id == AV_CODEC_ID_MPEG1VIDEO) {
        /* needed to avoid using macroblocks in which some coeffs overflow
           this doesnt happen with normal video, it just happens here as the
           motion of the chroma plane doesnt match the luma plane */
           c->mb_decision=2;
    }

    if (oc->oformat->flags & AVFMT_GLOBALHEADER)
        c->flags |= CODEC_FLAG_GLOBAL_HEADER;

    return st;
}

/*
AVStream * TLibavMovieGenerator::Private::addAudioStream(AVFormatContext *oc, AVCodec **codec, enum AVCodecID codec_id, AVAudioResampleContext *avAudioR)
{
    AVStream *st;
    AVCodecContext *c;
    int ret;

    // Find the audio encoder
    *codec = avcodec_find_encoder(codec_id);
    if (!(*codec)) {
        #ifdef TUP_DEBUG
            QString msg1 = "TLibavMovieGenerator::addAudioStream() - Fatal Error: Could not find audio codec!";
            QString msg2 = "TLibavMovieGenerator::addAudioStream() - Unavailable Codec ID: " + QString::number(codec_id);
            #ifdef Q_OS_WIN
                qDebug() << msg1;
                qDebug() << msg2;
            #else
                tError() << msg1;
                qDebug() << msg2;
            #endif
        #endif

        return NULL;
    }

    st = avformat_new_stream(oc, *codec);
    if (!st) {
        #ifdef TUP_DEBUG
            QString msg = "TLibavMovieGenerator::addAudioStream() - Fatal Error: Could not alloc stream!";
            #ifdef Q_OS_WIN
                qDebug() << msg;
            #else
                tError() << msg;
            #endif
        #endif

        return NULL;
    }

    c = st->codec;

    // Put sample parameters
    c->sample_fmt     = (*codec)->sample_fmts           ? (*codec)->sample_fmts[0]           : AV_SAMPLE_FMT_S16;
    c->sample_rate    = (*codec)->supported_samplerates ? (*codec)->supported_samplerates[0] : 44100;
    c->channel_layout = (*codec)->channel_layouts       ? (*codec)->channel_layouts[0]       : AV_CH_LAYOUT_STEREO;
    c->channels       = av_get_channel_layout_nb_channels(c->channel_layout);
    c->bit_rate       = 64000;

    st->time_base = (AVRational){ 1, c->sample_rate };

    // Some formats want stream headers to be separate
    if (oc->oformat->flags & AVFMT_GLOBALHEADER)
        c->flags |= CODEC_FLAG_GLOBAL_HEADER;

    // initialize sample format conversion;
    // to simplify the code, we always pass the data through lavr, even
    // if the encoder supports the generated format directly -- the price is
    // some extra data copying;
 
    avAudioR = avresample_alloc_context();
    if (!avAudioR) {
        #ifdef TUP_DEBUG
            QString msg = "TLibavMovieGenerator::addAudioStream() - Fatal Error: Could not allocate the resampling context!";
            #ifdef Q_OS_WIN
                qDebug() << msg;
            #else
                tError() << msg;
            #endif
        #endif

        return NULL;
    }

    av_opt_set_int(avAudioR, "in_sample_fmt", AV_SAMPLE_FMT_S16, 0);
    av_opt_set_int(avAudioR, "in_sample_rate", 44100, 0);
    av_opt_set_int(avAudioR, "in_channel_layout", AV_CH_LAYOUT_STEREO, 0);
    av_opt_set_int(avAudioR, "out_sample_fmt", c->sample_fmt, 0);
    av_opt_set_int(avAudioR, "out_sample_rate", c->sample_rate, 0);
    av_opt_set_int(avAudioR, "out_channel_layout", c->channel_layout, 0);

    ret = avresample_open(avAudioR);
    if (ret < 0) {
        #ifdef TUP_DEBUG
            QString msg = "TLibavMovieGenerator::addAudioStream() - Fatal Error: Could not open the resampling context!";
            #ifdef Q_OS_WIN
                qDebug() << msg;
            #else
                tError() << msg;
            #endif
        #endif

        return NULL;
    }

    return st;
}
*/

bool TLibavMovieGenerator::Private::openVideo(AVCodec *codec, AVStream *st)
{
    int ret;
    AVCodecContext *c = st->codec;

    // Open the codec
    ret = avcodec_open2(c, codec, NULL);
    if (ret < 0) {
        QString errorMsg = "Sorry, the video codec required is not installed in your system.";
        #ifdef TUP_DEBUG
            QString msg = "TLibavMovieGenerator::openVideo() - " + errorMsg;
            #ifdef Q_OS_WIN
                qDebug() << msg;
            #else
                tError() << msg;
            #endif
        #endif

        return false;
    }

    // Allocate and init a re-usable frame
    videoFrame = av_frame_alloc();

    if (!videoFrame) {
        errorMsg = "There is no available memory to export your project as a video";
        #ifdef TUP_DEBUG
            QString msg = "TLibavMovieGenerator::openVideo() - " + errorMsg;
            #ifdef Q_OS_WIN
                qDebug() << msg;
            #else
                tError() << msg;
            #endif
        #endif

        return false;
    }
	
    return true;
}

/*
AVFrame * TLibavMovieGenerator::Private::allocAudioFrame(enum AVSampleFormat sample_fmt, uint64_t channel_layout,
                                                         int sample_rate, int nb_samples)
{
    #ifdef TUP_DEBUG
        QString msg = "TLibavMovieGenerator::allocAudioFrame()";
        #ifdef Q_OS_WIN
            qWarning() << msg;
        #else
            tError() << msg;
        #endif
    #endif

    AVFrame *frame = av_frame_alloc();
    int ret;

    if (!frame) {
        #ifdef TUP_DEBUG
            QString msg = "TLibavMovieGenerator::allocAudioFrame() - "
                          "Fatal Error: Can't not allocate audio frame";
            #ifdef Q_OS_WIN
                qDebug() << msg;
            #else
                tError() << msg;
            #endif
        #endif

        return NULL;
    }

    frame->format = sample_fmt;
    frame->channel_layout = channel_layout;
    frame->sample_rate = sample_rate;
    frame->nb_samples = nb_samples;

    if (nb_samples) {
        ret = av_frame_get_buffer(frame, 0);
        if (ret < 0) {
            #ifdef TUP_DEBUG
                QString msg = "TLibavMovieGenerator::allocAudioFrame() - Fatal Error: Can't not allocate audio buffer";
                #ifdef Q_OS_WIN
                    qDebug() << msg;
                #else
                    tError() << msg;
                #endif
            #endif

            return NULL;
        }
    }

    return frame;
}

bool TLibavMovieGenerator::Private::openAudio(AVCodec *codec, AVStream *st)
{
    #ifdef TUP_DEBUG
        QString msg = "TLibavMovieGenerator::openAudio()";
        #ifdef Q_OS_WIN
            qWarning() << msg;
        #else
            tError() << msg;
        #endif
    #endif

    AVCodecContext *c;
    int nb_samples;

    c = st->codec;

    // Open sound codec 
    if (avcodec_open2(c, codec, NULL) < 0) {
        #ifdef TUP_DEBUG
            QString msg = "TLibavMovieGenerator::openAudio() - Fatal Error: Could not open audio codec";
            #ifdef Q_OS_WIN
                qDebug() << msg;
            #else
                tError() << msg;
            #endif
        #endif

        return false;
    }

    // Init signal generator
    t = 0;
    tincr = 2 * M_PI * 110.0 / c->sample_rate;

    // Increment frequency by 110 Hz per second 
    tincr2 = 2 * M_PI * 110.0 / c->sample_rate / c->sample_rate;

    if (c->codec->capabilities & CODEC_CAP_VARIABLE_FRAME_SIZE)
        nb_samples = 10000;
    else
        nb_samples = c->frame_size;

    audioFrame = allocAudioFrame(c->sample_fmt, c->channel_layout, c->sample_rate, nb_samples);
    tmpAudioFrame = allocAudioFrame(AV_SAMPLE_FMT_S16, AV_CH_LAYOUT_STEREO, 44100, nb_samples);

    return true;audio
}
*/

#define RGBtoYUV(r, g, b, y, u, v) \
  y = (uint8_t)(((int)30*r + (int)59*g + (int)11*b)/100); \
  u = (uint8_t)(((int)-17*r - (int)33*g + (int)50*b + 12800)/100); \
  v = (uint8_t)(((int)50*r - (int)42*g - (int)8*b + 12800)/100);

void TLibavMovieGenerator::Private::RGBtoYUV420P(const uint8_t *bufferRGB, uint8_t *bufferYUV, uint iRGBIncrement, bool bSwapRGB, int width, int height)
{
    const unsigned iPlaneSize = width * height;
    const unsigned iHalfWidth = width >> 1;

    // get pointers to the data
    uint8_t *yplane  = bufferYUV;
    uint8_t *uplane  = bufferYUV + iPlaneSize;
    uint8_t *vplane  = bufferYUV + iPlaneSize + (iPlaneSize >> 2);
    const uint8_t *bufferRGBIndex = bufferRGB;

    int iRGBIdx[3];
    iRGBIdx[0] = 0;
    iRGBIdx[1] = 1;
    iRGBIdx[2] = 2;
    if (bSwapRGB)  {
        iRGBIdx[0] = 2;
        iRGBIdx[2] = 0;
    }
 
    for (int y = 0; y < height; y++) {
         uint8_t *yline  = yplane + (y * width);
         uint8_t *uline  = uplane + ((y >> 1) * iHalfWidth);
         uint8_t *vline  = vplane + ((y >> 1) * iHalfWidth);

         for (int x=0; x<width; x+=2) {
              RGBtoYUV(bufferRGBIndex[iRGBIdx[0]], bufferRGBIndex[iRGBIdx[1]], bufferRGBIndex[iRGBIdx[2]], *yline, *uline, *vline);
              bufferRGBIndex += iRGBIncrement;
              yline++;
              RGBtoYUV(bufferRGBIndex[iRGBIdx[0]], bufferRGBIndex[iRGBIdx[1]], bufferRGBIndex[iRGBIdx[2]], *yline, *uline, *vline);
              bufferRGBIndex += iRGBIncrement;
              yline++;
              uline++;
              vline++;
         }
    }
}

bool TLibavMovieGenerator::Private::writeVideoFrame(const QString &movieFile, const QImage &image)
{
    #ifdef TUP_DEBUG
        QString msg = "TLibavMovieGenerator::writeVideoFrame() - Generating frame #" + QString::number(frameCount);
        #ifdef Q_OS_WIN
            qWarning() << msg;
        #else
            tWarning() << msg;
        #endif
    #endif

    AVCodecContext *c = video_st->codec;
    int w = c->width;
    int h = c->height;

    int got_output;
    AVPacket pkt;
    av_init_packet(&pkt);
    pkt.data = NULL; // packet data will be allocated by the encoder
    pkt.size = 0;

    if (movieFile.endsWith("gif", Qt::CaseInsensitive)) {
        QImage img = image.convertToFormat(Format_RGB888);
        avpicture_fill((AVPicture *)videoFrame, img.bits(), AV_PIX_FMT_RGB24, w, h);
    } else { 
        int size = avpicture_get_size(AV_PIX_FMT_YUV420P, w, h);
        uint8_t *pic_dat = (uint8_t *) av_malloc(size);
        RGBtoYUV420P(image.bits(), pic_dat, image.depth()/8, true, w, h);
        avpicture_fill((AVPicture *)videoFrame, pic_dat, AV_PIX_FMT_YUV420P, w, h);
        // videoFrame->pts += av_rescale_q(1, video_st->codec->time_base, video_st->time_base);
        videoFrame->pts += av_rescale_q(1, video_st->time_base, video_st->time_base);
    }

    int ret = avcodec_encode_video2(c, &pkt, videoFrame, &got_output);
    if (ret < 0) {
        errorMsg = "[1] Error while encoding the video of your project";
        #ifdef TUP_DEBUG
            QString msg = QString("") + "TLibavMovieGenerator::writeVideoFrame() - " + errorMsg;
            #ifdef Q_OS_WIN
                qDebug() << msg;
            #else
                tError() << msg;
            #endif
        #endif

        return false;
    }

    // If size is zero, it means the image was buffered.
    if (got_output) {
        if (c->coded_frame->key_frame)
            pkt.flags |= AV_PKT_FLAG_KEY;
        pkt.stream_index = video_st->index;

        // Write the compressed frame to the media file. 
        ret = av_interleaved_write_frame(oc, &pkt);
        av_free_packet(&pkt);
    } else {
        #ifdef TUP_DEBUG
            QString msg = QString("") + "TLibavMovieGenerator::writeVideoFrame() - Frame ignored! -> " + QString::number(frameCount);
            #ifdef Q_OS_WIN
                qDebug() << msg;
            #else
                tWarning() << msg;
            #endif
        #endif
        ret = 0;
    }

    if (ret != 0) {
        errorMsg = "[2] Error while encoding the video of your project";
        #ifdef TUP_DEBUG
            QString msg = QString("") + "TLibavMovieGenerator::writeVideoFrame() - " + errorMsg;
            #ifdef Q_OS_WIN
                qDebug() << msg;
            #else
                tError() << msg;
            #endif
        #endif

        return false;
    }
    frameCount++;

    return true;
}

/*
AVFrame * TLibavMovieGenerator::Private::getAudioFrame()
{
    #ifdef TUP_DEBUG
        QString msg = "TLibavMovieGenerator::getAudioFrame()";
        #ifdef Q_OS_WIN
            qWarning() << msg;
        #else
            tError() << msg;
        #endif
    #endif

    AVFrame *frame = tmpAudioFrame;avsaa
    int j, i, v;
    int16_t *q = (int16_t*) frame->data[0];

    // Check if we want to generate more frames
    printf("\n");
    printf("next_pts: %" PRId64 "\n", next_pts);
    printf(" Num: %i\n", audio_st->codec->time_base.num);
    printf(" Den: %i\n", audio_st->codec->time_base.den);

    if (av_compare_ts(next_pts, audio_st->codec->time_base, STREAM_DURATION, (AVRational){ 1, 1 }) >= 0) {
        // tError() << "TLibavMovieGenerator::getAudioFrame() - No more frames are required!";
        return NULL;
    }

    for (j = 0; j < frame->nb_samples; j++) {
        v = (int)(sin(t) * 10000);
        for (i = 0; i < audio_st->codec->channels; i++)
            *q++ = v;
        t += tincr;
        tincr += tincr2;
    }

    return frame;
}

int TLibavMovieGenerator::Private::encodeAudioFrame()
{
    #ifdef TUP_DEBUG
        QString msg = "TLibavMovieGenerator::encodeAudioFrame()";
        #ifdef Q_OS_WIN
            qWarning() << msg;
        #else
            tError() << msg;
        #endif
    #endif

    // Data and size must be 0 
    AVPacket pkt;  // = { 0 };
    int got_packet;

    av_init_packet(&pkt);
    avcodec_encode_audio2(audio_st->codec, &pkt, audioFrame, &got_packet);

    if (got_packet) {
        pkt.stream_index = audio_st->index;

        av_packet_rescale_ts(&pkt, audio_st->codec->time_base, audio_st->time_base);

        // Write the compressed frame to the media file.
        if (av_interleaved_write_frame(oc, &pkt) != 0) {
            // tError() << "TLibavMovieGenerator::encodeAudioFrame() - Error while writing audio frame!";

            return 0;
        }
    }

    return (audioFrame || got_packet) ? 0 : 1;
}

bool TLibavMovieGenerator::Private::writeAudioFrame()
{
    #ifdef TUP_DEBUG
        QString msg = "TLibavMovieGenerator::writeAudioFrame()";
        #ifdef Q_OS_WIN
            qWarning() << msg;
        #else
            tError() << msg;
        #endif
    #endif

    AVFrame *frame;
    int got_output = 0;
    int ret;

    frame = getAudioFrame();
    got_output |= !!frame;

    // Feed the data to avAudioR
    if (frame) {
        ret = avresample_convert(avAudioR, NULL, 0, 0, frame->extended_data, frame->linesize[0], frame->nb_samples);
        if (ret < 0) {
            // tError() << "TLibavMovieGenerator::writeAudioFrame() - Error feeding audio data to the resampler";

            return false;
        }
    } else {
        // tError() << "TLibavMovieGenerator::writeAudioFrame() - Frame is NULL!!!";

        return false;
    }

    while ((frame && avresample_available(avAudioR) >= audioFrame->nb_samples) ||
           (!frame && avresample_get_out_samples(avAudioR, 0))) {
        // When we pass a frame to the encoder, it may keep a reference to it
        // internally. Make sure we do not overwrite it here
        ret = av_frame_make_writable(audioFrame);
        if (ret < 0) { 
            // tError() << "TLibavMovieGenerator::writeAudioFrame() - ret < 0!!!";

            return false;
        }

        // the difference between the two avresample calls here is that the
        // first one just reads the already converted data that is buffered in
        // the lavr output buffer, while the second one also flushes the
        // resampler
        if (frame) {
            ret = avresample_read(avAudioR, audioFrame->extended_data,
                                  audioFrame->nb_samples);
        } else {
            ret = avresample_convert(avAudioR, audioFrame->extended_data,
                                     audioFrame->linesize[0], audioFrame->nb_samples,
                                     NULL, 0, 0);
        }

        if (ret < 0) {
            // tError() << "TLibavMovieGenerator::writeAudioFrame() - Error while resampling!";

            return false;
        } else if (frame && ret != audioFrame->nb_samples) {
            // tError() << "TLibavMovieGenerator::writeAudioFrame() - Too few samples returned from lavr!";

            return false;
        }

        audioFrame->nb_samples = ret;

        audioFrame->pts = next_pts++;
        next_pts += audioFrame->nb_samples;

        got_output |= encodeAudioFrame();
    }

    // return !got_output;

    return true;
}
*/

void TLibavMovieGenerator::handle(const QImage &image)
{
    if (!k->video_st) {
        #ifdef TUP_DEBUG
            QString msg = "TLibavMovieGenerator::handle() - The total of frames has been "
                          "processed (" + QString::number(k->streamDuration) + " seg)";
            #ifdef Q_OS_WIN
                qWarning() << msg;
            #else
                tWarning() << msg;
            #endif
        #endif

        return;
    }

    #ifdef TUP_DEBUG
        QString msg1 = "TLibavMovieGenerator::handle() - Duration: " + QString::number(k->streamDuration); 
        #ifdef Q_OS_WIN
            qWarning() << msg1;
        #else
            tWarning() << msg1;
        #endif
    #endif

    k->writeVideoFrame(k->movieFile, image);
	/*
    if (k->hasSounds)
        k->writeAudioFrame();
	*/
}

void TLibavMovieGenerator::Private::chooseFileExtension(int format)
{
    switch (format) {
        case WEBM:
            movieFile += ".webm";
        break;

        /* SQA: Obsolete format
        case SWF:
            movieFile += ".swf";
        break;
        */

        case MP4:
            movieFile += ".mp4";
        break;

        /* SQA: MPEG codec was removed because it crashes. Check the issue!
        case MPEG:
            movieFile += ".mpg";
        break;
        */

        /* SQA: Obsolete format
        case ASF:
            movieFile += ".asf";
        break;
        */

        case AVI:
            movieFile += ".avi";
        break;
        case MOV:
            movieFile += ".mov";
        break;
        case GIF:
            movieFile += ".gif";
        break;
        default:
            movieFile += ".mpg";
        break;
    }
}

void TLibavMovieGenerator::Private::closeVideo(AVStream *st)
{
    avcodec_close(st->codec);
    // av_free(videoFrame);
    av_frame_free(&videoFrame);
}

/*
void TLibavMovieGenerator::Private::closeAudio()
{
    avcodec_close(audio_st->codec);

    if (audioFrame)
        av_frame_free(&audioFrame);
    if (tmpAudioFrame)
        av_frame_free(&tmpAudioFrame);
    avresample_free(&avAudioR);
}
*/

void TLibavMovieGenerator::saveMovie(const QString &filename) 
{
    endVideo();
    createMovieFile(filename);
}

bool TLibavMovieGenerator::validMovieHeader() 
{ 
    return k->exception;
}

QString TLibavMovieGenerator::getErrorMsg() const
{
    return k->errorMsg;
}

void TLibavMovieGenerator::endVideo()
{
    av_write_trailer(k->oc);

    if (k->video_st)
        k->closeVideo(k->video_st);

	/*
    if (k->hasSounds) {
        if (k->audio_st)
            k->closeAudio();
    }
	*/

    if (!(k->fmt->flags & AVFMT_NOFILE))
        avio_close(k->oc->pb);

    avformat_free_context(k->oc);
}

void TLibavMovieGenerator::createMovieFile(const QString &fileName)
{
    if (QFile::exists(fileName)) 
        QFile::remove(fileName);

    if (QFile::copy(k->movieFile, fileName)) {
        if (QFile::exists(k->movieFile)) {
            if (QFile::remove(k->movieFile)) {
                #ifdef TUP_DEBUG
                    QString msg = QString("") + "TLibavMovieGenerator::createMovieFile() - Removing temp video file -> " + k->movieFile;
                    #ifdef Q_OS_WIN
                        qDebug() << msg;
                    #else
                        tWarning() << msg;
                    #endif
                #endif
            }
        }
    }
}

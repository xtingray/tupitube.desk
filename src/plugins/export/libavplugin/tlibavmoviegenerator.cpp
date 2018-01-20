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

#define STREAM_DURATION   10.0

struct TLibavMovieGenerator::Private
{
    AVFrame *videoFrame;
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
    AVStream *audio_st;
    AVAudioResampleContext *avAudioR;

    AVFormatContext *oc;
    AVOutputFormat *fmt;

    void chooseFileExtension(int format);

    bool openVideo(AVCodec *codec, AVStream *st, const QString &errorDetail);
    bool openAudio(AVStream *st);

    AVFrame *allocAudioFrame(enum AVSampleFormat sample_fmt, uint64_t channel_layout,
                               int sample_rate, int nb_samples);

    void RGBtoYUV420P(const uint8_t *bufferRGB, uint8_t *bufferYUV, uint iRGBIncrement, bool bSwapRGB, int width, int height);

    bool writeVideoFrame(const QString &movieFile, const QImage &image);

    AVFrame * getAudioFrame();
    int encodeAudioFrame();
    bool writeAudioFrame();

    void closeVideo(AVStream *st);
    void closeAudio();
};

static AVStream *addVideoStream(AVFormatContext *oc, AVCodec **codec, enum AVCodecID codec_id, const QString &movieFile, 
                                int width, int height, int fps, const QString &errorDetail)
{
    /*
    #ifdef TUP_DEBUG
        QString msg = "TLibavMovieGenerator::addVideoStream() - codec_id: " + QString::number(codec_id);
        #ifdef Q_OS_WIN
            qWarning() << msg;
        #else
            tWarning() << msg;
        #endif
    #endif
    */

    AVCodecContext *c;
    AVStream *st;
    QString errorMsg = "";

    // Find the encoder
    *codec = avcodec_find_encoder(codec_id);
    if (!(*codec)) {
        errorMsg = "libav error: Could not find encoder. " + errorDetail;
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
        return 0;
    }

    st = avformat_new_stream(oc, *codec);
    if (!st) {
        errorMsg = "libav error: Could not alloc stream. " + errorDetail; 
        #ifdef TUP_DEBUG
            QString msg = QString("") + "TLibavMovieGenerator::addVideoStream() - " + errorMsg;
            #ifdef Q_OS_WIN
                qDebug() << msg;
            #else
                tError() << msg;
            #endif
        #endif
        return 0;
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
	/* just for testing, we also add B frames */
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

static AVStream * addAudioStream(AVFormatContext *oc, enum AVCodecID codec_id, AVAudioResampleContext *avAudioR)
{
    AVStream *st;
    AVCodecContext *c;
    AVCodec *codec;
    int ret;

    /* find the audio encoder */
    codec = avcodec_find_encoder(codec_id);
    if (!codec) {
        fprintf(stderr, "codec not found\n");
        exit(1);
    }

    st = avformat_new_stream(oc, codec);
    if (!st) {
        fprintf(stderr, "Could not alloc stream\n");
        exit(1);
    }

    c = st->codec;

    /* put sample parameters */
    c->sample_fmt     = codec->sample_fmts           ? codec->sample_fmts[0]           : AV_SAMPLE_FMT_S16;
    c->sample_rate    = codec->supported_samplerates ? codec->supported_samplerates[0] : 44100;
    c->channel_layout = codec->channel_layouts       ? codec->channel_layouts[0]       : AV_CH_LAYOUT_STEREO;
    c->channels       = av_get_channel_layout_nb_channels(c->channel_layout);
    c->bit_rate       = 64000;

    st->time_base = (AVRational){ 1, c->sample_rate };

    // some formats want stream headers to be separate
    if (oc->oformat->flags & AVFMT_GLOBALHEADER)
        c->flags |= CODEC_FLAG_GLOBAL_HEADER;

    /* initialize sample format conversion;
     * to simplify the code, we always pass the data through lavr, even
     * if the encoder supports the generated format directly -- the price is
     * some extra data copying;
     */
    avAudioR = avresample_alloc_context();
    if (!avAudioR) {
        fprintf(stderr, "Error allocating the resampling context\n");
        exit(1);
    }

    av_opt_set_int(avAudioR, "in_sample_fmt", AV_SAMPLE_FMT_S16, 0);
    av_opt_set_int(avAudioR, "in_sample_rate", 44100, 0);
    av_opt_set_int(avAudioR, "in_channel_layout", AV_CH_LAYOUT_STEREO, 0);
    av_opt_set_int(avAudioR, "out_sample_fmt", c->sample_fmt, 0);
    av_opt_set_int(avAudioR, "out_sample_rate", c->sample_rate, 0);
    av_opt_set_int(avAudioR, "out_channel_layout", c->channel_layout, 0);

    ret = avresample_open(avAudioR);
    if (ret < 0) {
        fprintf(stderr, "Error opening the resampling context\n");
        exit(1);
    }

    return st;
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

bool TLibavMovieGenerator::Private::openVideo(AVCodec *codec, AVStream *st, const QString &errorDetail)
{
    int ret;
    AVCodecContext *c = st->codec;

    /* open the codec */
    ret = avcodec_open2(c, codec, NULL);
    if (ret < 0) {
        QString errorMsg = "The video codec required is not installed in your system. " + errorDetail;
        #ifdef TUP_DEBUG
            QString msg = QString("") + "TLibavMovieGenerator::openVideo() - " + errorMsg;
            #ifdef Q_OS_WIN
                qDebug() << msg;
            #else
                tError() << msg;
            #endif
        #endif
        return false;
    }

    /* allocate and init a re-usable frame */
    videoFrame = av_frame_alloc();

    if (!videoFrame) {
        errorMsg = "There is no available memory to export your project as a video";
        #ifdef TUP_DEBUG
            QString msg = QString("") + "TLibavMovieGenerator::openVideo() - " + errorMsg;
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

AVFrame *allocAudioFrame(enum AVSampleFormat sample_fmt, uint64_t channel_layout,
                         int sample_rate, int nb_samples)
{
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
        return 0;
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
            return 0;
        }
    }

    return frame;
}

bool TLibavMovieGenerator::Private::openAudio(AVStream *st)
{
    AVCodecContext *c;
    int nb_samples;

    c = st->codec;

    // Open sound codec 
    if (avcodec_open2(c, NULL, NULL) < 0) {
        #ifdef TUP_DEBUG
            QString msg = "TLibavMovieGenerator::openAudio() - Fatal Error: Could not open codec";
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

    return true;
}

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
        videoFrame->pts += av_rescale_q(1, video_st->codec->time_base, video_st->time_base);
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
    } else {
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

AVFrame * TLibavMovieGenerator::Private::getAudioFrame()
{
    AVFrame *frame = tmpAudioFrame;
    int j, i, v;
    int16_t *q = (int16_t*) audioFrame->data[0];

    // Check if we want to generate more frames
    if (av_compare_ts(next_pts, audio_st->codec->time_base,
                      STREAM_DURATION, (AVRational){ 1, 1 }) >= 0)
        return NULL;

    for (j = 0; j < audioFrame->nb_samples; j++) {
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
    // Data and size must be 0 
    AVPacket pkt;  // = { 0 };
    int got_packet;

    av_init_packet(&pkt);
    avcodec_encode_audio2(audio_st->codec, &pkt, audioFrame, &got_packet);

    if (got_packet) {
        pkt.stream_index = audio_st->index;

        av_packet_rescale_ts(&pkt, audio_st->codec->time_base, audio_st->time_base);

        /* Write the compressed frame to the media file. */
        if (av_interleaved_write_frame(oc, &pkt) != 0) {
            fprintf(stderr, "Error while writing audio frame\n");
            exit(1);
        }
    }

    return (audioFrame || got_packet) ? 0 : 1;
}

bool TLibavMovieGenerator::Private::writeAudioFrame()
{
    AVFrame *frame;
    int got_output = 0;
    int ret;

    frame = getAudioFrame();
    got_output |= !!frame;

    // Feed the data to avAudioR
    if (frame) {
        ret = avresample_convert(avAudioR, NULL, 0, 0,
                                 frame->extended_data, frame->linesize[0],
                                 frame->nb_samples);
        if (ret < 0) {
            fprintf(stderr, "Error feeding audio data to the resampler\n");
            exit(1);
        }
    }

    while ((frame && avresample_available(avAudioR) >= audioFrame->nb_samples) ||
           (!frame && avresample_get_out_samples(avAudioR, 0))) {
        // When we pass a frame to the encoder, it may keep a reference to it
        // internally. Make sure we do not overwrite it here
        ret = av_frame_make_writable(audioFrame);
        if (ret < 0)
            exit(1);

        /* the difference between the two avresample calls here is that the
         * first one just reads the already converted data that is buffered in
         * the lavr output buffer, while the second one also flushes the
         * resampler */
        if (frame) {
            ret = avresample_read(avAudioR, audioFrame->extended_data,
                                  audioFrame->nb_samples);
        } else {
            ret = avresample_convert(avAudioR, audioFrame->extended_data,
                                     audioFrame->linesize[0], audioFrame->nb_samples,
                                     NULL, 0, 0);
        }

        if (ret < 0) {
            fprintf(stderr, "Error while resampling\n");
            exit(1);
        } else if (frame && ret != audioFrame->nb_samples) {
            fprintf(stderr, "Too few samples returned from lavr\n");
            exit(1);
        }

        audioFrame->nb_samples = ret;

        audioFrame->pts = next_pts;
        next_pts += audioFrame->nb_samples;

        got_output |= encodeAudioFrame();
    }

    // return !got_output;

    return true;
}

void TLibavMovieGenerator::Private::closeVideo(AVStream *st)
{
    avcodec_close(st->codec);
    // av_free(videoFrame);
    av_frame_free(&videoFrame);
}

void TLibavMovieGenerator::Private::closeAudio()
{
    avcodec_close(audio_st->codec);
    av_frame_free(&audioFrame);
    av_frame_free(&tmpAudioFrame);
    avresample_free(&avAudioR);
}

void TLibavMovieGenerator::saveMovie(const QString &filename) 
{
    endVideo();
    createMovieFile(filename);
}

TLibavMovieGenerator::TLibavMovieGenerator(TMovieGeneratorInterface::Format format, int width, int height, 
                      int fps, double duration) : TMovieGenerator(width, height), k(new Private)
{
    errorDetail = "This is not a problem directly related to <b>TupiTube Desk</b>. "
                  "Please, check your libav installation and codec support. "
                  "More info: <b>http://libav.org</b>";

    k->movieFile = QDir::tempPath() + "/tupitube_video_" + TAlgorithm::randomString(12);
    k->chooseFileExtension(format);
    k->fps = fps;
    k->streamDuration = duration;
    k->exception = beginVideo();
}

TLibavMovieGenerator::TLibavMovieGenerator(TMovieGeneratorInterface::Format format, const QSize &size, 
                      int fps, double duration) : TMovieGenerator(size.width(), size.height()), k(new Private)
{
    errorDetail = "This is not a problem directly related to <b>TupiTube Desk</b>. "
                  "Please, check your libav installation and codec support. "
                  "More info: <b>http://libav.org</b>";

    k->movieFile = QDir::tempPath() + "/tupitube_video_" + TAlgorithm::randomString(12);
    k->chooseFileExtension(format);
    k->fps = fps;
    k->streamDuration = duration;
    k->exception = beginVideo();
}

TLibavMovieGenerator::~TLibavMovieGenerator()
{
    if (QFile::exists(k->movieFile)) 
        QFile::remove(k->movieFile);

    delete k;
}

bool TLibavMovieGenerator::beginVideo()
{
    int ret;
    AVCodec *video_codec = 0;

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
        k->video_st = addVideoStream(k->oc, &video_codec, k->fmt->video_codec, k->movieFile, width(), height(), k->fps, errorDetail);

    av_dump_format(k->oc, 0, k->movieFile.toLocal8Bit().data(), 1); 
	
    if (k->video_st) {
        k->openVideo(video_codec, k->video_st, errorDetail);
    } else {
        k->errorMsg = "<b>libav error:</b> Video codec required is not installed. " + errorDetail;
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

bool TLibavMovieGenerator::validMovieHeader() 
{ 
    return k->exception;
}

QString TLibavMovieGenerator::getErrorMsg() const
{
    return k->errorMsg;
}

void TLibavMovieGenerator::handle(const QImage& image)
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

    // k->frame->pts += av_rescale_q(1, k->video_st->codec->time_base, k->video_st->time_base);
    // k->frame->pts += av_rescale_q(1, k->video_st->time_base, k->video_st->time_base);
}

void TLibavMovieGenerator::endVideo()
{
    av_write_trailer(k->oc);

    if (k->video_st)
        k->closeVideo(k->video_st);

    if (!(k->fmt->flags & AVFMT_NOFILE))
        avio_close(k->oc->pb);

    avformat_free_context(k->oc);
}

void TLibavMovieGenerator::createMovieFile(const QString &fileName)
{
    if (QFile::exists(fileName)) 
        QFile::remove(fileName);

    QFile::copy(k->movieFile, fileName);
}

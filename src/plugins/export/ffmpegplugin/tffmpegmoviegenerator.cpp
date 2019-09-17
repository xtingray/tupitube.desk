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
// #define __STDC_CONSTANT_MACROS
#include <stdint.h>

#include "tffmpegmoviegenerator.h"
#include "talgorithm.h"

#include <QDir>

// Handy documentation about Libav library
// https://github.com/leandromoreira/ffmpeg-libav-tutorial
// http://libav-users.943685.n4.nabble.com/Save-AVFrame-to-jpg-file-td2314979.html

TFFmpegMovieGenerator::TFFmpegMovieGenerator(TMovieGeneratorInterface::Format format, const QSize &size, 
                      int fpsParam, double duration) : TMovieGenerator(size.width(), size.height())
{
    movieFile = QDir::tempPath() + "/tupitube_video_" + TAlgorithm::randomString(12);
    chooseFileExtension(format);
    fps = fpsParam;
    streamDuration = duration;
    next_pts = 0;
    hasSounds = false;

    exception = beginVideo();
}

TFFmpegMovieGenerator::~TFFmpegMovieGenerator()
{
}

bool TFFmpegMovieGenerator::beginVideo()
{
    int ret;
    AVCodec *video_codec = nullptr;

    // av_register_all();

    fmt = av_guess_format("ffh264", movieFile.toLocal8Bit().data(), nullptr);
    if (!fmt) {
        #ifdef TUP_DEBUG
            qWarning() << "TFFmpegMovieGenerator::beginVideo() - Can't guess format. Selecting MPEG by default...";
        #endif

        fmt = av_guess_format("mpeg", nullptr, nullptr);
    }

    if (!fmt) {
        errorMsg = "ffmpeg error: Output format variable is NULL.";
        #ifdef TUP_DEBUG
            qCritical() << "TFFmpegMovieGenerator::beginVideo() - " + errorMsg;
        #endif

        return false;
    }

    oc = avformat_alloc_context();
    if (!oc) {
        errorMsg = "ffmpeg error: Memory error while allocating format context.";
        #ifdef TUP_DEBUG
            qCritical() << "TFFmpegMovieGenerator::beginVideo() - " + errorMsg;
        #endif

        return false;
    }
    oc->oformat = fmt;

    if (!oc) {
        errorMsg = "ffmpeg error: Format context variable is NULL.";
        #ifdef TUP_DEBUG
            qCritical() << "TFFmpegMovieGenerator::beginVideo() - " + errorMsg;
        #endif

        return false;
    }

    video_st = nullptr;
    if (fmt->video_codec != AV_CODEC_ID_NONE)
        video_st = addVideoStream(oc, &video_codec, fmt->video_codec, movieFile, width(), height(), fps);
	
    av_dump_format(oc, 0, movieFile.toLocal8Bit().data(), 1);
	
    if (video_st) {
        bool success = openVideo(video_codec, video_st);
        if (!success) {
            errorMsg = "ffmpeg error: Could not initialize video codec.";
            #ifdef TUP_DEBUG
                qCritical() << "TFFmpegMovieGenerator::beginVideo() - " + errorMsg;
            #endif

            return false;
        }
    } else {
        errorMsg = "ffmpeg error: Video stream variable is NULL.";
        #ifdef TUP_DEBUG
            qCritical() << "TFFmpegMovieGenerator::beginVideo() - " + errorMsg;
        #endif

        return false;
    }

    if (!(fmt->flags & AVFMT_NOFILE)) {
        ret = avio_open(&oc->pb, movieFile.toLocal8Bit().data(), AVIO_FLAG_WRITE);
        if (ret < 0) {
            errorMsg = "ffmpeg error: could not open video file";
            #ifdef TUP_DEBUG
                qCritical() << "TFFmpegMovieGenerator::beginVideo() - " + errorMsg;
            #endif
            return false;
        }
    }

    avformat_write_header(oc, nullptr);

    if (videoFrame)
        videoFrame->pts = 0;

    frameCount = 0;

    return true;
}

AVStream * TFFmpegMovieGenerator::addVideoStream(AVFormatContext *oc, AVCodec **codec, enum AVCodecID codec_id,
                                 const QString &movieFile, int width, int height, int fps)
{
    #ifdef TUP_DEBUG
        qWarning() << "TFFmpegMovieGenerator::addVideoStream() - codec_id: " + QString::number(codec_id);
    #endif

    AVCodecContext *c;
    // AVCodecParameters *c;
    AVStream *st;
    QString errorMsg = "";

    // Find the video encoder
    *codec = avcodec_find_encoder(codec_id);
    if (!(*codec)) {
        errorMsg = "ffmpeg error: Could not find video encoder.";
        #ifdef TUP_DEBUG
            QString msg1 = "TFFmpegMovieGenerator::addVideoStream() - " + errorMsg;
            QString msg2 = "TFFmpegMovieGenerator::addVideoStream() - Unavailable Codec ID: " + QString::number(codec_id);
            qCritical() << msg1;
            qCritical() << msg2;
        #endif

        return nullptr;
    }

    st = avformat_new_stream(oc, *codec);
    if (!st) {
        errorMsg = "ffmpeg error: Could not video alloc stream."; 
        #ifdef TUP_DEBUG
            qCritical() << "TFFmpegMovieGenerator::addVideoStream() - " + errorMsg;
        #endif

        return nullptr;
    }
    st->id = static_cast<int>(oc->nb_streams - 1);

    c = st->codec;
    c->codec_id = codec_id;

    // Put sample parameters
    c->bit_rate = 6000000;
    if (fps == 1)
        c->bit_rate = 4000000;

    // Resolution must be a multiple of two
    c->width = width;  
    c->height = height; 

    // c->gop_size = 12;
    c->gop_size = 0;
    c->max_b_frames = 0;

    c->time_base.num = 1;
    c->time_base.den = fps;
    // c->time_base = (AVRational){1,fps};

    if (movieFile.endsWith("gif", Qt::CaseInsensitive)) {
        st->time_base.num = 1;
        st->time_base.den = fps;
        c->pix_fmt = AV_PIX_FMT_RGB24;
    } else {
        c->pix_fmt = AV_PIX_FMT_YUV420P;
        /*
        if (codec_id == AV_CODEC_ID_H264)
            av_opt_set(c->priv_data, "tune", "zerolatency", 0); 
            // av_opt_set(c->priv_data, "vprofile", "baseline", 0);
            // av_opt_set(c->priv_data, "preset", "slow", 0);
        */
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
        // c->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
        // SQA: Temporary code to support Libav
        c->flags |= CODEC_FLAG_GLOBAL_HEADER;

    return st;
}

bool TFFmpegMovieGenerator::openVideo(AVCodec *codec, AVStream *st)
{
    int ret;
    AVCodecContext *c = st->codec;

    // AVDictionary *opts;
    // if (c->codec_id == AV_CODEC_ID_H264)
    //     av_opt_set(&opts, "tune", "zerolatency", 0);

    // Open the codec
    ret = avcodec_open2(c, codec, nullptr);
    // ret = avcodec_open2(c, codec, &opts);

    if (ret < 0) {
        errorMsg = "ffmpeg error: Sorry, the video codec required is not installed in your system.";
        #ifdef TUP_DEBUG
            qCritical() << "TFFmpegMovieGenerator::openVideo() - " + errorMsg;
        #endif

        return false;
    }

    // Allocate and init a re-usable frame
    videoFrame = av_frame_alloc();

    if (!videoFrame) {
        errorMsg = "ffmpeg error: There is no available memory to export your project as a video";
        #ifdef TUP_DEBUG
            qCritical() << "TFFmpegMovieGenerator::openVideo() - " + errorMsg;
        #endif

        return false;
    }
	
    return true;
}

#define RGBtoYUV(r, g, b, y, u, v) \
  y = static_cast<uint8_t> ((static_cast<int>(30*r) + static_cast<int>(59*g) + static_cast<int>(11*b))/100); \
  u = static_cast<uint8_t> ((static_cast<int>(-17*r) - static_cast<int>(33*g) + static_cast<int>(50*b) + 12800)/100); \
  v = static_cast<uint8_t> ((static_cast<int>(50*r) - static_cast<int>(42*g) - static_cast<int>(8*b) + 12800)/100);

void TFFmpegMovieGenerator::RGBtoYUV420P(const uint8_t *bufferRGB, uint8_t *bufferYUV, uint iRGBIncrement,
                                        bool bSwapRGB, int width, int height)
{
    const unsigned iPlaneSize = static_cast<const unsigned int> (width * height);
    const unsigned iHalfWidth = static_cast<const unsigned int> (width >> 1);

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
         uint8_t *yline = yplane + (y * width);
         uint8_t *uline = uplane + (static_cast<unsigned int>(y >> 1) * iHalfWidth);
         uint8_t *vline = vplane + (static_cast<unsigned int>(y >> 1) * iHalfWidth);

         for (int x = 0; x < width; x += 2) {
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

bool TFFmpegMovieGenerator::writeVideoFrame(const QString &movieFile, const QImage &image)
{
    #ifdef TUP_DEBUG
        qInfo() << "TFFmpegMovieGenerator::writeVideoFrame() - Generating frame #" + QString::number(frameCount);
    #endif

    AVCodecContext *c = video_st->codec;
    int w = c->width;
    int h = c->height;

    int got_output;
    AVPacket pkt;
    av_init_packet(&pkt);
    pkt.data = nullptr; // packet data will be allocated by the encoder
    pkt.size = 0;

    if (movieFile.endsWith("gif", Qt::CaseInsensitive)) {
        QImage img = image.convertToFormat(Format_RGB888);
        avpicture_fill((AVPicture *)videoFrame, img.bits(), AV_PIX_FMT_RGB24, w, h);
    } else { 
        int size = avpicture_get_size(AV_PIX_FMT_YUV420P, w, h);
        uint8_t *pic_dat = static_cast<uint8_t *> (av_malloc(static_cast<size_t>(size)));
        RGBtoYUV420P(image.bits(), pic_dat, static_cast<uint>(image.depth()/8), true, w, h);
        avpicture_fill((AVPicture *)videoFrame, pic_dat, AV_PIX_FMT_YUV420P, w, h);

        videoFrame->format = AV_PIX_FMT_YUV420P;
        videoFrame->width = w;
        videoFrame->height = h;
        videoFrame->pts += av_rescale_q(1, video_st->codec->time_base, video_st->time_base);
        // videoFrame->pts += av_rescale_q(1, video_st->time_base, video_st->time_base);
    }

    int ret = avcodec_encode_video2(c, &pkt, videoFrame, &got_output);
    if (ret < 0) {
        errorMsg = "[1] Error while encoding the video of your project";
        #ifdef TUP_DEBUG
            qCritical() << "TFFmpegMovieGenerator::writeVideoFrame() - " + errorMsg;
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
            qWarning() << "TFFmpegMovieGenerator::writeVideoFrame() - Frame ignored! -> " + QString::number(frameCount);
        #endif
        ret = 0;
    }

    if (ret != 0) {
        errorMsg = "[2] Error while encoding the video of your project";
        #ifdef TUP_DEBUG
            qCritical() << "TFFmpegMovieGenerator::writeVideoFrame() - " + errorMsg;
        #endif

        return false;
    }
    frameCount++;

    return true;
}

void TFFmpegMovieGenerator::handle(const QImage &image)
{
    if (!video_st) {
        #ifdef TUP_DEBUG
            QString msg = "TFFmpegMovieGenerator::handle() - The total of frames has been "
                          "processed (" + QString::number(streamDuration) + " seg)";
            qInfo() << msg;
        #endif

        return;
    }

    #ifdef TUP_DEBUG
        QString msg1 = "TFFmpegMovieGenerator::handle() - Duration: " + QString::number(streamDuration);
        qInfo() << msg1;
    #endif

    writeVideoFrame(movieFile, image);
}

void TFFmpegMovieGenerator::chooseFileExtension(int format)
{
    switch (format) {
        case MP4:
            movieFile += ".mp4";
        break;

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
            movieFile += ".mp4";
        break;
    }
}

void TFFmpegMovieGenerator::closeVideo(AVStream *st)
{
    avcodec_close(st->codec);
    // av_free(videoFrame);
    av_frame_free(&videoFrame);
}

void TFFmpegMovieGenerator::saveMovie(const QString &filename) 
{
    endVideo();
    createMovieFile(filename);
}

bool TFFmpegMovieGenerator::validMovieHeader() 
{ 
    return exception;
}

QString TFFmpegMovieGenerator::getErrorMsg() const
{
    QString errorDetail = "This is not a problem directly related to <b>TupiTube Desk</b>. "
                          "Please, check your ffmpeg installation and codec support. "
                          "More info: <b>http://ffmpeg.org</b>";
    return errorDetail;
}

void TFFmpegMovieGenerator::endVideo()
{
    av_write_trailer(oc);

    if (video_st)
        closeVideo(video_st);

    if (!(fmt->flags & AVFMT_NOFILE))
        avio_close(oc->pb);

    avformat_free_context(oc);
}

void TFFmpegMovieGenerator::createMovieFile(const QString &fileName)
{
    if (QFile::exists(fileName)) 
        QFile::remove(fileName);

    if (QFile::copy(movieFile, fileName)) {
        if (QFile::exists(movieFile)) {
            if (QFile::remove(movieFile)) {
                #ifdef TUP_DEBUG
                    QString msg = QString("") + "TFFmpegMovieGenerator::createMovieFile() - Removing temp video file -> " + movieFile;
                    qInfo() << msg;
                #endif
            }
        }
    }
}

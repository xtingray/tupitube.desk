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
#include <stdint.h>

#include "tffmpegmoviegenerator.h"
#include "talgorithm.h"

#include <QDir>
#include <QTimer>

// Handy documentation about Libav library
// https://github.com/leandromoreira/ffmpeg-libav-tutorial
// http://libav-users.943685.n4.nabble.com/Save-AVFrame-to-jpg-file-td2314979.html

TFFmpegMovieGenerator::TFFmpegMovieGenerator(TMovieGeneratorInterface::Format format, const QSize &size, 
                                             int fpsParam, double duration) : TMovieGenerator(size.width(), size.height())
{
    #ifdef TUP_DEBUG
        qDebug() << "[TFFmpegMovieGenerator::TFFmpegMovieGenerator()] - fps -> " << fpsParam;
    #endif

    movieFile = QDir::tempPath() + "/tupitube_video_" + TAlgorithm::randomString(12);
    setFileExtension(format);

    videoW = size.width();
    videoH = size.height();
    fps = fpsParam;
    streamDuration = duration;
    next_pts = 0;
    hasSounds = false;

    exception = beginVideoFile();
}

TFFmpegMovieGenerator::~TFFmpegMovieGenerator()
{
}

void TFFmpegMovieGenerator::setFileExtension(int format)
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

bool TFFmpegMovieGenerator::beginVideoFile()
{
    int ret;
    codec = nullptr;

    // AVOutputFormat
    outputFormat = av_guess_format("ffh264", movieFile.toLocal8Bit().data(), nullptr);
    if (!outputFormat) {
        errorMsg = "ffmpeg error: Can't support MP4/MOV format.";
        #ifdef TUP_DEBUG
            qWarning() << "[TFFmpegMovieGenerator::beginVideoFile()] - " << errorMsg;
        #endif
        return false;
    }

    if (!outputFormat) {
        errorMsg = "ffmpeg error: Output format variable is NULL.";
        #ifdef TUP_DEBUG
            qCritical() << "[TFFmpegMovieGenerator::beginVideoFile()] - " << errorMsg;
        #endif
        return false;
    }

    // AVFormatContext
    formatContext = avformat_alloc_context();
    if (!formatContext) {
        errorMsg = "ffmpeg error: Memory error while allocating format context.";
        #ifdef TUP_DEBUG
            qCritical() << "[TFFmpegMovieGenerator::beginVideoFile()] - " << errorMsg;
        #endif
        return false;
    }

    formatContext->oformat = outputFormat;

    // AVStream
    videoCodecID = outputFormat->video_codec;
    video_st = addVideoStream();

    av_dump_format(formatContext, 0, movieFile.toLocal8Bit().data(), 1);
	
    if (video_st) {
        bool success = openVideoStream();
        if (!success) {
            errorMsg = "ffmpeg error: Could not initialize video codec.";
            #ifdef TUP_DEBUG
                qCritical() << "[TFFmpegMovieGenerator::beginVideoFile()] - " << errorMsg;
            #endif
            return false;
        }
    } else {
        errorMsg = "ffmpeg error: Video stream variable is NULL.";
        #ifdef TUP_DEBUG
            qCritical() << "[TFFmpegMovieGenerator::beginVideoFile()] - " << errorMsg;
        #endif
        return false;
    }

    if (!(outputFormat->flags & AVFMT_NOFILE)) {
        ret = avio_open(&formatContext->pb, movieFile.toLocal8Bit().data(), AVIO_FLAG_WRITE);
        if (ret < 0) {
            errorMsg = "ffmpeg error: Could not open video file";
            #ifdef TUP_DEBUG
                qCritical() << "[TFFmpegMovieGenerator::beginVideoFile()] - " << errorMsg;
            #endif
            return false;
        }
    }

    if (avformat_write_header(formatContext, nullptr) < 0) {
        errorMsg = "ffmpeg error: could not write video file header";
        #ifdef TUP_DEBUG
            qCritical() << "[TFFmpegMovieGenerator::beginVideoFile()] - " << errorMsg;
        #endif
        return false;
    }

    if (videoFrame)
        videoFrame->pts = 0;

    framesCount = 0;
    realFrames = 0;

    return true;
}

AVStream * TFFmpegMovieGenerator::addVideoStream()
{
    #ifdef TUP_DEBUG
        qDebug() << "[TFFmpegMovieGenerator::addVideoStream()] - codec_id: " << videoCodecID;
    #endif

    AVStream *st;

    // Find the video encoder
    codec = avcodec_find_encoder(videoCodecID);
    if (!codec) {
        errorMsg = "ffmpeg error: Could not find video encoder.";
        #ifdef TUP_DEBUG
            qCritical() << "[TFFmpegMovieGenerator::addVideoStream()] - " << errorMsg;
            qCritical() << "[TFFmpegMovieGenerator::addVideoStream()] - Unavailable Codec ID: " << videoCodecID;
        #endif
        return nullptr;
    }

    st = avformat_new_stream(formatContext, codec);
    if (!st) {
        errorMsg = "ffmpeg error: Could not video alloc stream."; 
        #ifdef TUP_DEBUG
            qCritical() << "[TFFmpegMovieGenerator::addVideoStream()] - " << errorMsg;
        #endif
        return nullptr;
    }

    /* SQA: Code pending for review
    codecContext = avcodec_alloc_context3(codec);
    if (!codecContext) {
        qDebug() << "Could not allocate video codec context";
        return nullptr;
    }
    if (avcodec_parameters_to_context(codecContext, st->codecpar) < 0) {
        qDebug() << "Could not copy parameters to context";
        return nullptr;
    }
    */

    codecContext = st->codec;

    // Put sample parameters
    codecContext->bit_rate = 6000000;
    if (fps == 1)
        codecContext->bit_rate = 4000000;

    // Resolution must be a multiple of two
    codecContext->width = videoW;
    codecContext->height = videoH;

    codecContext->gop_size = 0;
    codecContext->max_b_frames = 0;

    codecContext->time_base = (AVRational){1, fps};

    if (movieFile.endsWith("gif", Qt::CaseInsensitive)) {
        st->time_base.num = 1;
        st->time_base.den = fps;
        codecContext->pix_fmt = AV_PIX_FMT_RGB24;
    } else {
        codecContext->pix_fmt = AV_PIX_FMT_YUV420P;
    }

    if (formatContext->oformat->flags & AVFMT_GLOBALHEADER)
        codecContext->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;

    return st;
}

bool TFFmpegMovieGenerator::openVideoStream()
{
    // Open the codec
    int ret = avcodec_open2(codecContext, codec, nullptr);

    if (ret < 0) {
        errorMsg = "ffmpeg error: Can't open video codec.";
        #ifdef TUP_DEBUG
            qCritical() << "[TFFmpegMovieGenerator::openVideoStream()] - " << errorMsg;
        #endif
        return false;
    }

    // Allocate and init a re-usable frame
    // AVFrame
    videoFrame = av_frame_alloc();

    if (!videoFrame) {
        errorMsg = "ffmpeg error: There is no available memory to export your project as a video";
        #ifdef TUP_DEBUG
            qCritical() << "[TFFmpegMovieGenerator::openVideoStream()] - " << errorMsg;
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
                                        bool bSwapRGB)
{
    const unsigned iPlaneSize = static_cast<const unsigned int> (videoW * videoH);
    const unsigned iHalfWidth = static_cast<const unsigned int> (videoW >> 1);

    // get pointers to the data
    uint8_t *yplane = bufferYUV;
    uint8_t *uplane = bufferYUV + iPlaneSize;
    uint8_t *vplane = bufferYUV + iPlaneSize + (iPlaneSize >> 2);
    const uint8_t *bufferRGBIndex = bufferRGB;

    int iRGBIdx[3];
    iRGBIdx[0] = 0;
    iRGBIdx[1] = 1;
    iRGBIdx[2] = 2;
    if (bSwapRGB) {
        iRGBIdx[0] = 2;
        iRGBIdx[2] = 0;
    }
 
    for (int y = 0; y < videoH; y++) {
         uint8_t *yline = yplane + (y * videoW);
         uint8_t *uline = uplane + (static_cast<unsigned int>(y >> 1) * iHalfWidth);
         uint8_t *vline = vplane + (static_cast<unsigned int>(y >> 1) * iHalfWidth);

         for (int x = 0; x < videoW; x += 2) {
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

bool TFFmpegMovieGenerator::createVideoFrame(const QImage &image)
{
    #ifdef TUP_DEBUG
        qInfo() << "---";
        qInfo() << "[TFFmpegMovieGenerator::createVideoFrame()] - Generating frame #" << framesCount;
    #endif

    framesCount++;
    fflush(stdout);

    AVPacket pkt;
    av_init_packet(&pkt);
    pkt.data = nullptr; // packet data will be allocated by the encoder
    pkt.size = 0;

    if (movieFile.endsWith("gif", Qt::CaseInsensitive)) {
        QImage img = image.convertToFormat(Format_RGB888);
        av_image_fill_arrays(videoFrame->data, videoFrame->linesize, img.bits(), AV_PIX_FMT_YUV420P, videoW, videoH, 1);
    } else { 
        int size = av_image_get_buffer_size(AV_PIX_FMT_YUV420P, videoW, videoH, 1);
        uint8_t *pic_dat = static_cast<uint8_t *> (av_malloc(static_cast<size_t>(size)));
        RGBtoYUV420P(image.bits(), pic_dat, static_cast<uint>(image.depth()/8), true);
        av_image_fill_arrays(videoFrame->data, videoFrame->linesize, pic_dat, AV_PIX_FMT_YUV420P, videoW, videoH, 1);

        videoFrame->format = AV_PIX_FMT_YUV420P;
        videoFrame->width = videoW;
        videoFrame->height = videoH;
        videoFrame->pts += av_rescale_q(1, codecContext->time_base, video_st->time_base);
    }

    int ret = avcodec_send_frame(codecContext, videoFrame);
    if (ret < 0) {
        errorMsg = "ffmpeg error: Error while sending a frame for encoding";
        #ifdef TUP_DEBUG
            qDebug() << "[TFFmpegMovieGenerator::createVideoFrame()] - " << errorMsg;
        #endif
        return false;
    }

    while (ret >= 0) {
        ret = avcodec_receive_packet(codecContext, &pkt);
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
            #ifdef TUP_DEBUG
                qDebug() << "[TFFmpegMovieGenerator::createVideoFrame()] - Tracing EAGAIN conditional...";
            #endif
            return (ret == AVERROR(EAGAIN)) ? false : true;
        } else if (ret < 0) {
            errorMsg = "ffmpeg error: Error during encoding";
            #ifdef TUP_DEBUG
                qDebug() << "[TFFmpegMovieGenerator::createVideoFrame()] - " << errorMsg;
            #endif
            return false;
        }

        ret = writeVideoFrame(&pkt);

        if (ret < 0) {
           errorMsg = "ffmpeg error: Error while writing video frame";
           #ifdef TUP_DEBUG
               qDebug() << "[TFFmpegMovieGenerator::createVideoFrame()] - " << errorMsg;
           #endif
           return false;
        }

        av_packet_unref(&pkt);
    }

    return true;
}

int TFFmpegMovieGenerator::writeVideoFrame(AVPacket *pkt)
{
    #ifdef TUP_DEBUG
        qInfo() << "[TFFmpegMovieGenerator::writeVideoFrame()] - frame -> " << framesCount;
    #endif

    realFrames++;

    /* rescale output packet timestamp values from codec to stream timebase */
    av_packet_rescale_ts(pkt, video_st->time_base, video_st->time_base);
    pkt->stream_index = video_st->index;

    /* Write the compressed frame to the media file. */
    // return av_interleaved_write_frame(formatContext, pkt);
    return av_write_frame(formatContext, pkt);
}

void TFFmpegMovieGenerator::handle(const QImage &image)
{
    if (!video_st) {
        #ifdef TUP_DEBUG
            qInfo() << "[TFFmpegMovieGenerator::handle()] - The total of frames has been "
                       "processed (" << streamDuration << " seg)";
        #endif
        return;
    }

    #ifdef TUP_DEBUG
        qInfo() << "[TFFmpegMovieGenerator::handle()] - Duration: " << streamDuration;
    #endif

    createVideoFrame(image);
}

void TFFmpegMovieGenerator::saveMovie(const QString &filename) 
{
    #ifdef TUP_DEBUG
        qDebug() << "***";
        qDebug() << "[TFFmpegMovieGenerator::saveMovie()] - filename -> " << filename;
    #endif

    int missingFrames = framesCount - realFrames;

    if (missingFrames > 0) {
        for (int i=0; i<missingFrames; i++) {
            QImage image = QImage(videoW, videoH, QImage::Format_RGB32);
            image.fill(Qt::white);
            createVideoFrame(image);
        }
    }

    endVideoFile();
    copyMovieFile(filename);
}

void TFFmpegMovieGenerator::endVideoFile()
{
    av_write_trailer(formatContext);
    if (codecContext)
        avcodec_close(codecContext);
    av_frame_free(&videoFrame);

    if (!(outputFormat->flags & AVFMT_NOFILE))
        avio_close(formatContext->pb);

    avformat_free_context(formatContext);
}

void TFFmpegMovieGenerator::copyMovieFile(const QString &videoPath)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TFFmpegMovieGenerator::copyMovieFile()] - Video File -> " << videoPath;
        qDebug() << "[TFFmpegMovieGenerator::copyMovieFile()] - Temp File -> " << movieFile;
    #endif

    if (QFile::exists(videoPath)) {
        QFileInfo info(videoPath);
        if (!info.isFile()) {
            #ifdef TUP_DEBUG
                qInfo() << "[TFFmpegMovieGenerator::copyMovieFile()] - Fatal Error: Video path is NOT a file! -> " << videoPath;
            #endif
            return;
        }

        if (!QFile::remove(videoPath)) {
            #ifdef TUP_DEBUG
                qInfo() << "[TFFmpegMovieGenerator::copyMovieFile()] - Fatal Error: Can't remove file! -> " << videoPath;
            #endif
            return;
        }
    }

    if (QFile::copy(movieFile, videoPath)) {
        if (QFile::exists(movieFile)) {
            #ifdef TUP_DEBUG
                qInfo() << "[TFFmpegMovieGenerator::copyMovieFile()] - Trying to remove temp video file -> " << movieFile;
            #endif
 
            if (QFile::remove(movieFile)) {
                #ifdef TUP_DEBUG
                    qDebug() << "[TFFmpegMovieGenerator::copyMovieFile()] - Temp video file has been removed!";
                #endif
            } else {
                #ifdef TUP_DEBUG
                    qDebug() << "[TFFmpegMovieGenerator::copyMovieFile()] - Error: Can't remove temp video file";
                #endif
            }
        } else {
            #ifdef TUP_DEBUG
                qWarning() << "[TFFmpegMovieGenerator::copyMovieFile()] - Error: Temp video file wasn't found! -> " << movieFile;
            #endif
        }
    } else {
        #ifdef TUP_DEBUG
            qWarning() << "[TFFmpegMovieGenerator::copyMovieFile()] - Error: Can't create video file -> " << videoPath;
        #endif
    }
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

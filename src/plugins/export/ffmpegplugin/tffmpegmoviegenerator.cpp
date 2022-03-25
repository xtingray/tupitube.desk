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

#include "tapplicationproperties.h"
#include "tffmpegmoviegenerator.h"
#include "talgorithm.h"

#include <QDir>
#include <QTimer>

// Handy documentation about Libav library
// https://github.com/leandromoreira/ffmpeg-libav-tutorial
// http://libav-users.943685.n4.nabble.com/Save-AVFrame-to-jpg-file-td2314979.html

TFFmpegMovieGenerator::TFFmpegMovieGenerator(TMovieGeneratorInterface::Format format, const QSize &size, 
                                             int fpsParam, double duration, QList<SoundResource> soundsList)
                                             : TMovieGenerator(size.width(), size.height())
{
    #ifdef TUP_DEBUG
        qDebug() << "[TFFmpegMovieGenerator::TFFmpegMovieGenerator()] - fps -> " << fpsParam;
        qDebug() << "[TFFmpegMovieGenerator::TFFmpegMovieGenerator()] - duration -> " << duration;
    #endif

    movieFile = QDir::tempPath() + "/tupitube_video_" + TAlgorithm::randomString(12);
    setFileExtension(format);

    videoW = size.width();
    videoH = size.height();
    fps = fpsParam;
    mp4Duration = duration;
    videoPktCounter = 0;
    audioPktCounter = 0;

    sounds = soundsList;
    hasSounds = true;
    if (sounds.isEmpty())
        hasSounds = false;

    exception = initVideoFile();

    #ifdef TUP_DEBUG
        QFile videoFile(CACHE_DIR + "/video_packets.txt");
        if (videoFile.exists())
            videoFile.remove();
        QFile audioFile(CACHE_DIR + "/audio_packets.txt");
        if (audioFile.exists())
            audioFile.remove();
    #endif
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

bool TFFmpegMovieGenerator::initVideoFile()
{
    int ret;
    videoCodec = nullptr;

    // AVOutputFormat
    outputFormat = av_guess_format("ffh264", movieFile.toLocal8Bit().data(), nullptr);
    if (!outputFormat) {
        errorMsg = "ffmpeg error: Can't support MP4/MOV format.";
        #ifdef TUP_DEBUG
            qWarning() << "[TFFmpegMovieGenerator::initVideoFile()] - " << errorMsg;
        #endif
        return false;
    }

    avformat_alloc_output_context2(&formatContext, NULL, NULL, movieFile.toLocal8Bit().data());
    if (!formatContext) {
        errorMsg = "ffmpeg error: Memory error while allocating format context.";
        #ifdef TUP_DEBUG
            qCritical() << "[TFFmpegMovieGenerator::initVideoFile()] - " << errorMsg;
        #endif
        return false;
    }

    outputFormat = formatContext->oformat;
    videoCodecID = outputFormat->video_codec;

    // AVStream
    video_st = addVideoStream();
    if (video_st) {
        bool success = openVideoStream();
        if (!success) {
            errorMsg = "ffmpeg error: Could not initialize video codec.";
            #ifdef TUP_DEBUG
                qCritical() << "[TFFmpegMovieGenerator::initVideoFile()] - " << errorMsg;
            #endif
            return false;
        }
    } else {
        errorMsg = "ffmpeg error: Video stream variable is NULL.";
        #ifdef TUP_DEBUG
            qCritical() << "[TFFmpegMovieGenerator::initVideoFile()] - " << errorMsg;
        #endif
        return false;
    }

    if (hasSounds) {
        foreach(SoundResource item, sounds) {
            if (!addAudioStreams(item.path)) {
                errorMsg = "ffmpeg error: Could not add audio streams.";
                #ifdef TUP_DEBUG
                    qCritical() << "[TFFmpegMovieGenerator::initVideoFile()] - " << errorMsg;
                #endif
                return false;
            }
        }

        if (!openAudioStreams()) {
            errorMsg = "ffmpeg error: Could not open audio streams.";
            #ifdef TUP_DEBUG
                qCritical() << "[TFFmpegMovieGenerator::initVideoFile()] - " << errorMsg;
            #endif
            return false;
        }
    }

    av_dump_format(formatContext, 0, movieFile.toLocal8Bit().data(), 1);

    if (!(outputFormat->flags & AVFMT_NOFILE)) {
        ret = avio_open(&formatContext->pb, movieFile.toLocal8Bit().data(), AVIO_FLAG_WRITE);
        if (ret < 0) {
            errorMsg = "ffmpeg error: Could not open video file";
            #ifdef TUP_DEBUG
                qCritical() << "[TFFmpegMovieGenerator::initVideoFile()] - " << errorMsg;
            #endif
            return false;
        }
    }

    QString desc = QObject::tr("Animation created using TupiTube.com");
    av_dict_set(&formatContext->metadata, "movflags", "use_metadata_tags", 0);
    av_dict_set(&formatContext->metadata, "description", desc.toLatin1(), 0);

    if (avformat_write_header(formatContext, &formatContext->metadata) < 0) {
        errorMsg = "ffmpeg error: could not write video file header";
        #ifdef TUP_DEBUG
            qCritical() << "[TFFmpegMovieGenerator::initVideoFile()] - " << errorMsg;
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
        qDebug() << "[TFFmpegMovieGenerator::addVideoStream()] - codec_id -> " << avcodec_get_name(videoCodecID);
    #endif

    // Find the video encoder
    videoCodec = avcodec_find_encoder(videoCodecID);
    if (!videoCodec) {
        errorMsg = "ffmpeg error: Could not find video encoder.";
        #ifdef TUP_DEBUG
            qCritical() << "[TFFmpegMovieGenerator::addVideoStream()] - " << errorMsg;
            qCritical() << "[TFFmpegMovieGenerator::addVideoStream()] - Unavailable Codec ID -> " << avcodec_get_name(videoCodecID);
        #endif
        return nullptr;
    }

    AVStream *stream = avformat_new_stream(formatContext, videoCodec);
    if (!stream) {
        errorMsg = "ffmpeg error: Could not video alloc stream.";
        #ifdef TUP_DEBUG
            qCritical() << "[TFFmpegMovieGenerator::addVideoStream()] - " << errorMsg;
        #endif
        return nullptr;
    }

    stream->id = formatContext->nb_streams-1;
    videoCodecContext = avcodec_alloc_context3(videoCodec);
    if (!videoCodecContext) {
        fprintf(stderr, "Could not alloc an encoding context\n");
        exit(1);
    }

    videoCodecContext->codec_id = videoCodecID;
    // Put sample parameters
    videoCodecContext->bit_rate = 6000000;
    if (fps == 1)
        videoCodecContext->bit_rate = 4000000;

    // Resolution must be a multiple of two.
    videoCodecContext->width    = videoW;
    videoCodecContext->height   = videoH;
    /*
       timebase: This is the fundamental unit of time (in seconds) in terms
       of which frame timestamps are represented. For fixed-fps content,
       timebase should be 1/framerate and timestamp increments should be
       identical to 1.
     */
    stream->time_base = (AVRational){ 1, fps };
    videoCodecContext->time_base       = stream->time_base;

    videoCodecContext->gop_size      = 12; // emit one intra frame every twelve frames at most
    videoCodecContext->pix_fmt       = AV_PIX_FMT_YUV420P;
    if (videoCodecContext->codec_id == AV_CODEC_ID_MPEG2VIDEO) {
        // just for testing, we also add B-frames
        videoCodecContext->max_b_frames = 2;
    }
    if (videoCodecContext->codec_id == AV_CODEC_ID_MPEG1VIDEO) {
        /*
         Needed to avoid using macroblocks in which some coeffs overflow.
         This does not happen with normal video, it just happens here as
         the motion of the chroma plane does not match the luma plane.
        */
        videoCodecContext->mb_decision = 2;
    }

    if (formatContext->oformat->flags & AVFMT_GLOBALHEADER)
        videoCodecContext->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;

    return stream;
}

bool TFFmpegMovieGenerator::addAudioStreams(const QString &soundPath)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TFFmpegMovieGenerator::addAudioStream()] - soundPath -> " << soundPath;
    #endif

    QByteArray bytes = soundPath.toLocal8Bit();
    const char *inputFile = bytes.data();

    AVFormatContext *audioInputFormatContext = avformat_alloc_context();
    if (avformat_open_input(&audioInputFormatContext, inputFile, 0, 0) < 0) {
        qDebug() << "[TFFmpegMovieGenerator::addAudioStream()] - Warning: Could not open input file -> " << inputFile;
        return false;
    }

    if (avformat_find_stream_info(audioInputFormatContext, 0) < 0) {
        qDebug() << "[TFFmpegMovieGenerator::addAudioStream()] - Failed to retrieve input stream information";
        return false;
    }

    av_dump_format(audioInputFormatContext, 0, inputFile, 0);
    audioInputList << audioInputFormatContext;

    int streamsTotal = audioInputFormatContext->nb_streams;

    // Process all the streams from input audio
    int counter = 0;
    for (int i=0; i<streamsTotal; i++) {
        AVStream *in_stream = audioInputFormatContext->streams[i];
        AVCodecParameters *in_codecpar = in_stream->codecpar;
        if (in_codecpar->codec_type != AVMEDIA_TYPE_AUDIO)
            continue;

        enum AVCodecID audioCodecID = in_codecpar->codec_id;
        #ifdef TUP_DEBUG
            qDebug() << "[TFFmpegMovieGenerator::addAudioStream()] - Audio codec detected -> " << avcodec_get_name(audioCodecID);
        #endif

        AVCodec *codec = avcodec_find_encoder(audioCodecID);
        if (!codec) {
            errorMsg = "ffmpeg error: Could not find audio encoder.";
            #ifdef TUP_DEBUG
                qCritical() << "[TFFmpegMovieGenerator::addAudioStream()] - " << errorMsg;
                qCritical() << "[TFFmpegMovieGenerator::addAudioStream()] - Unavailable Codec ID: " << audioCodecID;
            #endif
            return false;
        }

        audioStreamIndexesList << in_stream->index;
        AVStream *stream = avformat_new_stream(formatContext, codec);
        if (!stream) {
            errorMsg = "ffmpeg error: Could not audio alloc stream.";
            #ifdef TUP_DEBUG
                qCritical() << "[TFFmpegMovieGenerator::addAudioStream()] - " << errorMsg;
            #endif
            return false;
        }

        int ret = avcodec_parameters_copy(stream->codecpar, in_codecpar);
        if (ret < 0) {
            fprintf(stderr, "Failed to copy codec parameters\n");
            return false;
        }
        stream->codecpar->codec_tag = 0;

        stream->id = formatContext->nb_streams - 1;
        AVCodecContext *codecContext = avcodec_alloc_context3(codec);
        if (!codecContext) {
            qDebug() << "[TFFmpegMovieGenerator::addAudioStream()] - Could not allocate audio codec context";
            return false;
        }

        codecContext->sample_fmt = codec->sample_fmts ? codec->sample_fmts[0] : AV_SAMPLE_FMT_FLTP;
        codecContext->bit_rate = audioInputFormatContext->bit_rate;

        if (codec->supported_samplerates) {
            codecContext->sample_rate = codec->supported_samplerates[0];
            for (int i = 0; codec->supported_samplerates[i]; i++) {
                if (codec->supported_samplerates[i] == 44100)
                    codecContext->sample_rate = 44100;
            }
        }
        codecContext->channels = av_get_channel_layout_nb_channels(codecContext->channel_layout);
        codecContext->channel_layout = AV_CH_LAYOUT_STEREO;
        if (codec->channel_layouts) {
            codecContext->channel_layout = codec->channel_layouts[0];
            for (int i = 0; codec->channel_layouts[i]; i++) {
                if (codec->channel_layouts[i] == AV_CH_LAYOUT_STEREO)
                    codecContext->channel_layout = AV_CH_LAYOUT_STEREO;
            }
        }
        codecContext->channels = av_get_channel_layout_nb_channels(codecContext->channel_layout);

        // Some formats want stream headers to be separate.
        if (formatContext->oformat->flags & AVFMT_GLOBALHEADER)
            codecContext->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;

        audioCodecList << codec;
        audioCodecContextList << codecContext;
        audioStreamList << stream;
        counter++;
    }

    audioStreamsTotalList << counter;

    return true;
}

bool TFFmpegMovieGenerator::openVideoStream()
{
    #ifdef TUP_DEBUG
        qDebug() << "[TFFmpegMovieGenerator::openVideoStream()]";
    #endif

    // Open the codec
    int ret = avcodec_open2(videoCodecContext, videoCodec, nullptr);
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

    // Copy the stream parameters to the muxer
    ret = avcodec_parameters_from_context(video_st->codecpar, videoCodecContext);
    if (ret < 0) {
        fprintf(stderr, "Could not copy the stream parameters\n");
        exit(1);
    }

    return true;
}

/*
AVFrame * TFFmpegMovieGenerator::allocAudioFrame(enum AVSampleFormat sample_fmt, uint64_t channel_layout, int sample_rate, int nb_samples)
{
    AVFrame *frame = av_frame_alloc();
    if (!frame) {
        qDebug() << "[TFFmpegMovieGenerator::allocAudioFrame()] - Error allocating an audio frame";
        return nullptr;
    }

    frame->format = sample_fmt;
    frame->channel_layout = channel_layout;
    frame->sample_rate = sample_rate;
    frame->nb_samples = nb_samples;

    if (nb_samples) {
        int ret = av_frame_get_buffer(frame, 0);
        if (ret < 0) {
            qDebug() << "[TFFmpegMovieGenerator::allocAudioFrame()] - Error allocating an audio buffer";
            return nullptr;
        }
    }

    return frame;
}
*/

bool TFFmpegMovieGenerator::openAudioStreams()
{
    #ifdef TUP_DEBUG
        qDebug() << "[TFFmpegMovieGenerator::openAudioStreams()]";
    #endif

    for(int i=0; i<audioCodecContextList.size(); i++) {
        // Open the codec
        int ret = avcodec_open2(audioCodecContextList.at(i), audioCodecList.at(i), nullptr);
        if (ret < 0) {
            errorMsg = "ffmpeg error: Can't open audio codec.";
            #ifdef TUP_DEBUG
                qCritical() << "[TFFmpegMovieGenerator::openAudioStreams()] - " << errorMsg;
            #endif
            return false;
        }
    }

    return true;
}

#define RGBtoYUV(r, g, b, y, u, v) \
  y = static_cast<uint8_t> ((static_cast<int>(30*r) + static_cast<int>(59*g) + static_cast<int>(11*b))/100); \
  u = static_cast<uint8_t> ((static_cast<int>(-17*r) - static_cast<int>(33*g) + static_cast<int>(50*b) + 12800)/100); \
  v = static_cast<uint8_t> ((static_cast<int>(50*r) - static_cast<int>(42*g) - static_cast<int>(8*b) + 12800)/100);

void TFFmpegMovieGenerator::RGBtoYUV420P(const uint8_t *bufferRGB, uint8_t *bufferYUV, uint iRGBIncrement, bool bSwapRGB)
{
    // const unsigned iPlaneSize = static_cast<const unsigned int> (videoW * videoH);
    // const unsigned iHalfWidth = static_cast<const unsigned int> (videoW >> 1);

    const unsigned iPlaneSize = (videoW * videoH);
    const unsigned iHalfWidth = (videoW >> 1);

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

    AVPacket *packet = av_packet_alloc();
    packet->data = nullptr; // packet data will be allocated by the encoder
    packet->size = 0;

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
        videoFrame->pts += av_rescale_q(1, videoCodecContext->time_base, video_st->time_base);
    }

    int ret = avcodec_send_frame(videoCodecContext, videoFrame);
    if (ret < 0) {
        errorMsg = "ffmpeg error: Error while sending a frame for encoding";
        #ifdef TUP_DEBUG
            qDebug() << "[TFFmpegMovieGenerator::createVideoFrame()] - " << errorMsg;
        #endif
        return false;
    }

    while (ret >= 0) {
        ret = avcodec_receive_packet(videoCodecContext, packet);
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

        ret = writeVideoFrame(packet);

        if (ret < 0) {
           errorMsg = "ffmpeg error: Error while writing video frame";
           #ifdef TUP_DEBUG
               qDebug() << "[TFFmpegMovieGenerator::createVideoFrame()] - " << errorMsg;
           #endif
           return false;
        }

        av_packet_unref(packet);
    }

    return true;
}

int TFFmpegMovieGenerator::writeVideoFrame(AVPacket *packet)
{
    #ifdef TUP_DEBUG
        qInfo() << "[TFFmpegMovieGenerator::writeVideoFrame()] - frame -> " << framesCount;
    #endif

    realFrames++;

    // rescale output packet timestamp values from codec to stream timebase
    av_packet_rescale_ts(packet, video_st->time_base, video_st->time_base);
    packet->stream_index = video_st->index;

    #ifdef TUP_DEBUG
        logPacket(Video, video_st->time_base, packet, "in");
    #endif

    // Write the compressed frame to the media file.
    return av_interleaved_write_frame(formatContext, packet);
}

/*
AVFrame * TFFmpegMovieGenerator::getSilentFrame()
{
    #ifdef TUP_DEBUG
        qDebug() << "[TFFmpegMovieGenerator::getSilentFrame()]";
    #endif

    AVFrame *frame = audioTmpFrame;
    int16_t *q = (int16_t*)frame->data[0];

    // check if we want to generate more frames
    if (av_compare_ts(next_pts, audioCodecContext->time_base, static_cast<int64_t>(STREAM_DURATION), (AVRational){ 1, 1 }) > 0) {
        #ifdef TUP_DEBUG
            qDebug() << "[TFFmpegMovieGenerator::getSilentFrame()] - Warning: Returning audio frame NULL! - STREAM_DURATION -> "
                     << STREAM_DURATION;
        #endif
        return nullptr;
    }

    int v;
    for (int j = 0; j <frame->nb_samples; j++) {
        v = 0xff;
        for (int i = 0; i < audioCodecContext->channels; i++)
            *q++ = v;
    }

    frame->pts = next_pts;
    next_pts += frame->nb_samples;

    return frame;
}
*/

/*
int TFFmpegMovieGenerator::writeAudioFrame(AVFrame *frame)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TFFmpegMovieGenerator::writeAudioFrame()]";
    #endif

    // send the frame to the encoder
    int ret = avcodec_send_frame(audioCodecContext, frame);
    if (ret < 0) {
        #ifdef TUP_DEBUG
            qDebug() << "[TFFmpegMovieGenerator::writeAudioFrame()] - Error sending a frame to the encoder -> " << ret;
        #endif
        return -1;
    }

    while (ret >= 0) {
        AVPacket *packet = av_packet_alloc();
        ret = avcodec_receive_packet(audioCodecContext, packet);
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
            break;
        else if (ret < 0) {
            #ifdef TUP_DEBUG
                qDebug() << "[TFFmpegMovieGenerator::writeAudioFrame()] - Error encoding a frame -> " << ret;
            #endif
            return -1;
        }

        // rescale output packet timestamp values from codec to stream timebase
        av_packet_rescale_ts(packet, audioCodecContext->time_base, audio_st->time_base);
        packet->stream_index = audio_st->index;

        #ifdef TUP_DEBUG
            logPacket(Audio, formatContext, packet);
        #endif

        // Write the compressed frame to the media file.
        ret = av_interleaved_write_frame(formatContext, packet);
        av_packet_unref(packet);
        if (ret < 0) {
            #ifdef TUP_DEBUG
                qDebug() << "[TFFmpegMovieGenerator::writeAudioFrame()] - Error while writing output packet -> " << ret;
            #endif
            return -1;
        }
    }

    return ret == AVERROR_EOF ? 1 : 0;
}
*/

/*
 * encode one audio frame and send it to the muxer
 * return 1 when encoding is finished, 0 otherwise
 */
/*
int TFFmpegMovieGenerator::createAudioFrame()
{
    #ifdef TUP_DEBUG
        qDebug() << "[TFFmpegMovieGenerator::createAudioFrame()]";
    #endif

    AVFrame *frame = getSilentFrame();
    if (frame) {
        // convert samples from native format to destination codec format, using the resampler
        // compute destination number of samples
        int dst_nb_samples = av_rescale_rnd(swr_get_delay(swr_ctx, audioCodecContext->sample_rate) + frame->nb_samples,
                                            audioCodecContext->sample_rate, audioCodecContext->sample_rate, AV_ROUND_UP);

        if (dst_nb_samples != frame->nb_samples) {
            #ifdef TUP_DEBUG
                qDebug() << "[TFFmpegMovieGenerator::createAudioFrame()] - Fatal Error: dst_nb_samples != frame->nb_samples";
            #endif
            return -1;
        }

        // when we pass a frame to the encoder, it may keep a reference to it
        // internally;
        // make sure we do not overwrite it here
        //
        if (av_frame_make_writable(audioFrame) < 0) {
            #ifdef TUP_DEBUG
                qDebug() << "[TFFmpegMovieGenerator::createAudioFrame()] - Fatal Error: Can't make frame writable!";
            #endif
            return -1;
        }

        // convert to destination format
        int ret = swr_convert(swr_ctx, frame->data, dst_nb_samples,
                             (const uint8_t **)frame->data, frame->nb_samples);
        if (ret < 0) {
            #ifdef TUP_DEBUG
                qDebug() << "[TFFmpegMovieGenerator::createAudioFrame()] - Error while converting";
            #endif
            return -1;
        }
        frame = audioFrame;

        frame->pts = av_rescale_q(samples_count, (AVRational){1, audioCodecContext->sample_rate}, audioCodecContext->time_base);
        samples_count += dst_nb_samples;
    }

    return writeAudioFrame(frame);
}
*/

void TFFmpegMovieGenerator::handle(const QImage &image)
{
    if (!video_st) {
        #ifdef TUP_DEBUG
            qInfo() << "[TFFmpegMovieGenerator::handle()] - The total of frames has been "
                       "processed (" << mp4Duration << " seg)";
        #endif
        return;
    }

    #ifdef TUP_DEBUG
        qInfo() << "[TFFmpegMovieGenerator::handle()] - Duration: " << mp4Duration;
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

    if (videoCodecContext)
        avcodec_close(videoCodecContext);
    av_frame_free(&videoFrame);

    if (hasSounds) {
        for(int i=0; i< audioCodecContextList.size(); i++) {
            if (audioCodecContextList.at(i))
                avcodec_close(audioCodecContextList.at(i));
        }
    }

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

double TFFmpegMovieGenerator::av_q2d(AVRational a)
{
    return a.num / (double) a.den;
}

QString TFFmpegMovieGenerator::rationalToString(AVRational a)
{
    return "Num:" + QString::number(a.num) + "/Den:" + QString::number(a.den);
}

QString TFFmpegMovieGenerator::formatTS(int64_t ts, AVRational tb)
{
    QString result = "";
    if (ts == AV_NOPTS_VALUE)
        result = "NOPTS";
    else
        result = QString::number(av_q2d(tb) * ts);

    return result;
}

/* SQA: Method just for debugging */
void TFFmpegMovieGenerator::logPacket(MediaType type, AVRational time_base, const AVPacket *pkt, const QString &direction)
{
    int counter = 0;
    QString prefix = "audio";
    if (type == Video) {
        prefix = "video";
        videoPktCounter++;
        counter = videoPktCounter;
    } else {
        audioPktCounter++;
        counter = audioPktCounter;
    }

    QString filename = CACHE_DIR + "/" + prefix + "_packets.txt";
    QFile file(filename);
    if (file.open(QIODevice::WriteOnly | QIODevice::Append)) {       
        QString record = QString::number(counter) + " | direction: " + direction + " | pts: " + QString::number(pkt->pts)
                + " | pts_time: " + formatTS(pkt->pts, time_base) + " | dts: " + QString::number(pkt->dts)
                + " | dts_time: " + formatTS(pkt->dts, time_base) + " | duration: " + QString::number(pkt->duration)
                + " | duration_time: " + formatTS(pkt->duration, time_base) + " | stream_index: " + QString::number(pkt->stream_index)
                + " | time_base: " + rationalToString(time_base);

        QTextStream stream(&file);
        stream << record << Qt::endl;
    }
}

void TFFmpegMovieGenerator::writeAudioStreams()
{
    #ifdef TUP_DEBUG
        qDebug() << "[TFFmpegMovieGenerator::writeAudioStreams()]";
    #endif

    int init = 0;
    int outIndex = 1;
    for(int i=0; i<audioInputList.size(); i++) {
        #ifdef TUP_DEBUG
            qDebug() << "[TFFmpegMovieGenerator::writeAudioStreams()] - audio -> " << sounds.at(i).path;
        #endif

        AVFormatContext *audioInputFormatContext = audioInputList.at(i);
        AVPacket *pkt = av_packet_alloc();
        int indexesTotal = audioStreamsTotalList.at(i);

        qDebug() << "init -> " << init;
        qDebug() << "indexesTotal -> " << indexesTotal;

        QList<int> validIndexes;
        for (int j=init; j<(init + indexesTotal); j++)
            validIndexes << audioStreamIndexesList[j];
        init = indexesTotal;

        qDebug() << "audioStreamsTotalList -> " << audioStreamsTotalList;
        qDebug() << "audioStreamIndexesList -> " << audioStreamIndexesList;
        qDebug() << "validIndexes -> " << validIndexes;

        while (1) {
            AVStream *in_stream;

            int ret = av_read_frame(audioInputFormatContext, pkt);
            if (ret < 0)
                break;

            qDebug() << "pkt->stream_index -> " << pkt->stream_index;

            if (!validIndexes.contains(pkt->stream_index)) {
                #ifdef TUP_DEBUG
                    qDebug() << "[TFFmpegMovieGenerator::writeAudioStreams()] - Warning: packet has invalid stream index! -> " << pkt->stream_index;
                #endif
                av_packet_unref(pkt);
                continue;
            }

            in_stream  = audioInputFormatContext->streams[pkt->stream_index];
            pkt->stream_index = outIndex;

            logPacket(Audio, in_stream->time_base, pkt, "in");
            AVRational outputTimebase = audioStreamList.at(i)->time_base;

            // copy packet
            pkt->pts = av_rescale_q_rnd(pkt->pts, in_stream->time_base, outputTimebase,
                                        static_cast<AVRounding>(AV_ROUND_NEAR_INF|AV_ROUND_PASS_MINMAX));
            pkt->dts = av_rescale_q_rnd(pkt->dts, in_stream->time_base, outputTimebase,
                                        static_cast<AVRounding>(AV_ROUND_NEAR_INF|AV_ROUND_PASS_MINMAX));
            pkt->duration = av_rescale_q(pkt->duration, in_stream->time_base, outputTimebase);
            pkt->pos = -1;

            logPacket(Audio, audioStreamList.at(i)->time_base, pkt, "out");
            float currentTime = av_q2d(outputTimebase) * pkt->pts;
            if (currentTime < mp4Duration) {
                qDebug() << "currentTime -> " << currentTime;
                qDebug() << "mp4Duration -> " << mp4Duration;

                ret = av_interleaved_write_frame(formatContext, pkt);
                if (ret < 0) {
                    fprintf(stderr, "Error muxing packet\n");
                    break;
                }
            } else {
                // qDebug() << "Sound frame dropped! - currentTime -> " << currentTime;
                break;
            }

            av_packet_unref(pkt);
        }
        outIndex++;
    }
}

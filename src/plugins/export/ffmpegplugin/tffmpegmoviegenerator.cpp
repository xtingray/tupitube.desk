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
    soundsTotal = sounds.size();
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
    #ifdef TUP_DEBUG
        qDebug() << "[TFFmpegMovieGenerator::initVideoFile()]";
    #endif

    int error;
    videoCodec = nullptr;

    // AVOutputFormat
    outputFormat = av_guess_format("ffh264", movieFile.toLocal8Bit().data(), nullptr);
    if (!outputFormat) {
        errorMsg = "ffmpeg error: Can't support MP4/MOV format.";
        #ifdef TUP_DEBUG
            qCritical() << "[TFFmpegMovieGenerator::initVideoFile()] - " << errorMsg;
        #endif
        return false;
    }

    avformat_alloc_output_context2(&formatContext, nullptr, nullptr, movieFile.toLocal8Bit().data());
    if (!formatContext) {
        errorMsg = "ffmpeg error: Memory error while allocating format context.";
        #ifdef TUP_DEBUG
            qCritical() << "[TFFmpegMovieGenerator::initVideoFile()] - " << errorMsg;
        #endif
        return false;
    }

    outputFormat = formatContext->oformat;
    videoCodecID = outputFormat->video_codec;
    audioOutputCodecID = outputFormat->audio_codec;

    // AVStream
    videoStream = addVideoStream();
    if (videoStream) {
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
            if (!loadInputAudio(item.path)) {
                errorMsg = "ffmpeg error: Could not load audio input streams.";
                #ifdef TUP_DEBUG
                    qCritical() << "[TFFmpegMovieGenerator::initVideoFile()] - " << errorMsg;
                #endif
                return false;
            }
        }

        if (!openAudioInputStreams()) {
            errorMsg = "ffmpeg error: Could not open audio input streams.";
            #ifdef TUP_DEBUG
                qCritical() << "[TFFmpegMovieGenerator::initVideoFile()] - " << errorMsg;
            #endif
            return false;
        }

        if (!initFilterGraph()) {
            errorMsg = "ffmpeg error: Could not initialize filters.";
            #ifdef TUP_DEBUG
                qCritical() << "[TFFmpegMovieGenerator::initVideoFile()] - " << errorMsg;
            #endif
            return false;
        }

        if (!openAudioOutputStream()) {
            errorMsg = "ffmpeg error: Could not open audio output stream.";
            #ifdef TUP_DEBUG
                qCritical() << "[TFFmpegMovieGenerator::initVideoFile()] - " << errorMsg;
            #endif
            return false;
        }

        if (!openAudioOutputCodec()) {
            errorMsg = "ffmpeg error: Could not open audio output codec.";
            #ifdef TUP_DEBUG
                qCritical() << "[TFFmpegMovieGenerator::initVideoFile()] - " << errorMsg;
            #endif
            return false;
        }
    }

    av_dump_format(formatContext, 0, movieFile.toLocal8Bit().data(), 1);

    if (!(outputFormat->flags & AVFMT_NOFILE)) {
        error = avio_open(&formatContext->pb, movieFile.toLocal8Bit().data(), AVIO_FLAG_WRITE);
        if (error < 0) {
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

    error = avformat_write_header(formatContext, &formatContext->metadata);
    if (error < 0) {
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
        qDebug() << "[TFFmpegMovieGenerator::addVideoStream()] - codec_id -> "
                 << avcodec_get_name(videoCodecID);
    #endif

    // Find the video encoder
    videoCodec = avcodec_find_encoder(videoCodecID);
    if (!videoCodec) {
        errorMsg = "ffmpeg error: Could not find video encoder.";
        #ifdef TUP_DEBUG
            qCritical() << "[TFFmpegMovieGenerator::addVideoStream()] - " << errorMsg;
            qCritical() << "[TFFmpegMovieGenerator::addVideoStream()] - Unavailable Codec ID -> "
                        << avcodec_get_name(videoCodecID);
        #endif
        return nullptr;
    }

    AVStream *videoStream = avformat_new_stream(formatContext, videoCodec);
    if (!videoStream) {
        errorMsg = "ffmpeg error: Could not video alloc stream.";
        #ifdef TUP_DEBUG
            qCritical() << "[TFFmpegMovieGenerator::addVideoStream()] - " << errorMsg;
        #endif
        return nullptr;
    }

    videoStream->id = formatContext->nb_streams - 1;
    videoCodecContext = avcodec_alloc_context3(videoCodec);
    if (!videoCodecContext) {
        errorMsg = "ffmpeg error: Could not initialize the video codec context.";
        #ifdef TUP_DEBUG
            qCritical() << "[TFFmpegMovieGenerator::addVideoStream()] - " << errorMsg;
        #endif
        return nullptr;
    }

    videoCodecContext->codec_id = videoCodecID;
    // Put sample parameters
    videoCodecContext->bit_rate = 6000000;
    if (fps == 1)
        videoCodecContext->bit_rate = 4000000;

    // Resolution must be a multiple of two.
    videoCodecContext->width = videoW;
    videoCodecContext->height = videoH;

    // timebase: This is the fundamental unit of time (in seconds) in terms
    // of which frame timestamps are represented. For fixed-fps content,
    // timebase should be 1/framerate and timestamp increments should be
    // identical to 1.
    videoStream->time_base = (AVRational){ 1, fps };
    videoCodecContext->time_base = videoStream->time_base;

    videoCodecContext->gop_size = 12; // emit one intra frame every twelve frames at most
    videoCodecContext->pix_fmt = AV_PIX_FMT_YUV420P;
    if (videoCodecContext->codec_id == AV_CODEC_ID_MPEG2VIDEO) {
        // just for testing, we also add B-frames
        videoCodecContext->max_b_frames = 2;
    }
    if (videoCodecContext->codec_id == AV_CODEC_ID_MPEG1VIDEO) {
        // Needed to avoid using macroblocks in which some coeffs overflow.
        // This does not happen with normal video, it just happens here as
        // the motion of the chroma plane does not match the luma plane.
        videoCodecContext->mb_decision = 2;
    }

    if (formatContext->oformat->flags & AVFMT_GLOBALHEADER)
        videoCodecContext->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;

    return videoStream;
}

bool TFFmpegMovieGenerator::loadInputAudio(const QString &soundPath)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TFFmpegMovieGenerator::loadInputAudio()] - soundPath -> " << soundPath;
    #endif

    QByteArray bytes = soundPath.toLocal8Bit();
    const char *inputFile = bytes.data();
    int error;

    AVFormatContext *audioInputFormatContext = avformat_alloc_context();
    error = avformat_open_input(&audioInputFormatContext, inputFile, 0, 0);
    if (error < 0) {
        #ifdef TUP_DEBUG
            qCritical() << "[TFFmpegMovieGenerator::loadInputAudio()] - Warning: Could not open input file -> "
                        << inputFile;
        #endif
        return false;
    }

    error = avformat_find_stream_info(audioInputFormatContext, 0);
    if (error < 0) {
        #ifdef TUP_DEBUG
            qCritical() << "[TFFmpegMovieGenerator::loadInputAudio()] - Failed to retrieve input stream information";
        #endif
        return false;
    }

    av_dump_format(audioInputFormatContext, 0, inputFile, 0);
    audioInputFormatContextList << audioInputFormatContext;

    // Process the stream from input audio
    AVStream *audioInputStream = audioInputFormatContext->streams[0];
    if (!audioInputStream) {
        errorMsg = "ffmpeg error: audio input stream is NULL!";
        #ifdef TUP_DEBUG
            qCritical() << "[TFFmpegMovieGenerator::loadInputAudio()] - " << errorMsg;
        #endif
        return false;
    }

    AVCodecParameters *audioInputCodecPar = audioInputStream->codecpar;
    if (audioInputCodecPar->codec_type != AVMEDIA_TYPE_AUDIO) {
        errorMsg = "ffmpeg error: No audio stream was found!";
        #ifdef TUP_DEBUG
            qCritical() << "[TFFmpegMovieGenerator::loadInputAudio()] - " << errorMsg;
        #endif
        return false;
    }

    enum AVCodecID audioInputCodecID = audioInputCodecPar->codec_id;
    #ifdef TUP_DEBUG
        qDebug() << "[TFFmpegMovieGenerator::loadInputAudio()] - Audio codec detected -> "
                 << avcodec_get_name(audioInputCodecID);
    #endif

    AVCodec *audioInputCodec = avcodec_find_decoder(audioInputCodecID);
    if (!audioInputCodec) {
        errorMsg = "ffmpeg error: Could not find audio decoder.";
        #ifdef TUP_DEBUG
            qCritical() << "[TFFmpegMovieGenerator::loadInputAudio()] - " << errorMsg;
            qCritical() << "[TFFmpegMovieGenerator::loadInputAudio()] - Unavailable Codec ID -> "
                        << avcodec_get_name(audioInputCodecID);
        #endif
        return false;
    }

    AVCodecContext *audioCodecContext = avcodec_alloc_context3(audioInputCodec);
    if (!audioCodecContext) {
        errorMsg = "ffmpeg error: Could not initialize audio codec context.";
        #ifdef TUP_DEBUG
            qCritical() << "[TFFmpegMovieGenerator::loadInputAudio()] - " << errorMsg;
        #endif
        return false;
    }

    audioCodecContext->sample_fmt = audioInputCodec->sample_fmts ? audioInputCodec->sample_fmts[0] : AV_SAMPLE_FMT_FLTP;
    audioCodecContext->bit_rate = audioInputFormatContext->bit_rate;

    if (audioInputCodec->supported_samplerates) {
        audioCodecContext->sample_rate = audioInputCodec->supported_samplerates[0];
        for (int i = 0; audioInputCodec->supported_samplerates[i]; i++) {
            if (audioInputCodec->supported_samplerates[i] == 44100)
                audioCodecContext->sample_rate = 44100;
        }
    }
    audioCodecContext->channels = av_get_channel_layout_nb_channels(audioCodecContext->channel_layout);
    audioCodecContext->channel_layout = AV_CH_LAYOUT_STEREO;
    if (audioInputCodec->channel_layouts) {
        audioCodecContext->channel_layout = audioInputCodec->channel_layouts[0];
        for (int i = 0; audioInputCodec->channel_layouts[i]; i++) {
            if (audioInputCodec->channel_layouts[i] == AV_CH_LAYOUT_STEREO)
                audioCodecContext->channel_layout = AV_CH_LAYOUT_STEREO;
        }
    }
    audioCodecContext->channels = av_get_channel_layout_nb_channels(audioCodecContext->channel_layout);

    // Some formats want stream headers to be separate.
    if (formatContext->oformat->flags & AVFMT_GLOBALHEADER)
        audioCodecContext->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;

    audioCodecList << audioInputCodec;
    audioInputCodecContextList << audioCodecContext;

    return true;
}

bool TFFmpegMovieGenerator::openVideoStream()
{
    #ifdef TUP_DEBUG
        qDebug() << "[TFFmpegMovieGenerator::openVideoStream()]";
    #endif

    // Open the codec
    int error = avcodec_open2(videoCodecContext, videoCodec, nullptr);
    if (error < 0) {
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

    // Copy the codec parameters into the video stream
    error = avcodec_parameters_from_context(videoStream->codecpar, videoCodecContext);
    if (error < 0) {
        errorMsg = "ffmpeg error: Could not copy the video codec parameters.";
        #ifdef TUP_DEBUG
            qCritical() << "[TFFmpegMovieGenerator::openVideoStream()] - " << errorMsg;
        #endif
        return false;
    }

    return true;
}

bool TFFmpegMovieGenerator::openAudioInputStreams()
{
    #ifdef TUP_DEBUG
        qDebug() << "[TFFmpegMovieGenerator::openAudioInputStreams()]";
    #endif

    for(int i=0; i<audioInputCodecContextList.size(); i++) {
        // Open the input audio codec
        int error = avcodec_open2(audioInputCodecContextList.at(i), audioCodecList.at(i), nullptr);
        if (error < 0) {
            errorMsg = "ffmpeg error: Can't open audio codec.";
            #ifdef TUP_DEBUG
                qCritical() << "[TFFmpegMovieGenerator::openAudioInputStreams()] - " << errorMsg;
            #endif
            return false;
        }
    }

    return true;
}

bool TFFmpegMovieGenerator::openAudioOutputStream()
{
    #ifdef TUP_DEBUG
        qDebug() << "[TFFmpegMovieGenerator::openAudioOutputStream()]";
    #endif

    audioOutputCodec = avcodec_find_encoder(audioOutputCodecID);
    if (!audioOutputCodec) {
        errorMsg = "ffmpeg error: Could not find audio encoder.";
        #ifdef TUP_DEBUG
            qCritical() << "[TFFmpegMovieGenerator::openAudioOutputStream()] - " << errorMsg;
            qCritical() << "[TFFmpegMovieGenerator::openAudioOutputStream()] - Unavailable Codec ID -> "
                        << avcodec_get_name(audioOutputCodecID);
        #endif
        return false;
    }

    audioOutputStream = avformat_new_stream(formatContext, audioOutputCodec);
    if (!audioOutputStream) {
        errorMsg = "ffmpeg error: Could not alloc audio stream.";
        #ifdef TUP_DEBUG
            qCritical() << "[TFFmpegMovieGenerator::openAudioOutputStream()] - " << errorMsg;
        #endif
        return false;
    }

    audioOutputCodecContext = avcodec_alloc_context3(audioOutputCodec);
    if (!audioOutputCodecContext) {
        errorMsg = "ffmpeg error: Could not initialize audio codec context.";
        #ifdef TUP_DEBUG
            qCritical() << "[TFFmpegMovieGenerator::openAudioOutputStream()] - " << errorMsg;
        #endif
        return false;
    }

    audioOutputCodecContext->sample_fmt = audioOutputCodec->sample_fmts ? audioOutputCodec->sample_fmts[0] : AV_SAMPLE_FMT_FLTP;
    audioOutputCodecContext->bit_rate = 196000;

    if (audioOutputCodec->supported_samplerates) {
        audioOutputCodecContext->sample_rate = audioOutputCodec->supported_samplerates[0];
        for (int i = 0; audioOutputCodec->supported_samplerates[i]; i++) {
            if (audioOutputCodec->supported_samplerates[i] == 44100)
                audioOutputCodecContext->sample_rate = 44100;
        }
    }
    audioOutputCodecContext->channels = av_get_channel_layout_nb_channels(audioOutputCodecContext->channel_layout);
    audioOutputCodecContext->channel_layout = AV_CH_LAYOUT_STEREO;
    if (audioOutputCodec->channel_layouts) {
        audioOutputCodecContext->channel_layout = audioOutputCodec->channel_layouts[0];
        for (int i = 0; audioOutputCodec->channel_layouts[i]; i++) {
            if (audioOutputCodec->channel_layouts[i] == AV_CH_LAYOUT_STEREO)
                audioOutputCodecContext->channel_layout = AV_CH_LAYOUT_STEREO;
        }
    }
    audioOutputCodecContext->channels = av_get_channel_layout_nb_channels(audioOutputCodecContext->channel_layout);

    // Some formats want stream headers to be separate.
    if (formatContext->oformat->flags & AVFMT_GLOBALHEADER)
        audioOutputCodecContext->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;

    return true;
}

bool TFFmpegMovieGenerator::openAudioOutputCodec()
{
    #ifdef TUP_DEBUG
        qDebug() << "[TFFmpegMovieGenerator::openAudioOutputCodec()]";
    #endif

    // Open the audio codec
    int ret = avcodec_open2(audioOutputCodecContext, audioOutputCodec, nullptr);
    if (ret < 0) {
        errorMsg = "ffmpeg error: Can't open audio output codec.";
        #ifdef TUP_DEBUG
            qCritical() << "[TFFmpegMovieGenerator::openAudioOutputCodec()] - " << errorMsg;
        #endif
        return false;
    }

    // Copy the codec parameters into the audio  stream
    ret = avcodec_parameters_from_context(audioOutputStream->codecpar, audioOutputCodecContext);
    if (ret < 0) {
        errorMsg = "ffmpeg error: Could not copy the audio codec parameters.";
        #ifdef TUP_DEBUG
            qCritical() << "[TFFmpegMovieGenerator::openAudioOutputCodec()] - " << errorMsg;
        #endif
        return false;
    }

    audioOutputStream->codecpar->codec_tag = 0;
    audioOutputStream->id = formatContext->nb_streams - 1;

    return true;
}

#define RGBtoYUV(r, g, b, y, u, v) \
  y = static_cast<uint8_t> ((static_cast<int>(30*r) + static_cast<int>(59*g) + static_cast<int>(11*b))/100); \
  u = static_cast<uint8_t> ((static_cast<int>(-17*r) - static_cast<int>(33*g) + static_cast<int>(50*b) + 12800)/100); \
  v = static_cast<uint8_t> ((static_cast<int>(50*r) - static_cast<int>(42*g) - static_cast<int>(8*b) + 12800)/100);

void TFFmpegMovieGenerator::RGBtoYUV420P(const uint8_t *bufferRGB, uint8_t *bufferYUV, uint iRGBIncrement, bool bSwapRGB)
{
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
        qDebug() << "---";
        qDebug() << "[TFFmpegMovieGenerator::createVideoFrame()] - Generating frame #" << framesCount;
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
        videoFrame->pts += av_rescale_q(1, videoCodecContext->time_base, videoStream->time_base);
    }

    int ret = avcodec_send_frame(videoCodecContext, videoFrame);
    if (ret < 0) {
        errorMsg = "ffmpeg error: Error while sending a frame for encoding";
        #ifdef TUP_DEBUG
            qCritical() << "[TFFmpegMovieGenerator::createVideoFrame()] - " << errorMsg;
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
                qCritical() << "[TFFmpegMovieGenerator::createVideoFrame()] - " << errorMsg;
            #endif
            return false;
        }

        ret = writeVideoFrame(packet);

        if (ret < 0) {
           errorMsg = "ffmpeg error: Error while writing video frame";
           #ifdef TUP_DEBUG
               qCritical() << "[TFFmpegMovieGenerator::createVideoFrame()] - " << errorMsg;
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
        qDebug() << "[TFFmpegMovieGenerator::writeVideoFrame()] - frame -> " << framesCount;
    #endif

    realFrames++;

    // rescale output packet timestamp values from codec to stream timebase
    av_packet_rescale_ts(packet, videoStream->time_base, videoStream->time_base);
    packet->stream_index = videoStream->index;

    #ifdef TUP_DEBUG
        logPacket(Video, videoStream->time_base, packet, "in");
    #endif

    // Write the compressed frame to the media file.
    return av_interleaved_write_frame(formatContext, packet);
}

void TFFmpegMovieGenerator::handle(const QImage &image)
{
    if (!videoStream) {
        #ifdef TUP_DEBUG
            qDebug() << "[TFFmpegMovieGenerator::handle()] - The total of frames has been "
                       "processed (" << mp4Duration << " seg)";
        #endif
        return;
    }

    #ifdef TUP_DEBUG
        qDebug() << "[TFFmpegMovieGenerator::handle()] - Duration: " << mp4Duration;
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
        for(int i=0; i< audioInputCodecContextList.size(); i++) {
            if (audioInputCodecContextList.at(i))
                avcodec_close(audioInputCodecContextList.at(i));
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
                qCritical() << "[TFFmpegMovieGenerator::copyMovieFile()] - Fatal Error: Video path is NOT a file! -> " << videoPath;
            #endif
            return;
        }

        if (!QFile::remove(videoPath)) {
            #ifdef TUP_DEBUG
                qCritical() << "[TFFmpegMovieGenerator::copyMovieFile()] - Fatal Error: Can't remove file! -> " << videoPath;
            #endif
            return;
        }
    }

    if (QFile::copy(movieFile, videoPath)) {
        if (QFile::exists(movieFile)) {
            #ifdef TUP_DEBUG
                qDebug() << "[TFFmpegMovieGenerator::copyMovieFile()] - Trying to remove temp video file -> " << movieFile;
            #endif
 
            if (QFile::remove(movieFile)) {
                #ifdef TUP_DEBUG
                    qDebug() << "[TFFmpegMovieGenerator::copyMovieFile()] - Temp video file has been removed!";
                #endif
            } else {
                #ifdef TUP_DEBUG
                    qCritical() << "[TFFmpegMovieGenerator::copyMovieFile()] - Error: Can't remove temp video file";
                #endif
            }
        } else {
            #ifdef TUP_DEBUG
                qCritical() << "[TFFmpegMovieGenerator::copyMovieFile()] - Error: Temp video file wasn't found! -> " << movieFile;
            #endif
        }
    } else {
        #ifdef TUP_DEBUG
            qCritical() << "[TFFmpegMovieGenerator::copyMovieFile()] - Error: Can't create video file -> " << videoPath;
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

QString TFFmpegMovieGenerator::formatTS(int64_t timeStamp, AVRational timeBase)
{
    QString result = "";
    if (timeStamp == AV_NOPTS_VALUE)
        result = "NOPTS";
    else
        result = QString::number(av_q2d(timeBase) * timeStamp);

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

/*
void TFFmpegMovieGenerator::writeAudioStreams()
{
    #ifdef TUP_DEBUG
        qDebug() << "[TFFmpegMovieGenerator::writeAudioStreams()]";
    #endif

    int init = 0;
    int outIndex = 1;
    for(int i=0; i<audioInputFormatContextList.size(); i++) {
        #ifdef TUP_DEBUG
            qDebug() << "[TFFmpegMovieGenerator::writeAudioStreams()] - audio -> " << sounds.at(i).path;
        #endif

        AVFormatContext *audioInputFormatContext = audioInputFormatContextList.at(i);
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
                    #ifdef TUP_DEBUG
                        qCritical() << "[TFFmpegMovieGenerator::writeAudioStreams()] - Error while muxing audio packet!";
                    #endif
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
*/

bool TFFmpegMovieGenerator::initFilterGraph()
{
    #ifdef TUP_DEBUG
        qDebug() << "[TFFmpegMovieGenerator::initFilterGraph()]";
    #endif

    QList<const AVFilter *> abufferFilterList;
    QList<AVFilterContext *> adelayFilterContextList;
    QList<const AVFilter *> adelayFilterList;
    AVFilterContext *amixFilterContext = nullptr;
    const AVFilter *amixFilter = nullptr;
    const AVFilter *abufferSinkFilter = nullptr;

    char args[512];
    int error;

    // Create a new filtergraph, which will contain all the filters.
    filterGraph = avfilter_graph_alloc();
    if (!filterGraph) {
        errorMsg = "ffmpeg error: Unable to create filter graph.";
        #ifdef TUP_DEBUG
            qCritical() << "[TFFmpegMovieGenerator::initFilterGraph() createVideoFrame()] - " << errorMsg;
        #endif
        return false;
    }

    /*
    // amix
    // Create mix filter.
    amixFilter = avfilter_get_by_name("amix");
    if (!amixFilter) {
        errorMsg = "ffmpeg error: Could not find the mix filter.";
        #ifdef TUP_DEBUG
            qCritical() << "[TFFmpegMovieGenerator::initFilterGraph()] - " << errorMsg;
        #endif
        return false;
    }

    snprintf(args, sizeof(args), "inputs=%d", soundsTotal);
    error = avfilter_graph_create_filter(&amixFilterContext, amixFilter, "amix", args, nullptr, filterGraph);
    if (error < 0) {
        errorMsg = "ffmpeg error: Cannot create audio amix filter.";
        #ifdef TUP_DEBUG
            qCritical() << "[TFFmpegMovieGenerator::initFilterGraph()] - " << errorMsg;
        #endif
        return false;
    }

    // Create the abuffersink filter;
    // it will be used to get the filtered data out of the graph.
    abufferSinkFilter = avfilter_get_by_name("abuffersink");
    if (!abufferSinkFilter) {
        errorMsg = "ffmpeg error: Could not find the abuffersink filter.";
        #ifdef TUP_DEBUG
            qCritical() << "[TFFmpegMovieGenerator::initFilterGraph()] - " << errorMsg;
        #endif
        return false;
    }

    abufferSinkContext = avfilter_graph_alloc_filter(filterGraph, abufferSinkFilter, "sink");
    if (!abufferSinkContext) {
        errorMsg = "ffmpeg error: Could not allocate the abuffersink instance.";
        #ifdef TUP_DEBUG
            qCritical() << "[TFFmpegMovieGenerator::initFilterGraph()] - " << errorMsg;
        #endif
        return false;
    }

    // Same sample fmts as the output file.
    int fmtList[] = { AV_SAMPLE_FMT_FLTP, AV_SAMPLE_FMT_NONE };
    error = av_opt_set_int_list(abufferSinkContext, "sample_fmts", fmtList,
                                AV_SAMPLE_FMT_NONE, AV_OPT_SEARCH_CHILDREN);
    if (error < 0) {
        errorMsg = "ffmpeg error: Could set options to the abuffersink instance.";
        #ifdef TUP_DEBUG
            qCritical() << "[TFFmpegMovieGenerator::initFilterGraph()] - " << errorMsg;
        #endif
        return false;
    }

    char ch_layout[64];
    av_get_channel_layout_string(ch_layout, sizeof(ch_layout), 0, soundsTotal);
    av_opt_set(abufferSinkContext, "channel_layout", ch_layout, AV_OPT_SEARCH_CHILDREN);

    error = avfilter_init_str(abufferSinkContext, nullptr);
    if (error < 0) {
        errorMsg = "ffmpeg error: Could not initialize the abuffersink instance.";
        #ifdef TUP_DEBUG
            qCritical() << "[TFFmpegMovieGenerator::initFilterGraph()] - " << errorMsg;
        #endif
        return false;
    }
    */

    for (int i=0; i<soundsTotal; i++) {
        #ifdef TUP_DEBUG
            qDebug() << "[TFFmpegMovieGenerator::initFilterGraph()] - iteration -> " << i;
        #endif
        // abuffer
        // Create the abuffer filter;
        // it will be used for feeding the data into the graph.
        const AVFilter *abufferFilter = avfilter_get_by_name("abuffer");
        if (!abufferFilter) {
            errorMsg = "ffmpeg error: Could not find the abuffer filter.";
            #ifdef TUP_DEBUG
                qCritical() << "[TFFmpegMovieGenerator::initFilterGraph()] - " + errorMsg;
            #endif
            return false;
        }
        abufferFilterList << abufferFilter;

        // buffer audio source: the decoded frames from the decoder will be inserted here.
        if (!audioInputCodecContextList[i]->channel_layout)
            audioInputCodecContextList[i]->channel_layout = av_get_default_channel_layout(audioInputCodecContextList[i]->channels);

        snprintf(args, sizeof(args),
                 "sample_rate=%d:sample_fmt=%s:channel_layout=0x%" PRIx64,
                 audioInputCodecContextList[i]->sample_rate,
                 av_get_sample_fmt_name(audioInputCodecContextList[i]->sample_fmt),
                 audioInputCodecContextList[i]->channel_layout);

        #ifdef TUP_DEBUG
            qDebug() << "[TFFmpegMovieGenerator::initFilterGraph()] - abuffer filter args -> " << args;
        #endif

        char srcIndex[512];
        snprintf(srcIndex, sizeof(srcIndex), "src%d", i);

        AVFilterContext *abufferFilterContext;
        error = avfilter_graph_create_filter(&abufferFilterContext, abufferFilter, srcIndex,
                                             args, nullptr, filterGraph);
        if (error < 0) {
            errorMsg = "ffmpeg error: Cannot create audio buffer source.";
            #ifdef TUP_DEBUG
                qCritical() << "[TFFmpegMovieGenerator::initFilterGraph()] - " + errorMsg;
            #endif
            return false;
        }
        abufferFilterContextList << abufferFilterContext;

        // * * *
        // adelay
        // Create the delay filter;
        const AVFilter *adelayFilter = avfilter_get_by_name("adelay");
        if (!adelayFilter) {
            errorMsg = "ffmpeg error: Could not find the adelay filter.";
            #ifdef TUP_DEBUG
                qCritical() << "[TFFmpegMovieGenerator::initFilterGraph()] - " + errorMsg;
            #endif
            return false;
        }
        adelayFilterList << adelayFilter;

        float millisecs = ((float) sounds.at(i).frame/ (float) fps);
        millisecs *= 1000;
        /*
        qDebug() << "sounds.at(i).frame -> " << sounds.at(i).frame;
        qDebug() << "fps -> " << fps;
        qDebug() << "equation -> " << ((float) sounds.at(i).frame/ (float) fps);
        float result = ((float) sounds.at(i).frame/ (float) fps);
        qDebug() << "result -> " << (result * 1000);
        */
        int delayTime = millisecs;
        snprintf(args, sizeof(args), "delays=%d:all=1", delayTime);

        #ifdef TUP_DEBUG
            qDebug() << "[TFFmpegMovieGenerator::initFilterGraph()] - adelay filter args -> " << args;
        #endif

        AVFilterContext *adelayFilterContext;
        error = avfilter_graph_create_filter(&adelayFilterContext, adelayFilter, "adelay",
                                             args, nullptr, filterGraph);
        if (error < 0) {
            errorMsg = "ffmpeg error: Cannot create audio adelay filter.";
            #ifdef TUP_DEBUG
                qCritical() << "[TFFmpegMovieGenerator::initFilterGraph()] - " + errorMsg;
            #endif
            return false;
        }
        adelayFilterContextList << adelayFilterContext;

        // Connecting the abuffer filter with the adelay filter
        error = avfilter_link(abufferFilterContextList[i], 0, adelayFilterContextList[i], 0);
        if (error < 0) {
            errorMsg = "ffmpeg error: Error connecting abuffer with adelay filters.";
            #ifdef TUP_DEBUG
                qCritical() << "[TFFmpegMovieGenerator::initFilterGraph()] - " + errorMsg;
            #endif
            return false;
        }

        if (i == 0) {
            // amix
            // Create mix filter.
            amixFilter = avfilter_get_by_name("amix");
            if (!amixFilter) {
                errorMsg = "ffmpeg error: Could not find the mix filter.";
                #ifdef TUP_DEBUG
                    qCritical() << "[TFFmpegMovieGenerator::initFilterGraph()] - " << errorMsg;
                #endif
                return false;
            }

            snprintf(args, sizeof(args), "inputs=%d", soundsTotal);
            error = avfilter_graph_create_filter(&amixFilterContext, amixFilter, "amix", args, nullptr, filterGraph);
            if (error < 0) {
                errorMsg = "ffmpeg error: Cannot create audio amix filter.";
                #ifdef TUP_DEBUG
                    qCritical() << "[TFFmpegMovieGenerator::initFilterGraph()] - " << errorMsg;
                #endif
                return false;
            }
        }

        // Connecting the adelay filter with the amix filter
        error = avfilter_link(adelayFilterContextList[i], 0, amixFilterContext, i);
        if (error < 0) {
            errorMsg = "ffmpeg error: Error connecting adelay with amix filters.";
            #ifdef TUP_DEBUG
                qCritical() << "[TFFmpegMovieGenerator::initFilterGraph()] - " + errorMsg;
            #endif
            return false;
        }
    } // end for

    // Create the abuffersink filter;
    // it will be used to get the filtered data out of the graph.
    abufferSinkFilter = avfilter_get_by_name("abuffersink");
    if (!abufferSinkFilter) {
        errorMsg = "ffmpeg error: Could not find the abuffersink filter.";
        #ifdef TUP_DEBUG
            qCritical() << "[TFFmpegMovieGenerator::initFilterGraph()] - " << errorMsg;
        #endif
        return false;
    }

    abufferSinkContext = avfilter_graph_alloc_filter(filterGraph, abufferSinkFilter, "sink");
    if (!abufferSinkContext) {
        errorMsg = "ffmpeg error: Could not allocate the abuffersink instance.";
        #ifdef TUP_DEBUG
            qCritical() << "[TFFmpegMovieGenerator::initFilterGraph()] - " << errorMsg;
        #endif
        return false;
    }

    // Same sample fmts as the output file.
    int fmtList[] = { AV_SAMPLE_FMT_FLTP, AV_SAMPLE_FMT_NONE };
    error = av_opt_set_int_list(abufferSinkContext, "sample_fmts", fmtList,
                                AV_SAMPLE_FMT_NONE, AV_OPT_SEARCH_CHILDREN);
    if (error < 0) {
        errorMsg = "ffmpeg error: Could set options to the abuffersink instance.";
        #ifdef TUP_DEBUG
            qCritical() << "[TFFmpegMovieGenerator::initFilterGraph()] - " << errorMsg;
        #endif
        return false;
    }

    char ch_layout[64];
    av_get_channel_layout_string(ch_layout, sizeof(ch_layout), 0, soundsTotal);
    av_opt_set(abufferSinkContext, "channel_layout", ch_layout, AV_OPT_SEARCH_CHILDREN);

    error = avfilter_init_str(abufferSinkContext, nullptr);
    if (error < 0) {
        errorMsg = "ffmpeg error: Could not initialize the abuffersink instance.";
        #ifdef TUP_DEBUG
            qCritical() << "[TFFmpegMovieGenerator::initFilterGraph()] - " << errorMsg;
        #endif
        return false;
    }

    // * * *
    error = avfilter_link(amixFilterContext, 0, abufferSinkContext, 0);
    if (error < 0) {
        errorMsg = "ffmpeg error: Error connecting amix and abuffer filters.";
        #ifdef TUP_DEBUG
            qCritical() << "[TFFmpegMovieGenerator::initFilterGraph()] - " << errorMsg;
        #endif
        return false;
    }

    // Configure the graph.
    error = avfilter_graph_config(filterGraph, nullptr);
    if (error < 0) {
        errorMsg = "ffmpeg error: Error while configuring graph.";
        #ifdef TUP_DEBUG
            qCritical() << "[TFFmpegMovieGenerator::initFilterGraph()] - " << errorMsg;
        #endif
        return false;
    }

    #ifdef TUP_DEBUG
        char* dump =avfilter_graph_dump(filterGraph, nullptr);
        av_log(nullptr, AV_LOG_ERROR, "Graph :\n%s\n", dump);
    #endif

    return true;
}

// Decode one audio frame from the input file.
int TFFmpegMovieGenerator::decodeAudioFrame(AVFrame *frame, AVFormatContext *inputFormatContext,
                            AVCodecContext *inputCodecContext, int *dataPresent, int *finished)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TFFmpegMovieGenerator::decodeAudioFrame()]";
    #endif

    // Packet used for temporary storage.
    AVPacket *inputPacket = av_packet_alloc();
    int error;

    // Read one audio frame from the input file into a temporary packet.
    if ((error = av_read_frame(inputFormatContext, inputPacket)) < 0) {
        // If we are the the end of the file, flush the decoder below.
        if (error == AVERROR_EOF) {
            *finished = 1;
        } else {
            errorMsg = "ffmpeg error: Could not read frame.";
            #ifdef TUP_DEBUG
                qCritical() << "[TFFmpegMovieGenerator::decodeAudioFrame()] - " << errorMsg;
            #endif
            return error;
        }
    }

    // submit the packet to the decoder
    error = avcodec_send_packet(inputCodecContext, inputPacket);
    if (error < 0) {
        errorMsg = "ffmpeg error: Error while sending packet to decode.";
        #ifdef TUP_DEBUG
            qCritical() << "[TFFmpegMovieGenerator::decodeAudioFrame()] - " << errorMsg;
        #endif
        return error;
    }

    // get all the available frames from the decoder
    while (error >= 0) {
        error = avcodec_receive_frame(inputCodecContext, frame);
        if (error < 0) {
            // those two return values are special and mean there is no output
            // frame available, but there were no errors during decoding
            if (error == AVERROR_EOF || error == AVERROR(EAGAIN)) {
                return error;
            }

            return error;
        }

        *dataPresent = 0;
        if (frame) {
            *dataPresent = 1;
            break;
        }
    }

    // If the decoder has not been flushed completely, we are not finished,
    // so that this function has to be called again.
    if (*finished && *dataPresent)
        *finished = 0;
    av_packet_unref(inputPacket);

    return 0;
}

// Encode one frame worth of audio to the output file.
int TFFmpegMovieGenerator::encodeAudioFrame(AVFrame *frame, AVFormatContext *outputFormatContext,
                                            AVCodecContext *outputCodecContext, int *dataPresent)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TFFmpegMovieGenerator::encodeAudioFrame()]";
    #endif

    // Packet used for temporary storage.
    AVPacket *ouputPacket = av_packet_alloc();
    int error;
    *dataPresent = 0;

    // send the frame for encoding
    error = avcodec_send_frame(outputCodecContext, frame);
    if (error == AVERROR_EOF) {
        error = 0;
        goto cleanup;
    } else if (error < 0) {
        errorMsg = "ffmpeg error: Could not send frame for encoding.";
        #ifdef TUP_DEBUG
            qCritical() << "[TFFmpegMovieGenerator::encodeAudioFrame()] - " << errorMsg;
        #endif
        goto cleanup;
    }

    // read all the available output packets (in general there may be any number of them
    while (1) {
        error = avcodec_receive_packet(outputCodecContext, ouputPacket);
        if (error == AVERROR(EAGAIN) || error == AVERROR_EOF) {
            return error;
        } else if (error < 0) {
            errorMsg = "ffmpeg error: Unexpected error.";
            #ifdef TUP_DEBUG
                qCritical() << "[TFFmpegMovieGenerator::encodeAudioFrame()] - " << errorMsg;
            #endif
            return error;
        }

        if (ouputPacket) {
            if ((error = av_write_frame(outputFormatContext, ouputPacket)) < 0) {
                errorMsg = "ffmpeg error: Could not write frame.";
                #ifdef TUP_DEBUG
                    qCritical() << "[TFFmpegMovieGenerator::encodeAudioFrame()] - " << errorMsg;
                #endif
                av_packet_unref(ouputPacket);
                return error;
            }

            av_packet_unref(ouputPacket);
            *dataPresent = 1;
            break;
        }
    }

    return 0;

    cleanup:
        av_frame_free(&frame);
        avcodec_free_context(&outputCodecContext);
        avformat_free_context(outputFormatContext);

    return error < 0 ? error : AVERROR_EXIT;
}

bool TFFmpegMovieGenerator::processAudioFiles()
{
    #ifdef TUP_DEBUG
        qDebug() << "[TFFmpegMovieGenerator::processAudioFiles()]";
    #endif

    int error = 0;
    int dataPresent = 0;
    int finished = 0;
    int totalOutSamples = 0;
    int soundFinished = 0;
    int soundsTotal = sounds.size();

    int inputFinished[soundsTotal];
    int inputToRead[soundsTotal];
    int totalSamples[soundsTotal];
    for (int i=0; i<soundsTotal; i++) {
        inputFinished[i] = 0;
        inputToRead[i] = 1;
        totalSamples[i] = 0;
    }

    while (soundFinished < soundsTotal) {
        int dataPresentInGraph = 0;
        for (int i=0; i<soundsTotal; i++) {
            if (inputFinished[i] || inputToRead[i] == 0)
                continue;

            inputToRead[i] = 0;

            AVFrame *frame = av_frame_alloc();
            if (!frame)
                goto end;

            if (!avcodec_is_open(audioInputCodecContextList[i])) {
                qDebug() << "*** ALERT 1!";
            }

            if (!av_codec_is_decoder(audioInputCodecContextList[i]->codec)) {
                qDebug() << "*** ALERT 2!";
            }

            const AVCodec *codec = audioInputCodecContextList[i]->codec;
            if (!codec) {
                qDebug() << "*** ALERT 3!";
            }

            if (!codec->decode) {
                qDebug() << "*** ALERT 4!";
            }

            if (!codec->receive_frame) {
                qDebug() << "*** ALERT 5!";
            }

            // Decode one frame worth of audio samples.
            if ((error = decodeAudioFrame(frame, audioInputFormatContextList[i], audioInputCodecContextList[i],
                                          &dataPresent, &finished))) {
                qDebug() << "";
                qDebug() << "*** ERROR -> " << error;

                goto end;
            }

            // If we are at the end of the file and there are no more samples
            // in the decoder which are delayed, we are actually finished.
            // This must not be treated as an error.
            if (finished && !dataPresent) {
                inputFinished[i] = 1;
                soundFinished++;
                av_log(nullptr, AV_LOG_INFO, "Input n°%d finished. Write NULL frame \n", i);

                error = av_buffersrc_write_frame(abufferFilterContextList.at(i), nullptr);
                if (error < 0) {
                    av_log(nullptr, AV_LOG_ERROR, "Error writing EOF null frame for input %d\n", i);
                    goto end;
                }
            } else if (dataPresent) { // If there is decoded data, convert and store it
                // push the audio data from decoded frame into the filtergraph
                error = av_buffersrc_write_frame(abufferFilterContextList.at(i), frame);
                if (error < 0) {
                    av_log(nullptr, AV_LOG_ERROR, "Error while feeding the audio filtergraph\n");
                    goto end;
                }

                av_log(nullptr, AV_LOG_INFO, "add %d samples on input %d (%d Hz, time=%f, ttime=%f)\n",
                       frame->nb_samples, i, audioInputCodecContextList[i]->sample_rate,
                       (double)frame->nb_samples / audioInputCodecContextList[i]->sample_rate,
                       (double)(totalSamples[i] += frame->nb_samples) / audioInputCodecContextList[i]->sample_rate);
            }

            av_frame_free(&frame);
            dataPresentInGraph = dataPresent | dataPresentInGraph;
        }

        if (dataPresentInGraph) {
            AVFrame *filterFrame = av_frame_alloc();
            // pull filtered audio from the filtergraph
            while (1) {
                error = av_buffersink_get_frame(abufferSinkContext, filterFrame);
                if (error == AVERROR(EAGAIN) || error == AVERROR_EOF) {
                    for (int i = 0 ; i < soundsTotal ; i++) {
                        if (av_buffersrc_get_nb_failed_requests(abufferFilterContextList[i]) > 0) {
                            inputToRead[i] = 1;
                            av_log(nullptr, AV_LOG_INFO, "Need to read input %d\n", i);
                        }
                    }
                    break;
                }

                if (error < 0) {
                    av_log(nullptr, AV_LOG_ERROR, "Error while getting filt_frame from sink\n");
                    goto end;
                }

                av_log(nullptr, AV_LOG_INFO, "remove %d samples from sink (%d Hz, time=%f, ttime=%f)\n",
                       filterFrame->nb_samples, audioOutputCodecContext->sample_rate,
                       (double)filterFrame->nb_samples / audioOutputCodecContext->sample_rate,
                       (double)(totalOutSamples += filterFrame->nb_samples) / audioOutputCodecContext->sample_rate);

                error = encodeAudioFrame(filterFrame, formatContext, audioOutputCodecContext, &dataPresent);
                if (error < 0) {
                    errorMsg = "ffmpeg error: Tracing error at encodeAudioFrame().";
                    #ifdef TUP_DEBUG
                        qCritical() << "[TFFmpegMovieGenerator::processAudioFiles()] - " << errorMsg;
                    #endif
                    goto end;
                }
                av_frame_unref(filterFrame);
            }

            av_frame_free(&filterFrame);
        } else {
            av_log(nullptr, AV_LOG_INFO, "No data in graph\n");
            for (int i=0; i<soundsTotal; i++) {
                inputToRead[i] = 1;
            }
        }
    }

    return true;

    end:
        if (error < 0 && error != AVERROR_EOF) {
            errorMsg = "ffmpeg error: Workflow has been interrupted!";
            #ifdef TUP_DEBUG
                qCritical() << "[TFFmpegMovieGenerator::processAudioFiles()] - " << errorMsg;
            #endif
            return false;
        }
        return true;
}

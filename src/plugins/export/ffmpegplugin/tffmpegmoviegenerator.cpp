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

// The output bit rate in bit/s
#define OUTPUT_BIT_RATE 96000
// The number of output channels
#define OUTPUT_CHANNELS 2

// Global timestamp for the audio frames.
static int64_t pts;

// Handy documentation about Libav library
// https://github.com/leandromoreira/ffmpeg-libav-tutorial
// http://libav-users.943685.n4.nabble.com/Save-AVFrame-to-jpg-file-td2314979.html

TFFmpegMovieGenerator::TFFmpegMovieGenerator(TMovieGeneratorInterface::Format format, const QSize &size, 
                                             int fpsParam, double duration, const QString &audio)
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
    samples_count = 0;
    pts = 0;

    hasSound = true;
    inputAudioPath = audio;
    if (inputAudioPath.isEmpty())
        hasSound = false;

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

    if (hasSound) {
        if (!loadInputAudio(inputAudioPath)) {
            errorMsg = "ffmpeg error: Could not load audio input streams.";
            #ifdef TUP_DEBUG
                qCritical() << "[TFFmpegMovieGenerator::initVideoFile()] - " << errorMsg;
            #endif
            return false;
        }

        if (!openAudioInputStream()) {
            errorMsg = "ffmpeg error: Could not open audio input streams.";
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
        qDebug() << "[TFFmpegMovieGenerator::loadInputAudio()] - soundPath -> " + soundPath;
    #endif

    QByteArray bytes = soundPath.toLocal8Bit();
    const char *inputFile = bytes.data();
    int error;

    audioInputFormatContext = avformat_alloc_context();
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

    // Process the stream from input audio
    audioInputStream = audioInputFormatContext->streams[0];
    if (!audioInputStream) {
        errorMsg = "ffmpeg error: audio input stream is NULL!";
        #ifdef TUP_DEBUG
            qCritical() << "[TFFmpegMovieGenerator::loadInputAudio()] - " + errorMsg;
        #endif
        return false;
    }

    AVCodecParameters *audioInputCodecPar = audioInputStream->codecpar;
    if (audioInputCodecPar->codec_type != AVMEDIA_TYPE_AUDIO) {
        errorMsg = "ffmpeg error: No audio stream was found!";
        #ifdef TUP_DEBUG
            qCritical() << "[TFFmpegMovieGenerator::loadInputAudio()] - " + errorMsg;
        #endif
        return false;
    }

    enum AVCodecID audioInputCodecID = audioInputCodecPar->codec_id;
    #ifdef TUP_DEBUG
        qDebug() << "[TFFmpegMovieGenerator::loadInputAudio()] - Audio codec detected -> "
                 << avcodec_get_name(audioInputCodecID);
    #endif

    audioInputCodec = avcodec_find_decoder(audioInputCodecID);
    if (!audioInputCodec) {
        errorMsg = "ffmpeg error: Could not find audio decoder.";
        #ifdef TUP_DEBUG
            qCritical() << "[TFFmpegMovieGenerator::loadInputAudio()] - " + errorMsg;
            qCritical() << "[TFFmpegMovieGenerator::loadInputAudio()] - Unavailable Codec ID -> "
                        << avcodec_get_name(audioInputCodecID);
        #endif
        return false;
    }

    audioInputCodecContext = avcodec_alloc_context3(audioInputCodec);
    if (!audioInputCodecContext) {
        errorMsg = "ffmpeg error: Could not initialize audio codec context.";
        #ifdef TUP_DEBUG
            qCritical() << "[TFFmpegMovieGenerator::loadInputAudio()] - " + errorMsg;
        #endif
        return false;
    }

    error = avcodec_parameters_to_context(audioInputCodecContext, audioInputCodecPar);
    if (error < 0) {
        errorMsg = "ffmpeg error: Can't copy codecpar values to input codec context.";
        #ifdef TUP_DEBUG
            qCritical() << "[TFFmpegMovieGenerator::loadInputAudio()] - " + errorMsg;
        #endif
        return false;
    }

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

bool TFFmpegMovieGenerator::openAudioInputStream()
{
    #ifdef TUP_DEBUG
        qDebug() << "[TFFmpegMovieGenerator::openAudioInputStreams()]";
    #endif

    // Open the input audio codec
    int error = avcodec_open2(audioInputCodecContext, audioInputCodec, nullptr);
    if (error < 0) {
        errorMsg = "ffmpeg error: Can't open audio codec.";
        #ifdef TUP_DEBUG
            qCritical() << "[TFFmpegMovieGenerator::openAudioInputStreams()] - " << errorMsg;
        #endif
        return false;
    }

    return true;
}

bool TFFmpegMovieGenerator::openAudioOutputStream()
{
    #ifdef TUP_DEBUG
        qDebug() << "[TFFmpegMovieGenerator::openAudioOutputStream()] - audio codec -> "
                 << avcodec_get_name(audioOutputCodecID);
    #endif

    int error;
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

    // Create a new audio stream in the output file container.
    if (!(audioOutputStream = avformat_new_stream(formatContext, nullptr))) {
        #ifdef TUP_DEBUG
            error = AVERROR(ENOMEM);
            qCritical() << "[TFFmpegMovieGenerator::openAudioOutputStream()] - "
                           "Fatal Error: Could not create new stream.";
            qCritical() << "ERROR CODE -> " << error;
        #endif
        return false;
    }

    audioOutputCodecContext = avcodec_alloc_context3(audioOutputCodec);
    if (!audioOutputCodecContext) {
        #ifdef TUP_DEBUG
            error = AVERROR(ENOMEM);
            qCritical() << "[TFFmpegMovieGenerator::openAudioOutputStream()] - "
                           "Fatal Error: Could not allocate an encoding context.";
            qCritical() << "ERROR CODE -> " << error;
        #endif
        avcodec_free_context(&audioOutputCodecContext);
        return false;
    }

    // Set the basic encoder parameters.
    // The input file's sample rate is used to avoid a sample rate conversion.
    audioOutputCodecContext->channels       = OUTPUT_CHANNELS;
    audioOutputCodecContext->channel_layout = av_get_default_channel_layout(OUTPUT_CHANNELS);
    audioOutputCodecContext->sample_rate    = audioInputCodecContext->sample_rate;
    audioOutputCodecContext->sample_fmt     = audioOutputCodec->sample_fmts[0];
    audioOutputCodecContext->bit_rate       = OUTPUT_BIT_RATE;

    // Allow the use of the experimental AAC encoder.
    audioOutputCodecContext->strict_std_compliance = FF_COMPLIANCE_EXPERIMENTAL;

    // Set the sample rate for the container.
    audioOutputStream->time_base.den = audioInputCodecContext->sample_rate;
    audioOutputStream->time_base.num = 1;

    // Some container formats (like MP4) require global headers to be present.
    // Mark the encoder so that it behaves accordingly.
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
    int error = avcodec_open2(audioOutputCodecContext, audioOutputCodec, nullptr);
    if (error < 0) {
        errorMsg = "ffmpeg error: Can't open audio output codec.";
        #ifdef TUP_DEBUG
            qCritical() << "[TFFmpegMovieGenerator::openAudioOutputCodec()] - " + errorMsg;
            qCritical() << "ERROR CODE -> " << error;
        #endif
        return false;
    }

    error = avcodec_parameters_from_context(audioOutputStream->codecpar, audioOutputCodecContext);
    if (error < 0) {
        #ifdef TUP_DEBUG
            qCritical() << "[TFFmpegMovieGenerator::openAudioOutputCodec()] - "
                           "Fatal Error: Could not initialize stream parameters.";
            qCritical() << "ERROR CODE -> " << error;
        #endif
        return false;
    }

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
    #ifdef TUP_DEBUG
        qDebug() << "[TFFmpegMovieGenerator::endVideoFile()]";
    #endif

    av_write_trailer(formatContext);

    if (videoCodecContext)
        avcodec_close(videoCodecContext);
    av_frame_free(&videoFrame);

    if (hasSound) {
        if (audioInputCodecContext) {
            avcodec_close(audioInputCodecContext);
            avcodec_free_context(&audioInputCodecContext);
        }

        if (audioInputFormatContext)
            avformat_close_input(&audioInputFormatContext);

        if (audioOutputCodecContext) {
            avcodec_close(audioOutputCodecContext);
            avcodec_free_context(&audioOutputCodecContext);
        }
    }

    if (formatContext) {
        if (!(outputFormat->flags & AVFMT_NOFILE))
            avio_close(formatContext->pb);
        avformat_free_context(formatContext);
    }
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
                qCritical() << "[TFFmpegMovieGenerator::copyMovieFile()] - Fatal Error: Video path is NOT a file! -> "
                            << videoPath;
            #endif
            return;
        }

        if (!QFile::remove(videoPath)) {
            #ifdef TUP_DEBUG
                qCritical() << "[TFFmpegMovieGenerator::copyMovieFile()] - Fatal Error: Can't remove file! -> "
                            << videoPath;
            #endif
            return;
        }
    }

    if (QFile::copy(movieFile, videoPath)) {
        if (QFile::exists(movieFile)) {
            #ifdef TUP_DEBUG
                qDebug() << "[TFFmpegMovieGenerator::copyMovieFile()] - Trying to remove temp video file -> "
                         << movieFile;
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
                qCritical() << "[TFFmpegMovieGenerator::copyMovieFile()] - Error: Temp video file wasn't found! -> "
                            << movieFile;
            #endif
        }
    } else {
        #ifdef TUP_DEBUG
            qCritical() << "[TFFmpegMovieGenerator::copyMovieFile()] - Error: Can't create video file -> "
                        << videoPath;
        #endif
    }
}

bool TFFmpegMovieGenerator::validMovieHeader()
{
    return exception;
}

QString TFFmpegMovieGenerator::getErrorMsg() const
{
    QString errorDetail = "It seems there was an internal error while exporting your animation.<br/>"
                          "Please, contact our technical support team.<br/>"
                          "More info: <b>https://tupitube.com</b>";
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

// SQA: Method just for debugging
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

// AUDIO SECTION

/**
 * Initialize one data packet for reading or writing.
 * @param[out] packet Packet to be initialized
 * @return Error code (0 if successful)
 */
int TFFmpegMovieGenerator::initPacket(AVPacket **packet)
{
    if (!(*packet = av_packet_alloc())) {
        fprintf(stderr, "Could not allocate packet\n");
        return AVERROR(ENOMEM);
    }
    return 0;
}

/**
 * Initialize one audio frame for reading from the input file.
 * @param[out] frame Frame to be initialized
 * @return Error code (0 if successful)
 */
int TFFmpegMovieGenerator::initInputFrame(AVFrame **frame)
{
    if (!(*frame = av_frame_alloc())) {
        fprintf(stderr, "Could not allocate input frame\n");
        return AVERROR(ENOMEM);
    }
    return 0;
}

/**
 * Initialize the audio resampler based on the input and output codec settings.
 * If the input and output sample formats differ, a conversion is required
 * libswresample takes care of this, but requires initialization.
 * @param      inputCodecContext  Codec context of the input file
 * @param      outputCodecContext Codec context of the output file
 * @param[out] resampleContext    Resample context for the required conversion
 * @return Error code (0 if successful)
 */
int TFFmpegMovieGenerator::initResampler(AVCodecContext *inputCodecContext,
                          AVCodecContext *outputCodecContext,
                          SwrContext **resampleContext)
{
    int error;

    /*
     * Create a resampler context for the conversion.
     * Set the conversion parameters.
     * Default channel layouts based on the number of channels
     * are assumed for simplicity (they are sometimes not detected
     * properly by the demuxer and/or decoder).
     */
    *resampleContext = swr_alloc_set_opts(nullptr,
                                          av_get_default_channel_layout(outputCodecContext->channels),
                                          outputCodecContext->sample_fmt,
                                          outputCodecContext->sample_rate,
                                          av_get_default_channel_layout(inputCodecContext->channels),
                                          inputCodecContext->sample_fmt,
                                          inputCodecContext->sample_rate,
                                          0, nullptr);
    if (!*resampleContext) {
        fprintf(stderr, "Could not allocate resample context\n");
        return AVERROR(ENOMEM);
    }
    /*
    * Perform a sanity check so that the number of converted samples is
    * not greater than the number of samples to be converted.
    * If the sample rates differ, this case has to be handled differently
    */
    av_assert0(outputCodecContext->sample_rate == inputCodecContext->sample_rate);

    /* Open the resampler with the specified parameters. */
    if ((error = swr_init(*resampleContext)) < 0) {
        fprintf(stderr, "Could not open resample context\n");
        swr_free(resampleContext);
        return error;
    }

    return 0;
}

/**
 * Initialize a FIFO buffer for the audio samples to be encoded.
 * @param[out] fifo                 Sample buffer
 * @param      outputCodecContext   Codec context of the output file
 * @return Error code (0 if successful)
 */
int TFFmpegMovieGenerator::initFifo(AVAudioFifo **fifo, AVCodecContext *outputCodecContext)
{
    // Create the FIFO buffer based on the specified output sample format.
    if (!(*fifo = av_audio_fifo_alloc(outputCodecContext->sample_fmt,
                                      outputCodecContext->channels, 1))) {
        fprintf(stderr, "Could not allocate FIFO\n");
        return AVERROR(ENOMEM);
    }
    return 0;
}

/**
 * Decode one audio frame from the input file.
 * @param      frame                Audio frame to be decoded
 * @param      inputFormatContext Format context of the input file
 * @param      inputCodecContext  Codec context of the input file
 * @param[out] dataPresent         Indicates whether data has been decoded
 * @param[out] finished             Indicates whether the end of file has
 *                                  been reached and all data has been
 *                                  decoded. If this flag is false, there
 *                                  is more data to be decoded, i.e., this
 *                                  function has to be called again.
 * @return Error code (0 if successful)
 */
int TFFmpegMovieGenerator::decodeAudioFrame(AVFrame *frame,
                              AVFormatContext *inputFormatContext,
                              AVCodecContext *inputCodecContext,
                              int *dataPresent, int *finished)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TFFmpegMovieGenerator::decodeAudioFrame()]";
    #endif

    // Packet used for temporary storage.
    AVPacket *inputPacket;
    int error;

    error = initPacket(&inputPacket);
    if (error < 0)
        return error;

    // Read one audio frame from the input file into a temporary packet.
    if ((error = av_read_frame(inputFormatContext, inputPacket)) < 0) {
        // If we are at the end of the file, flush the decoder below.
        if (error == AVERROR_EOF)
            *finished = 1;
        else {
            fprintf(stderr, "Could not read frame (error '%d')\n",
                    error);
            goto cleanup;
        }
    }

    #ifdef TUP_DEBUG
        logPacket(Audio, audioInputStream->time_base, inputPacket, "in");
    #endif

    // Send the audio frame stored in the temporary packet to the decoder.
    // The input audio stream decoder is used to do this.
    if ((error = avcodec_send_packet(inputCodecContext, inputPacket)) < 0) {
        fprintf(stderr, "Could not send packet for decoding (error '%d')\n",
                error);
        goto cleanup;
    }

    // Receive one frame from the decoder.
    error = avcodec_receive_frame(inputCodecContext, frame);
    // If the decoder asks for more data to be able to decode a frame,
    // return indicating that no data is present.
    if (error == AVERROR(EAGAIN)) {
        error = 0;
        goto cleanup;
    // If the end of the input file is reached, stop decoding.
    } else if (error == AVERROR_EOF) {
        *finished = 1;
        error = 0;
        goto cleanup;
    } else if (error < 0) {
        fprintf(stderr, "Could not decode frame (error '%d')\n",
                error);
        goto cleanup;
    // Default case: Return decoded data.
    } else {
        *dataPresent = 1;
        goto cleanup;
    }

    cleanup:
        av_packet_free(&inputPacket);

        return error;
}

/**
 * Initialize a temporary storage for the specified number of audio samples.
 * The conversion requires temporary storage due to the different format.
 * The number of audio samples to be allocated is specified in frame_size.
 * @param[out] convertedInputSamples   Array of converted samples. The
 *                                     dimensions are reference, channel
 *                                     (for multi-channel audio), sample.
 * @param      outputCodecContext      Codec context of the output file
 * @param      frameSize               Number of samples to be converted in
 *                                     each round
 * @return Error code (0 if successful)
 */
int TFFmpegMovieGenerator::initConvertedSamples(uint8_t ***convertedInputSamples,
                                  AVCodecContext *outputCodecContext,
                                  int frameSize)
{
    int error;

    /* Allocate as many pointers as there are audio channels.
     * Each pointer will later point to the audio samples of the corresponding
     * channels (although it may be nullptr for interleaved formats).
     */
    if (!(*convertedInputSamples = (uint8_t **) calloc(outputCodecContext->channels,
                                            sizeof(**convertedInputSamples)))) {
        fprintf(stderr, "Could not allocate converted input sample pointers\n");
        return AVERROR(ENOMEM);
    }

    // Allocate memory for the samples of all channels in one consecutive
    // block for convenience.
    if ((error = av_samples_alloc(*convertedInputSamples, nullptr,
                                  outputCodecContext->channels,
                                  frameSize,
                                  outputCodecContext->sample_fmt, 0)) < 0) {
        fprintf(stderr,
                "Could not allocate converted input samples (error '%d')\n",
                error);
        av_freep(&(*convertedInputSamples)[0]);
        free(*convertedInputSamples);
        return error;
    }
    return 0;
}

/**
 * Convert the input audio samples into the output sample format.
 * The conversion happens on a per-frame basis, the size of which is
 * specified by frame_size.
 * @param      inputData        Samples to be decoded. The dimensions are
 *                              channel (for multi-channel audio), sample.
 * @param[out] convertedData    Converted samples. The dimensions are channel
 *                              (for multi-channel audio), sample.
 * @param      frameSize        Number of samples to be converted
 * @param      resampleContext  Resample context for the conversion
 * @return Error code (0 if successful)
 */
int TFFmpegMovieGenerator::convertSamples(const uint8_t **inputData,
                           uint8_t **convertedData, const int frameSize,
                           SwrContext *resampleContext)
{
    int error;

    // Convert the samples using the resampler.
    if ((error = swr_convert(resampleContext,
                             convertedData, frameSize,
                             inputData    , frameSize)) < 0) {
        fprintf(stderr, "Could not convert input samples (error '%d')\n",
                error);
        return error;
    }

    return 0;
}

/**
 * Add converted input audio samples to the FIFO buffer for later processing.
 * @param fifo                    Buffer to add the samples to
 * @param convertedInputSamples   Samples to be added. The dimensions are channel
 *                                (for multi-channel audio), sample.
 * @param frameSize               Number of samples to be converted
 * @return Error code (0 if successful)
 */
int TFFmpegMovieGenerator::addSamplesToFifo(AVAudioFifo *fifo,
                               uint8_t **convertedInputSamples,
                               const int frameSize)
{
    int error;

    /* Make the FIFO as large as it needs to be to hold both,
     * the old and the new samples. */
    if ((error = av_audio_fifo_realloc(fifo, av_audio_fifo_size(fifo) + frameSize)) < 0) {
        fprintf(stderr, "Could not reallocate FIFO\n");
        return error;
    }

    // Store the new samples in the FIFO buffer.
    if (av_audio_fifo_write(fifo, (void **)convertedInputSamples,
                            frameSize) < frameSize) {
        fprintf(stderr, "Could not write data to FIFO\n");
        return AVERROR_EXIT;
    }
    return 0;
}

/**
 * Read one audio frame from the input file, decode, convert and store
 * it in the FIFO buffer.
 * @param      fifo                 Buffer used for temporary storage
 * @param      input_format_context Format context of the input file
 * @param      input_codec_context  Codec context of the input file
 * @param      output_codec_context Codec context of the output file
 * @param      resampler_context    Resample context for the conversion
 * @param[out] finished             Indicates whether the end of file has
 *                                  been reached and all data has been
 *                                  decoded. If this flag is false,
 *                                  there is more data to be decoded,
 *                                  i.e., this function has to be called
 *                                  again.
 * @return Error code (0 if successful)
 */
int TFFmpegMovieGenerator::readDecodeConvertAndStore(AVAudioFifo *fifo,
                                         AVFormatContext *inputFormatContext,
                                         AVCodecContext *inputCodecContext,
                                         AVCodecContext *outputCodecContext,
                                         SwrContext *resamplerContext,
                                         int *finished)
{
    /* Temporary storage of the input samples of the frame read from the file. */
    AVFrame *inputFrame = nullptr;
    /* Temporary storage for the converted input samples. */
    uint8_t **convertedInputSamples = nullptr;
    int dataPresent = 0;
    int ret = AVERROR_EXIT;

    /* Initialize temporary storage for one input frame. */
    if (initInputFrame(&inputFrame))
        goto cleanup;
    /* Decode one frame worth of audio samples. */
    if (decodeAudioFrame(inputFrame, inputFormatContext,
                           inputCodecContext, &dataPresent, finished))
        goto cleanup;
    /* If we are at the end of the file and there are no more samples
     * in the decoder which are delayed, we are actually finished.
     * This must not be treated as an error. */
    if (*finished) {
        ret = 0;
        goto cleanup;
    }
    /* If there is decoded data, convert and store it. */
    if (dataPresent) {
        /* Initialize the temporary storage for the converted input samples. */
        if (initConvertedSamples(&convertedInputSamples, outputCodecContext,
                                   inputFrame->nb_samples))
            goto cleanup;

        /* Convert the input samples to the desired output sample format.
         * This requires a temporary storage provided by converted_input_samples. */
        if (convertSamples((const uint8_t**)inputFrame->extended_data, convertedInputSamples,
                            inputFrame->nb_samples, resamplerContext))
            goto cleanup;

        /* Add the converted input samples to the FIFO buffer for later processing. */
        if (addSamplesToFifo(fifo, convertedInputSamples,
                                inputFrame->nb_samples))
            goto cleanup;
    }
    ret = 0;

    cleanup:
        /*
        if (convertedInputSamples) {
            // av_freep(&convertedInputSamples[0]);
            free(convertedInputSamples);
        }
        */
        convertedInputSamples = nullptr;
        av_frame_free(&inputFrame);

    return ret;
}

/**
 * Initialize one input frame for writing to the output file.
 * The frame will be exactly frame_size samples large.
 * @param[out] frame                Frame to be initialized
 * @param      output_codec_context Codec context of the output file
 * @param      frame_size           Size of the frame
 * @return Error code (0 if successful)
 */
int TFFmpegMovieGenerator::initOutputFrame(AVFrame **frame,
                             AVCodecContext *outputCodecContext,
                             int frameSize)
{
    int error;

    /* Create a new frame to store the audio samples. */
    if (!(*frame = av_frame_alloc())) {
        fprintf(stderr, "Could not allocate output frame\n");
        return AVERROR_EXIT;
    }

    /* Set the frame's parameters, especially its size and format.
     * av_frame_get_buffer needs this to allocate memory for the
     * audio samples of the frame.
     * Default channel layouts based on the number of channels
     * are assumed for simplicity. */
    (*frame)->nb_samples     = frameSize;
    (*frame)->channel_layout = outputCodecContext->channel_layout;
    (*frame)->format         = outputCodecContext->sample_fmt;
    (*frame)->sample_rate    = outputCodecContext->sample_rate;

    /* Allocate the samples of the created frame. This call will make
     * sure that the audio frame can hold as many samples as specified. */
    if ((error = av_frame_get_buffer(*frame, 0)) < 0) {
        fprintf(stderr, "Could not allocate output frame samples (error '%d')\n",
                error);
        av_frame_free(frame);
        return error;
    }

    return 0;
}

/**
 * Encode one frame worth of audio to the output file.
 * @param      frame                 Samples to be encoded
 * @param      output_format_context Format context of the output file
 * @param      output_codec_context  Codec context of the output file
 * @param[out] data_present          Indicates whether data has been
 *                                   encoded
 * @return Error code (0 if successful)
 */
int TFFmpegMovieGenerator::encodeAudioFrame(AVFrame *frame,
                              AVFormatContext *outputFormatContext,
                              AVCodecContext *outputCodecContext,
                              int *dataPresent)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TFFmpegMovieGenerator::encodeAudioFrame()]";
    #endif

    // Packet used for temporary storage.
    AVPacket *outputPacket;
    int error;

    error = initPacket(&outputPacket);
    if (error < 0)
        return error;

    // Set a timestamp based on the sample rate for the container.
    if (frame) {
        frame->pts = pts;
        pts += frame->nb_samples;
    }

    // Send the audio frame stored in the temporary packet to the encoder.
    // The output audio stream encoder is used to do this.
    error = avcodec_send_frame(outputCodecContext, frame);
    // The encoder signals that it has nothing more to encode.
    if (error == AVERROR_EOF) {
        error = 0;
        goto cleanup;
    } else if (error < 0) {
        #ifdef TUP_DEBUG
            qDebug() << "[TFFmpegMovieGenerator::encodeAudioFrame()] - "
                        "Fatal Error: Could not send packet for encoding (error '" << error << "')";
        #endif
        goto cleanup;
    }

    // Receive one encoded frame from the encoder.
    error = avcodec_receive_packet(outputCodecContext, outputPacket);
    // If the encoder asks for more data to be able to provide an
    // encoded frame, return indicating that no data is present.
    if (error == AVERROR(EAGAIN)) {
        error = 0;
        goto cleanup;
    // If the last frame has been encoded, stop encoding.
    } else if (error == AVERROR_EOF) {
        error = 0;
        goto cleanup;
    } else if (error < 0) {
        #ifdef TUP_DEBUG
            qDebug() << "[TFFmpegMovieGenerator::encodeAudioFrame()] - "
                        "Fatal Error: Could not encode frame (error '" << error << "')";
        #endif
        goto cleanup;
    // Default case: Return encoded data.
    } else {
        *dataPresent = 1;
    }

    #ifdef TUP_DEBUG
        logPacket(Audio, audioOutputStream->time_base, outputPacket, "out");
    #endif

    // Write one audio frame from the temporary packet to the output file.
    if (*dataPresent &&
        (error = av_write_frame(outputFormatContext, outputPacket)) < 0) {
        #ifdef TUP_DEBUG
            qDebug() << "[TFFmpegMovieGenerator::encodeAudioFrame()] - "
                        "Fatal Error: Could not write frame (error '" << error << "')";
        #endif
        goto cleanup;
    }

    cleanup:
        av_packet_free(&outputPacket);

    return error;
}

/**
 * Load one audio frame from the FIFO buffer, encode and write it to the
 * output file.
 * @param fifo                  Buffer used for temporary storage
 * @param output_format_context Format context of the output file
 * @param output_codec_context  Codec context of the output file
 * @return Error code (0 if successful)
 */
int TFFmpegMovieGenerator::loadEncodeAndWrite(AVAudioFifo *fifo,
                                 AVFormatContext *outputFormatContext,
                                 AVCodecContext *outputCodecContext)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TFFmpegMovieGenerator::loadEncodeAndWrite()]";
    #endif

    // Temporary storage of the output samples of the frame written to the file.
    AVFrame *outputFrame;
    // Use the maximum number of possible samples per frame.
    // If there is less than the maximum possible frame size in the FIFO
    // buffer use this number. Otherwise, use the maximum possible frame size.
    const int frame_size = FFMIN(av_audio_fifo_size(fifo),
                                 outputCodecContext->frame_size);
    int dataWritten;

    // Initialize temporary storage for one output frame.
    if (initOutputFrame(&outputFrame, outputCodecContext, frame_size))
        return AVERROR_EXIT;

    // Read as many samples from the FIFO buffer as required to fill the frame.
    // The samples are stored in the frame temporarily.
    if (av_audio_fifo_read(fifo, (void **)outputFrame->data, frame_size) < frame_size) {
        fprintf(stderr, "Could not read data from FIFO\n");
        av_frame_free(&outputFrame);
        return AVERROR_EXIT;
    }

    // Encode one frame worth of audio samples.
    if (encodeAudioFrame(outputFrame, outputFormatContext,
                           outputCodecContext, &dataWritten)) {
        av_frame_free(&outputFrame);
        return AVERROR_EXIT;
    }
    av_frame_free(&outputFrame);

    return 0;
}

int TFFmpegMovieGenerator::mergeAudioStream()
{
    SwrContext *resampleContext = nullptr;
    AVAudioFifo *fifo = nullptr;
    int ret = AVERROR_EXIT;

    // Initialize the resampler to be able to convert audio sample formats.
    if (initResampler(audioInputCodecContext, audioOutputCodecContext,
                       &resampleContext))
        goto cleanup;
    // Initialize the FIFO buffer to store audio samples to be encoded.
    if (initFifo(&fifo, audioOutputCodecContext))
        goto cleanup;

    // Loop as long as we have input samples to read or output samples
    // to write; abort as soon as we have neither.
    while (1) {
        // Use the encoder's desired frame size for processing.
        const int outputFrameSize = audioOutputCodecContext->frame_size;
        int finished = 0;

        // Make sure that there is one frame worth of samples in the FIFO
        // buffer so that the encoder can do its work.
        // Since the decoder's and the encoder's frame size may differ, we
        // need to FIFO buffer to store as many frames worth of input samples
        // that they make up at least one frame worth of output samples.
        while (av_audio_fifo_size(fifo) < outputFrameSize) {
            // Decode one frame worth of audio samples, convert it to the
            // output sample format and put it into the FIFO buffer.
            if (readDecodeConvertAndStore(fifo, audioInputFormatContext,
                                              audioInputCodecContext,
                                              audioOutputCodecContext,
                                              resampleContext, &finished))
            {
                #ifdef TUP_DEBUG
                    qDebug() << "[TFFmpegMovieGenerator::mergeAudioStream()] - Tracing readDecodeConvertAndStore() method...";
                #endif
                goto cleanup;
            }
            // If we are at the end of the input file, we continue
            // encoding the remaining audio samples to the output file.
            if (finished)
                break;
        }

        // If we have enough samples for the encoder, we encode them.
        // At the end of the file, we pass the remaining samples to
        // the encoder.
        while (av_audio_fifo_size(fifo) >= outputFrameSize ||
               (finished && av_audio_fifo_size(fifo) > 0))
            // Take one frame worth of audio samples from the FIFO buffer,
            // encode it and write it to the output file.
            if (loadEncodeAndWrite(fifo, formatContext,
                                      audioOutputCodecContext)) {
                #ifdef TUP_DEBUG
                    qDebug() << "[TFFmpegMovieGenerator::mergeAudioStream()] - Tracing loadEncodeAndWrite() method...";
                #endif
                goto cleanup;
            }

        // If we are at the end of the input file and have encoded
        // all remaining samples, we can exit this loop and finish.
        if (finished) {
            int data_written;
            // Flush the encoder as it may have delayed frames.
            do {
                data_written = 0;
                if (encodeAudioFrame(nullptr, formatContext,
                                     audioOutputCodecContext, &data_written)) {
                    #ifdef TUP_DEBUG
                        qDebug() << "[TFFmpegMovieGenerator::mergeAudioStream()] - Audio process done!";
                    #endif
                    goto cleanup;
                }
            } while (data_written);
            break;
        }
    }

    ret = 0;

    cleanup:
        if (fifo)
            av_audio_fifo_free(fifo);
        swr_free(&resampleContext);

        /*
        if (audioOutputCodecContext)
            avcodec_free_context(&audioOutputCodecContext);
        if (formatContext) {
            avio_closep(&formatContext->pb);
            avformat_free_context(formatContext);
        }
        if (audioInputCodecContext)
            avcodec_free_context(&audioInputCodecContext);
        if (audioInputFormatContext)
            avformat_close_input(&audioInputFormatContext);
        */
    return ret;
}

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
    // audioOutputCodecID = outputFormat->audio_codec;
    audioOutputCodecID = AV_CODEC_ID_AAC;

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
        qDebug() << "[TFFmpegMovieGenerator::loadInputAudio()] - soundPath -> " << soundPath;
    #endif

    QByteArray bytes = soundPath.toLocal8Bit();
    const char *inputFile = bytes.data();
    int error;

    audioInputFormatContext = avformat_alloc_context();
    error = avformat_open_input(&audioInputFormatContext, inputFile, 0, 0);
    if (error < 0) {
        #ifdef TUP_DEBUG
            qCritical() << "[TFFmpegMovieGenerator::loadInputAudio()] - "
                           "Warning: Could not open input file -> "
                        << inputFile;
        #endif
        return false;
    }

    error = avformat_find_stream_info(audioInputFormatContext, 0);
    if (error < 0) {
        #ifdef TUP_DEBUG
            qCritical() << "[TFFmpegMovieGenerator::loadInputAudio()] - "
                           "Failed to retrieve input stream information";
        #endif
        return false;
    }

    av_dump_format(audioInputFormatContext, 0, inputFile, 0);

    // Process the stream from input audio
    audioInputStream = audioInputFormatContext->streams[0];
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

    audioInputCodec = avcodec_find_decoder(audioInputCodecID);
    if (!audioInputCodec) {
        errorMsg = "ffmpeg error: Could not find audio decoder.";
        #ifdef TUP_DEBUG
            qCritical() << "[TFFmpegMovieGenerator::loadInputAudio()] - " << errorMsg;
            qCritical() << "[TFFmpegMovieGenerator::loadInputAudio()] - Unavailable Codec ID -> "
                        << avcodec_get_name(audioInputCodecID);
        #endif
        return false;
    }

    audioInputCodecContext = avcodec_alloc_context3(audioInputCodec);
    if (!audioInputCodecContext) {
        errorMsg = "ffmpeg error: Could not initialize audio codec context.";
        #ifdef TUP_DEBUG
            qCritical() << "[TFFmpegMovieGenerator::loadInputAudio()] - " << errorMsg;
        #endif
        return false;
    }

    error = avcodec_parameters_to_context(audioInputCodecContext, audioInputCodecPar);
    if (error < 0) {
        errorMsg = "ffmpeg error: Can't copy codecpar values to input codec context.";
        #ifdef TUP_DEBUG
            qCritical() << "[TFFmpegMovieGenerator::loadInputAudio()] - " << errorMsg;
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
            qCritical() << "[TFFmpegMovieGenerator::openAudioOutputCodec()] - " << errorMsg;
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

    int error = avcodec_send_frame(videoCodecContext, videoFrame);
    if (error < 0) {
        errorMsg = "ffmpeg error: Error while sending a frame for encoding";
        #ifdef TUP_DEBUG
            qCritical() << "[TFFmpegMovieGenerator::createVideoFrame()] - " << errorMsg;
        #endif
        return false;
    }

    while (error >= 0) {
        error = avcodec_receive_packet(videoCodecContext, packet);
        if (error == AVERROR(EAGAIN) || error == AVERROR_EOF) {
            #ifdef TUP_DEBUG
                qDebug() << "[TFFmpegMovieGenerator::createVideoFrame()] - Tracing EAGAIN conditional...";
            #endif
            return (error == AVERROR(EAGAIN)) ? false : true;
        } else if (error < 0) {
            errorMsg = "ffmpeg error: Error during encoding";
            #ifdef TUP_DEBUG
                qCritical() << "[TFFmpegMovieGenerator::createVideoFrame()] - " << errorMsg;
            #endif
            return false;
        }

        error = writeVideoFrame(packet);

        if (error < 0) {
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

bool TFFmpegMovieGenerator::writeAudioStream()
{
    #ifdef TUP_DEBUG
        qDebug() << "[TFFmpegMovieGenerator::writeAudioStream()]";
    #endif

    AVPacket *pkt = av_packet_alloc();

    while (1) {
        int ret = av_read_frame(audioInputFormatContext, pkt);
        if (ret < 0)
            break;

        AVStream *in_stream = audioInputFormatContext->streams[pkt->stream_index];
        pkt->stream_index = 1;

        logPacket(Audio, in_stream->time_base, pkt, "in");
        AVRational outputTimebase = in_stream->time_base;

        // copy packet
        pkt->pts = av_rescale_q_rnd(pkt->pts, in_stream->time_base, outputTimebase,
                                    static_cast<AVRounding>(AV_ROUND_NEAR_INF|AV_ROUND_PASS_MINMAX));
        pkt->dts = av_rescale_q_rnd(pkt->dts, in_stream->time_base, outputTimebase,
                                    static_cast<AVRounding>(AV_ROUND_NEAR_INF|AV_ROUND_PASS_MINMAX));
        pkt->duration = av_rescale_q(pkt->duration, in_stream->time_base, outputTimebase);
        pkt->pos = -1;

        logPacket(Audio, in_stream->time_base, pkt, "out");
        float currentTime = av_q2d(outputTimebase) * pkt->pts;
        if (currentTime < mp4Duration) {
            ret = av_interleaved_write_frame(formatContext, pkt);
            if (ret < 0) {
                av_packet_unref(pkt);
                #ifdef TUP_DEBUG
                    qCritical() << "[TFFmpegMovieGenerator::writeAudioStreams()] - Error while muxing audio packet!";
                #endif
                return false;
            }
        } else {
            break;
        }
    }
    av_packet_unref(pkt);

    return true;
}

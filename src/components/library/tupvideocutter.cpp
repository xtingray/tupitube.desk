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

#include "tupvideocutter.h"

TupVideoCutter::TupVideoCutter()
{
}

TupVideoCutter::~TupVideoCutter()
{
}

int TupVideoCutter::processFile(const QString &videoFile, const QString &outputPath, int frames)
{
    emit msgSent(tr("Loading video file..."));

    outputFolder = outputPath;
    imagesTotal = frames;

    #ifdef TUP_DEBUG
        qDebug() << "[TupVideoCutter::processFile()] - Initializing all the containers, codecs and protocols...";
    #endif

    // AVFormatContext holds the header information from the format (Container)
    // Allocating memory for this component
    // http://ffmpeg.org/doxygen/trunk/structAVFormatContext.html
    AVFormatContext *formatContext = avformat_alloc_context();
    if (!formatContext) {
        #ifdef TUP_DEBUG
            qWarning() << "[TupVideoCutter::processFile()] - ERROR could not allocate memory for Format Context";
        #endif

        return -1;
    }

    #ifdef TUP_DEBUG
        qDebug() << "[TupVideoCutter::processFile()] - Opening the input file and loading format (container) header - File -> " << videoFile;
    #endif

    // Open the file and read its header. The codecs are not opened.
    // The function arguments are:
    // AVFormatContext (the component we allocated memory for),
    // url (filename),
    // AVInputFormat (if you pass nullptr it'll do the auto detect)
    // and AVDictionary (which are options to the demuxer)
    // http://ffmpeg.org/doxygen/trunk/group__lavf__decoding.html
    QByteArray bytes = videoFile.toLocal8Bit();
    const char *inputFile = bytes.data();

    if (avformat_open_input(&formatContext, inputFile, nullptr, nullptr) != 0) {
        #ifdef TUP_DEBUG
            qWarning() << "[TupVideoCutter::processFile()] - ERROR could not open the file";
        #endif

        return -1;
    }

    // Now we have access to some information about our file
    // since we read its header we can say what format (container) it's
    // and some other information related to the format itself.
    #ifdef TUP_DEBUG
        qDebug() << "[TupVideoCutter::processFile()] - Format: " << formatContext->iformat->name
                 << ", Duration: " << formatContext->duration
                 << ", Bitrate: " << formatContext->bit_rate;
        qDebug() << "[TupVideoCutter::processFile()] - Finding stream info from format...";
    #endif

    // Read Packets from the Format to get stream information
    // this function populates pFormatContext->streams
    // (of size equals to pFormatContext->nb_streams)
    // the arguments are:
    // the AVFormatContext
    // and options contains options for codec corresponding to i-th stream.
    // On return each dictionary will be filled with options that were not found.
    // https://ffmpeg.org/doxygen/trunk/group__lavf__decoding.html

    if (avformat_find_stream_info(formatContext, nullptr) < 0) {
        #ifdef TUP_DEBUG
            qWarning() << "[TupVideoCutter::processFile()] - ERROR could not get the stream info";
        #endif

        return -1;
    }

    // The component that knows how to enCOde and DECode the stream
    // it's the codec (audio or video)
    // http://ffmpeg.org/doxygen/trunk/structAVCodec.html
    AVCodec *inputCodec = nullptr;
    // this component describes the properties of a codec used by the stream i
    // https://ffmpeg.org/doxygen/trunk/structAVCodecParameters.html
    AVCodecParameters *inputCodecParameters = nullptr;
    int videoStreamIndex = -1;

    // Loop though all the streams and print its main information
    int streamsTotal = (int) formatContext->nb_streams;
    for (int i = 0; i < streamsTotal; i++) {
        AVCodecParameters *codecParameters = nullptr;
        codecParameters = formatContext->streams[i]->codecpar;
        #ifdef TUP_DEBUG
            qDebug() << "[TupVideoCutter::processFile()] - AVStream->time_base before open coded -> "
                     << formatContext->streams[i]->time_base.num << "/" << formatContext->streams[i]->time_base.den;
            qDebug() << "[TupVideoCutter::processFile()] - AVStream->r_frame_rate before open coded -> "
                     << formatContext->streams[i]->r_frame_rate.num << "/" << formatContext->streams[i]->r_frame_rate.den;
            qDebug() << "[TupVideoCutter::processFile()] - AVStream->start_time -> " << formatContext->streams[i]->start_time;
            qDebug() << "[TupVideoCutter::processFile()] - AVStream->duration -> " << formatContext->streams[i]->duration;
            qDebug() << "[TupVideoCutter::processFile()] - Finding the proper decoder (CODEC)";
            qDebug() << "---";
        #endif

        AVCodec *codec = nullptr;

        // Finds the registered decoder for a codec ID
        // https://ffmpeg.org/doxygen/trunk/group__lavc__decoding.html
        codec = avcodec_find_decoder(codecParameters->codec_id);

        if (codec == nullptr) {
            #ifdef TUP_DEBUG
                qWarning() << "[TupVideoCutter::processFile()] - ERROR unsupported codec!";
            #endif
            // In this example if the codec is not found we just skip it
            continue;
        }

        // When the stream is a video we store its index, codec parameters and codec
        if (codecParameters->codec_type == AVMEDIA_TYPE_VIDEO) {
            if (videoStreamIndex == -1) {
                videoStreamIndex = i;
                inputCodec = codec;
                inputCodecParameters = codecParameters;
            }

            #ifdef TUP_DEBUG
                qDebug() << "[TupVideoCutter::processFile()] - Video Codec: resolution -> "
                         << codecParameters->width
                         << " x " << codecParameters->height;
            #endif
        } else if (codecParameters->codec_type == AVMEDIA_TYPE_AUDIO) {
            #ifdef TUP_DEBUG
                qDebug() << "[TupVideoCutter::processFile()] -  Audio Codec -> " << codecParameters->channels
                         << " channels, sample rate -> " << codecParameters->sample_rate;
            #endif
        }

        // Print its name, id and bitrate
        #ifdef TUP_DEBUG
            qDebug() << "[TupVideoCutter::processFile()] - Codec -> " << codec->name
                     << ", ID " << codec->id << ", bit_rate " << codecParameters->bit_rate;
        #endif
    }

    if (videoStreamIndex == -1) {
        #ifdef TUP_DEBUG
            qWarning() << "[TupVideoCutter::processFile()] - File %s does not contain a video stream! -> " << videoFile;
        #endif
        return -1;
    }

    // https://ffmpeg.org/doxygen/trunk/structAVCodecContext.html
    AVCodecContext *inputCodecContext = avcodec_alloc_context3(inputCodec);
    if (!inputCodecContext) {
        #ifdef TUP_DEBUG
            qWarning() << "[TupVideoCutter::processFile()] - Failed to allocated memory for AVCodecContext";
        #endif
        return -1;
    }

    // Fill the codec context based on the values from the supplied codec parameters
    // https://ffmpeg.org/doxygen/trunk/group__lavc__core.html
    if (avcodec_parameters_to_context(inputCodecContext, inputCodecParameters) < 0) {
        #ifdef TUP_DEBUG
            qWarning() << "[TupVideoCutter::processFile()] - Failed to copy codec params to codec context";
        #endif
       return -1;
    }

    // Initialize the AVCodecContext to use the given AVCodec.
    // https://ffmpeg.org/doxygen/trunk/group__lavc__core.html
    if (avcodec_open2(inputCodecContext, inputCodec, nullptr) < 0) {
        #ifdef TUP_DEBUG
            qWarning() << "[TupVideoCutter::processFile()] - Failed to open codec through avcodec_open2";
        #endif
        return -1;
    }

    // https://ffmpeg.org/doxygen/trunk/structAVFrame.html
    AVFrame *inputFrame = av_frame_alloc();
    if (!inputFrame) {
        #ifdef TUP_DEBUG
            qWarning() << "[TupVideoCutter::processFile()] - Failed to allocate memory for AVFrame";
        #endif
        return -1;
    }

    // https://ffmpeg.org/doxygen/trunk/structAVPacket.html
    AVPacket *inputPacket = av_packet_alloc();
    if (!inputPacket) {
        #ifdef TUP_DEBUG
            qWarning() << "[TupVideoCutter::processFile()] - Failed to allocate memory for AVPacket";
        #endif
        return -1;
    }

    emit msgSent(tr("Extracting images..."));

    int ret = 0;
    int counter = 0;
    // Fill the Packet with data from the Stream
    // https://ffmpeg.org/doxygen/trunk/group__lavf__decoding.html
    while (av_read_frame(formatContext, inputPacket) >= 0) {
        // If it's the video stream
        if (inputPacket->stream_index == videoStreamIndex) {
            #ifdef TUP_DEBUG
                qDebug() << "---";
                qDebug() << "[TupVideoCutter::processFile()]    - AVPacket->pts -> " << inputPacket->pts;
            #endif
            ret = decodePacket(inputPacket, inputCodecContext, inputFrame);
            if (ret < 0)
                break;
            // Stop it, otherwise we'll be saving hundreds of frames
            if (counter > imagesTotal)
                break;
            counter++;
        }
        // https://ffmpeg.org/doxygen/trunk/group__lavc__packet.html
        av_packet_unref(inputPacket);
    }

    #ifdef TUP_DEBUG
        qDebug() << "---";
        qDebug() << "[TupVideoCutter::processFile()] - Releasing all the resources...";
    #endif

    avformat_close_input(&formatContext);
    av_packet_free(&inputPacket);
    av_frame_free(&inputFrame);
    avcodec_free_context(&inputCodecContext);

    return 0;
}

int TupVideoCutter::decodePacket(AVPacket *pPacket, AVCodecContext *codecContext, AVFrame *frame)
{
    // Supply raw packet data as input to a decoder
    // https://ffmpeg.org/doxygen/trunk/group__lavc__decoding.html
    int ret = avcodec_send_packet(codecContext, pPacket);

    if (ret < 0) {
        #ifdef TUP_DEBUG
            qWarning() << "[TupVideoCutter::decodePacket()] - Error while sending a packet to the decoder -> " << ret;
        #endif
        return ret;
    }

    while (ret >= 0) {
        // Return decoded output data (into a frame) from a decoder
        // https://ffmpeg.org/doxygen/trunk/group__lavc__decoding.html
        ret = avcodec_receive_frame(codecContext, frame);
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
            break;
        } else if (ret < 0) {
            #ifdef TUP_DEBUG
                qWarning() << "[TupVideoCutter::decodePacket()] - Error while receiving a frame from the decoder -> " << ret;
            #endif
            return ret;
        }

        if (ret >= 0) {
            #ifdef TUP_DEBUG
                qDebug() << "[TupVideoCutter::decodePacket()]   - Frame -> " << codecContext->frame_number
                         << " (type=" << av_get_picture_type_char(frame->pict_type) << ", size=" << frame->pkt_size
                         << " bytes, format=" << frame->format << ") pts " << frame->pts
                         << " key_frame " << frame->key_frame
                         << " [DTS " "]" << frame->coded_picture_number;
            #endif

            int photogram = codecContext->frame_number;
            QString frameFilename = outputFolder + "/frame" + QString::number(photogram) + ".png";

            // Check if the frame is a planar YUV 4:2:0, 12bpp
            // That is the format of the provided .mp4 file
            // RGB formats will definitely not give a gray image
            // Other YUV image may do so, but untested, so give a warning
            if (frame->format != AV_PIX_FMT_YUV420P) {
                #ifdef TUP_DEBUG
                    qWarning() << "[TupVideoCutter::decodePacket()] - "
                                "Warning: the generated file may not be a grayscale image, "
                                "but could e.g. be just the R component if the video format is RGB";
                #endif
            }

            // To create the PNG files, the AVFrame data must be translated from YUV420P format into RGB24
            struct SwsContext *swsContext = sws_getContext(
                frame->width, frame->height, AV_PIX_FMT_YUV420P,
                frame->width, frame->height, AV_PIX_FMT_RGB24,
                SWS_BILINEAR, nullptr, nullptr, nullptr);

            // Allocate a new AVFrame for the output RGB24 image
            AVFrame* rgbFrame = av_frame_alloc();

            // Set the properties of the output AVFrame
            rgbFrame->format = AV_PIX_FMT_RGB24;
            rgbFrame->width = frame->width;
            rgbFrame->height = frame->height;

            int ret = av_frame_get_buffer(rgbFrame, 0);
            if (ret < 0) {
                #ifdef TUP_DEBUG
                    qWarning() << "[TupVideoCutter::decodePacket()] - Error while preparing RGB frame -> " << ret;
                #endif
                return ret;
            }

            #ifdef TUP_DEBUG
                qDebug() << "[TupVideoCutter::decodePacket()]   - Transforming frame format from YUV420P into RGB24...";
            #endif

            ret = sws_scale(swsContext, frame->data, frame->linesize, 0, frame->height, rgbFrame->data, rgbFrame->linesize);
            if (ret < 0) {
                #ifdef TUP_DEBUG
                    qWarning() << "[TupVideoCutter::decodePacket()] - Error while translating the frame format from YUV420P into RGB24 -> " << ret;
                #endif
                return ret;
            }

            // Save a frame into a .PNG file
            ret = saveFrameToPng(rgbFrame, frameFilename);
            if (ret < 0) {
                #ifdef TUP_DEBUG
                    qWarning() << "[TupVideoCutter::decodePacket()] - Failed to write PNG file";
                #endif
                return -1;
            }

            emit imageExtracted(photogram);

            av_frame_free(&rgbFrame);
        }
    }

    return 0;
}

// Function to save an AVFrame to a PNG file
int TupVideoCutter::saveFrameToPng(AVFrame *frame, const QString &outputFilename)
{
    int ret = 0;

    #ifdef TUP_DEBUG
        qDebug() << "[TupVideoCutter::saveFrameToPng()] - Creating PNG file -> " << outputFilename;
    #endif

    QByteArray bytes = outputFilename.toLocal8Bit();
    const char *pngFile = bytes.data();
    // Open the PNG file for writing
    FILE *fp = fopen(pngFile, "wb");
    if (!fp) {
        #ifdef TUP_DEBUG
            qWarning() << "[TupVideoCutter::saveFrameToPng()] - Failed to open file -> " << outputFilename;
        #endif
        return -1;
    }

    // Create the PNG write struct and info struct
    png_structp png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
    if (!png_ptr) {
        #ifdef TUP_DEBUG
            qWarning() << "[TupVideoCutter::saveFrameToPng()] - Failed to create PNG write struct";
        #endif
        fclose(fp);

        return -1;
    }

    png_infop info_ptr = png_create_info_struct(png_ptr);
    if (!info_ptr) {
        #ifdef TUP_DEBUG
            qWarning() << "[TupVideoCutter::saveFrameToPng()] - Failed to create PNG info struct";
        #endif
        png_destroy_write_struct(&png_ptr, nullptr);
        fclose(fp);

        return -1;
    }

    // Set up error handling for libpng
    if (setjmp(png_jmpbuf(png_ptr))) {
        #ifdef TUP_DEBUG
            qWarning() << "[TupVideoCutter::saveFrameToPng()] - Error writing PNG file";
        #endif
        png_destroy_write_struct(&png_ptr, &info_ptr);
        fclose(fp);

        return -1;
    }

    // Set the PNG file as the output for libpng
    png_init_io(png_ptr, fp);

    // Set the PNG image attributes
    png_set_IHDR(png_ptr, info_ptr, frame->width, frame->height, 8, PNG_COLOR_TYPE_RGB,
                 PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);

    // Allocate memory for the row pointers and fill them with the AVFrame data
    png_bytep *row_pointers = (png_bytep *) malloc(sizeof(png_bytep) * frame->height);
    for (int y = 0; y < frame->height; y++) {
        row_pointers[y] = (png_bytep) (frame->data[0] + y * frame->linesize[0]);
    }

    // Write the PNG file
    png_set_rows(png_ptr, info_ptr, row_pointers);
    png_write_png(png_ptr, info_ptr, PNG_TRANSFORM_IDENTITY, nullptr);

    // Clean up
    free(row_pointers);
    png_destroy_write_struct(&png_ptr, &info_ptr);
    fclose(fp);

    return ret;
}

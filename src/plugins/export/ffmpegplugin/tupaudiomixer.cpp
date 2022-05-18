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

#include "tupaudiomixer.h"
#include <QFile>

// The number of output channels
#define OUTPUT_CHANNELS 2
// The audio sample output format
#define OUTPUT_SAMPLE_FORMAT AV_SAMPLE_FMT_S16

TupAudioMixer::TupAudioMixer(int frames, QList<SoundResource> audioList, const QString &path)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupAudioMixer::TupAudioMixer()] - output path -> " << path;
    #endif

    fps = frames;
    sounds = audioList;
    soundsTotal = audioList.size();
    outputPath = path;
}

TupAudioMixer::~TupAudioMixer()
{
}

// Open an input file and the required decoder.
int TupAudioMixer::openInputFile(const char *filename)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupAudioMixer::openInputFile()] - Processing input file -> " + QString(filename);
    #endif

    AVCodec *input_codec;
    AVStream *in_stream;
    AVCodecParameters *in_codecpar;
    enum AVCodecID audioCodecID;
    int error;

    // Open the input file to read from it.
    AVFormatContext *inputFormatContext = avformat_alloc_context();
    if ((error = avformat_open_input(&inputFormatContext, filename, nullptr, nullptr)) < 0) {
        errorMsg = "Fatal Error: Could not open input file -> " + QString(filename);
        #ifdef TUP_DEBUG
            qCritical() << "[TupAudioMixer::openInputFile()] - " + errorMsg;
            qCritical() << "ERROR CODE -> " << error;
        #endif
        inputFormatContext = nullptr;
        return error;
    }

    // Get information on the input file (number of streams etc.).
    if ((error = avformat_find_stream_info(inputFormatContext, nullptr)) < 0) {
        errorMsg = "Fatal Error: Could not open find stream -> " + QString(filename);
        #ifdef TUP_DEBUG
            qCritical() << "[TupAudioMixer::openInputFile()] - " + errorMsg;
            qCritical() << "ERROR CODE -> " << error;
        #endif
        avformat_close_input(&inputFormatContext);
        return error;
    }

    // Make sure that there is only one stream in the input file.
    if (inputFormatContext->nb_streams != 1) {
        errorMsg = "Fatal Error: Expected one audio input stream, but found several.";
        #ifdef TUP_DEBUG
            qCritical() << "[TupAudioMixer::openInputFile()] - " + errorMsg;
        #endif
        avformat_close_input(&inputFormatContext);
        return AVERROR_EXIT;
    }

    av_dump_format(inputFormatContext, 0, filename, 0);

    in_stream = inputFormatContext->streams[0];
    in_codecpar = in_stream->codecpar;
    if (in_codecpar->codec_type != AVMEDIA_TYPE_AUDIO) {
        errorMsg = "Fatal Error: File input has no stream audio -> " + QString(filename);
        #ifdef TUP_DEBUG
            qCritical() << "[TupAudioMixer::openInputFile()] - " + errorMsg;
        #endif
        return -1;
    }

    audioCodecID = in_codecpar->codec_id;

    // Find a decoder for the audio stream.
    if (!(input_codec = avcodec_find_decoder(audioCodecID))) {
        errorMsg = "Fatal Error: Could not find input codec -> " + QString(avcodec_get_name(audioCodecID));
        #ifdef TUP_DEBUG
            qCritical() << "[TupAudioMixer::openInputFile()] - " + errorMsg;
        #endif
        avformat_close_input(&inputFormatContext);
        return AVERROR_EXIT;
    }

    // Creating codec context for the input file
    AVCodecContext *inputCodecContext;
    inputCodecContext = avcodec_alloc_context3(input_codec);
    if (!inputCodecContext) {
        errorMsg = "Fatal Error: Could not alloc memory for input codec context.";
        #ifdef TUP_DEBUG
            qCritical() << "[TupAudioMixer::openInputFile()] - " + errorMsg;
        #endif
        return -1;
    }

    error = avcodec_parameters_to_context(inputCodecContext, inputFormatContext->streams[0]->codecpar);
    if (error < 0) {
        errorMsg = "Fatal Error: Can't copy codecpar values to input codec context.";
        #ifdef TUP_DEBUG
            qCritical() << "[TupAudioMixer::openInputFile()] - " + errorMsg;
            qCritical() << "ERROR CODE -> " << error;
        #endif
        return error;
    }

    // Open the decoder for the audio stream to use it later.
    if ((error = avcodec_open2(inputCodecContext, input_codec, nullptr)) < 0) {
        errorMsg = "Fatal Error: Could not open input codec.";
        #ifdef TUP_DEBUG
            qCritical() << "[TupAudioMixer::openInputFile()] - " + errorMsg;
            qCritical() << "ERROR CODE -> " << error;
        #endif
        avformat_close_input(&inputFormatContext);
        return error;
    }

    inputFormatContextList << inputFormatContext;
    inputCodecContextList << inputCodecContext;

    return 0;
}

int TupAudioMixer::initFilterGraph()
{
    QList<const AVFilter *> abufferList;
    QList<const AVFilter *> adelayList;
    QList<AVFilterContext *> adelayContextList;

    const AVFilter *mixFilter;
    AVFilterContext *mixContext;
    const AVFilter *abuffersink;

    char args[512];
    char sourceTag[10];
    int error;
    
    // Create a new filtergraph, which will contain all the filters.
    filterGraph = avfilter_graph_alloc();
    if (!filterGraph) {
        errorMsg = "Fatal Error: Unable to create filter graph.";
        #ifdef TUP_DEBUG
            qCritical() << "[TupAudioMixer::initFilterGraph()] - " + errorMsg;
        #endif
        return AVERROR(ENOMEM);
    }

    for (int i=0; i<soundsTotal; i++) {
        // abuffer
        // Create the abuffer filter;
        // it will be used for feeding the data into the graph.
        const AVFilter *abufferFilter = avfilter_get_by_name("abuffer");
        if (!abufferFilter) {
            errorMsg = "Fatal Error: Could not find the abuffer filter.";
            #ifdef TUP_DEBUG
                qCritical() << "[TupAudioMixer::initFilterGraph()] - " + errorMsg;
            #endif
            return AVERROR_FILTER_NOT_FOUND;
        }

        // buffer audio source: the decoded frames from the decoder will be inserted here.
        if (!inputCodecContextList[i]->channel_layout)
            inputCodecContextList[i]->channel_layout = av_get_default_channel_layout(inputCodecContextList[i]->channels);

        snprintf(args, sizeof(args),
                 "sample_rate=%d:sample_fmt=%s:channel_layout=0x%" PRIx64,
                 inputCodecContextList[i]->sample_rate, av_get_sample_fmt_name(inputCodecContextList[i]->sample_fmt),
                 inputCodecContextList[i]->channel_layout);

        snprintf(sourceTag, sizeof(sourceTag), "src%d", i);
        AVFilterContext *abufferContext;
        error = avfilter_graph_create_filter(&abufferContext, abufferFilter, sourceTag,
                                             args, nullptr, filterGraph);
        if (error < 0) {
            errorMsg = "Fatal Error: Cannot create audio buffer source.";
            #ifdef TUP_DEBUG
                qCritical() << "[TupAudioMixer::initFilterGraph()] - " + errorMsg;
                qCritical() << "ERROR CODE -> " << error;
            #endif
            return error;
        }

        abufferList << abufferFilter;
        abufferContextList << abufferContext;

        // adelay
        // Create the delay filter;
        const AVFilter *adelayFilter = avfilter_get_by_name("adelay");
        if (!adelayFilter) {
            errorMsg = "Fatal Error: Could not find the adelay filter.";
            #ifdef TUP_DEBUG
                qCritical() << "[TupAudioMixer::initFilterGraph()] - " + errorMsg;
            #endif
            return AVERROR_FILTER_NOT_FOUND;
        }

        float millisecs = ((float) sounds.at(i).frame/ (float) fps);
        millisecs *= 1000;
        int delayTime = millisecs;
        snprintf(args, sizeof(args), "delays=%d:all=1", delayTime);

        #ifdef TUP_DEBUG
            qDebug() << "[TFFmpegMovieGenerator::initFilterGraph()] - adelay filter args -> " << args;
        #endif

        AVFilterContext *adelayContext;
        snprintf(args, sizeof(args), "delays=%d:all=1", delayTime);
        error = avfilter_graph_create_filter(&adelayContext, adelayFilter, "adelay", args, nullptr, filterGraph);
        if (error < 0) {
            errorMsg = "Fatal Error: Cannot create audio adelay filter.";
            #ifdef TUP_DEBUG
                qCritical() << "[TupAudioMixer::initFilterGraph()] - " + errorMsg;
                qCritical() << "ERROR CODE -> " << error;
            #endif
            return error;
        }

        adelayList << adelayFilter;
        adelayContextList << adelayContext;
    }

    if (soundsTotal > 1) {
        // amix
        // Create mix filter.
        mixFilter = avfilter_get_by_name("amix");
        if (!mixFilter) {
            errorMsg = "Fatal Error: Could not find the mix filter.";
            #ifdef TUP_DEBUG
                qCritical() << "[TFFmpegMovieGenerator::initFilterGraph()] - " + errorMsg;
            #endif
            return AVERROR_FILTER_NOT_FOUND;
        }

        snprintf(args, sizeof(args), "inputs=2");

        error = avfilter_graph_create_filter(&mixContext, mixFilter, "amix", args, nullptr, filterGraph);
        if (error < 0) {
            errorMsg = "Fatal Error: Cannot create audio amix filter.";
            #ifdef TUP_DEBUG
                qCritical() << "[TFFmpegMovieGenerator::initFilterGraph()] - " + errorMsg;
                qCritical() << "ERROR CODE -> " << error;
            #endif
            return error;
        }
    }
    
    // Finally create the abuffersink filter;
    // it will be used to get the filtered data out of the graph.

    abuffersink = avfilter_get_by_name("abuffersink");
    if (!abuffersink) {
        errorMsg = "Fatal Error: Could not find the abuffersink filter.";
        #ifdef TUP_DEBUG
            qCritical() << "[TFFmpegMovieGenerator::initFilterGraph()] - " + errorMsg;
        #endif
        return AVERROR_FILTER_NOT_FOUND;
    }
    
    abuffersinkContext = avfilter_graph_alloc_filter(filterGraph, abuffersink, "sink");
    if (!abuffersinkContext) {
        errorMsg = "Fatal Error: Could not allocate the abuffersink instance.";
        #ifdef TUP_DEBUG
            qCritical() << "[TFFmpegMovieGenerator::initFilterGraph()] - " + errorMsg;
        #endif
        return AVERROR(ENOMEM);
    }
    
    // Same sample fmts as the output file.
    int formats[] = { AV_SAMPLE_FMT_S16, AV_SAMPLE_FMT_NONE };
    error = av_opt_set_int_list(abuffersinkContext, "sample_fmts",
                                formats, AV_SAMPLE_FMT_NONE, AV_OPT_SEARCH_CHILDREN);
    char ch_layout[64];
    av_get_channel_layout_string(ch_layout, sizeof(ch_layout), 0, OUTPUT_CHANNELS);
    av_opt_set(abuffersinkContext, "channel_layout", ch_layout, AV_OPT_SEARCH_CHILDREN);

    if (error < 0) {
        errorMsg = "Fatal Error: Could set options to the abuffersink instance.";
        #ifdef TUP_DEBUG
            qCritical() << "[TFFmpegMovieGenerator::initFilterGraph()] - " + errorMsg;
            qCritical() << "ERROR CODE -> " << error;
        #endif
        return error;
    }

    error = avfilter_init_str(abuffersinkContext, nullptr);
    if (error < 0) {
        errorMsg = "Fatal Error: Could not initialize the abuffersink instance.";
        #ifdef TUP_DEBUG
            qCritical() << "[TFFmpegMovieGenerator::initFilterGraph()] - " + errorMsg;
            qCritical() << "ERROR CODE -> " << error;
        #endif
        return error;
    }
    
    // Connect the filters
    if (soundsTotal > 1) {
        for(int i=0; i<soundsTotal; i++) {
            error = avfilter_link(abufferContextList[i], i, adelayContextList[i], i);
            if (error >= 0)
                error = avfilter_link(adelayContextList[i], i, mixContext, i);
            if (error >= 0)
                error = avfilter_link(mixContext, i, abuffersinkContext, i);
            if (error < 0) {
                errorMsg = "Fatal Error: Couldn't connect filters.";
                #ifdef TUP_DEBUG
                    qCritical() << "[TFFmpegMovieGenerator::initFilterGraph()] - " + errorMsg;
                    qCritical() << "ERROR CODE -> " << error;
                #endif
                return error;
            }
        }
    } else {
        error = avfilter_link(abufferContextList[0], 0, adelayContextList[0], 0);
        if (error >= 0)
            error = avfilter_link(adelayContextList[0], 0, abuffersinkContext, 0);
        if (error < 0) {
            errorMsg = "Fatal Error: Couldn't connect filters for the input file.";
            #ifdef TUP_DEBUG
                qCritical() << "[TFFmpegMovieGenerator::initFilterGraph()] - " + errorMsg;
                qCritical() << "ERROR CODE -> " << error;
            #endif
            return error;
        }
    }

    // Configure the graph.
    error = avfilter_graph_config(filterGraph, nullptr);
    if (error < 0) {
        errorMsg = "Fatal Error: Error while configuring graph.";
        #ifdef TUP_DEBUG
            qCritical() << "[TFFmpegMovieGenerator::initFilterGraph()] - " + errorMsg;
            qCritical() << "ERROR CODE -> " << error;
        #endif
        return error;
    }

    #ifdef TUP_DEBUG
        char* dump = avfilter_graph_dump(filterGraph, nullptr);
        av_log(nullptr, AV_LOG_ERROR, "Graph :\n%s\n", dump);
    #endif

    return 0;
}


// Open an output file and the required encoder.
// Also set some basic encoder parameters.
// Some of these parameters are based on the input file's parameters.
int TupAudioMixer::openOutputFile(const char *filename, AVCodecContext *inputCodecContext)
{
    AVIOContext *outputIOContext = nullptr;
    AVStream *stream = nullptr;
    AVCodec *outputCodec = nullptr;
    int error;

    // Open the output file to write to it.
    if ((error = avio_open(&outputIOContext, filename, AVIO_FLAG_WRITE)) < 0) {
        errorMsg = "Fatal Error: Could not open output file -> " + QString(filename);
        #ifdef TUP_DEBUG
            qCritical() << "[TFFmpegMovieGenerator::openOutputFile()] - " + errorMsg;
            qCritical() << "ERROR CODE -> " << error;
        #endif
        return error;
    }
    
    // Create a new format context for the output container format.
    if (!(outputFormatContext = avformat_alloc_context())) {
        errorMsg = "Fatal Error: Could not allocate output format context.";
        #ifdef TUP_DEBUG
            qCritical() << "[TFFmpegMovieGenerator::openOutputFile()] - ";
        #endif
        return AVERROR(ENOMEM);
    }
    
    // Associate the output file (pointer) with the container format context.
    outputFormatContext->pb = outputIOContext;
    
    // Guess the desired container format based on the file extension.
    if (!(outputFormatContext->oformat = av_guess_format(nullptr, filename, nullptr))) {
        errorMsg = "Fatal Error: Could not find output file format.";
        #ifdef TUP_DEBUG
            qCritical() << "[TFFmpegMovieGenerator::openOutputFile()] - " + errorMsg;
        #endif
        return -1;
    }

    av_dump_format(outputFormatContext, 0, filename, 1);

    AVOutputFormat *outputFormat = outputFormatContext->oformat;
    enum AVCodecID audioCodecID = outputFormat->audio_codec;

    // Find the encoder to be used by its name.
    if (!(outputCodec = avcodec_find_encoder(audioCodecID))) {
        errorMsg = "Fatal Error: Could not find the encoder required.";
        #ifdef TUP_DEBUG
            qCritical() << "[TFFmpegMovieGenerator::openOutputFile()] - " + errorMsg;
        #endif
        goto cleanup;
    }

    // Create a new audio stream in the output file container.
    if (!(stream = avformat_new_stream(outputFormatContext, outputCodec))) {
        errorMsg = "Fatal Error: Could not create new stream.";
        #ifdef TUP_DEBUG
            qCritical() << "[TFFmpegMovieGenerator::openOutputFile()] - " + errorMsg;
            qCritical() << "ERROR CODE -> " << error;
        #endif
        error = AVERROR(ENOMEM);
        goto cleanup;
    }

    outputCodecContext = avcodec_alloc_context3(outputCodec);
    if (!outputCodecContext) {
        errorMsg = "Fatal Error: Can't alloc memory for output codec context.";
        #ifdef TUP_DEBUG
            qCritical() << "[TFFmpegMovieGenerator::openOutputFile()] - " + errorMsg;
        #endif
        return -1;
    }

    stream->id = outputFormatContext->nb_streams - 1;

    // Set the basic encoder parameters.
    outputCodecContext->channels       = OUTPUT_CHANNELS;
    outputCodecContext->channel_layout = av_get_default_channel_layout(OUTPUT_CHANNELS);
    outputCodecContext->sample_rate    = inputCodecContext->sample_rate;
    outputCodecContext->sample_fmt     = OUTPUT_SAMPLE_FORMAT;
    outputCodecContext->bit_rate       = inputCodecContext->bit_rate;

    // Some container formats (like MP4) require global headers to be present
    // Mark the encoder so that it behaves accordingly.
    if (outputFormatContext->oformat->flags & AVFMT_GLOBALHEADER)
        outputCodecContext->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
    
    error = avcodec_parameters_from_context(stream->codecpar, outputCodecContext);
    if (error < 0) {
        errorMsg = "Fatal Error: Could not copy codecpar from codec context.";
        #ifdef TUP_DEBUG
            qCritical() << "[TFFmpegMovieGenerator::openOutputFile()] - " + errorMsg;
            qCritical() << "ERROR CODE -> " << error;
        #endif
        return error;
    } 

    // Open the encoder for the audio stream to use it later.
    if ((error = avcodec_open2(outputCodecContext, outputCodec, nullptr)) < 0) {
        errorMsg = "Fatal Error: Could not open output codec";
        #ifdef TUP_DEBUG
            qCritical() << "[TFFmpegMovieGenerator::openOutputFile()] - " + errorMsg;
            qCritical() << "ERROR CODE -> " << error;
        #endif
        goto cleanup;
    }
    
    return 0;
    
    cleanup:
        avio_close(outputFormatContext->pb);
        avformat_free_context(outputFormatContext);
        outputFormatContext = nullptr;

    return error < 0 ? error : AVERROR_EXIT;
}

// Initialize one audio frame for reading from the input file
int TupAudioMixer::initInputFrame(AVFrame **frame)
{
    if (!(*frame = av_frame_alloc())) {
        errorMsg = "Fatal Error: Could not allocate input frame.";
        #ifdef TUP_DEBUG
            qCritical() << "[TFFmpegMovieGenerator::initInputFrame()] - " + errorMsg;
        #endif
        return AVERROR(ENOMEM);
    }

    return 0;
}

// Decode one audio frame from the input file.
int TupAudioMixer::decodeAudioFrame(AVFrame *frame, AVFormatContext *inputFormatContext,
                                    AVCodecContext *inputCodecContext, int *dataPresent, int *finished)
{    
    // Deprecated version
    // Packet used for temporary storage. 
    AVPacket *inputPacket = av_packet_alloc();
    int error;
	
    /// Read one audio frame from the input file into a temporary packet. 
    if ((error = av_read_frame(inputFormatContext, inputPacket)) < 0) {
        // If we are the the end of the file, flush the decoder below. 
        if (error == AVERROR_EOF)
            *finished = 1;
        else {
            errorMsg = "Fatal Error: Could not read frame.";
            #ifdef TUP_DEBUG
                qCritical() << "[TFFmpegMovieGenerator::decodeAudioFrame()] - " + errorMsg;
                qCritical() << "ERROR CODE -> " << error;
            #endif
            return error;
        }
    }

    // Decode the audio frame stored in the temporary packet.
    // The input audio stream decoder is used to do this.
    // If we are at the end of the file, pass an empty packet to the decoder
    // to flush it.
    
    // SQA: the method avcodec_decode_audio4 is deprecated. It must be replaced.
    if ((error = avcodec_decode_audio4(inputCodecContext, frame,
                                       dataPresent, inputPacket)) < 0) {
        errorMsg = "Fatal Error: Could not decode frame.";
        #ifdef TUP_DEBUG
            qCritical() << "[TFFmpegMovieGenerator::decodeAudioFrame()] - " + errorMsg;
            qCritical() << "ERROR CODE -> " << error;
        #endif
        av_packet_unref(inputPacket);
        return error;
    }

    // If the decoder has not been flushed completely, we are not finished,
    // so that this function has to be called again.
    if (*finished && *dataPresent)
        *finished = 0;

    av_packet_unref(inputPacket);

    return 0;

    /*
    // Devel version
    // Packet used for temporary storage.
    AVPacket *inputPacket = av_packet_alloc();
    int error;

    // Read one audio frame from the input file into a temporary packet.
    if ((error = av_read_frame(inputFormatContext, inputPacket)) < 0) {
        // If we are the the end of the file, flush the decoder below.
        if (error == AVERROR_EOF)
            *finished = 1;
        else {
            av_log(NULL, AV_LOG_ERROR, "Could not read frame (error '%s')\n",
                   getErrorText(error));
            return error;
        }
    }

    // submit the packet to the decoder
    error = avcodec_send_packet(inputCodecContext, inputPacket);
    if (error < 0) {
        av_log(NULL, AV_LOG_ERROR, "Error while sending packet to decode (error '%s')\n",
               getErrorText(error));
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
    */
}

// Encode one frame worth of audio to the output file.
int TupAudioMixer::encodeAudioFrame(AVFrame *frame, int *dataPresent)
{
    // Packet used for temporary storage.
    AVPacket *outputPacket = av_packet_alloc();
    int error;
    *dataPresent = 0;
  
    // send the frame for encoding
    error = avcodec_send_frame(outputCodecContext, frame);
    if (error == AVERROR_EOF) {
        error = 0;
        goto cleanup;
    } else if (error < 0) {
        errorMsg = "Fatal Error: Could not send frame for encoding.";
        #ifdef TUP_DEBUG
            qCritical() << "[TFFmpegMovieGenerator::decodeAudioFrame()] - " + errorMsg;
            qCritical() << "ERROR CODE -> " << error;
        #endif
        goto cleanup;
    }

    // read all the available output packets (in general there may be any number of them 
    while (1) {
        error = avcodec_receive_packet(outputCodecContext, outputPacket);
        if (error == AVERROR(EAGAIN) || error == AVERROR_EOF) {
            return error;
        } else if (error < 0) {
            errorMsg = "Fatal Error: Unexpected error.";
            #ifdef TUP_DEBUG
                qCritical() << "[TFFmpegMovieGenerator::decodeAudioFrame()] - " + errorMsg;
                qCritical() << "ERROR CODE -> " << error;
            #endif
            return error;
        }

        if (outputPacket) {
            if ((error = av_write_frame(outputFormatContext, outputPacket)) < 0) {
                errorMsg = "Fatal Error: Could not write frame.";
                #ifdef TUP_DEBUG
                    qCritical() << "[TFFmpegMovieGenerator::decodeAudioFrame()] - " + errorMsg;
                    qCritical() << "ERROR CODE -> " << error;
                #endif
                av_packet_unref(outputPacket);
                return error;
            }
    
            av_packet_unref(outputPacket);
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

bool TupAudioMixer::processAudioFiles()
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupAudioMixer::processAudioFiles()]";
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

    int percent = 0;
    int chunk = 100/(soundsTotal * 2);
    while (soundFinished < soundsTotal) {
        int dataPresentInGraph = 0;
        for (int i=0; i<soundsTotal; i++) {
            if (inputFinished[i] || inputToRead[i] == 0)
                continue;

            inputToRead[i] = 0;

            AVFrame *frame = av_frame_alloc();
            if (!frame)
                goto end;

            // Decode one frame worth of audio samples.
            if ((error = decodeAudioFrame(frame, inputFormatContextList[i], inputCodecContextList[i],
                                          &dataPresent, &finished))) {
                goto end;
            }

            // If we are at the end of the file and there are no more samples
            // in the decoder which are delayed, we are actually finished.
            // This must not be treated as an error.
            if (finished && !dataPresent) {
                inputFinished[i] = 1;
                soundFinished++;
                #ifdef TUP_DEBUG
                    qDebug() << "[TupAudioMixer::processAudioFile()] - Input #"
                                + QString::number(i) + " finished. Write NULL frame.";
                #endif

                error = av_buffersrc_write_frame(abufferContextList[i], nullptr);
                if (error < 0) {
                    errorMsg = "Fatal Error: Can't write EOF NULL frame for input " + QString::number(i) + ".";
                    #ifdef TUP_DEBUG
                        qCritical() << "[TupAudioMixer::processAudioFile()] - " + errorMsg;
                    #endif
                    goto end;
                }
            } else if (dataPresent) { // If there is decoded data, convert and store it
                // push the audio data from decoded frame into the filtergraph
                error = av_buffersrc_write_frame(abufferContextList[i], frame);
                if (error < 0) {
                    errorMsg = "Fatal Error: Critical issue while feeding the audio filtergraph.";
                    #ifdef TUP_DEBUG
                        qCritical() << "[TupAudioMixer::processAudioFile()] - " + errorMsg;
                    #endif
                    goto end;
                }

                #ifdef TUP_DEBUG
                    av_log(nullptr, AV_LOG_INFO, "- add %d samples on input %d (%d Hz, time=%f, ttime=%f)\n",
                           frame->nb_samples, i, inputCodecContextList[i]->sample_rate,
                           (double)frame->nb_samples / inputCodecContextList[i]->sample_rate,
                           (double)(totalSamples[i] += frame->nb_samples) / inputCodecContextList[i]->sample_rate);
                #endif
            }

            av_frame_free(&frame);
            dataPresentInGraph = dataPresent | dataPresentInGraph;

            emit progressChanged(percent);
            percent += chunk;
        }

        if (dataPresentInGraph) {
            AVFrame *filterFrame = av_frame_alloc();
            // pull filtered audio from the filtergraph
            while (1) {
                error = av_buffersink_get_frame(abuffersinkContext, filterFrame);
                if (error == AVERROR(EAGAIN) || error == AVERROR_EOF) {
                    for (int i = 0 ; i < soundsTotal ; i++) {
                        if (av_buffersrc_get_nb_failed_requests(abufferContextList[i]) > 0) {
                            inputToRead[i] = 1;
                            #ifdef TUP_DEBUG
                                qDebug() << "[TupAudioMixer::processAudioFile()] - Need to read input "
                                            + QString::number(i) + ".";
                            #endif
                        }
                    }
                    break;
                }

                if (error < 0) {
                    errorMsg = "Fatal Error: Critical issue while getting filterFrame from sink.";
                    #ifdef TUP_DEBUG
                        qCritical() << "[TupAudioMixer::processAudioFile()] - " + errorMsg;
                    #endif
                    goto end;
                }

                #ifdef TUP_DEBUG
                    av_log(nullptr, AV_LOG_INFO, "- remove %d samples from sink (%d Hz, time=%f, ttime=%f)\n",
                           filterFrame->nb_samples, outputCodecContext->sample_rate,
                           (double)filterFrame->nb_samples / outputCodecContext->sample_rate,
                           (double)(totalOutSamples += filterFrame->nb_samples) / outputCodecContext->sample_rate);
                #endif

                error = encodeAudioFrame(filterFrame, &dataPresent);
                if (error < 0) {
                    errorMsg = "Fatal Error: Tracing error at encodeAudioFrame().";
                    #ifdef TUP_DEBUG
                        qCritical() << "[TupAudioMixer::processAudioFiles()] - " + errorMsg;
                        qCritical() << "ERROR CODE -> " << error;
                    #endif
                    goto end;
                }
                av_frame_unref(filterFrame);

                emit progressChanged(percent);
                percent += chunk;
            }

            av_frame_free(&filterFrame);
        } else {
            #ifdef TUP_DEBUG
                qDebug() << "[TupAudioMixer::processAudioFile()] - No data in graph!";
            #endif

            for (int i=0; i<soundsTotal; i++) {
                inputToRead[i] = 1;
            }
        }
    }

    return true;

    end:
        if (error < 0 && error != AVERROR_EOF) {
            errorMsg = "Fatal Error: Workflow has been interrupted!";
            #ifdef TUP_DEBUG
                qCritical() << "[TupAudioMixer::processAudioFiles()] - " + errorMsg;
                qCritical() << "ERROR CODE -> " << error;
            #endif
            return false;
        }

    return true;
}

// Write the header of the output file container
int TupAudioMixer::writeOutputFileHeader(AVFormatContext *outputFormatContext)
{
    int error;
    if ((error = avformat_write_header(outputFormatContext, nullptr)) < 0) {
        errorMsg = "Fatal Error: Could not write output file header.";
        #ifdef TUP_DEBUG
            qCritical() << "[TupAudioMixer::writeOutputFileHeader()] - " + errorMsg;
            qCritical() << "ERROR CODE -> " << error;
        #endif
        return error;
    }

    return 0;
}

// Write the trailer of the output file container.
int TupAudioMixer::writeOutputFileTrailer(AVFormatContext *outputFormatContext)
{
    int error;
    if ((error = av_write_trailer(outputFormatContext)) < 0) {
        errorMsg = "Fatal Error: Could not write output file trailer.";
        #ifdef TUP_DEBUG
            qCritical() << "[TupAudioMixer::writeOutputFileTrailer()] - " + errorMsg;
            qCritical() << "ERROR CODE -> " << error;
        #endif
        return error;
    }

    return 0;
}

bool TupAudioMixer::mergeAudios()
{    
    int error;

    for (int i=0; i < soundsTotal; i++) {
        char *path = sounds[i].path.toLocal8Bit().data();
        if (openInputFile(path) < 0) {
            errorMsg = "Fatal Error: Error while opening file -> " + QString(path);
            #ifdef TUP_DEBUG
                qCritical() << "[TupAudioMixer::mergeAudios()] - " + errorMsg;
            #endif
            return false;
        }
    }

    // Set up the filtergraph.
    error = initFilterGraph();
    if (error < 0) {
        errorMsg = "Fatal Error: Can't initialize filters";
        #ifdef TUP_DEBUG
            qCritical() << "[TupAudioMixer::mergeAudios()] - " + errorMsg;
            qCritical() << "ERROR CODE -> " << error;
        #endif
        return false;
    }

    QFile file(outputPath);
    if (file.exists()) {
        if (!file.remove()) {
            errorMsg = "Fatal Error: Can't remove file -> " + outputPath;
            #ifdef TUP_DEBUG
                qCritical() << "[TupAudioMixer::mergeAudios()] - " + errorMsg;
            #endif
            return false;
        }
    }
    
    char *path = outputPath.toLocal8Bit().data();
    error = openOutputFile(path, inputCodecContextList[0]);
    if (error < 0) {
        errorMsg = "Fatal Error: Can't open output file -> " + outputPath;
        #ifdef TUP_DEBUG
            qCritical() << "[TupAudioMixer::mergeAudios()] - " + errorMsg;
            qCritical() << "ERROR CODE -> " << error;
        #endif
        return false;
    }

    if (writeOutputFileHeader(outputFormatContext) < 0) {
        errorMsg = "Fatal Error: Error while writing header outputfile.";
        #ifdef TUP_DEBUG
            qCritical() << "[TupAudioMixer::mergeAudios()] - " + errorMsg;
        #endif
        return false;
    }

    processAudioFiles();
    
    if (writeOutputFileTrailer(outputFormatContext) < 0) {
        errorMsg = "Fatal Error: Error while writing trailer outputfile.";
        #ifdef TUP_DEBUG
            qCritical() << "[TupAudioMixer::mergeAudios()] - " + errorMsg;
        #endif
        return false;
    }

    return true;
}

QString TupAudioMixer::getErrorMsg() const
{
    return errorMsg;
}

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

#ifdef __cplusplus
extern "C" {
    #include "libavformat/avformat.h"
    #include "libavcodec/avcodec.h"
    #include "libswscale/swscale.h"
}
#endif

int main()
{
    AVOutputFormat *format = av_guess_format("ffh264", NULL, NULL);

    AVFormatContext *formatContext = avformat_alloc_context();
    formatContext->oformat = format;

    AVCodecContext *codecContext;
    AVStream *stream;

    stream = avformat_new_stream(formatContext, 0);

    AVCodec *codec = avcodec_find_encoder(stream->codecpar->codec_id);
    if (!codec) {
        av_log(NULL, AV_LOG_ERROR, "Failed to find decoder for stream\n");
        return AVERROR_DECODER_NOT_FOUND;
    }

    codecContext = avcodec_alloc_context3(codec);
    codecContext->time_base.den = 24;
    codecContext->time_base.num = 1;
    codecContext->gop_size = 12;
    codecContext->pix_fmt = AV_PIX_FMT_YUV420P;

    av_free(formatContext);
    av_free(codecContext);
    av_free(stream);

    return 0;
}


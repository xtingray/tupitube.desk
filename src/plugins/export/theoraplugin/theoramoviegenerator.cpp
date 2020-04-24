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

#include "theoramoviegenerator.h"

TheoraMovieGenerator::TheoraMovieGenerator(const QSize &size, int fpsVar, double duration, int framesVar) : TMovieGenerator(size.width(), size.height())
{
    width = size.width();
    height = size.height();
    fps = fpsVar;
    streamDuration = duration;
    frames = framesVar;
    movieFile = QDir::tempPath() + "/tupi_video_" + TAlgorithm::randomString(12);
    movieFile += ".ogv";
    framesCounter = 0;

    exception = beginVideoFile();
}

TheoraMovieGenerator::~TheoraMovieGenerator()
{
    if (QFile::exists(movieFile))
        QFile::remove(movieFile);
}

bool TheoraMovieGenerator::beginVideoFile()
{ 
    QByteArray string = movieFile.toLocal8Bit();
    const char *file = string.data();

    ogg_fp = fopen(file, "wb");
    if (!ogg_fp) {
        #ifdef TUP_DEBUG
            QString msg = "TheoraMovieGenerator::beginVideoFile() - Error: couldn't open video file";
            #ifdef Q_OS_WIN
                qDebug() << msg;
            #else
                tError() << msg;
            #endif
        #endif

        return false;
    }

    srand(time(NULL));
    if (ogg_stream_init(&ogg_os, rand())) {
        #ifdef TUP_DEBUG
            QString msg = "TheoraMovieGenerator::beginVideoFile() - Error: couldn't create ogg stream state";
            #ifdef Q_OS_WIN
                qDebug() << msg;
            #else
                tError() << msg;
            #endif
        #endif
        return false;
    }

    th_info_init(&ti);
    ti.frame_width = ((width + 15) >> 4) << 4;
    ti.frame_height = ((height + 15) >> 4) << 4;
    ti.pic_width = width;
    ti.pic_height = height;
    ti.pic_x = 0;
    ti.pic_y = 0;
    ti.fps_numerator = fps;
    ti.fps_denominator = 1;
    ti.aspect_numerator = 0;
    ti.aspect_denominator = 0;
    ti.colorspace = TH_CS_UNSPECIFIED;
    ti.pixel_fmt = TH_PF_420;
    ti.target_bitrate = 6000000;
    ti.quality = 0;
    ti.keyframe_granule_shift = 6;

    td = th_encode_alloc(&ti);
    th_info_clear(&ti);

    ogg_uint32_t keyframe_frequency = 64;
    int buf_delay = -1;

    int ret = th_encode_ctl(td, TH_ENCCTL_SET_KEYFRAME_FREQUENCY_FORCE, &keyframe_frequency, sizeof(keyframe_frequency - 1));

    if (ret < 0) {
        #ifdef TUP_DEBUG
            QString msg = "TheoraMovieGenerator::beginVideoFile() - Error: could not set keyframe interval to " + QString::number((int)keyframe_frequency);
            #ifdef Q_OS_WIN
                qDebug() << msg;
            #else
                tError() << msg;
            #endif
        #endif
    }

    int arg = TH_RATECTL_CAP_UNDERFLOW;
    ret = th_encode_ctl(td, TH_ENCCTL_SET_RATE_FLAGS, &arg, sizeof(arg));
    if (ret < 0) {
        #ifdef TUP_DEBUG
            QString msg = "TheoraMovieGenerator::beginVideoFile() - Error: could not set encoder flags for soft encoding";
            #ifdef Q_OS_WIN
                qDebug() << msg;
            #else
                tError() << msg;
            #endif
        #endif
    }

    if (buf_delay < 0) {
        if (((int) keyframe_frequency*7>>1) > 5*fps/1)
            arg = keyframe_frequency*7>>1;
        else
            arg = 5*fps;

        ret = th_encode_ctl(td, TH_ENCCTL_SET_RATE_BUFFER, &arg, sizeof(arg));
        if (ret < 0) {
            #ifdef TUP_DEBUG
                QString msg = "TheoraMovieGenerator::beginVideoFile() - Error: Could not set rate control buffer for soft encoding";
                #ifdef Q_OS_WIN
                    qDebug() << msg;
                #else
                    tError() << msg;
                #endif
            #endif
        }
    }

    if (buf_delay >= 0) {
        ret = th_encode_ctl(td, TH_ENCCTL_SET_RATE_BUFFER, &buf_delay, sizeof(buf_delay));
        if (ret < 0) {
            #ifdef TUP_DEBUG
                QString msg = "TheoraMovieGenerator::beginVideoFile() - Error: could not set desired buffer delay";
                #ifdef Q_OS_WIN
                    qDebug() << msg;
                #else
                    tError() << msg;
                #endif
            #endif
        }
    }

    th_comment_init(&tc);

    if (th_encode_flushheader(td, &tc, &op) <= 0){
        #ifdef TUP_DEBUG
            QString msg = "TheoraMovieGenerator::beginVideoFile() - Error: internal Theora library error";
            #ifdef Q_OS_WIN
                qDebug() << msg;
            #else
                tError() << msg;
            #endif
        #endif
        return false;
    }

    th_comment_clear(&tc);

    ogg_stream_packetin(&ogg_os, &op);
    if (ogg_stream_pageout(&ogg_os, &og) != 1) {
        #ifdef TUP_DEBUG
            QString msg = "TheoraMovieGenerator::beginVideoFile() - Error: internal Ogg library error";
            #ifdef Q_OS_WIN
                qDebug() << msg;
            #else
                tError() << msg;
            #endif
        #endif
        return false;
    }
    fwrite(og.header, 1, og.header_len, ogg_fp);
    fwrite(og.body, 1, og.body_len, ogg_fp);

    for(;;) {
        ret = th_encode_flushheader(td, &tc, &op);
        if (ret < 0) {
            #ifdef TUP_DEBUG
                QString msg = "TheoraMovieGenerator::beginVideoFile() - Error: internal Theora library error";
                #ifdef Q_OS_WIN
                    qDebug() << msg;
                #else
                    tError() << msg;
                #endif
            #endif
            return false;
        } else if (!ret)
                   break;

        ogg_stream_packetin(&ogg_os, &op);
    }

    for(;;) {
        int result = ogg_stream_flush(&ogg_os, &og);
        if (result < 0) {
            #ifdef TUP_DEBUG
                QString msg = "TheoraMovieGenerator::beginVideoFile() - Error: internal Ogg library error";
                #ifdef Q_OS_WIN
                    qDebug() << msg;
                #else
                    tError() << msg;
                #endif
            #endif
            return false;
        }
        if (result == 0)
            break;

        fwrite(og.header, 1, og.header_len, ogg_fp);
        fwrite(og.body, 1, og.body_len, ogg_fp);
    }

    return true;
}

bool TheoraMovieGenerator::validMovieHeader()
{ 
    return exception;
}

QString TheoraMovieGenerator::getErrorMsg() const 
{
    return errorMsg;
}

void TheoraMovieGenerator::writeTheoraFrame(unsigned long w, unsigned long h, unsigned char *yuv, int last)
{
    int chroma_format = TH_PF_420;
    th_ycbcr_buffer ycbcr;
    ogg_packet op;
    ogg_page og;

    unsigned long yuv_w;
    unsigned long yuv_h;

    unsigned char *yuv_y;
    unsigned char *yuv_u;
    unsigned char *yuv_v;

    unsigned int x;
    unsigned int y;

    yuv_w = (w + 15) & ~15;
    yuv_h = (h + 15) & ~15;

    ycbcr[0].width = yuv_w;
    ycbcr[0].height = yuv_h;
    ycbcr[0].stride = yuv_w;
    ycbcr[1].width = (chroma_format == TH_PF_444) ? yuv_w : (yuv_w >> 1);
    ycbcr[1].stride = ycbcr[1].width;
    ycbcr[1].height = (chroma_format == TH_PF_420) ? (yuv_h >> 1) : yuv_h;
    ycbcr[2].width = ycbcr[1].width;
    ycbcr[2].stride = ycbcr[1].stride;
    ycbcr[2].height = ycbcr[1].height;

    ycbcr[0].data = yuv_y = (unsigned char*) malloc(ycbcr[0].stride * ycbcr[0].height);
    ycbcr[1].data = yuv_u = (unsigned char*) malloc(ycbcr[1].stride * ycbcr[1].height);
    ycbcr[2].data = yuv_v = (unsigned char*) malloc(ycbcr[2].stride * ycbcr[2].height);

    for (y = 0; y < h; y++) {
         for (x = 0; x < w; x++) {
              yuv_y[x + y * yuv_w] = yuv[3 * (x + y * w) + 0];
         }
    }

    if (chroma_format == TH_PF_420) {
        for (y = 0; y < h; y += 2) {
             for (x = 0; x < w; x += 2) {
                  yuv_u[(x >> 1) + (y >> 1) * (yuv_w >> 1)] = yuv[3 * (x + y * w) + 1];
                  yuv_v[(x >> 1) + (y >> 1) * (yuv_w >> 1)] = yuv[3 * (x + y * w) + 2];
             }
        }
    } else if (chroma_format == TH_PF_444) {
               for (y = 0; y < h; y++) {
                    for (x = 0; x < w; x++) {
                         yuv_u[x + y * ycbcr[1].stride] = yuv[3 * (x + y * w) + 1];
                         yuv_v[x + y * ycbcr[2].stride] = yuv[3 * (x + y * w) + 2];
                    }
               }
    } else {  // TH_PF_422
        for (y = 0; y < h; y += 1) {
             for (x = 0; x < w; x += 2) {
                  yuv_u[(x >> 1) + y * ycbcr[1].stride] = yuv[3 * (x + y * w) + 1];
                  yuv_v[(x >> 1) + y * ycbcr[2].stride] = yuv[3 * (x + y * w) + 2];
             }
        }
    }

    if (th_encode_ycbcr_in(td, ycbcr)) {
        #ifdef TUP_DEBUG
            QString msg = "TheoraMovieGenerator::theoraWriteFrame() - Error: could not encode frame";
            #ifdef Q_OS_WIN
                qDebug() << msg;
            #else
                tError() << msg;
            #endif
        #endif
        return;
    }

    if (!th_encode_packetout(td, last, &op)) {
        #ifdef TUP_DEBUG
            QString msg = "TheoraMovieGenerator::theoraWriteFrame() - Error: could not read packets";
            #ifdef Q_OS_WIN
                qDebug() << msg;
            #else
                tError() << msg;
            #endif
        #endif
        return;
    }

    ogg_stream_packetin(&ogg_os, &op);
    while (ogg_stream_pageout(&ogg_os, &og)) {
           fwrite(og.header, og.header_len, 1, ogg_fp);
           fwrite(og.body, og.body_len, 1, ogg_fp);
    }

    free(yuv_y);
    free(yuv_u);
    free(yuv_v);
}

unsigned char TheoraMovieGenerator::clamp(double d)
{
    if (d < 0)
        return 0;

    if (d > 255)
        return 255;

    return d;
}

void TheoraMovieGenerator::handle(const QImage &image)
{
    framesCounter++;
    QByteArray imageBytes((char *) image.bits(), image.sizeInBytes());

    unsigned char *yuv = (unsigned char *) malloc(width * height * 3);
    unsigned int x;
    unsigned int y;

    for (y = 0; y < height; y++) {
         for (x = 0; x < width; x++) {
              uint8_t r;
              uint8_t g;
              uint8_t b;

              b = (uint8_t) imageBytes.at((y*width*4) + (4 * x) + 0);
              g = (uint8_t) imageBytes.at((y*width*4) + (4 * x) + 1);
              r = (uint8_t) imageBytes.at((y*width*4) + (4 * x) + 2);

              yuv[3 * (x + width * y)] = clamp(0.299 * r + 0.587 * g + 0.114 * b);
              yuv[3 * (x + width * y) + 1] = clamp((0.436 * 255 - 0.14713 * r - 0.28886 * g + 0.436 * b) / 0.872);
              yuv[3 * (x + width * y) + 2] = clamp((0.615 * 255 + 0.615 * r - 0.51499 * g - 0.10001 * b) / 1.230);
         }
    }

    #ifdef TUP_DEBUG
        QString msg = "TheoraMovieGenerator::handle() -  Writing theora frame #" + QString::number(framesCounter);
        #ifdef Q_OS_WIN
            tWarning() << msg;
        #else
            qWarning() << msg;
        #endif
    #endif

    int last = 0;
    if (framesCounter == frames)
        last = 1;

    writeTheoraFrame(width, height, yuv, last);
    free(yuv);
}

void TheoraMovieGenerator::endVideoFile()
{
    th_encode_free(td);

    if (ogg_stream_flush(&ogg_os, &og)) {
        fwrite(og.header, og.header_len, 1, ogg_fp);
        fwrite(og.body, og.body_len, 0, ogg_fp);
    }

    if (ogg_fp) {
        fflush(ogg_fp);
        if (ogg_fp != stdout)
            fclose(ogg_fp);
    }

    ogg_stream_clear(&ogg_os);
}

void TheoraMovieGenerator::copyMovieFile(const QString &fileName)
{
    if (QFile::exists(fileName)) 
        QFile::remove(fileName);

    if (QFile::copy(movieFile, fileName)) {
        if (QFile::exists(movieFile)) {
            if (QFile::remove(movieFile)) {
                #ifdef TUP_DEBUG
                    QString msg = QString("") + "TheoraMovieGenerator::createMovieFile() - Removing temp video file -> " + movieFile;
                    #ifdef Q_OS_WIN
                        qDebug() << msg;
                    #else
                        tWarning() << msg;
                    #endif
                #endif
            }
        }
    }
}

void TheoraMovieGenerator::saveMovie(const QString &filename)
{
    endVideoFile();
    copyMovieFile(filename);
}

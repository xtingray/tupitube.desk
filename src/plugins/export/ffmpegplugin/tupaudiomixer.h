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

#ifndef TUPAUDIOMIXER_H
#define TUPAUDIOMIXER_H

#include "tglobal.h"

#ifdef __cplusplus
extern "C" {
#include <libavformat/avformat.h>
#include "libavfilter/avfilter.h"
#include "libavfilter/buffersink.h"
#include "libavfilter/buffersrc.h"
#include "libavutil/opt.h"
}
#endif

class TUPITUBE_PLUGIN TupAudioMixer: public QObject
{
    Q_OBJECT

    public:
        TupAudioMixer(int fps, QList<SoundResource> audioList, const QString &path, QList<double> durations);
        ~TupAudioMixer();

        bool mergeAudios();
        QString getErrorMsg() const;

    private:
        int initFilterGraph();
        int openInputFile(const char *filename);
        int openOutputFile(const char *filename, AVCodecContext *inputCodecContext);
        int initInputFrame(AVFrame **frame);
        int decodeAudioFrame(AVFrame *frame, AVFormatContext *inputFormatContext,
                             AVCodecContext *inputCodecContext, int *dataPresent, int *finished);
        int encodeAudioFrame(AVFrame *frame, int *dataPresent);
        bool processAudioFiles();
        int writeOutputFileHeader(AVFormatContext *outputFormatContext);
        int writeOutputFileTrailer(AVFormatContext *outputFormatContext);

    signals:
        void messageChanged(const QString &msg);
        void progressChanged(int percent);

    private:
        int fps;
        QList<SoundResource> sounds;
        int soundsTotal;
        QString errorMsg;
        QString outputPath;

        AVFormatContext *outputFormatContext;
        AVCodecContext *outputCodecContext;

        QList<AVFormatContext *> inputFormatContextList;
        QList<AVCodecContext *> inputCodecContextList;

        QList<AVFilterContext *> abufferContextList;
        QList<AVFilterContext *> filterContextList;
        AVFilterGraph *filterGraph;
        AVFilterContext *abuffersinkContext;

        QList<double> scenesDuration;
};

#endif

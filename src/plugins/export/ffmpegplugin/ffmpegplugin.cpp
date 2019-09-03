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

#include "ffmpegplugin.h"

FFmpegPlugin::FFmpegPlugin()
{
}

FFmpegPlugin::~FFmpegPlugin()
{
}

QString FFmpegPlugin::key() const
{
    return tr("Video Formats");
}

TupExportInterface::Formats FFmpegPlugin::availableFormats()
{
    // SQA: MPEG codec was removed because it crashes. Check the issue.
    // TupExportInterface::MPEG 

    // SQA: Obsolete formats
    // | TupExportInterface::SWF | TupExportInterface::ASF

    return TupExportInterface::WEBM | TupExportInterface::MP4 | TupExportInterface::AVI |
           TupExportInterface::MOV | TupExportInterface::GIF;
}

TMovieGeneratorInterface::Format FFmpegPlugin::videoFormat(TupExportInterface::Format format)
{
    switch (format) {
        case TupExportInterface::MP4:
            {
                return TFFmpegMovieGenerator::MP4;
            }
        case TupExportInterface::GIF:
            {
                return TFFmpegMovieGenerator::GIF;
            }
        case TupExportInterface::AVI:
            {
                return TFFmpegMovieGenerator::AVI;
            }
        /* SQA: MPEG codec was removed because it crashes. Check the issue
        case TupExportInterface::MPEG:
            {
                return TFFmpegMovieGenerator::MPEG;
            }
        break;
        */
        case TupExportInterface::MOV:
            {
                return TFFmpegMovieGenerator::MOV;
            }
        case TupExportInterface::WEBM:
            {
                return TFFmpegMovieGenerator::WEBM;
            }
        /* SQA: Obsolete formats
        case TupExportInterface::SWF:
            {
                return TFFmpegMovieGenerator::SWF;
            }
        break;
        case TupExportInterface::ASF:
            {
                return TFFmpegMovieGenerator::ASF;
            }
        break;
        */
        case TupExportInterface::PNG:
        case TupExportInterface::JPEG:
        case TupExportInterface::XPM:
        // case TupExportInterface::SMIL:
        case TupExportInterface::NONE:
            {
                return TFFmpegMovieGenerator::NONE;
            }
        default:
            {
                return TFFmpegMovieGenerator::NONE;
            }
    }

    // return TFFmpegMovieGenerator::NONE;
}

bool FFmpegPlugin::exportToFormat(const QColor color, const QString &filePath, const QList<TupScene *> &scenes, TupExportInterface::Format fmt, 
                                 const QSize &size, const QSize &newSize, int fps, TupLibrary *library)
{
    Q_UNUSED(newSize);

    qreal duration = 0;
    foreach (TupScene *scene, scenes)
        duration += static_cast<qreal>(scene->framesCount()) / static_cast<qreal>(fps);

    TMovieGeneratorInterface::Format format = videoFormat(fmt);
    if (format == TFFmpegMovieGenerator::NONE)
        return false;

    // SQA: Get sound files from library and pass them as QList to TFFmpegMovieGenerator 
    TFFmpegMovieGenerator *generator = new TFFmpegMovieGenerator(format, size, fps, duration);
    TupAnimationRenderer renderer(color, library);
    {
        if (!generator->validMovieHeader()) {
            errorMsg = generator->getErrorMsg();
            #ifdef TUP_DEBUG
                QString msg = "FFmpegPlugin::exportToFormat() - [ Fatal Error ] - Can't create video -> " + filePath;
                #ifdef Q_OS_WIN
                    qDebug() << msg;
                #else
                    tError() << msg;
                #endif
            #endif
            delete generator;

            return false;
        }

        QPainter painter(generator);
        painter.setRenderHint(QPainter::Antialiasing, true);
        foreach (TupScene *scene, scenes) {
            renderer.setScene(scene, size);
            int i = 0;
            while (renderer.nextPhotogram()) {
                #ifdef TUP_DEBUG
                    QString msg = "FFmpegPlugin::exportToFormat() - Rendering frame -> " + QString::number(i);
                    i++;
                    #ifdef Q_OS_WIN 
                        qDebug() << msg;
                    #else
                        tWarning() << msg;
                    #endif
                #endif
                renderer.render(&painter);
                generator->nextFrame();
                generator->reset();
            }
        }
    }

    generator->saveMovie(filePath);
    delete generator;

    return true;
}

bool FFmpegPlugin::exportFrame(int frameIndex, const QColor color, const QString &filePath, TupScene *scene, const QSize &size, TupLibrary *library)
{
    Q_UNUSED(frameIndex);
    Q_UNUSED(color);
    Q_UNUSED(filePath);
    Q_UNUSED(scene);
    Q_UNUSED(size);
    Q_UNUSED(library);

    return false;
}

QString FFmpegPlugin::getExceptionMsg() const {
    return errorMsg;
}

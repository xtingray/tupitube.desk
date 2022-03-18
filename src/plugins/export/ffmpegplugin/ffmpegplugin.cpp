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
#include "tuplayer.h"
#include "tupanimationrenderer.h"
#include "tupexportinterface.h"
#include "tffmpegmoviegenerator.h"

#include <QImage>
#include <QPainter>

FFmpegPlugin::FFmpegPlugin()
{
}

FFmpegPlugin::~FFmpegPlugin()
{
}

QString FFmpegPlugin::formatName() const
{
    return tr("Video Formats");
}

TupExportInterface::Plugin FFmpegPlugin::key()
{
    return TupExportInterface::VideoFormats;
}

TupExportInterface::Formats FFmpegPlugin::availableFormats()
{
    return TupExportInterface::MP4 | TupExportInterface::AVI | TupExportInterface::MOV;
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
        case TupExportInterface::PNG:
        case TupExportInterface::JPEG:
        case TupExportInterface::XPM:
        case TupExportInterface::NONE:
        default:
            {
                return TFFmpegMovieGenerator::NONE;
            }
    }
}

bool FFmpegPlugin::exportToFormat(const QColor color, const QString &filePath, const QList<TupScene *> &scenes,
                                  TupExportInterface::Format fmt, const QSize &size, const QSize &newSize, int fps,
                                  TupLibrary *library, bool waterMark)
{
    Q_UNUSED(newSize)

    #ifdef TUP_DEBUG
        qDebug() << "[FFmpegPlugin::exportToFormat()] - fps -> " << fps;
    #endif

    int frames = 0;
    double duration = 0;
    foreach (TupScene *scene, scenes) {
        duration += static_cast<double>(scene->framesCount()) / static_cast<double>(fps);
        frames += scene->framesCount();
    }

    TMovieGeneratorInterface::Format format = videoFormat(fmt);
    if (format == TFFmpegMovieGenerator::NONE)
        return false;

    QList<SoundResource> sounds;
    if (library) {
        QList<SoundResource> soundItems = library->soundResourcesList();
        if (!soundItems.isEmpty()) {
            foreach(SoundResource item, soundItems) {
                if (!item.muted)
                    sounds << item;
            }
        }
    }

    TFFmpegMovieGenerator *generator = new TFFmpegMovieGenerator(format, size, fps, duration, sounds);
    TupAnimationRenderer renderer(color, library, waterMark);
    {
        if (!generator->validMovieHeader()) {
            errorMsg = generator->getErrorMsg();
            #ifdef TUP_DEBUG
                qDebug() << "[FFmpegPlugin::exportToFormat()] - Fatal Error: Can't create video -> " << filePath;
            #endif
            delete generator;

            return false;
        }

        QPainter painter(generator);
        painter.setRenderHint(QPainter::Antialiasing, true);
        int photogram = 0;
        foreach (TupScene *scene, scenes) {
            renderer.setScene(scene, size);
            while (renderer.nextPhotogram()) {
                #ifdef TUP_DEBUG
                    qDebug() << "[FFmpegPlugin::exportToFormat()] - Rendering frame -> " << photogram;
                #endif
                renderer.render(&painter);
                generator->nextFrame();
                generator->reset();
                photogram++;
                if (frames > 0)
                    emit progressChanged((photogram * 100) / frames);
            }
        }
        // generator->endImagesStream();

        generator->writeAudioStreams();

        /*
        QList<SoundResource> sounds;
        if (library) {
            sounds = library->soundResourcesList();
            if (!sounds.isEmpty()) {
                bool hasSounds = true;
                foreach(SoundResource item, sounds) {
                    if (item.muted) {
                        hasSounds = false;
                        break;
                    }
                }
                if (hasSounds)
                    generator->processAudio(sounds);
            }
        }
        */
    }

    generator->saveMovie(filePath);
    delete generator;

    return true;
}

bool FFmpegPlugin::exportFrame(int frameIndex, const QColor color, const QString &filePath, TupScene *scene,
                               const QSize &size, TupLibrary *library, bool waterMark = false)
{
    Q_UNUSED(frameIndex)
    Q_UNUSED(color)
    Q_UNUSED(filePath)
    Q_UNUSED(scene)
    Q_UNUSED(size)
    Q_UNUSED(library)
    Q_UNUSED(waterMark)

    return false;
}

QString FFmpegPlugin::getExceptionMsg() const
{
    return errorMsg;
}

bool FFmpegPlugin::exportToAnimatic(const QString &filePath, const QList<QImage> images, const QList<int> frames,
                                    TupExportInterface::Format fmt, const QSize &size, int fps)
{
    #ifdef TUP_DEBUG
        qDebug() << "[FFmpegPlugin::exportAnimatic()] - fps -> " << fps;
        qDebug() << "[FFmpegPlugin::exportAnimatic()] - video path -> " << filePath;
    #endif

    double duration = static_cast<double>(images.count()) / static_cast<double>(fps);
    TMovieGeneratorInterface::Format format = videoFormat(fmt);
    if (format == TFFmpegMovieGenerator::NONE)
        return false;

    TFFmpegMovieGenerator *generator = new TFFmpegMovieGenerator(format, size, fps, duration);
    {
        if (!generator->validMovieHeader()) {
            errorMsg = generator->getErrorMsg();
            #ifdef TUP_DEBUG
                qDebug() << "[FFmpegPlugin::exportToAnimatic()] - Fatal Error: Can't create video -> " << filePath;
            #endif
            delete generator;
            return false;
        }

        for (int i=0; i<images.count(); i++) {
            int times = frames.at(i);
            for (int j=0; j<times; j++)
                generator->createVideoFrame(images.at(i));
        }
    }

    generator->saveMovie(filePath);
    delete generator;

    return true;
}

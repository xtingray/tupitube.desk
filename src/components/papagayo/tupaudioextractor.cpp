/***************************************************************************
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

#include "tupaudioextractor.h"

#include <QFile>

#define MAX_AUDIO_FRAMES 53000000

TupAudioExtractor::TupAudioExtractor(const char *path, bool reverse)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupAudioExtractor::TupAudioExtractor()] - reverse -> " << reverse;
    #endif

    numFrames = 0;
    numSamples = 0;
    samples = nullptr;
    sourcePath = path;

    if (path == nullptr) {
        #ifdef TUP_DEBUG
            qDebug() << "[TupAudioExtractor::TupAudioExtractor()] - Fatal Error: path value is empty -> " << path;
        #endif
		return;
    }

    if (!QFile::exists(path)) {
        #ifdef TUP_DEBUG
            qDebug() << "[TupAudioExtractor::TupAudioExtractor()] - Fatal Error: file doesn't exist -> " << path;
        #endif
        return;
    }

	// The sound file might already be in the right format (AIFF or WAV), try and read it. 
    bool soundRead = readSoundFile(path);
    if (soundRead && reverse) {
        int32 revI;
        float temp;
        #ifdef TUP_DEBUG
            qDebug() << "[TupAudioExtractor::TupAudioExtractor()] - soundInfo.frames -> " << soundInfo.frames;
        #endif
        for (int32 i = 0; i < soundInfo.frames / 2; i++) {
            revI = (int32)soundInfo.frames - 1 - i;
            #ifdef TUP_DEBUG
                qDebug() << "[TupAudioExtractor::TupAudioExtractor()] - soundInfo.channels -> " << soundInfo.channels;
            #endif
            for (int32 j = 0; j < soundInfo.channels; j++) {
                temp = samples[i * soundInfo.channels + j];
                samples[i * soundInfo.channels + j] = samples[revI * soundInfo.channels + j];
                samples[revI * soundInfo.channels + j] = temp;
			}
		}
    }
}

TupAudioExtractor::~TupAudioExtractor()
{
    if (samples)
        delete [] samples;
}

bool TupAudioExtractor::isValid() const
{
    if (samples == nullptr) {
        #ifdef TUP_DEBUG
            qDebug() << "[TupAudioExtractor::isValid()] - samples array is NULL!";
        #endif
		return false;
    }

    #ifdef TUP_DEBUG
        qDebug() << "[TupAudioExtractor::isValid()] - true!";
    #endif

	return true;
}

real TupAudioExtractor::duration() const
{
    if (samples == nullptr)
		return 0.0f;

    return (real)soundInfo.frames / (real)soundInfo.samplerate;
}

real TupAudioExtractor::getAmplitude(real startTime, real duration) const
{
    if (samples == nullptr || duration < 0.0f)
		return 0.0f;

    uint32 i, start, end;
    real sample, total = 0.0f;

    start = timeToSample(startTime, true);
    end = timeToSample(startTime + duration, true);
	if (end == start)
		return 0.0f;
    for (i = start; i < end; i++) {
        sample = PG_FABS(samples[i]);
		if (sample > 1.001f)
			continue;
		total += sample;
	}
	total = total / (real)(end - start);

	return total;
}

real TupAudioExtractor::getRMSAmplitude(real startTime, real duration) const
{
    if (samples == nullptr || duration < 0.0f)
		return 0.0f;

    uint32 i, start, end;
    real sample, total = 0.0f;

    start = timeToSample(startTime, true);
    end = timeToSample(startTime + duration, true);
	if (end == start)
		return 0.0f;

    for (i = start; i < end; i++) {
        sample = PG_FABS(samples[i]);
		if (sample > 1.001f)
			continue;
		total += sample * sample;
	}

	total = total / (real)(end - start);

	return (real)sqrt(total);
}

real TupAudioExtractor::getMaxAmplitude(real startTime, real duration) const
{
    if (samples == nullptr || duration < 0.0f)
		return 0.0f;

    uint32 i, start, end;
    real sample, max = -PG_HUGE;

    start = timeToSample(startTime, true);
    end = timeToSample(startTime + duration, true);
	if (end == start)
		return 0.0f;

    for (i = start; i < end; i++) {
        sample = PG_FABS(samples[i]);
		if (sample > 1.001f)
			continue;
		max = PG_MAX(max, sample);
	}

	return max;
}

uint32 TupAudioExtractor::getNumSamples() const
{
    return numSamples;
}

int32 TupAudioExtractor::sampleRate() const
{
    return soundInfo.samplerate;
}

real* TupAudioExtractor::buffer() const
{
    return samples;
}

uint32 TupAudioExtractor::timeToSample(real time, bool clamped) const
{
    if (samples == nullptr)
		return 0;

    uint32 sample;

    time = time * (real)(soundInfo.samplerate * soundInfo.channels);
	sample = PG_ROUND(time);
    if (soundInfo.channels) {
        while (sample % soundInfo.channels)
            sample--;
	}

    if (clamped) {
        if (sample > (numSamples - 1)) {
            return (numSamples -1);
        } else {
            return sample;
        }

        // return PG_CLAMP(sample, 0, numSamples - 1);
    } else {
		return sample;
    }
}

bool TupAudioExtractor::readSoundFile(const char *soundFilePath)
{    
    #ifdef TUP_DEBUG
        qDebug() << "[TupAudioExtractor::readSoundFile()] - soundFilePath -> " << soundFilePath;
    #endif

    SNDFILE *sndFile = sf_open(soundFilePath, SFM_READ, &soundInfo);
    if (!sndFile) {
        #ifdef TUP_DEBUG
            qDebug() << "[TupAudioExtractor::readSoundFile()] - sndFile is NULL!";
        #endif
        return false;
    }

    if (soundInfo.frames > MAX_AUDIO_FRAMES)
        soundInfo.frames = MAX_AUDIO_FRAMES;

    numSamples = (int32)(soundInfo.frames * soundInfo.channels);
    samples = new float[numSamples];

	{
        numFrames = sf_readf_float(sndFile, samples, soundInfo.frames);
	}

    sf_close(sndFile);

    return true;
}

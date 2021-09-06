#include "tupaudioextractor.h"

#define MAX_AUDIO_FRAMES 53000000

TupAudioExtractor::TupAudioExtractor(const char *path, bool reverse)
{
    numFrames = 0;
    numSamples = 0;
    samples = nullptr;
    sourcePath = path;

    if (path == nullptr)
		return;
	
	// The sound file might already be in the right format (AIFF or WAV), try and read it. 
    bool soundRead = readSoundFile(path);
    if (soundRead && reverse) {
        int32 revI;
        float temp;

        for (int32 i = 0; i < soundInfo.frames / 2; i++) {
            revI = (int32)soundInfo.frames - 1 - i;
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
    if (samples == nullptr)
		return false;

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

	uint32	sample;

    time = time * (real)(soundInfo.samplerate * soundInfo.channels);
	sample = PG_ROUND(time);
    if (soundInfo.channels) {
        while (sample % soundInfo.channels)
			sample--;
	}

	if (clamped)
        return PG_CLAMP(sample, 0, numSamples - 1);
	else
		return sample;
}

bool TupAudioExtractor::readSoundFile(const char *soundFilePath)
{
    SNDFILE *sndFile = sf_open(soundFilePath, SFM_READ, &soundInfo);
    if (!sndFile)
        return false;
    
    if (soundInfo.frames > MAX_AUDIO_FRAMES)
        soundInfo.frames = MAX_AUDIO_FRAMES;
    numSamples = (int32)(soundInfo.frames * soundInfo.channels);
    samples = new float[numSamples];

    /*	if (sndFormat == (SF_FORMAT_OGG | SF_FORMAT_VORBIS))
	{
		float		*bufPtr = fSamples;
		sf_count_t	sampleCount = fNumSamples;
		sf_count_t	chunkSize = 65536;
		sf_count_t	readCount;
		
		while (sampleCount > chunkSize)
		{
			readCount = sf_read_float(sndFile, bufPtr, chunkSize);
			if (readCount == 0)
				break;
			bufPtr += readCount;
			sampleCount -= readCount;
		}
		if (sampleCount > 0)
			sf_read_float(sndFile, bufPtr, chunkSize);
		fNumFrames = fSndInfo.frames;
	}
	else*/
	{
        numFrames = sf_readf_float(sndFile, samples, soundInfo.frames);
	}

    sf_close(sndFile);

    return true;
}

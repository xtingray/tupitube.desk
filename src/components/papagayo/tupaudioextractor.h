#ifndef TUPAUDIOEXTRACTOR_H
#define TUPAUDIOEXTRACTOR_H

#include "tuppg_config.h"
#include "sndfile.h"

//
// This class is the general audio extractor that wraps the library specific extractors.
// Its main purpose is to extract audio tracks from audio and video media and place them
// in either a .aiff or .wav temporary file depending on the platform and the library. 
// The audio library libsndfile will than read into memory the temporary file as sound samples.
//
// NOTE: If a path is already a .aiff or .wav file it is just read in directly by libsndfile.
//

class TupAudioExtractor
{
    public:
        TupAudioExtractor(const char *path, bool reverse = false);
        ~TupAudioExtractor();

        bool isValid() const;
        real duration() const;

        real getAmplitude(real startTime, real duration) const;
        real getRMSAmplitude(real startTime, real duration) const;
        real getMaxAmplitude(real startTime, real duration) const;

        uint32 getNumSamples() const;
        int32 sampleRate() const;
        real *buffer() const;
        uint32 timeToSample(real time, bool clamped) const;

    private:
        const char *sourcePath;

        SF_INFO soundInfo;
        uint32 numSamples;
        sf_count_t numFrames;
        float *samples;

        // This function uses the cross platform libsndfile to read .aiff and .wav files
        bool readSoundFile(const char *soundFilePath);
};

#endif

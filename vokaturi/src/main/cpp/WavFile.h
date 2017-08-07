//
// Created by Ashraf Khan on 05/Aug/2017.
//

#ifndef VOKATURIANDROID_WAVFILE_H
#define VOKATURIANDROID_WAVFILE_H

#ifdef __cplusplus
extern "C" {
#endif

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include "Vokaturi.h"
#define my  me ->

typedef struct {
    FILE *f;
    double samplingFrequency;
    int numberOfChannels;
    int numberOfSamples;
    int sampleOffset;
    unsigned char *cache;
    int cacheSize;
} VokaturiWavFile;

inline static void VokaturiWavFile_open (const char *fileName, VokaturiWavFile *me) {
    my f = fopen (fileName, "rb");
    if (my f == NULL) {
        fprintf (stderr, "VokaturiWavFile error: cannot open file %s.\n", fileName);
        return;
    }
    /*
        Check that this really is a WAV file,
        by checking for the header, which should be "RIFFxxxxWAVEfmt ",
        where the x'es can be anything.
    */
    char buffer [10000], *header = & buffer [0];
    fread (header, 1, 10000, my f);
    if (strncmp (header, "RIFF", 4) ||
        strncmp (header + 8, "WAVE", 4) ||
        strncmp (header + 12, "fmt ", 4))
    {
        fprintf (stderr, "VokaturiWavFile error: %s is not a simplistic WAV file.\n", fileName);
        fclose (my f);
        my f = NULL;
        return;
    }
    unsigned int fmtChunkSize =
            (unsigned char) header [16] |
            ((unsigned char) header [17]) << 8 |
            ((unsigned char) header [18]) << 16 |
            ((unsigned char) header [19]) << 24;
    if (fmtChunkSize != 16) {
        if (fmtChunkSize == 18) {
            /*
                This is a "fmt" chunk that has an additional 2 bytes that denote
                the size of the extension (which is zero).
            */
        } else {
            fprintf (stderr, "VokaturiWavFile error: %s has a fmt chunk of size %u.\n", fileName, fmtChunkSize);
            fclose (my f);
            my f = NULL;
            return;
        }
    }
    unsigned int audioFormat =
            (unsigned char) header [20] |
            ((unsigned char) header [21]) << 8;
    if (audioFormat != 1) {
        fprintf (stderr, "VokaturiWavFile error: %s has compressed audio format %u.\n", fileName, audioFormat);
        fclose (my f);
        my f = NULL;
        return;
    }
    unsigned int numberOfChannels =
            (unsigned char) header [22] |
            ((unsigned char) header [23]) << 8;
    if (numberOfChannels > 2) {
        fprintf (stderr, "VokaturiWavFile error: %s has more than two channels.\n", fileName);
        fclose (my f);
        my f = NULL;
        return;
    }
    my numberOfChannels = numberOfChannels;
    unsigned int sampleRate =
            (unsigned char) header [24] |
            ((unsigned char) header [25]) << 8 |
            ((unsigned char) header [26]) << 16 |
            ((unsigned char) header [27]) << 24;
    if (sampleRate < 8000) {
        fprintf (stderr, "VokaturiWavFile error: a sample rate of %u Hz is not supported.\n", sampleRate);
        fclose (my f);
        my f = NULL;
        return;
    }
    my samplingFrequency = sampleRate;
    unsigned int bitsPerSample =
            (unsigned char) header [34] |
            ((unsigned char) header [35]) << 8;
    if (bitsPerSample != 16) {
        fprintf (stderr, "VokaturiWavFile error: %s is not 16-bit PCM.\n", fileName);
        fclose (my f);
        my f = NULL;
        return;
    }
    if (fmtChunkSize == 18) header += 2;   // skip the optional word, which always contains zero

    if (! strncmp (header + 36, "data", 4)) {
        my sampleOffset = 44;
    } else if (! strncmp (header + 36, "PAD ", 4) || ! strncmp (header + 36, "fact", 4) || ! strncmp (header + 36, "LIST", 4)) {
        unsigned int chunkSize =
                (unsigned char) header [40] |
                ((unsigned char) header [41]) << 8 |
                ((unsigned char) header [42]) << 16 |
                ((unsigned char) header [43]) << 24;
        if (chunkSize > 9000) {
            fprintf (stderr, "VokaturiWavFile error: %s has a too large \"%.4s\" chunk.\n", fileName, header + 36);
            fclose (my f);
            my f = NULL;
            return;
        }
        if (! strncmp (header + 36 + 4 + 4 + chunkSize, "data", 4)) {
            my sampleOffset = 44 + 4 + 4 + chunkSize;
        } else {
            fprintf (stderr, "VokaturiWavFile error: %s has no data chunk.\n", fileName);
            fclose (my f);
            my f = NULL;
            return;
        }
    } else {
        fprintf (stderr, "VokaturiWavFile error: %s contains un unrecognized \"%.4s\" chunk.\n", fileName, header + 36);
        fclose (my f);
        my f = NULL;
        return;
    }
    const int bytesPerSample = 2;
    unsigned int dataChunkSize =
            (unsigned char) header [my sampleOffset - 4] |
            ((unsigned char) header [my sampleOffset - 3]) << 8 |
            ((unsigned char) header [my sampleOffset - 2]) << 16 |
            ((unsigned char) header [my sampleOffset - 1]) << 24;
    if (dataChunkSize < 100 * bytesPerSample * numberOfChannels) {
        fprintf (stderr, "VokaturiWavFile error: %s does not have at least 100 samples.\n", fileName);
        fclose (my f);
        my f = NULL;
        return;
    }
    my numberOfSamples = dataChunkSize / bytesPerSample / numberOfChannels;
    my cache = NULL;
    my cacheSize = 0;
}

inline static int VokaturiWavFile_valid (VokaturiWavFile *me) {
    return my f != NULL;
}

inline static void VokaturiWavFile_close (VokaturiWavFile *me) {
    if (my f) fclose (my f);
}

inline static void VokaturiWavFile_clear (VokaturiWavFile *me) {
    if (my f) {
        fclose (my f);
        my f = NULL;
    }
    if (my cache) {
        free (my cache);
        my cache = NULL;
        my cacheSize = 0;
    }
}

inline static void _VokaturiWavFile_readToCache (VokaturiWavFile *me,
                                                 int startSample,   // base-0
                                                 int numberOfSamples)
{
    const int bytesPerSample = 2;
    fseek (my f, my sampleOffset + startSample * bytesPerSample, SEEK_SET);
    int numberOfBytes = bytesPerSample * numberOfSamples * my numberOfChannels;
    if (numberOfBytes > my cacheSize) {
        if (my cache) free (my cache);
        my cache = (unsigned char *) malloc (my cacheSize = numberOfBytes);
    }
    fread (my cache, sizeof (char), numberOfBytes, my f);
}

inline static void VokaturiWavFile_fillSamples (VokaturiWavFile *me,
                                                int channel,   // 0 = left or only channel; 1 = right channel
                                                int startSample,   // base-0
                                                int numberOfSamples,
                                                double *samples)
{
    _VokaturiWavFile_readToCache (me, startSample, numberOfSamples);
    unsigned char *p = & my cache [0];
    for (int isamp = 0; isamp < numberOfSamples; isamp ++) {
        if (channel == 1) p += 2;   // skip left channel
        /* First narrow to 16 bits, then change signedness! */
        int16_t sample_int = (int16_t) (uint16_t) (p [0] | p [1] << 8);   // signed little-endian two bytes
        p += 2;
        samples [isamp] = sample_int / 32768.0;
        if (my numberOfChannels > 1 && channel == 0) p += 2;   // skip right channel
    }
}

inline static double * VokaturiWavFile_readAll (VokaturiWavFile *me, int channel) {
    double *samples = (double *) calloc (my numberOfSamples, sizeof * samples);
    if (samples == NULL) return NULL;
    VokaturiWavFile_fillSamples (me, channel, 0, my numberOfSamples, samples);
    return samples;
}

inline static void VokaturiWavFile_fillVoice (VokaturiWavFile *me, VokaturiVoice voice,
                                              int channel,   // 0 = left or only channel; 1 = right channel
                                              int startSample,   // base-0
                                              int numberOfSamples)
{
    _VokaturiWavFile_readToCache (me, startSample, numberOfSamples);
    unsigned char *p = & my cache [0];
    for (int isamp = 0; isamp < numberOfSamples; isamp ++) {
        if (channel == 1) p += 2;   // skip left channel
        /* First narrow to 16 bits, then change signedness! */
        int16_t sample_int = (int16_t) (uint16_t) (p [0] | p [1] << 8);   // signed little-endian two bytes
        p += 2;
        double sample = sample_int / 32768.0;
        VokaturiVoice_fill (voice, 1, & sample);
        if (my numberOfChannels > 1 && channel == 0) p += 2;   // skip right channel
    }
}

#ifdef __cplusplus
}
#endif

#endif //VOKATURIANDROID_WAVFILE_H

//
// Created by Ashraf Khan on 05/Aug/2017.
//

#ifndef VOKATURIANDROID_SPECTRUM_H
#define VOKATURIANDROID_SPECTRUM_H

#ifdef __cplusplus
extern "C" {
#endif


#include "Thing.h"
#include "fft_c.h"

/*
	A Spectrum is a structure that can hold the complex spectrum of a sound.
	The lowest frequency represented is 0 Hz, and the highest frequency represented
	is the Nyquist frequency of the sound.

	The attributes of a Spectrum are:

	- length
		The number of doubles required to hold the spectrum. This is always a power of 2.
		As the complex value at 0 Hz is real,
		and the complex value at the Nyquist frequency (sampling frequency / 2) is real as well,
		a Spectrum contains `length`/2+1 real values and `length`/2-1 imaginary values.
	- frequencyStepInHz
		The distance in Hz between consecutive points in the spectrum.
		This is related to the sampling frequency of the sound as follows:
			`frequencyStepInHz` * `length` == sampling frequency
	- samples
		The doubles that hold the spectrum. The length of this array is `length`.
		The complex spectral values are ordered as follows:
			samples [0] == re [0]   // point 0: 0 Hz
			samples [1] == re [1]   // point 1: `frequencyStepInHz` Hz
			samples [2] == im [1]
			samples [3] == re [2]
			samples [4] == im [2]
			...
			samples [`length` - 5] == re [`length`/2 - 2]
			samples [`length` - 4] == im [`length`/2 - 2]
			samples [`length` - 3] == re [`length`/2 - 1]
			samples [`length` - 2] == im [`length`/2 - 1]
			samples [`length` - 1] == re [`length`/2]
*/
typedef struct {
    double *samples;
    int length;
    double frequencyStepInHertz;
    double *smallCache;
    double *bigCache;
    long splitCache [32];
    bool valid;
} Spectrum;

inline static void Spectrum_initWithLengthAndFrequencyStep (Spectrum *me, int length, double frequencyStepInHertz) {
    my valid = false;
    my samples = (double *) calloc (length, sizeof * my samples);
    if (my samples == NULL) {
        return;
    }
    my length = length;
    my frequencyStepInHertz = frequencyStepInHertz;

    /*
        Make room for the caches for FFT, and initialize them.
    */
    my smallCache = (double *) calloc (length, sizeof * my smallCache);
    if (my smallCache == NULL) {
        return;
    }
    my bigCache = (double *) calloc (2 * length, sizeof * my bigCache);
    if (my bigCache == NULL) {
        return;
    }
    drfti1 (length, my bigCache, my splitCache);

    my valid = true;
}

inline static bool Spectrum_isValid (Spectrum *me) {
    return my valid;
}

inline static void Spectrum_destroy (Spectrum *me) {
    if (my samples) {
        free (my samples);
        my samples = NULL;
    }
    if (my smallCache) {
        free (my smallCache);
        my smallCache = NULL;
    }
    if (my bigCache) {
        free (my bigCache);
        my bigCache = NULL;
    }
    my valid = false;
}

inline static void Spectrum_square (Spectrum *me) {
    double re = my samples [0];
    double im = 0.0;
    my samples [0] = re * re + im * im;
    for (int ifreq = 1; ifreq < my length - 1; ifreq += 2) {
        re = my samples [ifreq];
        im = my samples [ifreq + 1];
        my samples [ifreq] = re * re + im * im;   // the square is real, so the real part receives it
        my samples [ifreq + 1] = 0.0;   // the imaginary part will therefore be zero
    }
    re = my samples [my length - 1];
    im = 0.0;
    my samples [my length - 1] = re * re + im * im;
}

inline static double Spectrum_computeBandEnergyDensity (Spectrum *me, double fmin, double fmax) {
    double energy = 0.0;
    int numberOfEnergiesAdded = 0;
    int imin = (int) ceil (fmin / my frequencyStepInHertz);
    int imax = (int) floor (fmax / my frequencyStepInHertz);
    if (imin <= 0) {
        double re = my samples [0];
        double im = 0.0;
        energy += re * re + im * im;
        numberOfEnergiesAdded += 1;
        imin = 1;
    }
    if (imax >= my length / 2) {
        double re = my samples [my length - 1];
        double im = 0.0;
        energy += re * re + im * im;
        numberOfEnergiesAdded += 1;
        imax = my length / 2 - 1;
    }
    for (int ifreq = imin; ifreq <= imax; ifreq ++) {
        double re = my samples [ifreq + ifreq - 1];   // 1, 3, 5, ..., spectrumLength - 5, spectrumLength - 3
        double im = my samples [ifreq + ifreq];   // 2, 4, 6, ..., spectrumLength - 4, spectrumLength - 2
        energy += re * re + im * im;
        numberOfEnergiesAdded += 1;
    }
    return ( numberOfEnergiesAdded > 0 ? energy / numberOfEnergiesAdded : -1.0 );
}


#ifdef __cplusplus
}
#endif

#endif //VOKATURIANDROID_SPECTRUM_H

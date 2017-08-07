//
// Created by Ashraf Khan on 05/Aug/2017.
//

#ifndef VOKATURIANDROID_SOUND_AND_PITCH_H
#define VOKATURIANDROID_SOUND_AND_PITCH_H

#ifdef __cplusplus
extern "C" {
#endif

#include <math.h>
#include "Sound.h"
#include "Pitch.h"

inline static void Sound_autocorrelation_into_Pitch (Sound *me, Pitch *you, double fmin, double fmax) {
    double dt = 1.0 / my samplingFrequencyInHertz;
    int imin = (int) ceil (1.0 / (fmax * dt));
    int imax = (int) floor (1.0 / (fmin * dt));
    if (imin <= 0) {
        imin = 1;
    }
    if (imax >= my length / 2 - 1) {
        imax = my length / 2 - 2;
    }
    double maximum = -1e300;
    double bestLag_samples = 0.0;
    for (int ilag = imin; ilag <= imax; ilag ++) {
        double octaveCost = 0.03;
        double left = my samples [ilag - 1];
        double mid = my samples [ilag];
        double right = my samples [ilag + 1];
        if (mid > 0.0 && mid >= left && mid >= right)
        {
            /*
                We found a local maximum.
                Perform a parabolic interpolation, following
                http://www.fon.hum.uva.nl/praat/manual/vector_peak_interpolation.html
            */
            double dy = 0.5 * (right - left);
            double d2y = 2.0 * mid - left - right;
            if (d2y != 0.0) {
                double lag_samples = (double) ilag + dy / d2y;
                double localMaximum = (mid + 0.5 * dy * dy / d2y) / my samples [0] - octaveCost * log2_rev (lag_samples);
                if (localMaximum > maximum) {
                    maximum = localMaximum;
                    bestLag_samples = lag_samples;
                }
            }
        }
    }
    if (bestLag_samples == 0.0) {
        your f0InHertz = 0.0;   // this signals failure
        your strength = 0.0;
    } else {
        double bestLag_seconds = dt * bestLag_samples;
        your f0InHertz = 1.0 / bestLag_seconds;
        your strength = maximum;
    }
}
#ifdef __cplusplus
}
#endif

#endif //VOKATURIANDROID_SOUND_AND_PITCH_H

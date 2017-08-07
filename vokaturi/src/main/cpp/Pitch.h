//
// Created by Ashraf Khan on 05/Aug/2017.
//

#ifndef VOKATURIANDROID_PITCH_H
#define VOKATURIANDROID_PITCH_H


#ifdef __cplusplus
extern "C" {
#endif

#include "Thing.h"
#include <math.h>

#define M_LOG2e 1.442685040888896340736

typedef struct {
    double f0InHertz;
    double strength;
} Pitch;

inline static double log2_rev(double x);

inline static double Pitch_getValueInSemitonesRe100Hz (Pitch *me) {
    const double pitchReference_Hz = 100.0;
    const double pitch_octavesRelativeToReference = log2_rev (my f0InHertz / pitchReference_Hz);
    const double numberOfSemitonesPerOctave = 12.0;
    const double pitch_semitonesRelativeToReference =
            pitch_octavesRelativeToReference * numberOfSemitonesPerOctave;
    return pitch_semitonesRelativeToReference;
}

inline static bool Pitch_isValid (Pitch *me) {
    return my f0InHertz > 0.0;
}

inline static double log2_rev(const double x)
{
    return log10(x) / M_LOG2e;
}

#ifdef __cplusplus
}
#endif


#endif //VOKATURIANDROID_PITCH_H

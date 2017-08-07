//
// Created by Ashraf Khan on 05/Aug/2017.
//

#ifndef VOKATURIANDROID_SOUND_AND_SPECTRUM_H
#define VOKATURIANDROID_SOUND_AND_SPECTRUM_H

#ifdef __cplusplus
extern "C" {
#endif

#include "Sound.h"
#include "Spectrum.h"

inline static void Sound_into_Spectrum (Sound *me, Spectrum *you) {
    for (int isamp = 0; isamp < my length; isamp ++) {
        your samples [isamp] = my samples [isamp];
    }
    for (int isamp = my length; isamp < your length; isamp ++) {
        your samples [isamp] = 0.0;
    }
    drftf1 (your length, your samples, your smallCache, your bigCache, your splitCache);
}

inline static void Spectrum_into_Sound (Spectrum *me, Sound *you) {
    drftb1 (my length, my samples, my smallCache, my bigCache, my splitCache);
    for (int isamp = 0; isamp < your length; isamp ++) {
        your samples [isamp] = my samples [isamp];
    }
}


#ifdef __cplusplus
}
#endif

#endif //VOKATURIANDROID_SOUND_AND_SPECTRUM_H

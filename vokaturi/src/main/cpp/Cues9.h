//
// Created by Ashraf Khan on 05/Aug/2017.
//

#ifndef VOKATURIANDROID_CUES9_H
#define VOKATURIANDROID_CUES9_H

#ifdef __cplusplus
extern "C" {
#endif

enum Cues9 {
    CUE_pitAve = 0,   // average pitch, in semitones
    CUE_pitDir,   // pitch direction, in cube-rooted semitones per second
    CUE_pitDyn,   // pitch dynamics for voiced parts, in semitones per second
    CUE_pitJit,   // jitter
    CUE_intAve,   // average intensity, in dB
    CUE_intDyn,   // intensity dynamics for voice parts, in dB per second
    CUE_intJit,   // shimmer
    CUE_spcSlo,   // spectral slope
    CUE_spcJit,
    NUMBER_OF_CUES9
};

typedef double CueStrengths9 [NUMBER_OF_CUES9];

/*
	Means and standard deviations of the cues, based on EmoDB.
*/
static double Cues9_mean [NUMBER_OF_CUES9] = { 10.0, -0.7, 46.0, 170.0, 75.0, 120.0, 120.0, -20.0, 270.0 };
static double Cues9_stdev [NUMBER_OF_CUES9] = { 6.0, 1.0, 20.0, 180.0, 3.0, 30.0, 35.0, 6.0, 80.0 };

#ifdef __cplusplus
}
#endif
#endif //VOKATURIANDROID_CUES9_H

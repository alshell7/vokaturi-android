//
// Created by Ashraf Khan on 05/Aug/2017.
//

#ifndef VOKATURIANDROID_VOKATURI_H
#define VOKATURIANDROID_VOKATURI_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct structVokaturiVoice *VokaturiVoice;

VokaturiVoice VokaturiVoice_create (
        double sample_rate,
        int buffer_length
);

typedef struct {
    double neutrality;
    double happiness;
    double sadness;
    double anger;
    double fear;
} VokaturiEmotionProbabilities;

void VokaturiVoice_setRelativePriorProbabilities (
        VokaturiVoice voice,
        VokaturiEmotionProbabilities *priorEmotionProbabilities
);

void VokaturiVoice_fill (
        VokaturiVoice voice,
        int num_samples,
        double samples []
);

typedef struct {
    int valid;   // 1 = "there were voiced frames, so that the measurements are valid"; 0 = "no voiced frames found"
    int num_frames_analyzed;
    int num_frames_lost;
} VokaturiQuality;

void VokaturiVoice_extract (
        VokaturiVoice voice,
        VokaturiQuality *quality,
        VokaturiEmotionProbabilities *emotionProbabilities
);

void VokaturiVoice_destroy (VokaturiVoice voice);

void VokaturiVoice_reset (VokaturiVoice voice);

const char *Vokaturi_versionAndLicense ();

#ifdef __cplusplus
}
#endif
#endif //VOKATURIANDROID_VOKATURI_H

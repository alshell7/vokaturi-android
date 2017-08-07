//
// Created by Ashraf Khan on 05/Aug/2017.
//

#ifndef VOKATURIANDROID_EMOTIONS_H
#define VOKATURIANDROID_EMOTIONS_H

#ifdef __cplusplus
extern "C" {
#endif

#include <assert.h>
#include <stdio.h>

enum Emotions {
    EMOTION_Neu = 0,   // neutrality
    EMOTION_Hap,   // happiness
    EMOTION_Sad,   // sadness
    EMOTION_Ang,   // anger
    EMOTION_Fea,   // fear
    NUMBER_OF_EMOTIONS
};

typedef double EmotionProbabilities [NUMBER_OF_EMOTIONS];

inline static void EmotionProbabilities_normalize (EmotionProbabilities emotionProbabilities) {
    double sumOfProbabilities = 0.0;
    for (int emotion = 0; emotion < NUMBER_OF_EMOTIONS; emotion ++) {
        sumOfProbabilities += emotionProbabilities [emotion];
    }
    if (sumOfProbabilities <= 0.0) return;
    for (int emotion = 0; emotion < NUMBER_OF_EMOTIONS; emotion ++) {
        emotionProbabilities [emotion] /= sumOfProbabilities;
    }
}

inline static void EmotionProbabilities_print (EmotionProbabilities emotionProbabilities, FILE *f) {
    fprintf (f, "Emotion probabilities:");
    for (int emotion = 0; emotion < NUMBER_OF_EMOTIONS; emotion ++) {
        fprintf (f, " %f", emotionProbabilities [emotion]);
    }
    fprintf (f, "\n");
}

inline static int EmotionProbabilities_getWinner (EmotionProbabilities emotionProbabilities) {
    int winner = -1;
    double probability = 0.0;
    for (int emotion = 0; emotion < NUMBER_OF_EMOTIONS; emotion ++) {
        if (emotionProbabilities [emotion] > probability) {
            winner = emotion;
            probability = emotionProbabilities [emotion];
        }
    }
    if (winner < 0) {
        EmotionProbabilities_print (emotionProbabilities, stderr);
    }
    assert (winner >= 0);
    return winner;
}

inline static void EmotionProbabilities_weigh (
        EmotionProbabilities emotionProbabilities,
        EmotionProbabilities relativeEmotionProbabilities)
{
    for (int emotion = 0; emotion < NUMBER_OF_EMOTIONS; emotion ++) {
        emotionProbabilities [emotion] *= relativeEmotionProbabilities [emotion];
    }
    EmotionProbabilities_normalize (emotionProbabilities);
}

#ifdef __cplusplus
}
#endif

#endif //VOKATURIANDROID_EMOTIONS_H

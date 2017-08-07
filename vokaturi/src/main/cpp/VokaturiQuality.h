//
// Created by Ashraf Khan on 05/Aug/2017.
//

#ifndef VOKATURIANDROID_VOKATURIQUALITY_H
#define VOKATURIANDROID_VOKATURIQUALITY_H

#ifdef __cplusplus
extern "C" {
#endif

#include "Thing.h"
#include "Vokaturi.h"

inline static void VokaturiQuality_error (VokaturiQuality *me) {
    my valid = false;
    my num_frames_analyzed = 0;
    my num_frames_lost = 0;
}

#ifdef __cplusplus
}
#endif

#endif //VOKATURIANDROID_VOKATURIQUALITY_H

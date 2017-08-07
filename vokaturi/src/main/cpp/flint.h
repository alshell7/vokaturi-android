//
// Created by Ashraf Khan on 05/Aug/2017.
//

#ifndef VOKATURIANDROID_FLINT_H
#define VOKATURIANDROID_FLINT_H

#ifdef __cplusplus
extern "C" {
#endif

#include <math.h>

typedef double flint;

inline static flint flint_div (flint a, flint b) { return floor (a / b); }
inline static flint flint_mod (flint a, flint b) { return a - floor (a / b) * b; }


#ifdef __cplusplus
}
#endif

#endif //VOKATURIANDROID_FLINT_H

/**
 * @file AABB.h
 * @ingroup AABB
 */

#ifndef _AABB_H_
#define _AABB_H_

#include <SDL.h>

typedef struct AABB_t {
    double dBottom;
    double dLeft;
    double dRight;
    double dTop;
} AABB;

SDL_bool BoxesDoIntersect(const AABB stBoxA, const AABB stBoxB);

#endif // _AABB_H_

/**
 * @file AABB.h
 * @ingroup AABB
 */

#pragma once
#include <SDL.h>

typedef struct AABB_t {
    double dBottom;
    double dLeft;
    double dRight;
    double dTop;
} AABB;

SDL_bool BoxesDoIntersect(const AABB stBoxA, const AABB stBoxB);

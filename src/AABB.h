/**
 * @file    AABB.h
 * @ingroup AABB
 */
#pragma once

#include <SDL.h>

/**
 * @typedef AABB
 * @brief   Axis-aligned bounding box handle type
 * @struct  AABB_t
 * @brief   Axis-aligned bounding box handle data
 */
typedef struct AABB_t
{
    double dBottom;
    double dLeft;
    double dRight;
    double dTop;

} AABB;

SDL_bool BoxesDoIntersect(const AABB stBoxA, const AABB stBoxB);

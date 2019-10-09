/**
 * @file    AABB.h
 * @brief   AABB handler include header
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
    double dBottom;  ///< Bottom edge position
    double dLeft;    ///< Left edge position
    double dRight;   ///< Right edge position
    double dTop;     ///< Top edge position

} AABB;

SDL_bool AABB_BoxesDoIntersect(const AABB stBoxA, const AABB stBoxB);

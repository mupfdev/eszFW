// SPDX-License-Identifier: Beerware
/**
 * @file    Background.h
 * @brief   Background handler include header
 * @ingroup Background
 */
#pragma once

#include <SDL.h>
#include "Constants.h"

/**
 * @typedef BGLayer
 * @brief   Background layer type
 * @struct  BGLayer_t
 * @brief   Background layer data
 */
typedef struct BGLayer_t
{
    SDL_Texture* pstLayer;   ///< Pointer to SDL2 texture
    Sint32       s32Width;   ///< Background width in pixel
    Sint32       s32Height;  ///< Background height in pixel
    double       dPosX;      ///< Position along the x-axis
    double       dPosY;      ///< Position along the y-axis
    double       dVelocity;  ///< Velocity

} BGLayer;

/**
 * @typedef Background
 * @brief   Background handle type
 * @struct  Background_t
 * @brief   Background handle data
 */
typedef struct Background_t
{
    Uint8     u8Num;       ///< Number of layers
    Alignment eAlignment;  ///< Background alignment
    Direction eDirection;  ///< Scroll direction
    BGLayer   acLayer[];   ///< Array of background layers

} Background;

Sint8 Background_Draw(
    const Direction eDirection,
    const Sint32    s32LogicalWindowHeight,
    const double    dCameraPosY,
    const double    dVelocity,
    SDL_Renderer*   pstRenderer,
    Background*     pstBackground);

void Background_Free(Background* Background);

Sint8 Background_Init(
    const Uint8     u8Num,
    const char*     pacFilenames[static u8Num],
    const Sint32    s32WindowWidth,
    const Alignment eAlignment,
    SDL_Renderer*   pstRenderer,
    Background**    pstBackground);

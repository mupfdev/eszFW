/**
 * @file    Background.h
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
    SDL_Texture* pstLayer;
    Sint32       s32Width;
    Sint32       s32Height;
    double       dPosX;
    double       dPosY;
    double       dVelocity;

} BGLayer;

/**
 * @typedef Background
 * @brief   Background handle type
 * @struct  Background_t
 * @brief   Background handle data
 */
typedef struct Background_t
{
    Uint8     u8Num;
    Alignment eAlignment;
    Direction eDirection;
    BGLayer   acLayer[];

} Background;

Sint8 DrawBackground(
    const Direction eDirection,
    const Sint32    s32LogicalWindowHeight,
    const double    dCameraPosY,
    const double    dVelocity,
    SDL_Renderer*   pstRenderer,
    Background*     pstBackground);

void FreeBackground(Background* Background);

Sint8 InitBackground(
    const Uint8     u8Num,
    const char*     pacFilenames[static u8Num],
    const Sint32    s32WindowWidth,
    const Alignment eAlignment,
    SDL_Renderer*   pstRenderer,
    Background**    pstBackground);

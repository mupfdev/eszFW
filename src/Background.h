/**
 * @file    Background.h
 * @ingroup Background
 */

#pragma once
#include <SDL.h>
#include "Constants.h"

typedef struct BGLayer_t
{
    SDL_Texture *pstLayer;
    Sint32       s32Width;
    Sint32       s32Height;
    double       dPosX;
    double       dPosY;
    double       dVelocity;
} BGLayer;

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
    SDL_Renderer   *pstRenderer,
    Background     *pstBackground);

void FreeBackground(Background *Background);

Sint8 InitBackground(
    const Uint8     u8BgNum,
    const char     *pacFilenames[static u8BgNum],
    const Sint32    s32WindowWidth,
    const Alignment eAlignment,
    SDL_Renderer   *pstRenderer,
    Background    **pstBackground);

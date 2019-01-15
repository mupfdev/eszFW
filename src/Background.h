/**
 * @file    Background.h
 * @ingroup Background
 */

#ifndef _BACKGROUND_H_
#define _BACKGROUND_H_

#ifdef __ANDROID__
#include <SDL.h>
#else
#include <SDL2/SDL.h>
#endif
#include <stdbool.h>
#include <stdint.h>

typedef struct BGLayer_t
{
    SDL_Texture *pstLayer;
    int32_t      s32Width;
    int32_t      s32Height;
    double       dPosX;
    double       dPosY;
    double       dVelocity;
} BGLayer;

typedef struct Background_t
{
    uint8_t u8Num;
    bool    bAlignment;
    bool    bOrientation;
    BGLayer acLayer[];
} Background;

int DrawBackground(
    const bool     bOrientation,
    const int32_t  s32LogicalWindowHeight,
    const double   dCameraPosY,
    const double   dVelocity,
    SDL_Renderer **pstRenderer,
    Background   **pstBackground);

void FreeBackground(Background **Background);

int InitBackground(
    const uint8_t  u8BgNum,
    const char    *pacFilenames[static u8BgNum],
    const int32_t  s32WindowWidth,
    const bool     bAlignment,
    SDL_Renderer **pstRenderer,
    Background   **pstBackground);

#endif // _BACKGROUND_H_

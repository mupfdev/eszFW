/**
 * @file RainbowJoe.c
 * @ingroup RainbowJoe
 * @defgroup RainbowJoe
 * @author Michael Fitzmayer
 * @copyright "THE BEER-WARE LICENCE" (Revision 42)
 */

#define QUIT(n) { sReturnValue = n; goto quit; }

#include <SDL2/SDL.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "AABB.h"
#include "Entity.h"
#include "Font.h"
#include "FPSLimiter.h"
#include "Input.h"
#include "Macros.h"
#include "Map.h"
#include "RainbowJoe.h"
#include "Video.h"

int InitGame()
{
    int     sReturnValue   = 0;
    Camera *pstCamera      = NULL;
    Entity *pstPlayer      = NULL;
    Font   *pstFont        = NULL;
    Map    *pstMap         = NULL;
    Sprite *pstSprite      = NULL;
    Video  *pstVideo       = NULL;
    double  dTimeA         = 0.f;

    if (-1 == InitVideo(256, 240, &pstVideo))                  { QUIT(-1); }
    if (-1 == InitCamera(&pstCamera))                          { QUIT(-1); }
    if (-1 == InitEntity(72, 72, &pstPlayer))                  { QUIT(-1); }
    if (-1 == InitMap("res/maps/Demo.tmx", &pstMap))           { QUIT(-1); }
    if (-1 == InitFont("res/ttf/FifteenNarrow.ttf", &pstFont)) { QUIT(-1); }

    SetPosition(16, 592, &pstPlayer);
    SetFrameOffset(0, 2, &pstPlayer);
    SetFontColour(255, 255, 255, &pstFont);

    if (-1 == InitSprite(
            "res/images/characters_7.png", 736, 128, 0, 0,
            &pstSprite, &pstVideo->pstRenderer))
    {
        QUIT(-1);
    }

    InitFPSLimiter(&dTimeA);
    while (1)
    {
        double dTimeB;
        double dDeltaTime;
        LimitFramerate(60, &dTimeA, &dTimeB, &dDeltaTime);

        const uint8_t *pu8KeyState;
        if (-1 == ReadInput(&pu8KeyState) || pu8KeyState[SDL_SCANCODE_Q])
        {
            goto quit;
        }

        CLEAR(pstPlayer->u16Flags, IS_MOVING);

        if (pu8KeyState[SDL_SCANCODE_LEFT])
        {
            SET(pstPlayer->u16Flags, IS_MOVING);
            SET(pstPlayer->u16Flags, IS_FLIPPED);
            SetAnimation(0, 3, &pstPlayer);
            pstPlayer->dPosX -= 75 * dDeltaTime;
        }
        if (pu8KeyState[SDL_SCANCODE_RIGHT])
        {
            SET(pstPlayer->u16Flags, IS_MOVING);
            CLEAR(pstPlayer->u16Flags, IS_FLIPPED);
            SetAnimation(0, 3, &pstPlayer);
            pstPlayer->dPosX += 75 * dDeltaTime;
        }

        SetCameraTargetEntity(
            pstVideo->s32WindowWidth,
            pstVideo->s32WindowHeight,
            pstVideo->dZoomLevel,
            &pstCamera,
            &pstPlayer);

        SetCameraBoundariesToMapSize(
            pstVideo->s32WindowWidth,
            pstVideo->s32WindowHeight,
            pstVideo->dZoomLevel,
            pstMap->u32Width,
            pstMap->u32Height,
            &pstCamera);

        UpdateEntity(dDeltaTime, &pstPlayer);

        if (-1 == RenderMap(
                "res/images/tileset.png", false, 0, "Background",
                pstCamera->dPosX, pstCamera->dPosY, &pstMap, &pstVideo->pstRenderer))
        {
            QUIT(-1);
        }

        RenderEntity(&pstPlayer, &pstCamera, &pstSprite, &pstVideo->pstRenderer);

        if (-1 == RenderMap(
                "res/images/tileset.png", false, 1, "Foreground",
                pstCamera->dPosX, pstCamera->dPosY, &pstMap, &pstVideo->pstRenderer))
        {
            QUIT(-1);
        }

        PrintText("X: ", 8, 4,  &pstVideo->pstRenderer, &pstFont);
        PrintText("Y: ", 8, 20, &pstVideo->pstRenderer, &pstFont);

        PrintNumber((int32_t)round(pstPlayer->dPosX), 24,  4, &pstVideo->pstRenderer, &pstFont);
        PrintNumber((int32_t)round(pstPlayer->dPosY), 24, 20, &pstVideo->pstRenderer, &pstFont);

        RenderScene(&pstVideo->pstRenderer);
    }

quit:
    FreeSprite(&pstSprite);
    FreeFont(&pstFont);
    FreeMap(&pstMap);
    FreeVideo(&pstVideo);
    return sReturnValue;
}

void QuitGame()
{
    SDL_Quit();
}

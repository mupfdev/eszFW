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
#include <stdbool.h>
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
    int     sReturnValue = 0;
    Camera *pstCamera    = NULL;
    Entity *pstPlayer    = NULL;
    Font   *pstFont      = NULL;
    Map    *pstMap       = NULL;
    Object *pstPlrSpawn  = NULL;
    Sprite *pstSprite    = NULL;
    Video  *pstVideo     = NULL;
    bool    bDebug       = true;
    double  dTimeA       = 0.f;

    if (-1 == InitVideo(256, 240, false, &pstVideo))           { QUIT(-1); }
    if (-1 == InitCamera(&pstCamera))                          { QUIT(-1); }
    if (-1 == InitEntity(0, 0, &pstPlayer))                    { QUIT(-1); }
    if (-1 == InitMap("res/maps/Demo.tmx", &pstMap))           { QUIT(-1); }
    if (-1 == InitObject(&pstPlrSpawn))                        { QUIT(-1); }
    if (-1 == InitFont("res/ttf/FifteenNarrow.ttf", &pstFont)) { QUIT(-1); }

    GetSingleObjectByName("Player", &pstMap, &pstPlrSpawn);
    SetPosition(pstPlrSpawn->dPosX, pstPlrSpawn->dPosY, &pstPlayer);
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

        CLEAR(pstPlayer->u16Flags, IS_IN_MID_AIR);
        CLEAR(pstPlayer->u16Flags, IS_WALKING);

        if (pu8KeyState[SDL_SCANCODE_LSHIFT])
        {
            if (pu8KeyState[SDL_SCANCODE_D]) { bDebug = true; }
        }
        else
        {
            if (pu8KeyState[SDL_SCANCODE_D]) { bDebug = false; }
        }

        if (pu8KeyState[SDL_SCANCODE_0])
        {
            SetZoomLevel(pstVideo->dInitialZoomLevel, &pstVideo);
        }
        if (pu8KeyState[SDL_SCANCODE_1])
        {
            SetZoomLevel(pstVideo->dZoomLevel - 0.01, &pstVideo);
        }
        if (pu8KeyState[SDL_SCANCODE_2])
        {
            SetZoomLevel(pstVideo->dZoomLevel + 0.01, &pstVideo);
        }

        if (pu8KeyState[SDL_SCANCODE_LEFT])
        {
            SET(pstPlayer->u16Flags, IS_WALKING);
            SetOrientation(LEFT, &pstPlayer);
            SetAnimation(0, 3, &pstPlayer);
        }
        if (pu8KeyState[SDL_SCANCODE_RIGHT])
        {
            SET(pstPlayer->u16Flags, IS_WALKING);
            SetOrientation(RIGHT, &pstPlayer);
            SetAnimation(0, 3, &pstPlayer);
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
            pstMap->u16Width,
            pstMap->u16Height,
            &pstCamera);

        // Set up collision detection.
        if (false == IsOnPlatform(pstPlayer->dPosX, pstPlayer->dPosY, 18.0, &pstMap))
        {
            SET(pstPlayer->u16Flags, IS_IN_MID_AIR);
        }
        UpdateEntity(dDeltaTime, pstMap->dGravitation, &pstPlayer);

        if (-1 == DrawMap(
                "res/images/tileset.png", false, 0, "BG",
                pstCamera->dPosX, pstCamera->dPosY, &pstMap, &pstVideo->pstRenderer))
        {
            QUIT(-1);
        }

        ConnectMapEndsForEntity(pstMap->u16Width, pstMap->u16Height, &pstPlayer);
        DrawEntity(&pstPlayer, &pstCamera, &pstSprite, &pstVideo->pstRenderer);

        if (-1 == DrawMap(
                "res/images/tileset.png", false, 1, "FG",
                pstCamera->dPosX, pstCamera->dPosY, &pstMap, &pstVideo->pstRenderer))
        {
            QUIT(-1);
        }

        if (bDebug)
        {
            PrintText("X:  ", 8, 4,  &pstVideo->pstRenderer, &pstFont);
            PrintText("Y:  ", 8, 20, &pstVideo->pstRenderer, &pstFont);
            PrintText("HV: ", 8, 36, &pstVideo->pstRenderer, &pstFont);
            PrintText("VV: ", 8, 52, &pstVideo->pstRenderer, &pstFont);
            PrintNumber((int32_t)round(pstPlayer->dPosX),      32,  4, &pstVideo->pstRenderer, &pstFont);
            PrintNumber((int32_t)round(pstPlayer->dPosY),      32, 20, &pstVideo->pstRenderer, &pstFont);
            PrintNumber((int32_t)round(pstPlayer->dVelocityX), 32, 36, &pstVideo->pstRenderer, &pstFont);
            PrintNumber((int32_t)round(pstPlayer->dVelocityY), 32, 52, &pstVideo->pstRenderer, &pstFont);
        }

        RenderScene(&pstVideo->pstRenderer);
    }

quit:
    FreeSprite(&pstSprite);
    FreeFont(&pstFont);
    FreeObject(&pstPlrSpawn);
    FreeMap(&pstMap);
    FreeVideo(&pstVideo);
    return sReturnValue;
}

void QuitGame()
{
    SDL_Quit();
}

/**
 * @file RainbowJoe.c
 * @ingroup RainbowJoe
 * @defgroup RainbowJoe
 * @author Michael Fitzmayer
 * @copyright "THE BEER-WARE LICENCE" (Revision 42)
 */

#define QUIT_FAILURE { sExecStatus = EXIT_FAILURE; goto quit; }
#define QUIT_SUCCESS { sExecStatus = EXIT_SUCCESS; goto quit; }

#include <SDL2/SDL.h>
#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
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
    int     sExecStatus = EXIT_SUCCESS;
    bool    bPause      = false;
    Camera *pstCamera   = NULL;
    Entity *pstPlayer   = NULL;
    Font   *pstFont     = NULL;
    Map    *pstMap      = NULL;
    Object *pstPlrSpawn = NULL;
    Sprite *pstSprite   = NULL;
    Video  *pstVideo    = NULL;
    bool    bDebug      = false;
    double  dTimeA      = 0.f;

    if (-1 == InitVideo(640, 480, 240, false, &pstVideo))      { QUIT_FAILURE; }
    if (-1 == InitCamera(&pstCamera))                          { QUIT_FAILURE; }
    if (-1 == InitEntity(0, 0, &pstPlayer))                    { QUIT_FAILURE; }
    if (-1 == InitMap("res/maps/Demo.tmx", &pstMap))           { QUIT_FAILURE; }
    if (-1 == InitObject(&pstPlrSpawn))                        { QUIT_FAILURE; }
    if (-1 == InitFont("res/ttf/FifteenNarrow.ttf", &pstFont)) { QUIT_FAILURE; }

    GetSingleObjectByName("Player", &pstMap, &pstPlrSpawn);
    SetPosition(pstPlrSpawn->dPosX, pstPlrSpawn->dPosY, &pstPlayer);
    SetFrameOffset(0, 2, &pstPlayer);
    SetFontColour(255, 255, 255, &pstFont);

    if (-1 == InitSprite(
            "res/images/characters_7.png", 736, 128, 0, 0,
            &pstSprite, &pstVideo->pstRenderer))
    {
        QUIT_FAILURE;
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
            QUIT_SUCCESS;
        }

        if (bPause)
        {
            if (pu8KeyState[SDL_SCANCODE_ESCAPE]) { bPause = false; }
            continue;
        }

        ResetEntity(&pstPlayer);

        if (pu8KeyState[SDL_SCANCODE_D])      { bDebug = true; }
        if (pu8KeyState[SDL_SCANCODE_ESCAPE]) { bDebug = false; }
        if (pu8KeyState[SDL_SCANCODE_LEFT])   { Move(LEFT, 8.0, 4.0, 0, 3, &pstPlayer);  }
        if (pu8KeyState[SDL_SCANCODE_RIGHT])  { Move(RIGHT, 8.0, 4.0, 0, 3, &pstPlayer); }
        if (pu8KeyState[SDL_SCANCODE_P])
        {
            PrintText("PAUSE", 8, 4,  &pstVideo->pstRenderer, &pstFont);
            bPause = true;
        }

        // Set zoom level dynamically in relation to vertical velocity.
        if (0.0 < pstPlayer->dVelocityY)
        {
            pstVideo->dZoomLevel -= dDeltaTime / 3.5f;
            if (1.0 > pstVideo->dZoomLevel)
            {
                pstVideo->dZoomLevel = 1;
            }
        }
        else
        {
            pstVideo->dZoomLevel += dDeltaTime / 1.75f;
            if (pstVideo->dZoomLevel > pstVideo->dInitialZoomLevel)
            {
                pstVideo->dZoomLevel = pstVideo->dInitialZoomLevel;
            }
        }
        SetZoomLevel(pstVideo->dZoomLevel, &pstVideo);

        // Follow player entity and set camera boudnaries to map size.
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
            Drop(&pstPlayer);
        }

        UpdateEntity(dDeltaTime, pstMap->dGravitation, &pstPlayer);

        if (-1 == DrawMap(
                "res/images/tileset.png", false, 0, "BG",
                pstCamera->dPosX, pstCamera->dPosY, &pstMap, &pstVideo->pstRenderer))
        {
            QUIT_FAILURE;
        }

        ConnectMapEndsForEntity(pstMap->u16Width, pstMap->u16Height, &pstPlayer);
        DrawEntity(&pstPlayer, &pstCamera, &pstSprite, &pstVideo->pstRenderer);

        if (-1 == DrawMap(
                "res/images/tileset.png", false, 1, "FG",
                pstCamera->dPosX, pstCamera->dPosY, &pstMap, &pstVideo->pstRenderer))
        {
            QUIT_FAILURE;
        }

        if (bDebug && false == bPause)
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
    return sExecStatus;
}

void QuitGame()
{
    SDL_Quit();
}

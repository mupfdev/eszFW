/**
 * @file Main.c
 * @ingroup RainbowJoe
 * @defgroup RainbowJoe
 * @author Michael Fitzmayer
 * @copyright "THE BEER-WARE LICENCE" (Revision 42)
 */

#define QUIT_FAILURE { sExecStatus = -1; goto quit; }
#define QUIT_SUCCESS { sExecStatus =  0; goto quit; }

#ifdef __ANDROID__
#include <SDL.h>
#else
#include <SDL2/SDL.h>
#endif
#include <eszFW.h>
#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "Main.h"

int RainbowJoe(Video **pstVideo)
{
    int         sExecStatus   = 0;
    Background *pstBackground = NULL;
    Camera     *pstCamera     = NULL;
    Entity     *pstPlayer     = NULL;
    Font       *pstFont       = NULL;
    Map        *pstMap        = NULL;
    Object     *pstPlrSpawn   = NULL;
    Sprite     *pstSprite     = NULL;
    Touch      *pstTouch      = NULL;
    bool        bDebug        = false;
    bool        bPause        = false;
    double      dTimeA        = 0.f;

    const char *pacBgFileNames[4] =
        {
            "res/images/sky.png",
            "res/images/clouds.png",
            "res/images/sea.png",
            "res/images/far-grounds.png",
        };

    if (-1 == InitCamera(&pstCamera))                          { QUIT_FAILURE; }
    if (-1 == InitEntity(0, 0, &pstPlayer))                    { QUIT_FAILURE; }
    if (-1 == InitMap("res/maps/Demo.tmx", 24, &pstMap))       { QUIT_FAILURE; }
    if (-1 == InitObject(&pstPlrSpawn))                        { QUIT_FAILURE; }
    if (-1 == InitFont("res/ttf/FifteenNarrow.ttf", &pstFont)) { QUIT_FAILURE; }
    if (-1 == InitBackground(
            4,
            pacBgFileNames,
            (*pstVideo)->s32WindowWidth,
            BOTTOM,
            &(*pstVideo)->pstRenderer,
            &pstBackground))
    {
        QUIT_FAILURE;
    }

    if (-1 == InitSprite(
            "res/images/characters_7.png", 736, 128, 0, 0,
            &pstSprite, &(*pstVideo)->pstRenderer))
    {
        QUIT_FAILURE;
    }

    if (-1 == InitTouch((*pstVideo)->s32WindowWidth, (*pstVideo)->s32WindowHeight, &pstTouch))
    {
        QUIT_FAILURE;
    }

    GetSingleObjectByName("Player", &pstMap, &pstPlrSpawn);
    SetPosition(pstPlrSpawn->dPosX, pstPlrSpawn->dPosY, &pstPlayer);
    SetFrameOffset(0, 2, &pstPlayer);
    SetFontColour(255, 0, 0, &pstFont);

    InitFPSLimiter(&dTimeA);
    while (1)
    {
        double dTimeB;
        double dDeltaTime;
        LimitFramerate(60, &dTimeA, &dTimeB, &dDeltaTime);

        const uint8_t *pu8KeyState;
        if (-1 == ReadInput(&pu8KeyState) || pu8KeyState[SDL_SCANCODE_Q] || pu8KeyState[SDL_SCANCODE_AC_BACK])
        {
            QUIT_SUCCESS;
        }

        if (bPause)
        {
            if (pu8KeyState[SDL_SCANCODE_ESCAPE]) { bPause = false; }
            continue;
        }

        ResetEntity(&pstPlayer);
        GetTouchPosition(&pstTouch);
        SetCameraLock(false, &pstCamera);

        if (pu8KeyState[SDL_SCANCODE_D])      { bDebug = true;  }
        if (pu8KeyState[SDL_SCANCODE_ESCAPE]) { bDebug = false; }
        if (pu8KeyState[SDL_SCANCODE_P])      { bPause = true;  }
        if (pu8KeyState[SDL_SCANCODE_LSHIFT]) { SetCameraLock(true, &pstCamera); }

        if (IsCameraLocked(&pstCamera))
        {
            if (pu8KeyState[SDL_SCANCODE_UP])    { pstCamera->dPosY -= 5.0f; }
            if (pu8KeyState[SDL_SCANCODE_DOWN])  { pstCamera->dPosY += 5.0f; }
            if (pu8KeyState[SDL_SCANCODE_LEFT])  { pstCamera->dPosX -= 5.0f; }
            if (pu8KeyState[SDL_SCANCODE_RIGHT]) { pstCamera->dPosX += 5.0f; }
        }
        else
        {
            if (pu8KeyState[SDL_SCANCODE_LEFT])  { Move(LEFT , 6.0, 3.0, 0, 3, &pstPlayer); }
            if (pu8KeyState[SDL_SCANCODE_RIGHT]) { Move(RIGHT, 6.0, 3.0, 0, 3, &pstPlayer); }
        }

        // Rudimentary touch controls for testing purposes only.
        if (pstTouch->s32PosX != 0)
        {
            if (pstTouch->s32PosX < ((*pstVideo)->s32WindowWidth / 3))
            {
                Move(LEFT,  6.0, 3.0, 0, 3, &pstPlayer);
            }
            else if (pstTouch->s32PosX >= (((*pstVideo)->s32WindowWidth / 3) * 2))
            {
                Move(RIGHT, 6.0, 3.0, 0, 3, &pstPlayer);
            }
        }

        // Follow player entity and set camera boudnaries to map size.
        SetCameraTargetEntity(
            (*pstVideo)->s32LogicalWindowWidth,
            (*pstVideo)->s32LogicalWindowHeight,
            &pstCamera,
            &pstPlayer);

        SetCameraBoundariesToMapSize(
            (*pstVideo)->s32LogicalWindowWidth,
            (*pstVideo)->s32LogicalWindowHeight,
            pstMap->u16Width,
            pstMap->u16Height,
            &pstCamera);

        // Set zoom level dynamically in relation to vertical velocity.
        #ifndef __ANDROID__
        if (0.0 < pstPlayer->dVelocityY)
        {
            (*pstVideo)->dZoomLevel -= dDeltaTime / 3.5f;
            if (1.0 > (*pstVideo)->dZoomLevel)
            {
                (*pstVideo)->dZoomLevel = 1;
            }
        }
        else
        {
            (*pstVideo)->dZoomLevel += dDeltaTime / 1.75f;
            if ((*pstVideo)->dZoomLevel > (*pstVideo)->dInitialZoomLevel)
            {
                (*pstVideo)->dZoomLevel = (*pstVideo)->dInitialZoomLevel;
            }
        }
        SetZoomLevel((*pstVideo)->dZoomLevel, &(*pstVideo));
        #endif

        // Set up collision detection.
        if (false == IsOnTileOfType(
                "Platform",
                pstPlayer->dPosX,
                pstPlayer->dPosY,
                18.0,
                &pstMap))
        {
            Drop(&pstPlayer);
        }

        UpdateEntity(dDeltaTime, pstMap->dGravitation, pstMap->u8MeterInPixel, &pstPlayer);

        if (-1 == DrawBackground(
                pstPlayer->bOrientation,
                (*pstVideo)->s32LogicalWindowHeight,
                pstCamera->dPosY,
                pstPlayer->dVelocityX,
                &(*pstVideo)->pstRenderer,
                &pstBackground))
        {
            QUIT_FAILURE;
        }

        if (-1 == DrawMap(
                0, "res/images/tileset.png", true, "BG",
                pstCamera->dPosX, pstCamera->dPosY,
                &pstMap, &(*pstVideo)->pstRenderer))
        {
            QUIT_FAILURE;
        }

        ConnectMapEndsForEntity(pstMap->u16Width, pstMap->u16Height, &pstPlayer);
        DrawEntity(&pstPlayer, &pstCamera, &pstSprite, &(*pstVideo)->pstRenderer);

        if (-1 == DrawMap(
                1, "res/images/tileset.png", false, "FG",
                pstCamera->dPosX, pstCamera->dPosY,
                &pstMap, &(*pstVideo)->pstRenderer))
        {
            QUIT_FAILURE;
        }

        if (bDebug)
        {
            PrintText("X:  ", 8, 4,  &(*pstVideo)->pstRenderer, &pstFont);
            PrintText("Y:  ", 8, 20, &(*pstVideo)->pstRenderer, &pstFont);
            PrintText("HV: ", 8, 36, &(*pstVideo)->pstRenderer, &pstFont);
            PrintText("VV: ", 8, 52, &(*pstVideo)->pstRenderer, &pstFont);
            PrintNumber((int32_t)round(pstPlayer->dPosX),      32,  4, &(*pstVideo)->pstRenderer, &pstFont);
            PrintNumber((int32_t)round(pstPlayer->dPosY),      32, 20, &(*pstVideo)->pstRenderer, &pstFont);
            PrintNumber((int32_t)round(pstPlayer->dVelocityX), 32, 36, &(*pstVideo)->pstRenderer, &pstFont);
            PrintNumber((int32_t)round(pstPlayer->dVelocityY), 32, 52, &(*pstVideo)->pstRenderer, &pstFont);
        }

        RenderScene(&(*pstVideo)->pstRenderer);
    }

quit:
    FreeTouch(&pstTouch);
    FreeSprite(&pstSprite);
    FreeBackground(&pstBackground);
    FreeFont(&pstFont);
    FreeObject(&pstPlrSpawn);
    FreeMap(&pstMap);
    return sExecStatus;
}

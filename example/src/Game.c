/**
 * @file Game.c
 * @ingroup Game
 * @defgroup Game
 * @author Michael Fitzmayer
 * @copyright "THE BEER-WARE LICENCE" (Revision 42)
 */

#ifdef __ANDROID__
#include <SDL.h>
#else
#include <SDL2/SDL.h>
#endif
#include <eszfw.h>
#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include "Game.h"
#include "Resources.h"
#include "Render.h"
#include "World.h"

int SDL_main(int sArgC, char *pacArgV[])
{
    int        sReturnValue = 0;
    bool       bIsRunning   = true;
    Resources *pstRes       = NULL;
    double     dTimeA       = 0.f;
    double     dTimeB       = 0.f;
    double     dDeltaTime   = 0.f;
    bool       bOrientation = LEFT;
    bool       bIsMoving    = false;
    SDL_Event  stEvent;

    (void)sArgC;
    (void)pacArgV;

    sReturnValue = InitResources(&pstRes);
    if (-1 == sReturnValue) { bIsRunning = false; };

    #ifdef __ANDROID__
    double  dZoomLevel    = 0.f;
    double  dZoomLevelMin = (double)pstRes->pstVideo->s32WindowHeight / (double)pstRes->pstMap->u16Height;
    double  dZoomLevelMax = pstRes->pstVideo->dInitialZoomLevel;
    int32_t s32TouchPosX  = 0;
    int32_t s32WindowW    = pstRes->pstVideo->s32LogicalWindowWidth;
    #endif

    InitFPSLimiter(&dTimeA, &dTimeB, &dDeltaTime);
    while (bIsRunning)
    {
        // Limit framerate.
        LimitFramerate(60, &dTimeA, &dTimeB, &dDeltaTime);

        // Render scene.
        sReturnValue = Render(&pstRes);

        if (-1 == sReturnValue)
        {
            bIsRunning = false;
            continue;
        };

        // Reset entity flags.
        ResetEntity(&pstRes->pstEntity[0]);

        // Handle events.
        while(SDL_PollEvent(&stEvent) != 0)
        {
            if (stEvent.type == SDL_QUIT )
            {
                bIsRunning = false;
            }
            #ifndef __ANDROID__
            else if (SDL_KEYDOWN == stEvent.type)
            {
                switch(stEvent.key.keysym.sym)
                {
                  case SDLK_q:
                      bIsRunning = false;
                      break;
                  case SDLK_LEFT:
                      bOrientation = LEFT;
                      bIsMoving    = true;
                      break;
                  case SDLK_RIGHT:
                      bOrientation = RIGHT;
                      bIsMoving    = true;
                      break;
                }
            }
            else if (SDL_KEYUP == stEvent.type)
            {
                switch(stEvent.key.keysym.sym)
                {
                  case SDLK_LEFT:
                      bIsMoving = false;
                      break;
                  case SDLK_RIGHT:
                      bIsMoving = false;
                      break;
                }
            }
            #else // __ANDROID__
            else if (SDL_KEYDOWN == stEvent.type)
            {
                if (stEvent.key.keysym.sym == SDLK_AC_BACK)
                {
                    bIsRunning = false;
                }
            }
            else if (SDL_FINGERDOWN == stEvent.type)
            {
                s32TouchPosX = (int32_t)round(stEvent.tfinger.x * s32WindowW);

                if (s32TouchPosX < (s32WindowW / 2))
                {
                    bOrientation = LEFT;
                    bIsMoving    = true;
                }
                else
                {
                    bOrientation = RIGHT;
                    bIsMoving    = true;
                }
            }
            else if (SDL_FINGERUP == stEvent.type)
            {
                bIsMoving = false;
            }
            else if (SDL_MULTIGESTURE == stEvent.type)
            {
                bIsMoving  = false;
                dZoomLevel = pstRes->pstVideo->dZoomLevel;
                if (0.002 < fabs(stEvent.mgesture.dDist))
                {
                    if (0 < stEvent.mgesture.dDist)
                    {
                        // Pinch open.
                        dZoomLevel += 5.f * dDeltaTime;
                    }
                    else
                    {
                        // Pinch close.
                        dZoomLevel -= 5.f * dDeltaTime;
                    }

                    if (dZoomLevel <= dZoomLevelMin)
                    {
                        dZoomLevel = dZoomLevelMin;
                    }
                    if (dZoomLevel >= dZoomLevelMax)
                    {
                        dZoomLevel = dZoomLevelMax;
                    }

                    SetZoomLevel(dZoomLevel, &pstRes->pstVideo);
                }
            }
            #endif // __ANDROID__
        }

        // Set the player's idle animation.
        if (! IsEntityMoving(&pstRes->pstEntity[0]))
        {
            AnimateEntity(true, &pstRes->pstEntity[0]);
            SetFrameOffset(0, 0, &pstRes->pstEntity[0]);
            SetAnimation(
                0, 11,
                pstRes->pstEntity[0]->dAnimSpeed,
                &pstRes->pstEntity[0]);
        }

        // Move player entity.
        if (bIsMoving)
        {
            AnimateEntity(true, &pstRes->pstEntity[0]);
            MoveEntity(
                bOrientation, 6.0, 3.0, 1, 7,
                pstRes->pstEntity[0]->dAnimSpeed,
                1,
                &pstRes->pstEntity[0]);
        }

        // Unlock camera.
        SetCameraLock(false, &pstRes->pstCamera);

        // Update game logic.
        UpdateWorld(dDeltaTime, &pstRes);
    }

    FreeResources(&pstRes);
    return sReturnValue;
}

void QuitGame()
{
    SDL_Quit();
}

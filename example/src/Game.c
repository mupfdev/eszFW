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
#include <stdbool.h>
#include <stdint.h>
#include "Controls.h"
#include "Game.h"
#include "Resources.h"
#include "Render.h"
#include "World.h"

int SDL_main(int sArgC, char *pacArgV[])
{
    int        sReturnValue  = 0;
    bool       bIsRunning    = true;
    Resources *pstResources = NULL;
    bool       bPause        = false;
    double     dTimeA        = 0.f;
    double     dTimeB        = 0.f;
    double     dDeltaTime    = 0.f;

    (void)sArgC;
    (void)pacArgV;

    sReturnValue = InitResources(&pstResources);
    if (-1 == sReturnValue) { bIsRunning = false; };

    InitFPSLimiterTest(&dTimeA, &dTimeB, &dDeltaTime);
    while (bIsRunning)
    {
        //double dTimeB;
        //double dDeltaTime;
        //LimitFramerate(60, &dTimeA, &dTimeB, &dDeltaTime);

        // Set the player's idle animation.
        ResetEntity(&pstResources->pstEntity[ENT_PLAYER]);
        AnimateEntity(true, &pstResources->pstEntity[ENT_PLAYER]);
        SetFrameOffset(0, 0, &pstResources->pstEntity[ENT_PLAYER]);
        SetAnimation(
            0, 11,
            pstResources->pstEntity[ENT_PLAYER]->dAnimSpeed,
            &pstResources->pstEntity[ENT_PLAYER]);

        SetCameraLock(false, &pstResources->pstCamera);
        UpdateInput(
            pstResources->pstCamera->dPosX,
            pstResources->pstCamera->dPosY,
            &pstResources->pstInput);

        switch (UpdateControls(bPause, &pstResources))
        {
          case 1:
              bIsRunning = false;
              break;
          case 2:
              bPause = false;
              break;
          case 3:
              bPause = true;
              break;
        }

        if (bPause) { continue; }

        UpdateWorld(dDeltaTime, &pstResources);
        sReturnValue = Render(&pstResources);

        if (-1 == sReturnValue)
        {
            bIsRunning = false;
            continue;
        };

        LimitFramerate(60, &dTimeA, &dTimeB, &dDeltaTime);
    }

    FreeResources(&pstResources);
    return sReturnValue;
}

void QuitGame()
{
    SDL_Quit();
}

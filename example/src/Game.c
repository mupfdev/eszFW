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
#include "GameData.h"
#include "Render.h"
#include "WorldLogic.h"

int SDL_main(int sArgC, char *pacArgV[])
{
    int       sReturnValue = 0;
    bool      bIsRunning   = true;
    GameData *pstGameData  = NULL;
    bool      bPause       = false;
    double    dTimeA       = 0.f;

    (void)sArgC;
    (void)pacArgV;

    sReturnValue = InitGameData(&pstGameData);
    if (-1 == sReturnValue) { bIsRunning = false; };

    InitFPSLimiter(&dTimeA);
    while (bIsRunning)
    {
        double         dTimeB;
        double         dDeltaTime;
        LimitFramerate(60, &dTimeA, &dTimeB, &dDeltaTime);

        UpdateWorldStart(&pstGameData);
        UpdateInput(&pstGameData->pstInput);

        switch (UpdateControls(bPause, &pstGameData))
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

        UpdateWorldEnd(dDeltaTime, &pstGameData);
        sReturnValue = Render(&pstGameData);

        if (-1 == sReturnValue)
        {
            bIsRunning = false;
            continue;
        };
    }

    FreeGameData(&pstGameData);
    return sReturnValue;
}

void QuitGame()
{
    SDL_Quit();
}

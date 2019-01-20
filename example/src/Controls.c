/**
 * @file Controls.c
 * @ingroup Controls
 * @defgroup Controls
 * @author Michael Fitzmayer
 * @copyright "THE BEER-WARE LICENCE" (Revision 42)
 */

#ifdef __ANDROID__
#include <SDL.h>
#else
#include <SDL2/SDL.h>
#endif
#include <eszFW.h>
#include <stdbool.h>
#include "Controls.h"
#include "Resources.h"

int UpdateControls(const bool bPause, Resources **pstResources)
{
    // Ignore all keys except ESC when game is paused.
    if (bPause)
    {
        if (IsKeyPressed(SDL_SCANCODE_ESCAPE, &(*pstResources)->pstInput)) { return 2; }
        return 0;
    }

    if (IsKeyPressed(SDL_SCANCODE_Q, &(*pstResources)->pstInput))       { return 1; }
    if (IsKeyPressed(SDL_SCANCODE_AC_BACK, &(*pstResources)->pstInput)) { return 1; }
    if (IsKeyPressed(SDL_SCANCODE_P, &(*pstResources)->pstInput))       { return 3; }

    if (IsKeyPressed(SDL_SCANCODE_LSHIFT, &(*pstResources)->pstInput))
    {
        SetCameraLock(true, &(*pstResources)->pstCamera);
    }

    // Unlocked camera controls.
    if (IsCameraLocked(&(*pstResources)->pstCamera))
    {
        double dPosX = 0.f;
        double dPosY = 0.f;

        if (IsKeyPressed(SDL_SCANCODE_UP, &(*pstResources)->pstInput))    { dPosY -= 5.f; }
        if (IsKeyPressed(SDL_SCANCODE_DOWN, &(*pstResources)->pstInput))  { dPosY += 5.f; }
        if (IsKeyPressed(SDL_SCANCODE_LEFT, &(*pstResources)->pstInput))  { dPosX -= 5.f; }
        if (IsKeyPressed(SDL_SCANCODE_RIGHT, &(*pstResources)->pstInput)) { dPosX += 5.f; }

        (*pstResources)->pstCamera->dPosX += dPosX;
        (*pstResources)->pstCamera->dPosY += dPosY;
    }
    else
    {
        bool bMovePlayer  = false;
        bool bOrientation = LEFT;

        // Player controls.
        if (IsKeyPressed(SDL_SCANCODE_LEFT, &(*pstResources)->pstInput))
        {
            bMovePlayer  = true;
            bOrientation = LEFT;
        }
        if (IsKeyPressed(SDL_SCANCODE_RIGHT, &(*pstResources)->pstInput))
        {
            bMovePlayer  = true;
            bOrientation = RIGHT;
        }
        // Experimental touchscreen controls.
        #ifdef __ANDROID__
        double s32TouchDispPosX   = (*pstResources)->pstInput->s32TouchDispPosX;
        double s32HalfWindowWidth = (*pstResources)->pstVideo->s32LogicalWindowWidth / 2;

        if (s32TouchDispPosX < s32HalfWindowWidth) { bOrientation = LEFT;  }
        else                                       { bOrientation = RIGHT; }

        if (! BoxesDoIntersect(
                (*pstResources)->pstEntity[ENT_PLAYER]->stBB,
                (*pstResources)->pstInput->stTouchBB))
        {
            // Prevent player from running at start.
            if (0 != (*pstResources)->pstInput->s32TouchDispPosX) { bMovePlayer = true;  }
            else                                                  { bMovePlayer = false; }
        }
        #endif // __ANDROID__

        if (bMovePlayer)
        {
            AnimateEntity(true, &(*pstResources)->pstEntity[ENT_PLAYER]);
            MoveEntity(
                bOrientation, 6.0, 3.0, 1, 7,
                (*pstResources)->pstEntity[ENT_PLAYER]->dAnimSpeed,
                1,
                &(*pstResources)->pstEntity[ENT_PLAYER]);
        }
    }

    return 0;
}

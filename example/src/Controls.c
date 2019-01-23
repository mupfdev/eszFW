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

int UpdateControls(Resources **pstResources)
{
    if (IsKeyPressed(SDL_SCANCODE_Q, &(*pstResources)->pstInput))       { return 0; }
    if (IsKeyPressed(SDL_SCANCODE_AC_BACK, &(*pstResources)->pstInput)) { return 0; }

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
        int32_t s32TouchPosX       = (*pstResources)->pstInput->s32TouchDispPosX;
        int32_t s32HalfWindowWidth = (*pstResources)->pstVideo->s32LogicalWindowWidth / 2;

        if (s32TouchPosX < s32HalfWindowWidth)
        {
            bOrientation = LEFT;
        }
        else
        {
            bOrientation = RIGHT;
        }
        // Halt when player entity intersect touch position.
        if (! BoxesDoIntersect(
                (*pstResources)->pstEntity[ENT_PLAYER]->stBB,
                (*pstResources)->pstInput->stTouchBB))
        {
            if (0 != s32TouchPosX)
            {
                bMovePlayer = true;
            }
            else
            {
                bMovePlayer = false;
            }
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

    return 1;
}

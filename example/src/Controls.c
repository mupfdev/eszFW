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
#include "GameData.h"

int UpdateControls(const bool bPause, GameData **pstGameData)
{
    // Ignore all keys except ESC when game is paused.
    if (bPause)
    {
        if (IsKeyPressed(SDL_SCANCODE_ESCAPE, &(*pstGameData)->pstInput)) { return 2; }
        return 0;
    }

    if (IsKeyPressed(SDL_SCANCODE_Q, &(*pstGameData)->pstInput))       { return 1; }
    if (IsKeyPressed(SDL_SCANCODE_AC_BACK, &(*pstGameData)->pstInput)) { return 1; }
    if (IsKeyPressed(SDL_SCANCODE_P, &(*pstGameData)->pstInput))       { return 3; }

    if (IsKeyPressed(SDL_SCANCODE_LSHIFT, &(*pstGameData)->pstInput))
    {
        SetCameraLock(true, &(*pstGameData)->pstCamera);
    }

    // Unlocked camera controls.
    if (IsCameraLocked(&(*pstGameData)->pstCamera))
    {
        if (IsKeyPressed(SDL_SCANCODE_UP, &(*pstGameData)->pstInput))
        {
            (*pstGameData)->pstCamera->dPosY -= 5.0f;
        }
        if (IsKeyPressed(SDL_SCANCODE_DOWN, &(*pstGameData)->pstInput))
        {
            (*pstGameData)->pstCamera->dPosY += 5.0f;
        }
        if (IsKeyPressed(SDL_SCANCODE_LEFT, &(*pstGameData)->pstInput))
        {
            (*pstGameData)->pstCamera->dPosX -= 5.0f;
        }
        if (IsKeyPressed(SDL_SCANCODE_RIGHT, &(*pstGameData)->pstInput)) {
            (*pstGameData)->pstCamera->dPosX += 5.0f;
        }
    }
    else
    {
        // Player controls.
        #ifdef __ANDROID__
        if (((*pstGameData)->pstVideo->s32WindowWidth / 3) >= GetTouchPosX(&(*pstGameData)->pstInput))
        #else
        if (IsKeyPressed(SDL_SCANCODE_LEFT, &(*pstGameData)->pstInput))
        #endif
        {
            AnimateEntity(true, &(*pstGameData)->pstEntity[ENT_PLAYER]);
            MoveEntity(
                    LEFT, 6.0, 3.0, 1, 7,
                    (*pstGameData)->pstEntity[ENT_PLAYER]->dAnimSpeed,
                    1,
                    &(*pstGameData)->pstEntity[ENT_PLAYER]);
        }
        #ifdef __ANDROID__
        if (((*pstGameData)->pstVideo->s32WindowWidth / 3) * 2 <= GetTouchPosX(&(*pstGameData)->pstInput))
        #else
        if (IsKeyPressed(SDL_SCANCODE_RIGHT, &(*pstGameData)->pstInput))
        #endif
        {
            AnimateEntity(true, &(*pstGameData)->pstEntity[ENT_PLAYER]);
            MoveEntity(
                RIGHT, 6.0, 3.0, 0, 7,
                (*pstGameData)->pstEntity[ENT_PLAYER]->dAnimSpeed,
                1,
                &(*pstGameData)->pstEntity[ENT_PLAYER]);
        }
    }

    return 0;
}

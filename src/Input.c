/**
 * @file Input.c
 * @ingroup Input
 * @defgroup Input
 * @author Michael Fitzmayer
 * @copyright "THE BEER-WARE LICENCE" (Revision 42)
 */

#ifdef __ANDROID__
#include <SDL.h>
#else
#include <SDL2/SDL.h>
#endif
#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include "AABB.h"
#include "Input.h"

void FreeInput(Input **pstInput)
{
    free(*pstInput);
}

int InitInput(
    const int32_t s32WindowWidth,
    const int32_t s32WindowHeight,
    Input **pstInput)
{
    *pstInput = NULL;
    *pstInput = malloc(sizeof(struct Input_t));
    if (! *pstInput)
    {
        SDL_LogError(
            SDL_LOG_CATEGORY_APPLICATION,
            "InitInput(): error allocating memory.\n");
        return -1;
    }

    (*pstInput)->pu8KeyState       = SDL_GetKeyboardState(NULL);
    (*pstInput)->s32WindowWidth    = s32WindowWidth;
    (*pstInput)->s32WindowHeight   = s32WindowHeight;
    (*pstInput)->s32TouchDispPosX  = 0;
    (*pstInput)->s32TouchDispPosY  = 0;
    (*pstInput)->s32TouchPosX      = 0;
    (*pstInput)->s32TouchPosY      = 0;
    (*pstInput)->stTouchBB.dBottom = 0.f;
    (*pstInput)->stTouchBB.dLeft   = 0.f;
    (*pstInput)->stTouchBB.dRight  = 0.f;
    (*pstInput)->stTouchBB.dTop    = 0.f;
    (*pstInput)->u32TouchType      = 0;

    return 0;
}

bool IsKeyPressed(const uint16_t u16Scancode, Input **pstInput)
{
    if ((*pstInput)->pu8KeyState[u16Scancode])
    {
        return true;
    }
    else
    {
        return false;
    }
}

bool UpdateInput(
    const double dCameraPosX,
    const double dCameraPosY,
    Input      **pstInput)
{
    SDL_Event stEvent;

    SDL_PollEvent(&stEvent);
    SDL_PumpEvents();

    if (0 < SDL_PeepEvents(0, 0, SDL_PEEKEVENT, SDL_QUIT, SDL_QUIT))
    {
        return true;
    }

    (*pstInput)->u32TouchType = stEvent.type;

    if (SDL_FINGERDOWN   == stEvent.type ||
        SDL_FINGERUP     == stEvent.type ||
        SDL_FINGERMOTION == stEvent.type)
    {
        int32_t s32PosX = (int32_t)round(stEvent.tfinger.x * (*pstInput)->s32WindowWidth);
        int32_t s32PosY = (int32_t)round(stEvent.tfinger.y * (*pstInput)->s32WindowHeight);

        (*pstInput)->s32TouchDispPosX = s32PosX;
        (*pstInput)->s32TouchDispPosY = s32PosY;
        (*pstInput)->s32TouchPosX     = s32PosX + dCameraPosX;
        (*pstInput)->s32TouchPosY     = s32PosY + dCameraPosY;

        // Update axis-aligned bounding box.
        (*pstInput)->stTouchBB.dBottom = (*pstInput)->s32TouchPosY + 8.f;
        (*pstInput)->stTouchBB.dLeft   = (*pstInput)->s32TouchPosX - 8.f;
        (*pstInput)->stTouchBB.dRight  = (*pstInput)->s32TouchPosX + 8.f;
        (*pstInput)->stTouchBB.dTop    = (*pstInput)->s32TouchPosY - 8.f;
    }

    return false;
}

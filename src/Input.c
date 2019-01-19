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
#include "Input.h"

void FreeInput(Input **pstInput)
{
    free(*pstInput);
}

int32_t GetTouchPosX(Input **pstInput)
{
    return (*pstInput)->s32TouchPosX;
}

int32_t GetTouchPosY(Input **pstInput)
{
    return (*pstInput)->s32TouchPosY;
}

int InitInput(
    int32_t s32WindowWidth,
    int32_t s32WindowHeight,
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
    (*pstInput)->s32TouchPosX      = s32WindowWidth  / 2;
    (*pstInput)->s32TouchPosY      = s32WindowHeight / 2;
    (*pstInput)->u32TouchType      = 0;
    (*pstInput)->stTouchBB.dBottom = 0;
    (*pstInput)->stTouchBB.dLeft   = 0;
    (*pstInput)->stTouchBB.dRight  = 0;
    (*pstInput)->stTouchBB.dTop    = 0;

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

bool UpdateInput(Input **pstInput)
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
        (*pstInput)->s32TouchPosX = (int32_t)round(stEvent.tfinger.x * (*pstInput)->s32WindowWidth);
        (*pstInput)->s32TouchPosY = (int32_t)round(stEvent.tfinger.y * (*pstInput)->s32WindowHeight);
    }

    // Update axis-aligned bounding box.
    (*pstInput)->stTouchBB.dBottom = (*pstInput)->s32TouchPosY + 32;
    (*pstInput)->stTouchBB.dLeft   = (*pstInput)->s32TouchPosX - 32;
    (*pstInput)->stTouchBB.dRight  = (*pstInput)->s32TouchPosX + 32;
    (*pstInput)->stTouchBB.dTop    = (*pstInput)->s32TouchPosY - 32;

    return false;
}

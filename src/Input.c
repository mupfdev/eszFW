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

void FreeTouch(Touch **pstTouch)
{
    free(*pstTouch);
}

int InitTouch(
    int32_t s32WindowWidth,
    int32_t s32WindowHeight,
    Touch **pstTouch)
{
    *pstTouch = NULL;
    *pstTouch = malloc(sizeof(struct Touch_t));
    if (NULL == *pstTouch)
    {
        SDL_LogError(
            SDL_LOG_CATEGORY_APPLICATION,
            "InitTouch(): error allocating memory.\n");
        return -1;
    }

    (*pstTouch)->s32WindowWidth  = s32WindowWidth;
    (*pstTouch)->s32WindowHeight = s32WindowHeight;
    (*pstTouch)->s32PosX         = 0;
    (*pstTouch)->s32PosY         = 0;

    return 0;
}

int ReadInput(const uint8_t **pu8KeyState)
{
    SDL_PumpEvents();
    *pu8KeyState = SDL_GetKeyboardState(NULL);

    if (0 < SDL_PeepEvents(0, 0, SDL_PEEKEVENT, SDL_QUIT, SDL_QUIT))
    {
        return -1;
    }

    return 0;
}

void GetTouchPosition(Touch **pstTouch)
{
    SDL_Event stEvent;
    SDL_PollEvent(&stEvent);

    if(SDL_FINGERDOWN == stEvent.type || SDL_FINGERMOTION == stEvent.type)
    {
        (*pstTouch)->s32PosX = (int32_t)round(stEvent.tfinger.x * (*pstTouch)->s32WindowWidth);
        (*pstTouch)->s32PosY = (int32_t)round(stEvent.tfinger.y * (*pstTouch)->s32WindowHeight);
    }
}

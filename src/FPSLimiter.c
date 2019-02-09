/**
 * @file FPSLimiter.c
 * @ingroup FPSLimiter
 * @defgroup FPSLimiter
 * @author Michael Fitzmayer
 * @copyright "THE BEER-WARE LICENCE" (Revision 42)
 */

#include <SDL.h>
#include "FPSLimiter.h"

void InitFPSLimiter(double *dTimeA, double *dTimeB, double *dDeltaTime)
{
    SDL_Log("Initialise frame-rate limiter.\n");
    *dTimeA     = SDL_GetTicks();
    *dTimeB     = SDL_GetTicks();
    *dDeltaTime = (*dTimeB - *dTimeA) / 1000.f;
}

void LimitFramerate(const Uint16 u16FPS, double *dTimeA, double *dTimeB, double *dDeltaTime)
{
    *dTimeB     = SDL_GetTicks();
    *dDeltaTime = (*dTimeB - *dTimeA) / 1000.f;
    *dTimeA     = *dTimeB;

    if (u16FPS > 0)
    {
        SDL_Delay(1000.0f / (double)u16FPS - *dDeltaTime);
    }
}

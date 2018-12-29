/**
 * @file FPSLimiter.c
 * @ingroup FPSLimiter
 * @defgroup FPSLimiter
 * @author Michael Fitzmayer
 * @copyright "THE BEER-WARE LICENCE" (Revision 42)
 */

#include <SDL2/SDL.h>
#include <stdint.h>
#include "FPSLimiter.h"

void InitFPSLimiter(double *dTimeA)
{
    SDL_Log("Initialise frame-rate limiter.\n");
    *dTimeA = SDL_GetTicks();
}

void LimitFramerate(
    const uint16_t u16FPS,
    double *dTimeA,
    double *dTimeB,
    double *dDeltaTime
)
{
    *dTimeB     = SDL_GetTicks();
    *dDeltaTime = (*dTimeB - *dTimeA) / 1000.0f;
    *dTimeA     = *dTimeB;
    if (0 < u16FPS)
    {
        SDL_Delay(1000.0f / (double)u16FPS - *dDeltaTime);
    }
}

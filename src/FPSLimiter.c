/**
 * @file FPSLimiter.c
 * @ingroup FPSLimiter
 * @defgroup FPSLimiter
 * @author Michael Fitzmayer
 * @copyright "THE BEER-WARE LICENCE" (Revision 42)
 */

#ifdef __ANDROID__
#include <SDL.h>
#else
#include <SDL2/SDL.h>
#endif
#include <stdint.h>
#include "FPSLimiter.h"

void InitFPSLimiterTest(
    double *dTimeA,
    double *dTimeB,
    double *dDeltaTime)
{
    SDL_Log("Initialise frame-rate limiter.\n");
    *dTimeA = SDL_GetTicks();
    *dTimeB     = SDL_GetTicks();
    *dDeltaTime = (*dTimeB - *dTimeA) / 1000.f;
}

void InitFPSLimiter(double *dTimeA)
{
    SDL_Log("Initialise frame-rate limiter.\n");
    *dTimeA = SDL_GetTicks();
}

void LimitFramerate(
    const uint16_t u16FPS,
    double        *dTimeA,
    double        *dTimeB,
    double        *dDeltaTime
)
{
    *dTimeB     = SDL_GetTicks();
    *dDeltaTime = (*dTimeB - *dTimeA) / 1000.f;
    *dTimeA     = *dTimeB;

    SDL_Delay(1000.0f / (double)u16FPS - *dDeltaTime);
}

/**
 * @file FPSLimiter.h
 * @ingroup FPSLimiter
 */

#ifndef _FPS_LIMITER_H_
#define _FPS_LIMITER_H_

#include <stdint.h>

void InitFPSLimiter(
    double *dTimeA,
    double *dTimeB,
    double *dDeltaTime);

void LimitFramerate(
    const uint16_t u16FPS,
    double        *dTimeA,
    double        *dTimeB,
    double        *dDeltaTime);

#endif // _FPS_LIMITER_H_

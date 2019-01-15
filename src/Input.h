/**
 * @file Input.h
 * @ingroup Input
 */

#ifndef _INPUT_H_
#define _INPUT_H_

#include <stdbool.h>
#include <stdint.h>

typedef struct Touch_t
{
    int32_t s32WindowWidth;
    int32_t s32WindowHeight;
    int32_t s32PosX;
    int32_t s32PosY;
} Touch;

void FreeTouch(Touch **pstTouch);

int InitTouch(
    int32_t s32WindowWidth,
    int32_t s32WindowHeight,
    Touch **pstTouch);

int ReadInput(const uint8_t **pu8Input);

void GetTouchPosition(Touch **pstTouch);

#endif // _INPUT_H_

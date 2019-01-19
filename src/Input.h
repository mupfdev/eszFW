/**
 * @file Input.h
 * @ingroup Input
 */

#ifndef _INPUT_H_
#define _INPUT_H_

#include <stdbool.h>
#include <stdint.h>
#include "AABB.h"

typedef struct Input_t
{
    const uint8_t *pu8KeyState;
    int32_t        s32WindowWidth;
    int32_t        s32WindowHeight;
    int32_t        s32TouchPosX;
    int32_t        s32TouchPosY;
    uint32_t       u32TouchType;
    AABB           stTouchBB;
} Input;

void FreeInput(Input **pstInput);
int32_t GetTouchPosX(Input **pstInput);
int32_t GetTouchPosY(Input **pstInput);

int InitInput(
    int32_t s32WindowWidth,
    int32_t s32WindowHeight,
    Input **pstInput);

bool IsKeyPressed(const uint16_t u16Scancode, Input **pstInput);
bool UpdateInput(Input **pstInput);

#endif // _INPUT_H_

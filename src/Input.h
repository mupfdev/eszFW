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
    int32_t        s32TouchDispPosX;
    int32_t        s32TouchDispPosY;
    int32_t        s32TouchPosX;
    int32_t        s32TouchPosY;
    AABB           stTouchBB;
    uint8_t        u8TouchBBWidth;
    uint8_t        u8TouchBBHeight;
    uint32_t       u32TouchType;
} Input;

void FreeInput(Input **pstInput);

int InitInput(
    const int32_t s32WindowWidth,
    const int32_t s32WindowHeight,
    Input **pstInput);

bool IsKeyPressed(const uint16_t u16Scancode, Input **pstInput);

bool UpdateInput(
    const double dCameraPosX,
    const double dCameraPosY,
    Input      **pstInput);

void SetTouchBBSize(
    const uint8_t u8Width,
    const uint8_t u8Height,
    Input       **pstInput);

#endif // _INPUT_H_

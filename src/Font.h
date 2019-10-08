/**
 * @file    Font.h
 * @ingroup Font
 */
#pragma once

#include <SDL.h>
#include <SDL_ttf.h>

/**
 * @typedef Font
 * @brief   Font handle type
 * @struct  Font_t
 * @brief   Font handle data
 */
typedef struct Font_t
{
    TTF_Font*  pstTTF;
    SDL_Colour stColour;

} Font;

void  Font_Free(Font* pstFont);
Sint8 Font_Init(const char* pacFileName, Font** pstFont);

Sint8 Font_PrintNumber(
    const Sint32  s32Number,
    const Sint32  s32PosX,
    const Sint32  s32PosY,
    const Font*   pstFont,
    SDL_Renderer* pstRenderer);

Sint8 Font_PrintText(
    const char*   pacText,
    const Sint32  s32PosX,
    const Sint32  s32PosY,
    const Font*   pstFont,
    SDL_Renderer* pstRenderer);

void Font_SetColour(const Uint8 u8Red, const Uint8 u8Green, const Uint8 u8Blue, Font* pstFont);

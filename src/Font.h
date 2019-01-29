/**
 * @file Font.h
 * @ingroup Font
 */

#ifndef _FONT_H_
#define _FONT_H_

#include <SDL.h>
#include <SDL_ttf.h>

typedef struct Font_t
{
    TTF_Font  *pstTTF;
    SDL_Colour stColour;
} Font;

void FreeFont(Font *pstFont);
int  InitFont(const char *pacFileName, Font **pstFont);

int PrintNumber(
    const int32_t  s32Number,
    const int      sPosX,
    const int      sPosY,
    SDL_Renderer *pstRenderer,
    Font         *pstFont);

int PrintText(
    const char    *pacText,
    const int      sPosX,
    const int      sPosY,
    SDL_Renderer *pstRenderer,
    const Font   *pstFont);

void SetFontColour(const uint8_t u8Red, const uint8_t u8Green, const uint8_t u8Blue, Font *pstFont);

#endif // _FONT_H_

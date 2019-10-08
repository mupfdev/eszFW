/**
 * @file      Font.c
 * @ingroup   Font
 * @defgroup  Font Font and text handler
 * @author    Michael Fitzmayer
 * @copyright "THE BEER-WARE LICENCE" (Revision 42)
 */

#include <SDL.h>
#include <SDL_ttf.h>
#include "Font.h"

void Font_Free(Font* pstFont)
{
    TTF_Quit();

    SDL_free(pstFont);
    SDL_Log("Close font.\n");
}

Sint8 Font_Init(const char* pacFileName, Font** pstFont)
{
    *pstFont = SDL_calloc(sizeof(struct Font_t), sizeof(Sint8));
    if (!*pstFont)
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "InitCamera(): error allocating memory.\n");
        return -1;
    }

    if (-1 == TTF_Init())
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "%s\n", TTF_GetError());
        return -1;
    }

    (*pstFont)->pstTTF = TTF_OpenFont(pacFileName, 16);
    if (!(*pstFont)->pstTTF)
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "%s\n", TTF_GetError());
        return -1;
    }

    SDL_Log("Load TrueType font file: %s.\n", pacFileName);

    return 0;
}

Sint8 Font_PrintNumber(
    const Sint32  s32Number,
    const Sint32  s32PosX,
    const Sint32  s32PosY,
    const Font*   pstFont,
    SDL_Renderer* pstRenderer)
{
    char acNumber[12] = { 0 };  // Signed 10 digit number + \0.
    char acOutput[12] = { 0 };
    SDL_snprintf(acNumber, 12, "%11d", s32Number);

    Uint8 u8Idx2 = 0;
    for (Uint8 u8Idx1 = 0; '\0' != acNumber[u8Idx1]; u8Idx1++)
    {
        if (' ' != acNumber[u8Idx1])
        {
            acOutput[u8Idx2] = acNumber[u8Idx1];
            u8Idx2++;
        }
    }

    if (-1 == PrintText(acOutput, s32PosX, s32PosY, pstFont, pstRenderer))
    {
        return -1;
    }
    return 0;
}

Sint8 Font_PrintText(
    const char*   pacText,
    const Sint32  s32PosX,
    const Sint32  s32PosY,
    const Font*   pstFont,
    SDL_Renderer* pstRenderer)
{
    Sint8        s8ReturnValue = 0;
    SDL_Surface* pstSurface    = NULL;
    SDL_Texture* pstTexture    = NULL;
    SDL_Rect     stDst;
    SDL_Rect     stSrc;

    pstSurface = TTF_RenderText_Solid(pstFont->pstTTF, pacText, pstFont->stColour);
    if (!pstSurface)
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "%s\n", TTF_GetError());
        s8ReturnValue = -1;
        goto exit;
    }

    pstTexture = SDL_CreateTextureFromSurface(pstRenderer, pstSurface);
    if (!pstTexture)
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "%s\n", SDL_GetError());
        s8ReturnValue = -1;
        goto exit;
    }

    if (0 > SDL_QueryTexture(pstTexture, NULL, NULL, &stSrc.w, &stSrc.h))
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "%s\n", SDL_GetError());
        s8ReturnValue = -1;
        goto exit;
    }

    stSrc.x = 0;
    stSrc.y = 0;
    stDst.x = s32PosX - (stSrc.w / 2);
    stDst.y = s32PosY - (stSrc.h / 2);

    if (0 > stDst.x)
    {
        stDst.x = 0;
    }
    if (0 > stDst.y)
    {
        stDst.y = 0;
    }

    stDst.w = stSrc.w;
    stDst.h = stSrc.h;

    if (0 > SDL_RenderCopy(pstRenderer, pstTexture, &stSrc, &stDst))
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "%s\n", SDL_GetError());
        s8ReturnValue = -1;
        goto exit;
    }

exit:
    if (pstTexture)
    {
        SDL_DestroyTexture(pstTexture);
    }
    if (pstSurface)
    {
        SDL_FreeSurface(pstSurface);
    }

    return s8ReturnValue;
}

void Font_SetFontColour(const Uint8 u8Red, const Uint8 u8Green, const Uint8 u8Blue, Font* pstFont)
{
    pstFont->stColour.r = u8Red;
    pstFont->stColour.g = u8Green;
    pstFont->stColour.b = u8Blue;
}

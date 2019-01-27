/**
 * @file Font.c
 * @ingroup Font
 * @defgroup Font
 * @author Michael Fitzmayer
 * @copyright "THE BEER-WARE LICENCE" (Revision 42)
 */

#include <SDL.h>
#include <SDL_ttf.h>
#include <string.h>
#include "Font.h"

void FreeFont(Font **pstFont)
{
    free(*pstFont);
    TTF_Quit();
    SDL_Log("Close font.\n");
}

int InitFont(const char *pacFileName, Font **pstFont)
{
    *pstFont = NULL;
    *pstFont = malloc(sizeof(struct Font_t));
    if (! *pstFont)
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "%s\n", SDL_GetError());
        return -1;
    }

    if (-1 == TTF_Init())
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "%s\n", TTF_GetError());
        return -1;
    }

    (*pstFont)->pstTTF = TTF_OpenFont(pacFileName, 16);
    if (! (*pstFont)->pstTTF)
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "%s\n", TTF_GetError());
        return -1;
    }

    (*pstFont)->stColour.r = 0;
    (*pstFont)->stColour.g = 0;
    (*pstFont)->stColour.b = 0;

    SDL_Log("Load TrueType font file: %s.\n", pacFileName);

    return 0;
}

int PrintNumber(
    const int32_t  s32Number,
    const int      sPosX,
    const int      sPosY,
    SDL_Renderer **pstRenderer,
    Font         **pstFont)
{
    char acNumber[12] = { 0 }; // Signed 10 digit number + \0.
    char acOutput[12] = { 0 };
    snprintf(acNumber, 12, "%11d", s32Number);

    int sIdx2 = 0;
    for (int sIdx1 = 0; '\0' != acNumber[sIdx1]; sIdx1++)
    {
        if (' ' != acNumber[sIdx1])
        {
            acOutput[sIdx2] = acNumber[sIdx1];
            sIdx2++;
        }
    }

    if (-1 == PrintText(acOutput, sPosX, sPosY, pstRenderer, pstFont))
    {
        return -1;
    }
    return 0;
}

int PrintText(
    const char    *pacText,
    const int      sPosX,
    const int      sPosY,
    SDL_Renderer **pstRenderer,
    Font         **pstFont)
{
    int          sReturnValue = 0;
    SDL_Surface *pstSurface   = NULL;
    SDL_Texture *pstTexture   = NULL;
    SDL_Rect     stDst;
    SDL_Rect     stSrc;

    pstSurface = TTF_RenderText_Solid((*pstFont)->pstTTF, pacText, (*pstFont)->stColour);
    if (! pstSurface)
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "%s\n", TTF_GetError());
        sReturnValue = -1;
        goto exit;
    }

    pstTexture = SDL_CreateTextureFromSurface((*pstRenderer), pstSurface);
    if (! pstSurface)
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "%s\n", SDL_GetError());
        sReturnValue = -1;
        goto exit;
    }

    if (0 > SDL_QueryTexture(pstTexture, NULL, NULL, &stSrc.w, &stSrc.h))
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "%s\n", SDL_GetError());
        sReturnValue = -1;
        goto exit;
    }

    stSrc.x = 0;
    stSrc.y = 0;
    stDst.x = sPosX;
    stDst.y = sPosY;
    stDst.w = stSrc.w;
    stDst.h = stSrc.h;

    if (0 > SDL_RenderCopy((*pstRenderer), pstTexture, &stSrc, &stDst))
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "%s\n", SDL_GetError());
        sReturnValue = -1;
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

    return sReturnValue;
}

void SetFontColour(
    const uint8_t u8Red,
    const uint8_t u8Green,
    const uint8_t u8Blue,
    Font        **pstFont)
{
    (*pstFont)->stColour.r = u8Red;
    (*pstFont)->stColour.g = u8Green;
    (*pstFont)->stColour.b = u8Blue;
}

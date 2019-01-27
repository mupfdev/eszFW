/**
 * @file      Background.c
 * @ingroup   Background
 * @defgroup  Background
 * @author    Michael Fitzmayer
 * @copyright "THE BEER-WARE LICENCE" (Revision 42)
 */

#include <SDL.h>
#include <SDL_image.h>
#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include "Background.h"
#include "Macros.h"

static int _DrawLayer(
    const uint8_t  u8Index,
    const int32_t  s32LogicalWindowHeight,
    const double   dCameraPosY,
    SDL_Renderer **pstRenderer,
    Background   **pstBackground)
{
    int32_t  s32Width = 0;
    double   dPosXa;
    double   dPosXb;
    SDL_Rect stDst;

    if (0 != SDL_QueryTexture((*pstBackground)->acLayer[u8Index].pstLayer, NULL, NULL, &s32Width, NULL))
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "%s\n", SDL_GetError());
        return -1;
    }

    if ((*pstBackground)->acLayer[u8Index].dPosX < -s32Width)
    {
        (*pstBackground)->acLayer[u8Index].dPosX = +s32Width;
    }

    if ((*pstBackground)->acLayer[u8Index].dPosX > +s32Width)
    {
        (*pstBackground)->acLayer[u8Index].dPosX = -s32Width;
    }

    dPosXa = (*pstBackground)->acLayer[u8Index].dPosX;
    if (dPosXa > 0)
    {
        dPosXb = dPosXa - s32Width;
    }
    else
    {
        dPosXb = dPosXa + s32Width;
    }

    if (0 < (*pstBackground)->acLayer[u8Index].dVelocity)
    {
        if (RIGHT == (*pstBackground)->bOrientation)
        {
            (*pstBackground)->acLayer[u8Index].dPosX -= (*pstBackground)->acLayer[u8Index].dVelocity;
        }
        else
        {
            (*pstBackground)->acLayer[u8Index].dPosX += (*pstBackground)->acLayer[u8Index].dVelocity;
        }
    }

    if (TOP == (*pstBackground)->bAlignment)
    {
        stDst.y = (*pstBackground)->acLayer[u8Index].dPosY - dCameraPosY;
    }
    else
    {
        stDst.y = (*pstBackground)->acLayer[u8Index].dPosY
            + (s32LogicalWindowHeight - (*pstBackground)->acLayer[u8Index].s32Height);
    }

    stDst.x = dPosXa;
    stDst.w = s32Width;
    stDst.h = (*pstBackground)->acLayer[u8Index].s32Height;

    if (-1 == SDL_RenderCopyEx(
            (*pstRenderer),
            (*pstBackground)->acLayer[u8Index].pstLayer,
            NULL, &stDst, 0, NULL, SDL_FLIP_NONE))
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "%s\n", SDL_GetError());
        return -1;
    }

    stDst.x = dPosXb;
    if (-1 == SDL_RenderCopyEx(
            (*pstRenderer),
            (*pstBackground)->acLayer[u8Index].pstLayer,
            NULL, &stDst, 0, NULL, SDL_FLIP_NONE))
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "%s\n", SDL_GetError());
        return -1;
    }

    return 0;
}

static int _RenderLayer(
    const char    *pacFileName,
    const int32_t  s32WindowWidth,
    SDL_Renderer **pstRenderer,
    SDL_Texture  **pstLayer)
{
    int          sReturnValue   = 0;
    SDL_Texture *pstImage       = NULL;
    int32_t      s32ImageWidth  = 0;
    int32_t      s32ImageHeight = 0;
    int32_t      s32LayerHeight = 0;
    int32_t      s32LayerWidth  = 0;
    uint8_t      u8WidthFactor  = 0;

    pstImage = IMG_LoadTexture((*pstRenderer), pacFileName);
    if (! pstImage)
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "%s\n", IMG_GetError());
        sReturnValue = -1;
        goto exit;
    }

    if (0 != SDL_QueryTexture(pstImage, NULL, NULL, &s32ImageWidth, &s32ImageHeight))
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "%s\n", SDL_GetError());
        sReturnValue = -1;
        goto exit;
    }

    u8WidthFactor  = ceil((double)s32WindowWidth / (double)s32ImageWidth);
    s32LayerWidth  = s32ImageWidth * u8WidthFactor;
    s32LayerHeight = s32ImageHeight;
    (*pstLayer)    = NULL;
    (*pstLayer)    = SDL_CreateTexture(
        (*pstRenderer),
        SDL_PIXELFORMAT_ARGB8888,
        SDL_TEXTUREACCESS_TARGET,
        s32LayerWidth,
        s32LayerHeight);

    if (! (*pstLayer))
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "%s\n", SDL_GetError());
        sReturnValue = -1;
        goto exit;
    }

    if (0 != SDL_SetRenderTarget((*pstRenderer), (*pstLayer)))
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "%s\n", SDL_GetError());
        sReturnValue = -1;
        goto exit;
    }

    SDL_Rect stDst;
    stDst.x = 0;
    for (uint8_t u8Index = 0; u8Index < u8WidthFactor; u8Index++)
    {
        stDst.y  = 0;
        stDst.w  = s32ImageWidth;
        stDst.h  = s32ImageHeight;
        SDL_RenderCopy((*pstRenderer), pstImage, NULL, &stDst);
        stDst.x += s32ImageWidth;
    }

    if (0 != SDL_SetTextureBlendMode((*pstLayer), SDL_BLENDMODE_BLEND))
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "%s\n", SDL_GetError());
        sReturnValue = -1;
        goto exit;
    }

    if (0 != SDL_SetRenderTarget((*pstRenderer), NULL))
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "%s\n", SDL_GetError());
        sReturnValue = -1;
        goto exit;
    }

exit:
    if (-1 == sReturnValue)
    {
        if ((*pstLayer))
        {
            SDL_DestroyTexture((*pstLayer));
        }
        if (pstImage)
        {
            SDL_DestroyTexture(pstImage);
        }
    }

    return sReturnValue;
}

int DrawBackground(
    const bool     bOrientation,
    const int32_t  s32LogicalWindowHeight,
    const double   dCameraPosY,
    const double   dVelocity,
    SDL_Renderer **pstRenderer,
    Background   **pstBackground)
{
    (*pstBackground)->bOrientation = bOrientation;

    double dFactor = (*pstBackground)->u8Num + 1;
    for (uint8_t u8Index = 0; u8Index < (*pstBackground)->u8Num; u8Index++)
    {
        (*pstBackground)->acLayer[u8Index].dVelocity = dVelocity / dFactor;
        dFactor--;

        _DrawLayer(
            u8Index,
            s32LogicalWindowHeight,
            dCameraPosY,
            &(*pstRenderer),
            &(*pstBackground));
    }

    return 0;
}

void FreeBackground(Background **pstBackground)
{
    free(*pstBackground);
    SDL_Log("Unload parallax scrolling background.\n");
}

int InitBackground(
    const uint8_t  u8Num,
    const char    *pacFileNames[static u8Num],
    const int32_t  s32WindowWidth,
    const bool     bAlignment,
    SDL_Renderer **pstRenderer,
    Background   **pstBackground)
{
    *pstBackground = NULL;
    *pstBackground = malloc(
        sizeof(struct Background_t)
        + (u8Num * sizeof(struct BGLayer_t)));
    if (! *pstBackground)
    {
        SDL_LogError(
            SDL_LOG_CATEGORY_APPLICATION,
            "InitBackground(): error allocating memory.\n");
        return -1;
    }

    (*pstBackground)->u8Num      = u8Num;
    (*pstBackground)->bAlignment = bAlignment;

    SDL_Log("Initialise parallax scrolling background with %d layers:\n", u8Num);
    for (uint8_t u8Index = 0; u8Index < u8Num; u8Index++)
    {
        if (-1 == _RenderLayer(
                pacFileNames[u8Index],
                s32WindowWidth,
                &(*pstRenderer),
                &(*pstBackground)->acLayer[u8Index].pstLayer))
        {
            return -1;
        }
        SDL_Log("  Render background layer %d layer: %s.\n", u8Index + 1, pacFileNames[u8Index]);
    }

    for (uint8_t u8Index = 0; u8Index < u8Num; u8Index++)
    {
        (*pstBackground)->acLayer[u8Index].dPosX     = 0;
        (*pstBackground)->acLayer[u8Index].dPosY     = 0;
        (*pstBackground)->acLayer[u8Index].dVelocity = 0;

        if (0 != SDL_QueryTexture(
                (*pstBackground)->acLayer[u8Index].pstLayer,
                NULL,
                NULL,
                &(*pstBackground)->acLayer[u8Index].s32Width,
                &(*pstBackground)->acLayer[u8Index].s32Height))
        {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "%s\n", SDL_GetError());
            return -1;
        }
    }

    return 0;
}

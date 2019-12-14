// SPDX-License-Identifier: Beerware
/**
 * @file      Background.c
 * @brief     Background handler source
 * @ingroup   Background
 * @ingroup   Background Parallax-scrolling background handler
 * @author    Michael Fitzmayer
 * @copyright "THE BEER-WARE LICENCE" (Revision 42)
 */

#include <SDL.h>
#include <SDL_image.h>
#include "Background.h"
#include "Constants.h"

static Sint8 _DrawLayer(
    const Uint8   u8Index,
    const Sint32  s32LogicalWindowHeight,
    const double  dCameraPosY,
    SDL_Renderer* pstRenderer,
    Background*   pstBackground)
{
    Sint32   s32Width = 0;
    double   dPosXa;
    double   dPosXb;
    SDL_Rect stDst;

    if (0 != SDL_QueryTexture(
            pstBackground->acLayer[u8Index].pstLayer,
            NULL,
            NULL,
            &s32Width,
            NULL))
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "%s\n", SDL_GetError());
        return -1;
    }

    if (pstBackground->acLayer[u8Index].dPosX < -s32Width)
    {
        pstBackground->acLayer[u8Index].dPosX = +s32Width;
    }

    if (pstBackground->acLayer[u8Index].dPosX > +s32Width)
    {
        pstBackground->acLayer[u8Index].dPosX = -s32Width;
    }

    dPosXa = pstBackground->acLayer[u8Index].dPosX;
    if (dPosXa > 0)
    {
        dPosXb = dPosXa - s32Width;
    }
    else
    {
        dPosXb = dPosXa + s32Width;
    }

    if (0 < pstBackground->acLayer[u8Index].dVelocity)
    {
        if (RIGHT == pstBackground->eDirection)
        {
            pstBackground->acLayer[u8Index].dPosX -= pstBackground->acLayer[u8Index].dVelocity;
        }
        else
        {
            pstBackground->acLayer[u8Index].dPosX += pstBackground->acLayer[u8Index].dVelocity;
        }
    }

    if (TOP == pstBackground->eAlignment)
    {
        stDst.y = pstBackground->acLayer[u8Index].dPosY - dCameraPosY;
    }
    else
    {
        stDst.y = pstBackground->acLayer[u8Index].dPosY +
            (s32LogicalWindowHeight - pstBackground->acLayer[u8Index].s32Height);
    }

    stDst.x = dPosXa;
    stDst.w = s32Width;
    stDst.h = pstBackground->acLayer[u8Index].s32Height;

    if (-1 == SDL_RenderCopyEx(
            pstRenderer,
            pstBackground->acLayer[u8Index].pstLayer,
            NULL,
            &stDst,
            0,
            NULL,
            SDL_FLIP_NONE))
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "%s\n", SDL_GetError());
        return -1;
    }

    stDst.x = dPosXb;
    if (-1 == SDL_RenderCopyEx(
            pstRenderer,
            pstBackground->acLayer[u8Index].pstLayer,
            NULL,
            &stDst,
            0,
            NULL,
            SDL_FLIP_NONE))
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "%s\n", SDL_GetError());
        return -1;
    }

    return 0;
}

static Sint8 _RenderLayer(
    const char*   pacFileName,
    const Sint32  s32WindowWidth,
    SDL_Renderer* pstRenderer,
    SDL_Texture** pstLayer)
{
    Sint8        s8ReturnValue  = 0;
    SDL_Texture* pstImage       = NULL;
    Sint32       s32ImageWidth  = 0;
    Sint32       s32ImageHeight = 0;
    Sint32       s32LayerHeight = 0;
    Sint32       s32LayerWidth  = 0;
    Uint8        u8WidthFactor  = 0;

    pstImage = IMG_LoadTexture(pstRenderer, pacFileName);
    if (!pstImage)
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "%s\n", IMG_GetError());
        s8ReturnValue = -1;
        goto exit;
    }

    if (0 != SDL_QueryTexture(pstImage, NULL, NULL, &s32ImageWidth, &s32ImageHeight))
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "%s\n", SDL_GetError());
        s8ReturnValue = -1;
        goto exit;
    }

    u8WidthFactor  = SDL_ceil((double)s32WindowWidth / (double)s32ImageWidth);
    s32LayerWidth  = s32ImageWidth * u8WidthFactor;
    s32LayerHeight = s32ImageHeight;
    (*pstLayer)    = SDL_CreateTexture(
        pstRenderer,
        SDL_PIXELFORMAT_ARGB8888,
        SDL_TEXTUREACCESS_TARGET,
        s32LayerWidth,
        s32LayerHeight);

    if (!(*pstLayer))
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "%s\n", SDL_GetError());
        s8ReturnValue = -1;
        goto exit;
    }

    if (0 != SDL_SetRenderTarget(pstRenderer, (*pstLayer)))
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "%s\n", SDL_GetError());
        s8ReturnValue = -1;
        goto exit;
    }

    SDL_Rect stDst;
    stDst.x = 0;
    for (Uint8 u8Index = 0; u8Index < u8WidthFactor; u8Index++)
    {
        stDst.y = 0;
        stDst.w = s32ImageWidth;
        stDst.h = s32ImageHeight;
        SDL_RenderCopy(pstRenderer, pstImage, NULL, &stDst);
        stDst.x += s32ImageWidth;
    }

    if (0 != SDL_SetTextureBlendMode((*pstLayer), SDL_BLENDMODE_BLEND))
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "%s\n", SDL_GetError());
        s8ReturnValue = -1;
        goto exit;
    }

    if (0 != SDL_SetRenderTarget(pstRenderer, NULL))
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "%s\n", SDL_GetError());
        s8ReturnValue = -1;
        goto exit;
    }

exit:
    if (-1 == s8ReturnValue)
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

    return s8ReturnValue;
}

/**
 * @brief   Draw background
 * @details Draws the layers of a parallax-scrolling background
 * @param   eDirection
 *          Scroll direction
 * @param   s32LogicalWindowHeight
 *          Logical window height in pixel
 * @param   dCameraPosY
 *          Camera position along the y-axis
 * @param   dVelocity
 *          Scroll velocity
 * @param   pstRenderer
 *          Pointer to SDL2 rendering context
 * @param   pstBackground
 *          Pointer to background handle
 * @return  Error code
 * @retval  0: OK
 */
Sint8 Background_Draw(
    const Direction eDirection,
    const Sint32    s32LogicalWindowHeight,
    const double    dCameraPosY,
    const double    dVelocity,
    SDL_Renderer*   pstRenderer,
    Background*     pstBackground)
{
    pstBackground->eDirection = eDirection;

    double dFactor = pstBackground->u8Num + 1;
    for (Uint8 u8Index = 0; u8Index < pstBackground->u8Num; u8Index++)
    {
        pstBackground->acLayer[u8Index].dVelocity = dVelocity / dFactor;
        dFactor -= 0.5f;

        _DrawLayer(u8Index, s32LogicalWindowHeight, dCameraPosY, pstRenderer, pstBackground);
    }

    return 0;
}

/**
 * @brief   Free background
 * @details Frees up allocated memory and unload parallax-scrolling
 *          background
 * @param   pstBackground
 *          Pointer to background handle
 */
void Background_Free(Background* pstBackground)
{
    SDL_free(pstBackground);
    SDL_Log("Unload parallax scrolling background.\n");
}

/**
 * @brief   Initialise background
 * @details Initialises parallax-scrolling background
 * @param   u8Num
 *          Number of backgrounds
 * @param   pacFileNames
 *          Pointer to array with list of filenames
 * @param   s32WindowWidth
 *          Window width in pixel
 * @param   eAlignment
 *          Background alignment
 * @param   pstRenderer
 *          Pointer to SDL2 rendering context
 * @param   pstBackground
 *          Pointer to background handle
 * @return  Error code
 * @retval   0: OK
 * @retval  -1: Error
 */
Sint8 Background_Init(
    const Uint8     u8Num,
    const char*     pacFileNames[static u8Num],
    const Sint32    s32WindowWidth,
    const Alignment eAlignment,
    SDL_Renderer*   pstRenderer,
    Background**    pstBackground)
{
    *pstBackground =
        SDL_calloc(sizeof(struct Background_t) + (u8Num * sizeof(struct BGLayer_t)), sizeof(Sint8));
    if (!*pstBackground)
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "InitBackground(): error allocating memory.\n");
        return -1;
    }

    (*pstBackground)->u8Num      = u8Num;
    (*pstBackground)->eAlignment = eAlignment;

    SDL_Log("Initialise parallax scrolling background with %d layers:\n", u8Num);

    for (Uint8 u8Index = 0; u8Index < u8Num; u8Index++)
    {
        if (-1 == _RenderLayer(
                pacFileNames[u8Index],
                s32WindowWidth,
                pstRenderer,
                &(*pstBackground)->acLayer[u8Index].pstLayer))
        {
            return -1;
        }
        SDL_Log("  Render background layer %d layer: %s.\n", u8Index + 1, pacFileNames[u8Index]);
    }

    for (Uint8 u8Index = 0; u8Index < u8Num; u8Index++)
    {
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

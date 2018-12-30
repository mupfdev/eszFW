/**
 * @file Map.c
 * @ingroup Map
 * @defgroup Map
 * @author Michael Fitzmayer
 * @copyright "THE BEER-WARE LICENCE" (Revision 42)
 */

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <stdbool.h>
#include <stdint.h>
#include <tmx.h>
#include "Map.h"

int DrawMap(
    const char    *pacTilesetImageFileName,
    const bool     bRenderBgColour,
    const uint16_t u16Index,
    const char    *pacLayerName,
    const double   dCameraPosX,
    const double   dCameraPosY,
    Map          **pstMap,
    SDL_Renderer **pstRenderer)
{
    SDL_Texture *pstTileset = NULL;
    tmx_layer   *pstLayers  = (*pstMap)->pstTmxMap->ly_head;

    // The texture has already been rendered and can be drawn.
    if ((*pstMap)->pstTexture[u16Index])
    {
        double dRenderPosX = (*pstMap)->dPosX - dCameraPosX;
        double dRenderPosY = (*pstMap)->dPosY - dCameraPosY;

        SDL_Rect stDst =
        {
            dRenderPosX,
            dRenderPosY,
            (*pstMap)->pstTmxMap->width  * (*pstMap)->pstTmxMap->tile_width,
            (*pstMap)->pstTmxMap->height * (*pstMap)->pstTmxMap->tile_height
        };

        if (-1 == SDL_RenderCopyEx(
                (*pstRenderer),
                (*pstMap)->pstTexture[u16Index],
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
    // Otherwise render the texture once.
    (*pstMap)->pstTexture[u16Index] = SDL_CreateTexture(
        (*pstRenderer),
        SDL_PIXELFORMAT_ARGB8888,
        SDL_TEXTUREACCESS_TARGET,
        (*pstMap)->pstTmxMap->width  * (*pstMap)->pstTmxMap->tile_width,
        (*pstMap)->pstTmxMap->height * (*pstMap)->pstTmxMap->tile_height);

    if (NULL == (*pstMap)->pstTexture[u16Index])
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "%s\n", SDL_GetError());
        return -1;
    }

    pstTileset = IMG_LoadTexture(
        (*pstRenderer),
        pacTilesetImageFileName);

    if (NULL == pstTileset)
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "%s\n", IMG_GetError());
        return -1;
    }

    if (0 != SDL_SetRenderTarget((*pstRenderer), (*pstMap)->pstTexture[u16Index]))
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "%s\n", SDL_GetError());
        SDL_DestroyTexture(pstTileset);
        return -1;
    }

    if (bRenderBgColour)
    {
        SDL_SetRenderDrawColor(
            (*pstRenderer),
            ((*pstMap)->pstTmxMap->backgroundcolor >> 16) & 0xFF,
            ((*pstMap)->pstTmxMap->backgroundcolor >>  8) & 0xFF,
            ((*pstMap)->pstTmxMap->backgroundcolor)       & 0xFF,
            255);
    }

    while(pstLayers)
    {
        uint32_t     u32Gid;
        SDL_Rect     stDst;
        SDL_Rect     stSrc;
        tmx_tileset *pstTS;

        if (L_LAYER == pstLayers->type)
        {
            if ((pstLayers->visible) && (NULL != strstr(pstLayers->name, pacLayerName)))
            {
                for (uint32_t u32IndexH = 0; u32IndexH < (*pstMap)->pstTmxMap->height; u32IndexH++)
                {
                    for (uint32_t u32IndexW = 0; u32IndexW < (*pstMap)->pstTmxMap->width; u32IndexW++)
                    {
                        u32Gid = pstLayers->content.gids[
                            (u32IndexH * (*pstMap)->pstTmxMap->width) + u32IndexW]
                            & TMX_FLIP_BITS_REMOVAL;
                        if (NULL != (*pstMap)->pstTmxMap->tiles[u32Gid])
                        {
                            pstTS    = (*pstMap)->pstTmxMap->tiles[u32Gid]->tileset;
                            stSrc.x  = (*pstMap)->pstTmxMap->tiles[u32Gid]->ul_x;
                            stSrc.y  = (*pstMap)->pstTmxMap->tiles[u32Gid]->ul_y;
                            stSrc.w  = stDst.w   = pstTS->tile_width;
                            stSrc.h  = stDst.h   = pstTS->tile_height;
                            stDst.x  = u32IndexW * pstTS->tile_width;
                            stDst.y  = u32IndexH * pstTS->tile_height;
                            SDL_RenderCopy((*pstRenderer), pstTileset, &stSrc, &stDst);
                        }
                    }
                }
                SDL_Log("Render TMX map layer: %s\n", pstLayers->name);
            }
        }
        pstLayers = pstLayers->next;
    }

    // Switch back to default render target.
    if (0 != SDL_SetRenderTarget((*pstRenderer), NULL))
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "%s\n", SDL_GetError());
        return -1;
    }

    if (0 != SDL_SetTextureBlendMode((*pstMap)->pstTexture[u16Index], SDL_BLENDMODE_BLEND))
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "%s\n", SDL_GetError());
        return -1;
    }

    return 0;
}

void FreeMap(Map **pstMap)
{
    tmx_map_free((*pstMap)->pstTmxMap);
    free(*pstMap);

    SDL_Log("Unload TMX map.\n");
}

/*
int GetObjectCount(Map **pstMa)
{
    // TODO
}
*/

int InitMap(const char *pacFileName, Map **pstMap)
{
    *pstMap = malloc(sizeof(struct Map_t));
    if (NULL == *pstMap)
    {
        SDL_LogError(
            SDL_LOG_CATEGORY_APPLICATION,
            "InitVideo(): error allocating memory.\n");
        return -1;
    }

    (*pstMap)->pstTmxMap = tmx_load(pacFileName);
    if (NULL == (*pstMap)->pstTmxMap)
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "%s\n", tmx_strerr());
        free(*pstMap);
        return -1;
    }

    (*pstMap)->u32Height  = (*pstMap)->pstTmxMap->height * (*pstMap)->pstTmxMap->tile_height;
    (*pstMap)->u32Width   = (*pstMap)->pstTmxMap->width  * (*pstMap)->pstTmxMap->tile_width;
    (*pstMap)->dPosX = 0;
    (*pstMap)->dPosY = 0;

    for (uint16_t u16Index = 0; u16Index < MAP_TEXTURES; u16Index++)
    {
        (*pstMap)->pstTexture[u16Index] = NULL;
    }

    SDL_Log("Load TMX map file: %s.\n", pacFileName);

    return 0;
}

bool IsMapTileOfType(
    const char *pacType,
    const Map **pstMap,
    double      dPosX,
    double      dPosY)
{
    dPosX /= (*pstMap)->pstTmxMap->tile_width;
    dPosY /= (*pstMap)->pstTmxMap->tile_height;

    // Set boundaries to prevent segfault.
    if ( (dPosX < 0) ||
         (dPosY < 0) ||
         (dPosX > (*pstMap)->pstTmxMap->width) ||
         (dPosY > (*pstMap)->pstTmxMap->height) )
    {
        return false;
    }

    tmx_layer *pstLayers = (*pstMap)->pstTmxMap->ly_head;
    while(pstLayers)
    {
        uint16_t u16Gid =
            pstLayers->content.gids[
                ((int32_t)dPosY * (*pstMap)->pstTmxMap->width) + (int32_t)dPosX
                ] & TMX_FLIP_BITS_REMOVAL;

        if (NULL != (*pstMap)->pstTmxMap->tiles[u16Gid])
        {
            if (NULL != (*pstMap)->pstTmxMap->tiles[u16Gid]->type)
            {
                if (0 == strcmp(pacType, (*pstMap)->pstTmxMap->tiles[u16Gid]->type))
                {
                    return true;
                }
            }
        }
        pstLayers = pstLayers->next;
    }

    return false;
}

/**
 * @file Map.c
 * @ingroup Map
 * @defgroup Map
 * @author Michael Fitzmayer
 * @copyright "THE BEER-WARE LICENCE" (Revision 42)
 */

#ifdef __ANDROID__
#include <SDL.h>
#include <SDL_image.h>
#else
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#endif
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <tmx.h>
#include "Map.h"

static uint16_t _ClearGidFlags(uint16_t u16Gid)
{
    return u16Gid & TMX_FLIP_BITS_REMOVAL;
}

static void _FindObjectByName(
    const char *pacName,
    tmx_object *pstTmxObject,
    Object    **pstObject)
{
    if (pstObject)
    {
        if (0 == strncmp(pacName, pstTmxObject->name, 20))
        {
            (*pstObject)->dPosX = pstTmxObject->x;
            (*pstObject)->dPosY = pstTmxObject->y;
        }
        else if (pstTmxObject && pstTmxObject->next)
        {
            _FindObjectByName(pacName, pstTmxObject->next, pstObject);
        }
    }
}

static void _GetObjectCount(tmx_object *pstObject, uint16_t **pu16ObjectCount)
{
    if (pstObject)
    {
        (**pu16ObjectCount)++;
    }

    if (pstObject && pstObject->next)
    {
        _GetObjectCount(pstObject->next, pu16ObjectCount);
    }
}

static void _GetGravitation(tmx_property *pProperty, void *dGravitation)
{
    if (0 == strncmp(pProperty->name, "Gravitation", 11))
    {
        if (PT_FLOAT == pProperty->type)
        {
            *((double*)dGravitation) = pProperty->value.decimal;
        }
    }
}

int DrawMap(
    const uint16_t u16Index,
    const char    *pacTilesetImageFileName,
    const bool     bRenderBgColour,
    const char    *pacLayerName,
    const double   dCameraPosX,
    const double   dCameraPosY,
    Map          **pstMap,
    SDL_Renderer **pstRenderer)
{
    SDL_Texture *pstTileset = NULL;
    tmx_layer   *pstLayer   = (*pstMap)->pstTmxMap->ly_head;

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

    if (! (*pstMap)->pstTexture[u16Index])
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "%s\n", SDL_GetError());
        return -1;
    }

    pstTileset = IMG_LoadTexture(
        (*pstRenderer),
        pacTilesetImageFileName);

    if (! pstTileset)
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "%s\n", IMG_GetError());
        return -1;
    }

    if (0 != SDL_SetRenderTarget((*pstRenderer), (*pstMap)->pstTexture[u16Index]))
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "%s\n", SDL_GetError());
        if (pstTileset)
        {
            SDL_DestroyTexture(pstTileset);
        }
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

    while(pstLayer)
    {
        uint16_t     u16Gid;
        SDL_Rect     stDst;
        SDL_Rect     stSrc;
        tmx_tileset *pstTS;

        if (L_LAYER == pstLayer->type)
        {
            if ((pstLayer->visible) && (strstr(pstLayer->name, pacLayerName)))
            {
                for (uint16_t u16IndexH = 0; u16IndexH < (*pstMap)->pstTmxMap->height; u16IndexH++)
                {
                    for (uint32_t u16IndexW = 0; u16IndexW < (*pstMap)->pstTmxMap->width; u16IndexW++)
                    {
                        u16Gid = _ClearGidFlags(pstLayer->content.gids[
                            (u16IndexH * (*pstMap)->pstTmxMap->width) + u16IndexW]);

                        if ((*pstMap)->pstTmxMap->tiles[u16Gid])
                        {
                            pstTS    = (*pstMap)->pstTmxMap->tiles[u16Gid]->tileset;
                            stSrc.x  = (*pstMap)->pstTmxMap->tiles[u16Gid]->ul_x;
                            stSrc.y  = (*pstMap)->pstTmxMap->tiles[u16Gid]->ul_y;
                            stSrc.w  = stDst.w   = pstTS->tile_width;
                            stSrc.h  = stDst.h   = pstTS->tile_height;
                            stDst.x  = u16IndexW * pstTS->tile_width;
                            stDst.y  = u16IndexH * pstTS->tile_height;
                            SDL_RenderCopy((*pstRenderer), pstTileset, &stSrc, &stDst);
                        }
                    }
                }
                SDL_Log("Render TMX map layer: %s\n", pstLayer->name);
            }
        }
        pstLayer = pstLayer->next;
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
    if (! pstMap)
    {
        tmx_map_free((*pstMap)->pstTmxMap);
    }
    free((*pstMap));

    SDL_Log("Unload TMX map.\n");
}

void FreeObject(Object **pstObject)
{
    free(*pstObject);
}

void GetSingleObjectByName(
    const char *pacName,
    Map       **pstMap,
    Object    **pstObject)
{
    tmx_layer *pstLayer;

    pstLayer = (*pstMap)->pstTmxMap->ly_head;
    while(pstLayer)
    {
        if (L_OBJGR == pstLayer->type)
        {
            _FindObjectByName(pacName, pstLayer->content.objgr->head, &(*pstObject));
        }

        pstLayer = pstLayer->next;
    }
}

uint16_t GetObjectCount(Map **pstMap)
{
    uint16_t   u16ObjectCount  = 0;
    uint16_t  *pu16ObjectCount = &u16ObjectCount;
    tmx_layer *pstLayer;

    pstLayer = (*pstMap)->pstTmxMap->ly_head;
    while(pstLayer)
    {
        if (L_OBJGR == pstLayer->type)
        {
            _GetObjectCount(pstLayer->content.objgr->head, &pu16ObjectCount);
        }

        pstLayer = pstLayer->next;

    }

    return u16ObjectCount;
}

int InitMap(
    const char   *pacFileName,
    const uint8_t u8MeterInPixel,
    Map         **pstMap)
{
    *pstMap = malloc(sizeof(struct Map_t));
    if (! *pstMap)
    {
        SDL_LogError(
            SDL_LOG_CATEGORY_APPLICATION,
            "InitMap(): error allocating memory.\n");
        return -1;
    }

    (*pstMap)->pstTmxMap = tmx_load(pacFileName);
    if (! (*pstMap)->pstTmxMap)
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "%s\n", tmx_strerr());
        return -1;
    }

    (*pstMap)->u16Height      = (*pstMap)->pstTmxMap->height * (*pstMap)->pstTmxMap->tile_height;
    (*pstMap)->u16Width       = (*pstMap)->pstTmxMap->width  * (*pstMap)->pstTmxMap->tile_width;
    (*pstMap)->dPosX          = 0.f;
    (*pstMap)->dPosY          = 0.f;
    (*pstMap)->dGravitation   = 0.f;
    (*pstMap)->u8MeterInPixel = u8MeterInPixel;

    for (uint16_t u16Index = 0; u16Index < MAP_TEXTURES; u16Index++)
    {
        (*pstMap)->pstTexture[u16Index] = NULL;
    }

    SDL_Log("Load TMX map file: %s.\n", pacFileName);
    SetGravitation(0, true, pstMap);

    return 0;
}

int InitObject(Object **pstObject)
{
    *pstObject = malloc(sizeof(struct Object_t));
    if (! *pstObject)
    {
        SDL_LogError(
            SDL_LOG_CATEGORY_APPLICATION,
            "InitObject(): error allocating memory.\n");
        return -1;
    }

    (*pstObject)->dPosX = 0.f;
    (*pstObject)->dPosY = 0.f;

    return 0;
}

bool IsMapCoordOfType(
    const char *pacType,
    double      dPosX,
    double      dPosY,
    Map       **pstMap)
{
    tmx_layer *pstLayers;
    dPosX /= (double)(*pstMap)->pstTmxMap->tile_width;
    dPosY /= (double)(*pstMap)->pstTmxMap->tile_height;

    // Set boundaries to prevent segfault.
    if ( (dPosX < 0.0) ||
         (dPosY < 0.0) ||
         (dPosX > (double)(*pstMap)->pstTmxMap->width) ||
         (dPosY > (double)(*pstMap)->pstTmxMap->height) )
    {
        return false;
    }

    pstLayers = (*pstMap)->pstTmxMap->ly_head;
    while(pstLayers)
    {
        uint16_t u16TsTileCount = (*pstMap)->pstTmxMap->tiles[1]->tileset->tilecount;
        uint16_t u16Gid         = _ClearGidFlags(pstLayers->content.gids[
            ((int32_t)dPosY * (*pstMap)->pstTmxMap->width) + (int32_t)dPosX]);

        if (u16Gid > u16TsTileCount)
        {
            return false;
        }

        if ((*pstMap)->pstTmxMap->tiles[u16Gid])
        {
            if ((*pstMap)->pstTmxMap->tiles[u16Gid]->type)
            {
                if (0 == strncmp(pacType, (*pstMap)->pstTmxMap->tiles[u16Gid]->type, 20))
                {
                    return true;
                }
            }
        }

        pstLayers = pstLayers->next;
    }

    return false;
}

bool IsOnTileOfType(
    const char   *pacType,
    const double  dPosX,
    const double  dPosY,
    const uint8_t u8EntityHeight,
    Map        **pstMap)
{
    if (IsMapCoordOfType(pacType, dPosX, dPosY + (double)(u8EntityHeight / 2.f), &(*pstMap)))
    {
        return true;
    }

    return false;
}

void SetGravitation(
    const double  dGravitation,
    const bool    bUseTmxConstant,
    Map         **pstMap)
{
    if (bUseTmxConstant)
    {
        double       *dTmxConstant = NULL;
        tmx_property *pProperty;

        dTmxConstant = &(*pstMap)->dGravitation;
        pProperty    = (*pstMap)->pstTmxMap->properties;
        tmx_property_foreach(pProperty, _GetGravitation, (void *)dTmxConstant);
        (*pstMap)->dGravitation = *dTmxConstant;
    }
    else
    {
        (*pstMap)->dGravitation = dGravitation;
    }

    SDL_Log(
        "Set gravitational constant to %f (g*%dpx/s^2).\n",
        (*pstMap)->dGravitation,
        (*pstMap)->u8MeterInPixel);
}

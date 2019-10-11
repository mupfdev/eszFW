/**
 * @file      Map.c
 * @ingroup   Map
 * @defgroup  Map Map/Level handler
 * @author    Michael Fitzmayer
 * @copyright "THE BEER-WARE LICENCE" (Revision 42)
 */

#include <SDL.h>
#include <SDL_image.h>
#include "Constants.h"
#include "Map.h"

static Uint16 _ClearGidFlags(Uint16 u16Gid)
{
    return u16Gid & TMX_FLIP_BITS_REMOVAL;
}

static void _GetObject(tmx_object* pstTmxObject, Uint16 u16Index, Object astObject[])
{
    if (pstTmxObject)
    {
        astObject[u16Index].u16Id     = pstTmxObject->id;
        astObject[u16Index].u32PosX   = pstTmxObject->x;
        astObject[u16Index].u32PosY   = pstTmxObject->y;
        astObject[u16Index].u16Width  = pstTmxObject->width;
        astObject[u16Index].u16Height = pstTmxObject->height;

        SDL_strlcpy(astObject[u16Index].acName, pstTmxObject->name, OBJECT_NAME_LEN - 1);
        SDL_strlcpy(astObject[u16Index].acType, pstTmxObject->type, OBJECT_TYPE_LEN - 1);

        astObject[u16Index].stBB.dBottom =
            (double)astObject[u16Index].u32PosY + (double)astObject[u16Index].u16Height / 2.f;
        astObject[u16Index].stBB.dLeft =
            (double)astObject[u16Index].u32PosX - (double)astObject[u16Index].u16Width / 2.f;
        astObject[u16Index].stBB.dRight =
            (double)astObject[u16Index].u32PosX + (double)astObject[u16Index].u16Width / 2.f;
        astObject[u16Index].stBB.dTop =
            (double)astObject[u16Index].u32PosY - (double)astObject[u16Index].u16Height / 2.f;

        if (astObject[u16Index].stBB.dLeft <= 0)
        {
            astObject[u16Index].stBB.dLeft = 0;
        }

        if (astObject[u16Index].stBB.dTop <= 0)
        {
            astObject[u16Index].stBB.dTop = 0;
        }
    }

    if (pstTmxObject && pstTmxObject->next)
    {
        u16Index++;
        _GetObject(pstTmxObject->next, u16Index, &(*astObject));
    }
}

static void _GetObjectCount(tmx_object* pstObject, Uint16** pu16ObjectCount)
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

static void _GetGravitation(tmx_property* pProperty, void* dGravitation)
{
    if (0 == SDL_strncmp(pProperty->name, "Gravitation", 11))
    {
        if (PT_FLOAT == pProperty->type)
        {
            *((double*)dGravitation) = pProperty->value.decimal;
        }
    }
}

/**
 * @brief   Draw Map
 * @details Draws the map on screen
 * @param   u16Index
 *          The texture index; the total amount of layers per map is
 *          defined by MAP_TEXTURES.
 * @param   bRenderAnimTiles
 *          If set to 1, all animated tiles will be rendered in this
 *          call.
 * @param   bRenderBgColour
 *          Determine if the map's background colour should be rendered
 * @param   pacLayerName
 *          Sub-string of the layer(s) to render
 * @param   dCameraPosX
 *          Camera position along the x-axis
 * @param   dCameraPosY
 *          Camera position along the y-axis
 * @param   pstMap
 *          Pointer to map handle
 * @param   pstRenderer
 *          Pointer to SDL2 rendering context
 * @return  Error code
 * @retval  0:  OK
 * @retval  -1: Error
 */
Sint8 Map_Draw(
    const Uint16   u16Index,
    const SDL_bool bRenderAnimTiles,
    const SDL_bool bRenderBgColour,
    const char*    pacLayerName,
    const double   dCameraPosX,
    const double   dCameraPosY,
    Map*           pstMap,
    SDL_Renderer*  pstRenderer)
{
    double     dDeltaTime = (double)APPROX_TIME_PER_FRAME / (double)TIME_FACTOR;
    tmx_layer* pstLayer   = pstMap->pstTmxMap->ly_head;

    // Load tileset image once.
    if (!pstMap->pstTileset)
    {
        pstMap->pstTileset = IMG_LoadTexture(pstRenderer, pstMap->acTilesetImage);
        if (!pstMap->pstTileset)
        {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "%s\n", IMG_GetError());
            return -1;
        }
    }

    // Update and render animated tiles.
    pstMap->dAnimDelay += dDeltaTime;

    if (0 < pstMap->u16AnimTileSize &&
        pstMap->dAnimDelay > (1.f / pstMap->dAnimSpeed - dDeltaTime) && bRenderAnimTiles)
    {
        if (!pstMap->pstAnimTexture)
        {
            pstMap->pstAnimTexture = SDL_CreateTexture(
                pstRenderer,
                SDL_PIXELFORMAT_ARGB8888,
                SDL_TEXTUREACCESS_TARGET,
                pstMap->pstTmxMap->width * pstMap->pstTmxMap->tile_width,
                pstMap->pstTmxMap->height * pstMap->pstTmxMap->tile_height);
        }

        if (!pstMap->pstAnimTexture)
        {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "%s\n", SDL_GetError());
            return -1;
        }

        if (0 != SDL_SetRenderTarget(pstRenderer, pstMap->pstAnimTexture))
        {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "%s\n", SDL_GetError());
            return -1;
        }

        for (Uint16 u16Idx = 0; u16Idx < pstMap->u16AnimTileSize; u16Idx++)
        {
            Uint16       u16Gid        = pstMap->acAnimTile[u16Idx].u16Gid;
            Uint16       u16TileId     = pstMap->acAnimTile[u16Idx].u16TileId + 1;
            Uint16       u16NextTileId = 0;
            SDL_Rect     stDst;
            SDL_Rect     stSrc;
            tmx_tileset* pstTS;

            pstTS   = pstMap->pstTmxMap->tiles[1]->tileset;
            stSrc.x = pstMap->pstTmxMap->tiles[u16TileId]->ul_x;
            stSrc.y = pstMap->pstTmxMap->tiles[u16TileId]->ul_y;
            stSrc.w = stDst.w = pstTS->tile_width;
            stSrc.h = stDst.h = pstTS->tile_height;
            stDst.x           = pstMap->acAnimTile[u16Idx].s16DstX;
            stDst.y           = pstMap->acAnimTile[u16Idx].s16DstY;
            SDL_RenderCopy(pstRenderer, pstMap->pstTileset, &stSrc, &stDst);

            pstMap->acAnimTile[u16Idx].u8FrameCount++;
            if (pstMap->acAnimTile[u16Idx].u8FrameCount >= pstMap->acAnimTile[u16Idx].u8AnimLen)
            {
                pstMap->acAnimTile[u16Idx].u8FrameCount = 0;
            }

            u16NextTileId = pstMap->pstTmxMap->tiles[u16Gid]
                                ->animation[pstMap->acAnimTile[u16Idx].u8FrameCount]
                                .tile_id;
            pstMap->acAnimTile[u16Idx].u16TileId = u16NextTileId;
        }

        if (pstMap->dAnimDelay > 1.f / pstMap->dAnimSpeed)
        {
            pstMap->dAnimDelay = 0.f;
        }

        // Switch back to default render target.
        if (0 != SDL_SetRenderTarget(pstRenderer, NULL))
        {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "%s\n", SDL_GetError());
            return -1;
        }

        if (0 != SDL_SetTextureBlendMode(pstMap->pstAnimTexture, SDL_BLENDMODE_BLEND))
        {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "%s\n", SDL_GetError());
            return -1;
        }
    }

    // The texture has already been rendered and can be drawn.
    if (pstMap->pstTexture[u16Index])
    {
        double   dRenderPosX = pstMap->dPosX - dCameraPosX;
        double   dRenderPosY = pstMap->dPosY - dCameraPosY;
        SDL_Rect stDst       = { dRenderPosX,
                           dRenderPosY,
                           pstMap->pstTmxMap->width * pstMap->pstTmxMap->tile_width,
                           pstMap->pstTmxMap->height * pstMap->pstTmxMap->tile_height };

        if (-1 ==
            SDL_RenderCopyEx(
                pstRenderer, pstMap->pstTexture[u16Index], NULL, &stDst, 0, NULL, SDL_FLIP_NONE))
        {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "%s\n", SDL_GetError());
            return -1;
        }

        if (bRenderAnimTiles)
        {
            if (pstMap->pstAnimTexture)
            {
                if (-1 ==
                    SDL_RenderCopyEx(
                        pstRenderer, pstMap->pstAnimTexture, NULL, &stDst, 0, NULL, SDL_FLIP_NONE))
                {
                    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "%s\n", SDL_GetError());
                    return -1;
                }
            }
        }

        return 0;
    }

    // Otherwise render the texture once.
    pstMap->pstTexture[u16Index] = SDL_CreateTexture(
        pstRenderer,
        SDL_PIXELFORMAT_ARGB8888,
        SDL_TEXTUREACCESS_TARGET,
        pstMap->pstTmxMap->width * pstMap->pstTmxMap->tile_width,
        pstMap->pstTmxMap->height * pstMap->pstTmxMap->tile_height);

    if (!pstMap->pstTexture[u16Index])
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "%s\n", SDL_GetError());
        return -1;
    }

    if (0 != SDL_SetRenderTarget(pstRenderer, pstMap->pstTexture[u16Index]))
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "%s\n", SDL_GetError());
        return -1;
    }

    if (bRenderBgColour)
    {
        SDL_SetRenderDrawColor(
            pstRenderer,
            (pstMap->pstTmxMap->backgroundcolor >> 16) & 0xFF,
            (pstMap->pstTmxMap->backgroundcolor >> 8) & 0xFF,
            (pstMap->pstTmxMap->backgroundcolor) & 0xFF,
            255);
    }

    while (pstLayer)
    {
        SDL_bool     bRenderLayer = 1;
        Uint16       u16Gid;
        SDL_Rect     stDst;
        SDL_Rect     stSrc;
        tmx_tileset* pstTS;

        if (L_LAYER == pstLayer->type)
        {
            if (pacLayerName)
            {
                if (!SDL_strstr(pstLayer->name, pacLayerName))
                {
                    bRenderLayer = 0;
                }
            }
            if (pstLayer->visible && bRenderLayer)
            {
                for (Uint16 u16IndexH = 0; u16IndexH < pstMap->pstTmxMap->height; u16IndexH++)
                {
                    for (Uint32 u16IndexW = 0; u16IndexW < pstMap->pstTmxMap->width; u16IndexW++)
                    {
                        u16Gid = _ClearGidFlags(
                            pstLayer->content
                                .gids[(u16IndexH * pstMap->pstTmxMap->width) + u16IndexW]);
                        if (pstMap->pstTmxMap->tiles[u16Gid])
                        {
                            pstTS   = pstMap->pstTmxMap->tiles[1]->tileset;
                            stSrc.x = pstMap->pstTmxMap->tiles[u16Gid]->ul_x;
                            stSrc.y = pstMap->pstTmxMap->tiles[u16Gid]->ul_y;
                            stSrc.w = stDst.w = pstTS->tile_width;
                            stSrc.h = stDst.h = pstTS->tile_height;
                            stDst.x           = u16IndexW * pstTS->tile_width;
                            stDst.y           = u16IndexH * pstTS->tile_height;
                            SDL_RenderCopy(pstRenderer, pstMap->pstTileset, &stSrc, &stDst);

                            if (bRenderAnimTiles && pstMap->pstTmxMap->tiles[u16Gid]->animation)
                            {
                                Uint8  u8AnimLen;
                                Uint16 u16TileId;
                                u8AnimLen = pstMap->pstTmxMap->tiles[u16Gid]->animation_len;
                                u16TileId = pstMap->pstTmxMap->tiles[u16Gid]->animation[0].tile_id;
                                pstMap->acAnimTile[pstMap->u16AnimTileSize].u16Gid    = u16Gid;
                                pstMap->acAnimTile[pstMap->u16AnimTileSize].u16TileId = u16TileId;
                                pstMap->acAnimTile[pstMap->u16AnimTileSize].s16DstX   = stDst.x;
                                pstMap->acAnimTile[pstMap->u16AnimTileSize].s16DstY   = stDst.y;
                                pstMap->acAnimTile[pstMap->u16AnimTileSize].u8FrameCount = 0;
                                pstMap->acAnimTile[pstMap->u16AnimTileSize].u8AnimLen = u8AnimLen;
                                pstMap->u16AnimTileSize++;

                                // Prevent buffer overflow.
                                if (pstMap->u16AnimTileSize >= ANIM_TILE_MAX)
                                {
                                    pstMap->u16AnimTileSize = ANIM_TILE_MAX;
                                }
                            }
                        }
                    }
                }
                SDL_Log("Render TMX map layer: %s\n", pstLayer->name);
            }
        }
        pstLayer = pstLayer->next;
    }
    // Switch back to default render target.
    if (0 != SDL_SetRenderTarget(pstRenderer, NULL))
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "%s\n", SDL_GetError());
        return -1;
    }

    if (0 != SDL_SetTextureBlendMode(pstMap->pstTexture[u16Index], SDL_BLENDMODE_BLEND))
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "%s\n", SDL_GetError());
        return -1;
    }

    return 0;
}

/**
 * @brief   Free map
 * @details Frees up allocated memory and unloads map
 * @param   pstMap
 *          Pointer to map handle
 */
void Map_Free(Map* pstMap)
{
    if (pstMap)
    {
        if (!pstMap->pstTmxMap)
        {
            tmx_map_free(pstMap->pstTmxMap);
        }

        if (pstMap->pstTileset)
        {
            SDL_DestroyTexture(pstMap->pstTileset);
        }

        for (Uint8 u8Index = 0; u8Index < MAP_TEXTURES; u8Index++)
        {
            if (pstMap->pstTexture[u8Index])
            {
                SDL_DestroyTexture(pstMap->pstTexture[u8Index]);
            }
        }

        if (pstMap->pstAnimTexture)
        {
            SDL_DestroyTexture(pstMap->pstAnimTexture);
        }

        SDL_free(pstMap);
        SDL_Log("Unload TMX map.\n");
    }
}

/**
 * @brief   Get objects
 * @details Retreive objects from map
 * @param   pstMap
 *          Pointer to map handle
 * @param   astObject
 *          Array to store the objects
 */
void Map_GetObjects(const Map* pstMap, Object astObject[])
{
    tmx_layer*  pstLayer = pstMap->pstTmxMap->ly_head;
    tmx_object* pstTmxObject;

    while (pstLayer)
    {
        if (L_OBJGR == pstLayer->type)
        {
            pstTmxObject = pstLayer->content.objgr->head;
            _GetObject(pstTmxObject, 0, &(*astObject));
        }
        pstLayer = pstLayer->next;
    }
}

/**
 * @brief   Get object count
 * @details Return total object count of map
 * @param   pstMap
 *          Pointer to map handle
 * @return  Number of objects in the map
 */
Uint16 Map_GetObjectCount(Map* pstMap)
{
    Uint16     u16ObjectCount  = 0;
    Uint16*    pu16ObjectCount = &u16ObjectCount;
    tmx_layer* pstLayer;

    if (pstMap->u16ObjectCount)
    {
        return pstMap->u16ObjectCount;
    }

    pstLayer = pstMap->pstTmxMap->ly_head;
    while (pstLayer)
    {
        if (L_OBJGR == pstLayer->type)
        {
            _GetObjectCount(pstLayer->content.objgr->head, &pu16ObjectCount);
        }

        pstLayer = pstLayer->next;
    }
    pstMap->u16ObjectCount = u16ObjectCount;

    return u16ObjectCount;
}

/**
 * @brief   Get object name
 * @details Get name of an object
 * @param   pstObject
 *          Pointer to object handle
 * @return  The object name as a string
 */
char* Map_GetObjectName(Object* pstObject)
{
    return pstObject->acName;
}

/**
 * @brief   Get object type
 * @details Get type of an object
 * @param   pstObject
 *          Pointer to object handle
 * @return  The object type as a string
 */
char* Map_GetObjectType(Object* pstObject)
{
    return pstObject->acType;
}

/**
 * @brief   Initialise map
 * @details Initialises/load map
 * @param   pacFileName
 *          Path and filename of the TMX map to load
 * @param   pacTilesetImage
 *          Path and filename of the tileset image
 * @param   u8MeterInPixel
 *          Definition of meter in pixel
 * @param   pstMap
 *          Pointer to map handle
 * @return  Error code
 * @retval  0:  OK
 * @retval  -1: Error
 */
Sint8 Map_Init(
    const char* pacFileName,
    const char* pacTilesetImage,
    const Uint8 u8MeterInPixel,
    Map**       pstMap)
{
    Uint32 u32Size = sizeof(struct Map_t) + TS_IMG_PATH_LEN;

    *pstMap = SDL_calloc(u32Size, sizeof(Sint8));
    if (!*pstMap)
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "InitMap(): error allocating memory.\n");
        return -1;
    }

    (*pstMap)->pstTmxMap = tmx_load(pacFileName);
    if (!(*pstMap)->pstTmxMap)
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "%s\n", tmx_strerr());
        return -1;
    }

    (*pstMap)->u16ObjectCount = Map_GetObjectCount(*pstMap);

    if ((*pstMap)->u16ObjectCount > 0)
    {
        u32Size = u32Size + ((*pstMap)->u16ObjectCount * sizeof(struct Object_t));
        *pstMap = SDL_realloc(*pstMap, u32Size);

        if (!*pstMap)
        {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "InitMap(): error re-allocating memory.\n");
            return -1;
        }

        Map_GetObjects((*pstMap), (*pstMap)->astObject);
        #ifdef DEBUG
        Map_ShowMapObjects(*pstMap);
        #endif
    }

    (*pstMap)->u16Height      = (*pstMap)->pstTmxMap->height * (*pstMap)->pstTmxMap->tile_height;
    (*pstMap)->u16Width       = (*pstMap)->pstTmxMap->width * (*pstMap)->pstTmxMap->tile_width;
    (*pstMap)->u8MeterInPixel = u8MeterInPixel;
    (*pstMap)->dAnimSpeed     = 6.25f;

    SDL_strlcat((*pstMap)->acTilesetImage, pacTilesetImage, TS_IMG_PATH_LEN - 1);

    for (Uint8 u8Index = 0; u8Index < MAP_TEXTURES; u8Index++)
    {
        (*pstMap)->pstTexture[u8Index] = NULL;
    }
    (*pstMap)->pstAnimTexture = NULL;
    (*pstMap)->pstTileset     = NULL;

    SDL_Log(
        "Load TMX map file: %s containing %d object(s).\n", pacFileName, (*pstMap)->u16ObjectCount);
    Map_SetGravitation(0, 1, *pstMap);

    return 0;
}

/**
 * @brief   Check if map coordinate is of specific type
 * @details Checks if a coodinate is of a specific type
 * @param   pacType
 *          Name of the type to check for
 * @param   pstMap
 *          Pointer to map handle
 * @param   dPosX
 *          Coordinate along the x-axis
 * @param   dPosY
 *          Coordinate along the y-axis
 * @return  Boolean state
 * @retval  SDL_TRUE: Map coordinate is of specific type
 * @retval  SDL_FALSE: Map coordinate is not of specific type
 */
SDL_bool Map_IsCoordOfType(const char* pacType, const Map* pstMap, double dPosX, double dPosY)
{
    tmx_layer* pstLayers;
    dPosX /= (double)pstMap->pstTmxMap->tile_width;
    dPosY /= (double)pstMap->pstTmxMap->tile_height;

    // Set boundaries to prevent segfault.
    if ((dPosX < 0.0) || (dPosY < 0.0) || (dPosX > (double)pstMap->pstTmxMap->width) ||
        (dPosY > (double)pstMap->pstTmxMap->height))
    {
        return SDL_FALSE;
    }

    pstLayers = pstMap->pstTmxMap->ly_head;
    while (pstLayers)
    {
        if (pstLayers->type != L_LAYER)
        {
            pstLayers = pstLayers->next;
            continue;
        }

        Uint16 u16Gid = _ClearGidFlags(
            pstLayers->content.gids[((int32_t)dPosY * pstMap->pstTmxMap->width) + (int32_t)dPosX]);

        if (0 == u16Gid)
        {
            pstLayers = pstLayers->next;
            continue;
        }

        if (pstMap->pstTmxMap->tiles[u16Gid])
        {
            if (pstMap->pstTmxMap->tiles[u16Gid]->type)
            {
                if (0 == SDL_strncmp(pacType, pstMap->pstTmxMap->tiles[u16Gid]->type, 20))
                {
                    return SDL_TRUE;
                }
            }
        }

        pstLayers = pstLayers->next;
    }

    return SDL_FALSE;
}

/**
 * @brief   Determine if object is of specific type
 * @details Determines if an object is of a specific type
 * @param   pacType
 *          The type name to check for
 * @param   pstObject
 *          Pointer to object handle
 * @return  Boolean state
 * @retval  SDL_TRUE: Object is of specific type
 * @retval  SDL_FALSE: Object is not of specific type
 */
SDL_bool Map_IsObjectOfType(const char* pacType, Object* pstObject)
{
    if (0 == SDL_strncmp(pacType, Map_GetObjectType(pstObject), OBJECT_TYPE_LEN))
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

/**
 * @brief   Determine if entity/object is on-top of tile of specific
 *          type
 * @details Determines if an entity/object is on-top of a tile of a
 *          specific type
 * @param   pacType
 *          The type name to check for
 * @param   dPosX
 *          Position along the x-axis
 * @param   dPosY
 *          Position along the y-axis
 * @param   u8EntityHeight
 *          Height ob the entity/object in pixel
 * @param   pstMap
 *          Pointer to map handle
 * @return  Boolean state
 * @retval  SDL_TRUE: entity is on-top of tile of specific type
 * @retval  SDL_FALSE: entity is not on-top of tile of specific type
 */
SDL_bool Map_IsOnTileOfType(
    const char*  pacType,
    const double dPosX,
    const double dPosY,
    const Uint8  u8EntityHeight,
    const Map*   pstMap)
{
    return Map_IsCoordOfType(pacType, pstMap, dPosX, dPosY + (double)(u8EntityHeight / 2.f));
}

/**
 * @brief   Set map gravitation
 * @details Sets the gravitational constant of the map
 * @param   dGravitation
 * @param   bUseTmxConstant
 * @param   pstMap
 */
void Map_SetGravitation(const double dGravitation, const SDL_bool bUseTmxConstant, Map* pstMap)
{
    if (bUseTmxConstant)
    {
        double*       dTmxConstant = NULL;
        tmx_property* pProperty;

        dTmxConstant = &pstMap->dGravitation;
        pProperty    = pstMap->pstTmxMap->properties;
        tmx_property_foreach(pProperty, _GetGravitation, (void*)dTmxConstant);
        pstMap->dGravitation = *dTmxConstant;
    }
    else
    {
        pstMap->dGravitation = dGravitation;
    }

    SDL_Log(
        "Set gravitational constant to %f (g*%dpx/s^2).\n",
        pstMap->dGravitation,
        pstMap->u8MeterInPixel);
}

/**
 * @brief   Set speed of animated tiles
 * @details Sets the speed of animated tiles
 * @param   dAnimSpeed
 *          Animation speed
 * @param   pstMap
 *          Pointer to map handle
 */
void Map_SetTileAnimationSpeed(const double dAnimSpeed, Map* pstMap)
{
    pstMap->dAnimSpeed = dAnimSpeed;
}

/**
 * @brief   Show map objects
 * @details Prints a list of all map objects
 * @param   pstMap
 *          Pointer to map handle
 * @remark  This function is mainly for debugging purposes
 */
void Map_ShowObjects(const Map* pstMap)
{
    if ((*pstMap).u16ObjectCount > 0)
    {
        for (Uint16 u16Index = 0; u16Index < (*pstMap).u16ObjectCount; u16Index++)
        {
            SDL_Log("Object %d\n", u16Index);
            SDL_Log("  ID:   %d\n", (*pstMap).astObject[u16Index].u16Id);
            SDL_Log("  X:    %d\n", (*pstMap).astObject[u16Index].u32PosX);
            SDL_Log("  Y:    %d\n", (*pstMap).astObject[u16Index].u32PosY);
            SDL_Log("  W:    %d\n", (*pstMap).astObject[u16Index].u16Width);
            SDL_Log("  H:    %d\n", (*pstMap).astObject[u16Index].u16Height);
            SDL_Log("  NAME: %s\n", (*pstMap).astObject[u16Index].acName);
            SDL_Log("  TYPE: %s\n", (*pstMap).astObject[u16Index].acType);
            SDL_Log("  BB B: %f\n", (*pstMap).astObject[u16Index].stBB.dBottom);
            SDL_Log("  BB L: %f\n", (*pstMap).astObject[u16Index].stBB.dLeft);
            SDL_Log("  BB R: %f\n", (*pstMap).astObject[u16Index].stBB.dRight);
            SDL_Log("  BB T: %f\n", (*pstMap).astObject[u16Index].stBB.dTop);
        }
    }
}

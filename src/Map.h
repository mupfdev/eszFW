// SPDX-License-Identifier: Beerware
/**
 * @file    Map.h
 * @ingroup Map
 */
#pragma once

#include <SDL.h>
#include <tmx.h>
#include "AABB.h"

/**
 * @typedef MapConstants
 * @brief   Map constants handle type
 * @enum    MapConstants_t
 * @brief   Map constants enumeration
 */
typedef enum MapConstants_t
{
    ANIM_TILE_MAX   = 500,  ///< Max. number of animated tiles
    MAP_TEXTURES    = 4,    ///< Max. textures per map (not to be confused with map layers)
    TS_IMG_PATH_LEN = 64,   ///< Max. tileset image path length
    OBJECT_NAME_LEN = 50,   ///< Max. object name length
    OBJECT_TYPE_LEN = 15    ///< Max. object type length

} MapConstants;

/**
 * @typedef AnimTile
 * @brief   Animated tile handle type
 * @struct  AnimTile_t
 * @brief   Animated tile data
 */
typedef struct AnimTile_t
{
    Uint16 u16Gid;        ///< GID
    Uint16 u16TileId;     ///< Tile ID
    Sint16 s16DstX;       ///< Destination coordinate along the x-axis
    Sint16 s16DstY;       ///< Destination coordinate along the y-axis
    Uint8  u8FrameCount;  ///< Frame count
    Uint8  u8AnimLen;     ///< Animation length

} AnimTile;

/**
 * @typedef Object
 * @brief   Object handle type
 * @struct  Object_t
 * @brief   Object handle data
 */
typedef struct Object_t
{
    Uint16 u16Id;                    ///< Object ID
    Uint32 u32PosX;                  ///< Position along the x-axis
    Uint32 u32PosY;                  ///< Position along the y-axis
    Uint16 u16Width;                 ///< Object width in pixel
    Uint16 u16Height;                ///< Object height in pixel
    char   acName[OBJECT_NAME_LEN];  ///< Object name
    char   acType[OBJECT_TYPE_LEN];  ///< Object type
    AABB   stBB;                     ///< Axis-aligned bounding box

} Object;

/**
 * @typedef Map
 * @brief   Map handle type
 * @struct  Map_t
 * @brief   Map handle data
 */
typedef struct Map_t
{
    tmx_map*     pstTmxMap;                        ///< TMX map handle
    SDL_Texture* pstAnimTexture;                   ///< Texture for animated tiles
    SDL_Texture* pstTexture[MAP_TEXTURES];         ///< Map textures
    SDL_Texture* pstTileset;                       ///< Tileset texture
    Uint16       u16Height;                        ///< Map height in pixel
    Uint16       u16Width;                         ///< Map width in pixel
    double       dPosX;                            ///< Position along the x-axis
    double       dPosY;                            ///< Position along the y-axis
    double       dGravitation;                     ///< Gravitational constant
    Uint8        u8MeterInPixel;                   ///< Definition of meter in pixel
    char         acTilesetImage[TS_IMG_PATH_LEN];  ///< Tileset image
    double       dAnimDelay;                       ///< Animation delay
    double       dAnimSpeed;                       ///< Animation speed
    Uint16       u16AnimTileSize;                  ///< Animated tile size
    AnimTile     acAnimTile[ANIM_TILE_MAX];        ///< Animated tiles
    Uint16       u16ObjectCount;                   ///< Object count
    Object       astObject[];                      ///< Objects

} Map;

Sint8 Map_Draw(
    const Uint16   u16Index,
    const SDL_bool bRenderAnimTiles,
    const SDL_bool bRenderBgColour,
    const char*    pacLayerName,
    const double   dCameraPosX,
    const double   dCameraPosY,
    Map*           pstMap,
    SDL_Renderer*  pstRenderer);

void   Map_Free(Map* pstMap);
void   Map_GetObjects(const Map* pstMap, Object astObject[]);
Uint16 Map_GetObjectCount(Map* pstMap);
char*  Map_GetObjectName(Object* pstObject);
char*  Map_GetObjectType(Object* pstObject);

Sint8 Map_Init(
    const char* pacFileName,
    const char* pacTilesetImage,
    const Uint8 u8MeterInPixel,
    Map**       pstMap);

SDL_bool Map_IsCoordOfType(const char* pacType, const Map* pstMap, double dPosX, double dPosY);

SDL_bool Map_IsObjectOfType(const char* pacType, Object* pstObject);

SDL_bool Map_IsOnTileOfType(
    const char*  pacType,
    const double dPosX,
    const double dPosY,
    const Uint8  u8EntityHeight,
    const Map*   pstMap);

void Map_SetGravitation(const double dGravitation, const SDL_bool bUseTmxConstant, Map* pstMap);
void Map_SetTileAnimationSpeed(const double dAnimSpeed, Map* pstMap);
void Map_ShowObjects(const Map* pstMap);

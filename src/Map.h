/**
 * @file Map.h
 * @ingroup Map
 */

#ifndef _MAP_H_
#define _MAP_H_

#include <SDL.h>
#include <tmx.h>
#include "AABB.h"

typedef enum MapConstants_t
{
    ANIM_TILE_MAX   = 500,
    // Max textures per map (not to be confused with map layers).
    MAP_TEXTURES    = 4,
    TS_IMG_PATH_LEN = 64,
    OBJECT_NAME_LEN = 50,
    OBJECT_TYPE_LEN = 15
} MapConstants;

typedef struct AnimTile_t
{
    Uint16 u16Gid;
    Uint16 u16TileId;
    Sint16 s16DstX;
    Sint16 s16DstY;
    Uint8  u8FrameCount;
    Uint8  u8AnimLen;
} AnimTile;

typedef struct Object_t
{
    Uint16 u16Id;
    Uint32 u32PosX;
    Uint32 u32PosY;
    Uint16 u16Width;
    Uint16 u16Height;
    char   acName[OBJECT_NAME_LEN];
    char   acType[OBJECT_TYPE_LEN];
    AABB   stBB;
} Object;

typedef struct Map_t
{
    tmx_map     *pstTmxMap;
    SDL_Texture *pstAnimTexture;
    SDL_Texture *pstTexture[MAP_TEXTURES];
    SDL_Texture *pstTileset;
    Uint16       u16Height;
    Uint16       u16Width;
    double       dPosX;
    double       dPosY;
    double       dGravitation;
    Uint8        u8MeterInPixel;
    char         acTilesetImage[TS_IMG_PATH_LEN];
    double       dAnimDelay;
    double       dAnimSpeed;
    Uint16       u16AnimTileSize;
    AnimTile     acAnimTile[ANIM_TILE_MAX];
    Uint16       u16ObjectCount;
    Object       astObject[];
} Map;

Sint8 DrawMap(
    const Uint16   u16Index,
    const SDL_bool bRenderAnimTiles,
    const SDL_bool bRenderBgColour,
    const char    *pacLayerName,
    const double   dCameraPosX,
    const double   dCameraPosY,
    Map           *pstMap,
    SDL_Renderer  *pstRenderer);

void FreeMap(Map *pstMap);
void GetObjects(const Map *pstMap, Object astObject[]);
Uint16 GetObjectCount(Map *pstMap);
char *GetObjectName(Object *pstObject);
char *GetObjectType(Object *pstObject);

Sint8 InitMap(const char *pacFileName, const char *pacTilesetImage, const Uint8 u8MeterInPixel, Map **pstMap);

SDL_bool IsMapCoordOfType(
    const char *pacType,
    const Map  *pstMap,
    double      dPosX,
    double      dPosY);

SDL_bool IsObjectOfType(const char *pacType, Object *pstObject);

SDL_bool IsOnTileOfType(
    const char  *pacType,
    const double dPosX,
    const double dPosY,
    const Uint8  u8EntityHeight,
    const Map   *pstMap);

void SetGravitation(const double dGravitation, const SDL_bool bUseTmxConstant, Map *pstMap);
void SetTileAnimationSpeed(const double dAnimSpeed, Map *pstMap);
void ShowMapObjects(const Map *pstMap);

#endif // _MAP_H_

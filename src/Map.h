/**
 * @file Map.h
 * @ingroup Map
 */

#ifndef _MAP_H_
#define _MAP_H_

#include <SDL.h>
#include <stdbool.h>
#include <stdint.h>
#include <tmx.h>

typedef enum MapConstants_t
{
    NORMAL_LAYER    = 0,
    ANIM_LAYER      = 1,
    ANIM_TILE_MAX   = 500,
    MAP_TEXTURES    = 4,
    TS_IMG_PATH_LEN = 64
} MapConstants;

typedef struct AnimTile_t
{
    uint16_t u16Gid;
    uint16_t u16TileId;
    int16_t  s16DstX;
    int16_t  s16DstY;
    uint8_t  u8FrameCount;
    uint8_t  u8AnimLen;
} AnimTile;

typedef struct Map_t
{
    tmx_map     *pstTmxMap;
    SDL_Texture *pstTexture[2][MAP_TEXTURES];
    SDL_Texture *pstTileset;
    uint16_t     u16Height;
    uint16_t     u16Width;
    double       dPosX;
    double       dPosY;
    double       dGravitation;
    uint8_t      u8MeterInPixel;
    char         acTilesetImage[TS_IMG_PATH_LEN];
    uint16_t     u16AnimTileSize;
    double       dAnimDelay;
    double       dAnimSpeed;
    AnimTile     acAnimTile[ANIM_TILE_MAX];
} Map;

typedef struct Object_t
{
    double dPosX;
    double dPosY;
} Object;

int DrawMap(
    const uint16_t u16Index,
    const bool     bRenderAnimTiles,
    const bool     bRenderBgColour,
    const char    *pacLayerName,
    const double   dCameraPosX,
    const double   dCameraPosY,
    const double   dDeltaTime,
    Map           *pstMap,
    SDL_Renderer  *pstRenderer);

void     FreeMap(Map *pstMap);
void     FreeObject(Object *pstObject);
void     GetSingleObjectByName(const char *pacName, const Map *pstMap, Object **pstObject);
uint16_t GetObjectCount(const Map *pstMap);
int      InitMap(const char *pacFileName, const char *pacTilesetImage, const uint8_t u8MeterInPixel, Map **pstMap);
int      InitObject(Object **pstObject);

bool IsMapCoordOfType(
    const char *pacType,
    double      dPosX,
    double      dPosY,
    const Map  *pstMap);

bool IsOnTileOfType(
    const char   *pacType,
    const double  dPosX,
    const double  dPosY,
    const uint8_t u8EntityHeight,
    const Map    *pstMap);

void SetGravitation(const double dGravitation, const bool bUseTmxConstant, Map *pstMap);
void SetTileAnimationSpeed(const double dAnimSpeed, Map *pstMap);

#endif // _MAP_H_

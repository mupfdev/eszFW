/**
 * @file Map.h
 * @ingroup Map
 */

#ifndef _MAP_H_
#define _MAP_H_

#ifdef __ANDROID__
#include <SDL.h>
#else
#include <SDL2/SDL.h>
#endif
#include <stdbool.h>
#include <stdint.h>
#include <tmx.h>

#ifndef MAP_TEXTURES
#define MAP_TEXTURES 3
#endif

typedef struct Map_t
{
    tmx_map     *pstTmxMap;
    SDL_Texture *pstTexture[MAP_TEXTURES];
    uint16_t     u16Height;
    uint16_t     u16Width;
    double       dPosX;
    double       dPosY;
    double       dGravitation;
    uint8_t      u8MeterInPixel;
} Map;

typedef struct Object_t
{
    double dPosX;
    double dPosY;
} Object;

int DrawMap(
    const uint16_t u16Index,
    const char    *pacTilesetImageFileName,
    const bool     bRenderBgColour,
    const char    *pacLayerName,
    const double   dCameraPosX,
    const double   dCameraPosY,
    Map          **pstMap,
    SDL_Renderer **pstRenderer);

void FreeMap(Map **pstMap);
void FreeObject(Object **pstObject);

void GetSingleObjectByName(
    const char *pacName,
    Map       **pstMap,
    Object    **pstObject);

uint16_t GetObjectCount(Map **pstMap);

int InitMap(
    const char   *pacFileName,
    const uint8_t u8MeterInPixel,
    Map         **pstMap);

int InitObject(Object **pstObject);

bool IsMapCoordOfType(
    const char *pacType,
    double      dPosX,
    double      dPosY,
    Map       **pstMap);

bool IsOnTileOfType(
    const char   *pacType,
    const double  dPosX,
    const double  dPosY,
    const uint8_t u8EntityHeight,
    Map        **pstMap);

void SetGravitation(
    const double  dGravitation,
    const bool    bUseTmxConstant,
    Map         **pstMap);

#endif // _MAP_H_

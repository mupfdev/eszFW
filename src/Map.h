/**
 * @file Map.h
 * @ingroup Map
 */

#ifndef _MAP_H_
#define _MAP_H_

#include <SDL2/SDL.h>
#include <stdbool.h>
#include <stdint.h>
#include <tmx.h>
#include "Config.h"

typedef struct Map_t
{
    tmx_map     *pstTmxMap;
    SDL_Texture *pstTexture[MAP_TEXTURES];
    uint32_t     u32Height;
    uint32_t     u32Width;
    double       dPosX;
    double       dPosY;
    double       dGravitation;
} Map;

int DrawMap(
    const char    *pacTilesetImageFileName,
    const bool     bRenderBgColour,
    const uint16_t u16Index,
    const char    *pacLayerName,
    const double   dCameraPosX,
    const double   dCameraPosY,
    Map          **pstMap,
    SDL_Renderer **pstRenderer);

void FreeMap(Map **pstMap);
int InitMap(const char *pacFileName, Map **pstMap);

bool IsMapTileOfType(
    const char *pacType,
    const Map **pstMap,
    double      dPosX,
    double      dPosY);

void SetGravitation(
    const double  dGravitation,
    const bool    bUseTmxConstant,
    Map         **pstMap);

#endif // _MAP_H_

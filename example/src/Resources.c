/**
 * @file Resources.c
 * @ingroup Resources
 * @defgroup Resources
 * @author Michael Fitzmayer
 * @copyright "THE BEER-WARE LICENCE" (Revision 42)
 */

#include <eszFW.h>
#include <stdbool.h>
#include <stdlib.h>
#include "Resources.h"

void FreeResources(Resources **pstResources)
{
    FreeSprite(&(*pstResources)->pstSprite);
    FreeBackground(&(*pstResources)->pstBackground);
    FreeFont(&(*pstResources)->pstFont);
    FreeObject(&(*pstResources)->pstObject[OBJ_PLAYER_SPAWN]);
    FreeMap(&(*pstResources)->pstMap);
    FreeEntity(&(*pstResources)->pstEntity[ENT_PLAYER]);
    FreeInput(&(*pstResources)->pstInput);
    FreeCamera(&(*pstResources)->pstCamera);
    FreeVideo(&(*pstResources)->pstVideo);
    free(*pstResources);
}

int InitResources(Resources **pstResources)
{
    int         sReturnValue      = 0;
    bool        bFullscreen       = false;
    const char *pacBgFileNames[4] = {
        "res/images/sky.png",
        "res/images/clouds.png",
        "res/images/sea.png",
        "res/images/far-grounds.png",
    };

    *pstResources = NULL;
    *pstResources = malloc(sizeof(struct Resources_t));

    if (! *pstResources)
    {
        SDL_LogError(
            SDL_LOG_CATEGORY_APPLICATION,
            "Init(): error allocating memory.\n");
        return -1;
    }

    (*pstResources)->pstEntity[ENT_PLAYER]       = NULL;
    (*pstResources)->pstObject[OBJ_PLAYER_SPAWN] = NULL;;
    (*pstResources)->pstBackground               = NULL;
    (*pstResources)->pstCamera                   = NULL;
    (*pstResources)->pstInput                    = NULL;
    (*pstResources)->pstFont                     = NULL;
    (*pstResources)->pstVideo                    = NULL;
    (*pstResources)->pstMap                      = NULL;
    (*pstResources)->pstSprite                   = NULL;

    #ifdef __ANDROID__
    bFullscreen = true;
    #endif

    sReturnValue = InitVideo(
            "Rainbow Joe", 640, 360, 384, 216,
            bFullscreen, &(*pstResources)->pstVideo);
    if (-1 == sReturnValue) { goto exit; }

    sReturnValue = InitCamera(&(*pstResources)->pstCamera);
    if (-1 == sReturnValue) { goto exit; }

    sReturnValue = InitInput(
        (*pstResources)->pstVideo->s32LogicalWindowWidth,
        (*pstResources)->pstVideo->s32LogicalWindowHeight,
        &(*pstResources)->pstInput);
    if (-1 == sReturnValue) { goto exit; }

    sReturnValue = InitEntity(0, 0, 24, 40, &(*pstResources)->pstEntity[ENT_PLAYER]);
    if (-1 == sReturnValue) { goto exit; }

    sReturnValue = InitMap("res/maps/Demo.tmx", 32, &(*pstResources)->pstMap);
    if (-1 == sReturnValue) { goto exit; }

    sReturnValue = InitObject(&(*pstResources)->pstObject[OBJ_PLAYER_SPAWN]);
    if (-1 == sReturnValue) { goto exit; }

    sReturnValue = InitFont("res/ttf/FifteenNarrow.ttf", &(*pstResources)->pstFont);
    if (-1 == sReturnValue) { goto exit; }

    sReturnValue = InitBackground(
            4, pacBgFileNames, (*pstResources)->pstVideo->s32WindowWidth, BOTTOM,
            &(*pstResources)->pstVideo->pstRenderer, &(*pstResources)->pstBackground);
    if (-1 == sReturnValue) { goto exit; }

    /*sReturnValue = InitSprite(
            "res/images/characters_7.png", 736, 128, 0, 0,
            &(*pstResources)->pstSprite, &(*pstResources)->pstVideo->pstRenderer);
    if (-1 == sReturnValue) { goto exit; }*/

    sReturnValue = InitSprite(
            "res/images/player.png", 360, 80, 0, 0,
            &(*pstResources)->pstSprite, &(*pstResources)->pstVideo->pstRenderer);
    if (-1 == sReturnValue) { goto exit; }

    GetSingleObjectByName(
        "Player",
        &(*pstResources)->pstMap,
        &(*pstResources)->pstObject[OBJ_PLAYER_SPAWN]);

    SetPosition(
        (*pstResources)->pstObject[OBJ_PLAYER_SPAWN]->dPosX,
        (*pstResources)->pstObject[OBJ_PLAYER_SPAWN]->dPosY,
        &(*pstResources)->pstEntity[ENT_PLAYER]);

    SetFrameOffset(0, 0, &(*pstResources)->pstEntity[ENT_PLAYER]);
    SetFontColour(0xfe, 0x95, 0x14, &(*pstResources)->pstFont);

    exit:
    return sReturnValue;
}

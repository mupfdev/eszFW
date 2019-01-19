/**
 * @file GameData.c
 * @ingroup GameData
 * @defgroup GameData
 * @author Michael Fitzmayer
 * @copyright "THE BEER-WARE LICENCE" (Revision 42)
 */

#include <eszFW.h>
#include <stdbool.h>
#include <stdlib.h>
#include "GameData.h"

void FreeGameData(GameData **pstGameData)
{
    FreeSprite(&(*pstGameData)->pstSprite);
    FreeBackground(&(*pstGameData)->pstBackground);
    FreeFont(&(*pstGameData)->pstFont);
    FreeObject(&(*pstGameData)->pstObject[OBJ_PLAYER_SPAWN]);
    FreeMap(&(*pstGameData)->pstMap);
    FreeEntity(&(*pstGameData)->pstEntity[ENT_PLAYER]);
    FreeInput(&(*pstGameData)->pstInput);
    FreeCamera(&(*pstGameData)->pstCamera);
    FreeVideo(&(*pstGameData)->pstVideo);
    free(*pstGameData);
}

int InitGameData(GameData **pstGameData)
{
    int         sReturnValue      = 0;
    bool        bFullscreen       = false;
    const char *pacBgFileNames[4] = {
        "res/images/sky.png",
        "res/images/clouds.png",
        "res/images/sea.png",
        "res/images/far-grounds.png",
    };

    *pstGameData = NULL;
    *pstGameData = malloc(sizeof(struct GameData_t));

    if (! *pstGameData)
    {
        SDL_LogError(
            SDL_LOG_CATEGORY_APPLICATION,
            "Init(): error allocating memory.\n");
        return -1;
    }

    (*pstGameData)->pstEntity[ENT_PLAYER]       = NULL;
    (*pstGameData)->pstObject[OBJ_PLAYER_SPAWN] = NULL;;
    (*pstGameData)->pstBackground               = NULL;
    (*pstGameData)->pstCamera                   = NULL;
    (*pstGameData)->pstInput                    = NULL;
    (*pstGameData)->pstFont                     = NULL;
    (*pstGameData)->pstVideo                    = NULL;
    (*pstGameData)->pstMap                      = NULL;
    (*pstGameData)->pstSprite                   = NULL;

    #ifdef __ANDROID__
    bFullscreen = true;
    #endif

    sReturnValue = InitVideo(
            "Rainbow Joe", 640, 360, 384, 216,
            bFullscreen, &(*pstGameData)->pstVideo);
    if (-1 == sReturnValue) { goto exit; }

    sReturnValue = InitCamera(&(*pstGameData)->pstCamera);
    if (-1 == sReturnValue) { goto exit; }

    sReturnValue = InitInput(
        (*pstGameData)->pstVideo->s32WindowWidth,
        (*pstGameData)->pstVideo->s32WindowHeight,
        &(*pstGameData)->pstInput);
    if (-1 == sReturnValue) { goto exit; }

    sReturnValue = InitEntity(0, 0, 24, 40, &(*pstGameData)->pstEntity[ENT_PLAYER]);
    if (-1 == sReturnValue) { goto exit; }

    sReturnValue = InitMap("res/maps/Demo.tmx", 32, &(*pstGameData)->pstMap);
    if (-1 == sReturnValue) { goto exit; }

    sReturnValue = InitObject(&(*pstGameData)->pstObject[OBJ_PLAYER_SPAWN]);
    if (-1 == sReturnValue) { goto exit; }

    sReturnValue = InitFont("res/ttf/FifteenNarrow.ttf", &(*pstGameData)->pstFont);
    if (-1 == sReturnValue) { goto exit; }

    sReturnValue = InitBackground(
            4, pacBgFileNames, (*pstGameData)->pstVideo->s32WindowWidth, BOTTOM,
            &(*pstGameData)->pstVideo->pstRenderer, &(*pstGameData)->pstBackground);
    if (-1 == sReturnValue) { goto exit; }

    /*sReturnValue = InitSprite(
            "res/images/characters_7.png", 736, 128, 0, 0,
            &(*pstGameData)->pstSprite, &(*pstGameData)->pstVideo->pstRenderer);
    if (-1 == sReturnValue) { goto exit; }*/

    sReturnValue = InitSprite(
            "res/images/player.png", 360, 80, 0, 0,
            &(*pstGameData)->pstSprite, &(*pstGameData)->pstVideo->pstRenderer);
    if (-1 == sReturnValue) { goto exit; }

    GetSingleObjectByName(
        "Player",
        &(*pstGameData)->pstMap,
        &(*pstGameData)->pstObject[OBJ_PLAYER_SPAWN]);

    SetPosition(
        (*pstGameData)->pstObject[OBJ_PLAYER_SPAWN]->dPosX,
        (*pstGameData)->pstObject[OBJ_PLAYER_SPAWN]->dPosY,
        &(*pstGameData)->pstEntity[ENT_PLAYER]);

    SetFrameOffset(0, 0, &(*pstGameData)->pstEntity[ENT_PLAYER]);
    SetFontColour(255, 0, 0, &(*pstGameData)->pstFont);

    exit:
    return sReturnValue;
}

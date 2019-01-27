/**
 * @file Resources.c
 * @ingroup Resources
 * @defgroup Resources
 * @author Michael Fitzmayer
 * @copyright "THE BEER-WARE LICENCE" (Revision 42)
 */

#include <eszfw.h>
#include <stdbool.h>
#include <stdlib.h>
#include "Resources.h"

void FreeResources(Resources **pstRes)
{
    FreeMusic(&(*pstRes)->pstMusic);
    FreeAudio(&(*pstRes)->pstAudio);
    FreeSprite(&(*pstRes)->pstSprite);
    FreeBackground(&(*pstRes)->pstBackground);
    FreeFont(&(*pstRes)->pstFont);
    FreeObject(&(*pstRes)->pstObject[0]);
    FreeMap(&(*pstRes)->pstMap);
    FreeEntity(&(*pstRes)->pstEntity[0]);
    FreeCamera(&(*pstRes)->pstCamera);
    FreeVideo(&(*pstRes)->pstVideo);

    if (*pstRes)
    {
        free(*pstRes);
    }
}

int InitResources(Resources **pstRes)
{
    int         sReturnValue      = 0;
    const char *pacBgFileNames[4] = {
        "res/images/sky.png",
        "res/images/clouds.png",
        "res/images/sea.png",
        "res/images/far-grounds.png",
    };

    *pstRes = NULL;
    *pstRes = malloc(sizeof(struct Resources_t));

    if (! *pstRes)
    {
        SDL_LogError(
            SDL_LOG_CATEGORY_APPLICATION,
            "Init(): error allocating memory.\n");
        return -1;
    }

    (*pstRes)->pstEntity[0]       = NULL;
    (*pstRes)->pstObject[0] = NULL;;
    (*pstRes)->pstAudio                    = NULL;
    (*pstRes)->pstBackground               = NULL;
    (*pstRes)->pstCamera                   = NULL;
    (*pstRes)->pstFont                     = NULL;
    (*pstRes)->pstMap                      = NULL;
    (*pstRes)->pstMusic                    = NULL;
    (*pstRes)->pstSprite                   = NULL;
    (*pstRes)->pstVideo                    = NULL;

    sReturnValue = InitVideo(
        "Rainbow Joe", 640, 360, 384, 216,
        false, &(*pstRes)->pstVideo);
    if (-1 == sReturnValue)
    {
        goto exit;
    }

    sReturnValue = InitCamera(&(*pstRes)->pstCamera);
    if (-1 == sReturnValue)
    {
        goto exit;
    }

    sReturnValue = InitEntity(0, 0, 24, 40, &(*pstRes)->pstEntity[0]);
    if (-1 == sReturnValue)
    {
        goto exit;
    }

    sReturnValue = InitMap("res/maps/Demo.tmx", 32, &(*pstRes)->pstMap);
    if (-1 == sReturnValue)
    {
        goto exit;
    }

    sReturnValue = InitObject(&(*pstRes)->pstObject[0]);
    if (-1 == sReturnValue)
    {
        goto exit;
    }

    sReturnValue = InitFont("res/ttf/FifteenNarrow.ttf", &(*pstRes)->pstFont);
    if (-1 == sReturnValue)
    {
        goto exit;
    }

    sReturnValue = InitBackground(
        4, pacBgFileNames, (*pstRes)->pstVideo->s32WindowWidth, BOTTOM,
        &(*pstRes)->pstVideo->pstRenderer, &(*pstRes)->pstBackground);
    if (-1 == sReturnValue)
    {
        goto exit;
    }

    /*sReturnValue = InitSprite(
        "res/images/characters_7.png", 736, 128, 0, 0,
        &(*pstRes)->pstSprite, &(*pstRes)->pstVideo->pstRenderer);
    if (-1 == sReturnValue)
    {
        goto exit;
    }*/

    sReturnValue = InitSprite(
        "res/images/player.png", 360, 80, 0, 0,
        &(*pstRes)->pstSprite, &(*pstRes)->pstVideo->pstRenderer);
    if (-1 == sReturnValue)
    {
        goto exit;
    }

    sReturnValue = InitAudio(&(*pstRes)->pstAudio);
    if (-1 == sReturnValue)
    {
        goto exit;
    }

    sReturnValue = InitMusic(
        "res/music/LeftRightExcluded.ogg", -1,
        &(*pstRes)->pstMusic);
    PlayMusic(3000, &(*pstRes)->pstMusic);

    GetSingleObjectByName(
        "Player",
        &(*pstRes)->pstMap,
        &(*pstRes)->pstObject[0]);

    SetPosition(
        (*pstRes)->pstObject[0]->dPosX,
        (*pstRes)->pstObject[0]->dPosY,
        &(*pstRes)->pstEntity[0]);

    SetFrameOffset(0, 0, &(*pstRes)->pstEntity[0]);
    SetFontColour(0xfe, 0x95, 0x14, &(*pstRes)->pstFont);

    exit:
    return sReturnValue;
}

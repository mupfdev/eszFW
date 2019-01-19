/**
 * @file WorldLogic.c
 * @ingroup WorldLogic
 * @defgroup WorldLogic
 * @author Michael Fitzmayer
 * @copyright "THE BEER-WARE LICENCE" (Revision 42)
 */

#include <eszFW.h>
#include <stdbool.h>
#include <stdlib.h>
#include "GameData.h"
#include "WorldLogic.h"

void UpdateWorldStart(GameData **pstGameData)
{
    ResetEntity(&(*pstGameData)->pstEntity[ENT_PLAYER]);
    SetCameraLock(false, &(*pstGameData)->pstCamera);
    AnimateEntity(true, &(*pstGameData)->pstEntity[ENT_PLAYER]);
    SetFrameOffset(0, 0, &(*pstGameData)->pstEntity[ENT_PLAYER]);
    SetAnimation(
        0, 11,
        (*pstGameData)->pstEntity[ENT_PLAYER]->dAnimSpeed,
        &(*pstGameData)->pstEntity[ENT_PLAYER]);
}

void UpdateWorldEnd(const double dDeltaTime, GameData **pstGameData)
{
    // Follow player entity and set camera boudnaries to map size.
    SetCameraTargetEntity(
        (*pstGameData)->pstVideo->s32LogicalWindowWidth,
        (*pstGameData)->pstVideo->s32LogicalWindowHeight,
        &(*pstGameData)->pstCamera,
        &(*pstGameData)->pstEntity[ENT_PLAYER]);

    SetCameraBoundariesToMapSize(
        (*pstGameData)->pstVideo->s32LogicalWindowWidth,
        (*pstGameData)->pstVideo->s32LogicalWindowHeight,
        (*pstGameData)->pstMap->u16Width,
        (*pstGameData)->pstMap->u16Height,
        &(*pstGameData)->pstCamera);

    // Set zoom level dynamically in relation to vertical velocity.
    #ifndef __ANDROID__
    if (0.0 < (*pstGameData)->pstEntity[ENT_PLAYER]->dVelocityY)
    {
        (*pstGameData)->pstVideo->dZoomLevel -= dDeltaTime / 3.5f;
        if (1.0 > (*pstGameData)->pstVideo->dZoomLevel)
        {
            (*pstGameData)->pstVideo->dZoomLevel = 1;
        }
    }
    else
    {
        (*pstGameData)->pstVideo->dZoomLevel += dDeltaTime / 1.75f;
        if ((*pstGameData)->pstVideo->dZoomLevel > (*pstGameData)->pstVideo->dInitialZoomLevel)
        {
            (*pstGameData)->pstVideo->dZoomLevel = (*pstGameData)->pstVideo->dInitialZoomLevel;
        }
    }
    SetZoomLevel((*pstGameData)->pstVideo->dZoomLevel, &(*pstGameData)->pstVideo);
    #endif

    // Set up collision detection.
    if (false == IsOnTileOfType(
            "Platform",
            (*pstGameData)->pstEntity[ENT_PLAYER]->dPosX,
            (*pstGameData)->pstEntity[ENT_PLAYER]->dPosY,
            (*pstGameData)->pstEntity[ENT_PLAYER]->u8Height,
            &(*pstGameData)->pstMap))
    {
        AnimateEntity(false, &(*pstGameData)->pstEntity[ENT_PLAYER]);
        SetFrameOffset(0, 0, &(*pstGameData)->pstEntity[ENT_PLAYER]);
        SetAnimation(
            13, 13,
            (*pstGameData)->pstEntity[ENT_PLAYER]->dAnimSpeed,
            &(*pstGameData)->pstEntity[ENT_PLAYER]);
        DropEntity(&(*pstGameData)->pstEntity[ENT_PLAYER]);
    }

    UpdateEntity(
        dDeltaTime,
        (*pstGameData)->pstMap->dGravitation,
        (*pstGameData)->pstMap->u8MeterInPixel,
        &(*pstGameData)->pstEntity[ENT_PLAYER]);

    ConnectMapEndsForEntity(
        (*pstGameData)->pstMap->u16Width,
        (*pstGameData)->pstMap->u16Height,
        &(*pstGameData)->pstEntity[ENT_PLAYER]);
}

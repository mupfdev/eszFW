/**
 * @file World.c
 * @ingroup World
 * @defgroup World
 * @author Michael Fitzmayer
 * @copyright "THE BEER-WARE LICENCE" (Revision 42)
 */

#include <eszFW.h>
#include <stdbool.h>
#include <stdlib.h>
#include "Resources.h"
#include "World.h"

void UpdateWorld(const double dDeltaTime, Resources **pstResources)
{
    // Set up collision detection.
    if (false == IsOnTileOfType(
            "Platform",
            (*pstResources)->pstEntity[ENT_PLAYER]->dPosX,
            (*pstResources)->pstEntity[ENT_PLAYER]->dPosY,
            (*pstResources)->pstEntity[ENT_PLAYER]->u8Height,
            &(*pstResources)->pstMap))
    {
        AnimateEntity(false, &(*pstResources)->pstEntity[ENT_PLAYER]);
        SetFrameOffset(0, 0, &(*pstResources)->pstEntity[ENT_PLAYER]);
        SetAnimation(
            13, 13,
            (*pstResources)->pstEntity[ENT_PLAYER]->dAnimSpeed,
            &(*pstResources)->pstEntity[ENT_PLAYER]);
        DropEntity(&(*pstResources)->pstEntity[ENT_PLAYER]);
    }

    UpdateEntity(
        dDeltaTime,
        (*pstResources)->pstMap->dGravitation,
        (*pstResources)->pstMap->u8MeterInPixel,
        &(*pstResources)->pstEntity[ENT_PLAYER]);

    // Follow player entity and set camera boudnaries to map size.
    SetCameraTargetEntity(
        (*pstResources)->pstVideo->s32LogicalWindowWidth,
        (*pstResources)->pstVideo->s32LogicalWindowHeight,
        &(*pstResources)->pstCamera,
        &(*pstResources)->pstEntity[ENT_PLAYER]);

    SetCameraBoundariesToMapSize(
        (*pstResources)->pstVideo->s32LogicalWindowWidth,
        (*pstResources)->pstVideo->s32LogicalWindowHeight,
        (*pstResources)->pstMap->u16Width,
        (*pstResources)->pstMap->u16Height,
        &(*pstResources)->pstCamera);

    // Set zoom level dynamically in relation to vertical velocity.
    #ifndef __ANDROID__
    if (0.0 < (*pstResources)->pstEntity[ENT_PLAYER]->dVelocityY)
    {
        (*pstResources)->pstVideo->dZoomLevel -= dDeltaTime / 3.5f;
        if (1.0 > (*pstResources)->pstVideo->dZoomLevel)
        {
            (*pstResources)->pstVideo->dZoomLevel = 1;
        }
    }
    else
    {
        (*pstResources)->pstVideo->dZoomLevel += dDeltaTime / 1.75f;
        if ((*pstResources)->pstVideo->dZoomLevel > (*pstResources)->pstVideo->dInitialZoomLevel)
        {
            (*pstResources)->pstVideo->dZoomLevel = (*pstResources)->pstVideo->dInitialZoomLevel;
        }
    }
    SetZoomLevel((*pstResources)->pstVideo->dZoomLevel, &(*pstResources)->pstVideo);
    #endif // __ANDROID__

    ConnectMapEndsForEntity(
        (*pstResources)->pstMap->u16Width,
        (*pstResources)->pstMap->u16Height,
        &(*pstResources)->pstEntity[ENT_PLAYER]);
}

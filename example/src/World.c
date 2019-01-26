/**
 * @file World.c
 * @ingroup World
 * @defgroup World
 * @author Michael Fitzmayer
 * @copyright "THE BEER-WARE LICENCE" (Revision 42)
 */

#include <eszfw.h>
#include <stdbool.h>
#include <stdlib.h>
#include "Resources.h"
#include "World.h"

void UpdateWorld(const double dDeltaTime, Resources **pstRes)
{
    // Set up collision detection.
    if (false == IsOnTileOfType(
            "Platform",
            (*pstRes)->pstEntity[0]->dPosX,
            (*pstRes)->pstEntity[0]->dPosY,
            (*pstRes)->pstEntity[0]->u8Height,
            &(*pstRes)->pstMap))
    {
        AnimateEntity(false, &(*pstRes)->pstEntity[0]);
        SetFrameOffset(0, 0, &(*pstRes)->pstEntity[0]);
        SetAnimation(
            13, 13,
            (*pstRes)->pstEntity[0]->dAnimSpeed,
            &(*pstRes)->pstEntity[0]);
        DropEntity(&(*pstRes)->pstEntity[0]);
    }

    UpdateEntity(
        dDeltaTime,
        (*pstRes)->pstMap->dGravitation,
        (*pstRes)->pstMap->u8MeterInPixel,
        &(*pstRes)->pstEntity[0]);

    // Follow player entity and set camera boudnaries to map size.
    SetCameraTargetEntity(
        (*pstRes)->pstVideo->s32LogicalWindowWidth,
        (*pstRes)->pstVideo->s32LogicalWindowHeight,
        &(*pstRes)->pstCamera,
        &(*pstRes)->pstEntity[0]);

    SetCameraBoundariesToMapSize(
        (*pstRes)->pstVideo->s32LogicalWindowWidth,
        (*pstRes)->pstVideo->s32LogicalWindowHeight,
        (*pstRes)->pstMap->u16Width,
        (*pstRes)->pstMap->u16Height,
        &(*pstRes)->pstCamera);

    // Set zoom level dynamically in relation to vertical velocity.
    #ifndef __ANDROID__
    if (0.0 < (*pstRes)->pstEntity[0]->dVelocityY)
    {
        (*pstRes)->pstVideo->dZoomLevel -= dDeltaTime / 3.5f;
        if (1.0 > (*pstRes)->pstVideo->dZoomLevel)
        {
            (*pstRes)->pstVideo->dZoomLevel = 1;
        }
    }
    else
    {
        (*pstRes)->pstVideo->dZoomLevel += dDeltaTime / 1.75f;
        if ((*pstRes)->pstVideo->dZoomLevel > (*pstRes)->pstVideo->dInitialZoomLevel)
        {
            (*pstRes)->pstVideo->dZoomLevel = (*pstRes)->pstVideo->dInitialZoomLevel;
        }
    }
    SetZoomLevel((*pstRes)->pstVideo->dZoomLevel, &(*pstRes)->pstVideo);
    #endif // __ANDROID__

    ConnectMapEndsForEntity(
        (*pstRes)->pstMap->u16Width,
        (*pstRes)->pstMap->u16Height,
        &(*pstRes)->pstEntity[0]);
}

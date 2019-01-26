/**
 * @file Render.c
 * @ingroup Render
 * @defgroup Render
 * @author Michael Fitzmayer
 * @copyright "THE BEER-WARE LICENCE" (Revision 42)
 */

#include <eszfw.h>
#include <stdbool.h>
#include "Resources.h"
#include "Render.h"

int Render(Resources **pstResources)
{
    if (-1 == DrawBackground(
            (*pstResources)->pstEntity[0]->bOrientation,
            (*pstResources)->pstVideo->s32LogicalWindowHeight,
            (*pstResources)->pstCamera->dPosY,
            (*pstResources)->pstEntity[0]->dVelocityX,
            &(*pstResources)->pstVideo->pstRenderer,
            &(*pstResources)->pstBackground))
    {
        return -1;
    }

    if (-1 == DrawMap(
            0, "res/images/tileset.png", true, "BG",
            (*pstResources)->pstCamera->dPosX,
            (*pstResources)->pstCamera->dPosY,
            &(*pstResources)->pstMap,
            &(*pstResources)->pstVideo->pstRenderer))
    {
        return -1;
    }

    if (-1 == DrawEntity(
            &(*pstResources)->pstEntity[0],
            &(*pstResources)->pstCamera,
            &(*pstResources)->pstSprite,
            &(*pstResources)->pstVideo->pstRenderer))
    {
        return -1;
    }

    if (-1 == DrawMap(
            1, "res/images/tileset.png", false, "FG",
            (*pstResources)->pstCamera->dPosX,
            (*pstResources)->pstCamera->dPosY,
            &(*pstResources)->pstMap,
            &(*pstResources)->pstVideo->pstRenderer))
    {
        return -1;
    }

    RenderScene(&(*pstResources)->pstVideo->pstRenderer);

    return 0;
}

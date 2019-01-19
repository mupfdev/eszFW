/**
 * @file Render.c
 * @ingroup Render
 * @defgroup Render
 * @author Michael Fitzmayer
 * @copyright "THE BEER-WARE LICENCE" (Revision 42)
 */

#include <eszFW.h>
#include <stdbool.h>
#include "GameData.h"
#include "Render.h"

int Render(GameData **pstGameData)
{
    if (-1 == DrawBackground(
        (*pstGameData)->pstEntity[ENT_PLAYER]->bOrientation,
        (*pstGameData)->pstVideo->s32LogicalWindowHeight,
        (*pstGameData)->pstCamera->dPosY,
        (*pstGameData)->pstEntity[ENT_PLAYER]->dVelocityX,
        &(*pstGameData)->pstVideo->pstRenderer,
        &(*pstGameData)->pstBackground))
    {
        return -1;
    }

    if (-1 == DrawMap(
        0, "res/images/tileset.png", true, "BG",
        (*pstGameData)->pstCamera->dPosX,
        (*pstGameData)->pstCamera->dPosY,
        &(*pstGameData)->pstMap,
        &(*pstGameData)->pstVideo->pstRenderer))
    {
        return -1;
    }

    if (-1 == DrawEntity(
        &(*pstGameData)->pstEntity[ENT_PLAYER],
        &(*pstGameData)->pstCamera,
        &(*pstGameData)->pstSprite,
        &(*pstGameData)->pstVideo->pstRenderer))
    {
        return -1;
    }

    if (-1 == DrawMap(
        1, "res/images/tileset.png", false, "FG",
        (*pstGameData)->pstCamera->dPosX,
        (*pstGameData)->pstCamera->dPosY,
        &(*pstGameData)->pstMap,
        &(*pstGameData)->pstVideo->pstRenderer))
    {
        return -1;
    }

    RenderScene(&(*pstGameData)->pstVideo->pstRenderer);

    return 0;
}

/**
 * @file GameData.h
 * @ingroup GameData
 */

#ifndef _INIT_H_
#define _INIT_H_

#define ENT_PLAYER       0
#define OBJ_PLAYER_SPAWN 0

#include <eszFW.h>

typedef struct GameData_t
{
    Entity *pstEntity[1];
    Object *pstObject[1];

    Background *pstBackground;
    Camera     *pstCamera;
    Font       *pstFont;
    Input      *pstInput;
    Video      *pstVideo;
    Map        *pstMap;
    Sprite     *pstSprite;
} GameData;

void FreeGameData(GameData **pstGameData);
int InitGameData(GameData **pstGameData);

#endif // _INIT_H_

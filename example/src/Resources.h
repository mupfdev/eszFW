/**
 * @file Resources.h
 * @ingroup Resources
 */

#ifndef _INIT_H_
#define _INIT_H_

#define ENT_PLAYER       0
#define OBJ_PLAYER_SPAWN 0

#include <eszFW.h>

typedef struct Resources_t
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
} Resources;

void FreeResources(Resources **pstResources);
int InitResources(Resources **pstResources);

#endif // _RESOURCES_H_

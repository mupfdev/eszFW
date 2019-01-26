/**
 * @file Resources.h
 * @ingroup Resources
 */

#ifndef _INIT_H_
#define _INIT_H_

#include <eszfw.h>

typedef struct Resources_t
{
    Entity *pstEntity[1];
    Object *pstObject[1];

    Audio      *pstAudio;
    Background *pstBackground;
    Camera     *pstCamera;
    Font       *pstFont;
    Map        *pstMap;
    Music      *pstMusic;
    Sprite     *pstSprite;
    Video      *pstVideo;
} Resources;

void FreeResources(Resources **pstRes);
int InitResources(Resources **pstRes);

#endif // _RESOURCES_H_

/**
 * @file Entity.h
 * @ingroup Entity
 */

#ifndef _ENTITY_H_
#define _ENTITY_H_

#include <SDL.h>
#include "AABB.h"

typedef struct Bullet_t
{
    AABB     stBB;
    double   dPosX;
    double   dPosY;
    Uint8    u8Size;
    SDL_bool bOrientation;
    double   dVelocity;
} Bullet;

typedef struct Camera_t
{
    Uint16 u16Flags;
    double dPosX;
    double dPosY;
    Sint32 s32MaxPosX;
    Sint32 s32MaxPosY;
} Camera;

typedef struct Entity_t
{
    AABB     stBB;
    Uint16   u16Flags;
    double   dPosX;
    double   dPosY;
    double   dSpawnPosX;
    double   dSpawnPosY;
    SDL_bool bIsJumping;
    SDL_bool bOrientation;
    double   dAcceleration;
    double   dVelocityX;
    double   dMaxVelocityX;
    double   dVelocityY;
    Uint16   u16Width;
    Uint16   u16Height;
    Uint8    u8FrameOffsetX;
    Uint8    u8FrameOffsetY;
    Uint8    u8AnimFrame;
    Uint8    u8AnimStart;
    Uint8    u8AnimEnd;
    double   dAnimDelay;
    double   dAnimSpeed;
} Entity;

typedef struct Sprite_t
{
    SDL_Texture *pstTexture;
    Uint16       u16Width;
    Uint16       u16Height;
    Uint16       u16ImageOffsetX;
    Uint16       u16ImageOffsetY;
} Sprite;

void AnimateEntity(SDL_bool bAnimate, Entity *pstEntity);
void ConnectHorizontalMapEndsForEntity(const Uint16 u16MapWidth, Entity *pstEntity);
void ConnectMapEndsForEntity(const Uint16 u16MapWidth, const Uint16 u16MapHeight, Entity *pstEntity);
void ConnectVerticalMapEndsForEntity(const Uint16 u16MapHeight, Entity *pstEntity);
int  DrawEntity(const Entity *pstEntity, const Camera *pstCamera, const Sprite *pstSprite, SDL_Renderer *pstRenderer);
void DropEntity(Entity *pstEntity);
void FreeCamera(Camera *pstCamera);
void FreeEntity(Entity *pstEntity);
void FreeSprite(Sprite *pstSprite);
int  InitCamera(Camera **pstCamera);

int InitEntity(
    const double dPosX,
    const double dPosY,
    const Uint16 u16Width,
    const Uint16 u16Height,
    Entity     **pstEntity);

int InitSprite(
    const char   *pacFileName,
    const Uint16  u16Width,
    const Uint16  u16Height,
    const Uint16  u16ImageOffsetX,
    const Uint16  u16ImageOffsetY,
    Sprite      **pstSprite,
    SDL_Renderer *pstRenderer);

SDL_bool IsCameraLocked(const Camera *pstCamera);
SDL_bool IsEntityMoving(const Entity *pstEntity);
SDL_bool IsEntityRising(const Entity *pstEntity);
void JumpEntity(const double dForce, Entity *pstEntity);
void LockCamera(Camera *pstCamera);
void MoveEntity(Entity *pstEntity);

void MoveEntityFull(
    const SDL_bool bOrientation,
    const double   dAcceleration,
    const double   dMaxVelocityX,
    const Uint8    u8AnimStart,
    const Uint8    u8AnimEnd,
    const double   dAnimSpeed,
    const Uint8    u8FrameOffsetY,
    Entity        *pstEntity);

void ResetEntity(Entity *pstEntity);
void ResetEntityToSpawnPosition(Entity *pstEntity);

int SetCameraBoundariesToMapSize(
    const Sint32 s32LogicalWindowWidth,
    const Sint32 s32LogicalWindowHeight,
    const Uint16 u16MapWidth,
    const Uint16 u16MapHeight,
    Camera      *pstCamera);

void SetCameraTargetEntity(
    const Sint32  s32LogicalWindowWidth,
    const Sint32  s32LogicalWindowHeight,
    const Entity *pstEntity,
    Camera       *pstCamera);

void SetAnimation(const Uint8 u8AnimStart, const Uint8 u8AnimEnd, const double dAnimSpeed, Entity *pstEntity);
void SetFrameOffset(const Uint8 u8OffsetX, const Uint8 u8OffsetY, Entity *pstEntity);
void SetOrientation(const SDL_bool bOrientation, Entity *pstEntity);
void SetPosition(const double dPosX, const double dPosY, Entity *pstEntity);
void SetSpawnPosition(const double dPosX, const double dPosY, Entity *pstEntity);
void SetSpeed(const double dAcceleration, const double dMaxVelocityX, Entity *pstEntity);
void StopEntity(Entity *pstEntity);
void UnlockCamera(Camera *pstCamera);

void UpdateEntity(
    const double dDeltaTime,
    const double dGravitation,
    const Uint8  u8MeterInPixel,
    Entity      *pstEntity);

#endif // _ENTITY_H_

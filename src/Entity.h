/**
 * @file    Entity.h
 * @brief   Entity handler include header
 * @ingroup Entity
 */
#pragma once

#include <SDL.h>
#include "AABB.h"
#include "Constants.h"

/**
 * @typedef Bullet
 * @brief   Bullet handle type
 * @struct  Bullet_t
 * @brief   Bullet handle data
 */
typedef struct Bullet_t
{
    AABB     stBB;          ///< Axis-aligned bounding box
    double   dPosX;         ///< Position along the x-axis
    double   dPosY;         ///< Position along the y-axis
    Uint8    u8Size;        ///< Bullet size in pixel
    SDL_bool bOrientation;  ///< The bullet's orientation
    double   dVelocity;     ///< Velocity

} Bullet;

/**
 * @typedef Camera
 * @brief   Camera handle type
 * @struct  Camera_t
 * @brief   Camera handle data
 */
typedef struct Camera_t
{
    Uint16 u16Flags;    ///< Camera flags
    double dPosX;       ///< Position along the x-axis
    double dPosY;       ///< Position along the y-axis
    Sint32 s32MaxPosX;  ///< Maximum position along the x-axis
    Sint32 s32MaxPosY;  ///< Maximum position along the y-axis

} Camera;

/**
 * @typedef Entity
 * @brief   Entity handle type
 * @struct  Entity_t
 * @brief   Entity handle data
 */
typedef struct Entity_t
{
    AABB      stBB;            ///< Axis-aligned bounding box
    Uint16    u16Flags;        ///< Flag mask
    double    dPosX;           ///< Position along the x-axis
    double    dPosY;           ///< Position along the y-axis
    double    dSpawnPosX;      ///< Spawn position along the x-axis
    double    dSpawnPosY;      ///< Spawn position along the y-axis
    SDL_bool  bIsJumping;      ///< Current jumping-state
    Direction eDirection;      ///< Direction
    double    dAcceleration;   ///< Acceleration
    double    dVelocityX;      ///< Velocity along the x-axis
    double    dMaxVelocityX;   ///< Max velocity along the x-axis
    double    dVelocityY;      ///< Velocity along the y-axis
    Uint16    u16Width;        ///< Entity width in pixel
    Uint16    u16Height;       ///< Entity height in pixel
    Uint8     u8FrameOffsetX;  ///< Frame x-offset in frames
    Uint8     u8FrameOffsetY;  ///< Frame y-offset in frames
    Uint8     u8AnimFrame;     ///< Current animation frame
    Uint8     u8AnimStart;     ///< Animation start
    Uint8     u8AnimEnd;       ///< Animation end
    double    dAnimDelay;      ///< Animation delay
    double    dAnimSpeed;      ///< Animation speed

} Entity;

/**
 * @typedef Sprite
 * @brief   Sprite handle type
 * @struct  Sprite_t
 * @brief   Sprite handle data
 */
typedef struct Sprite_t
{
    SDL_Texture* pstTexture;       ///< SDL2 texture
    Uint16       u16Width;         ///< Sprite width
    Uint16       u16Height;        ///< Sprite height
    Uint16       u16ImageOffsetX;  ///< Image x-offset in pixel
    Uint16       u16ImageOffsetY;  ///< Image y-offset in pixel

} Sprite;

void Entity_Animate(SDL_bool bAnimate, Entity* pstEntity);
void Entity_ConnectHorizontalMapEnds(const Uint16 u16MapWidth, Entity* pstEntity);

void Entity_ConnectMapEnds(
    const Uint16 u16MapWidth,
    const Uint16 u16MapHeight,
    Entity*      pstEntity);

void Entity_ConnectVerticalMapEnds(const Uint16 u16MapHeight, Entity* pstEntity);

int  Entity_Draw(
     const Entity* pstEntity,
     const Camera* pstCamera,
     const Sprite* pstSprite,
     SDL_Renderer* pstRenderer);

void Entity_Drop(Entity* pstEntity);
void Entity_Free(Entity* pstEntity);
void Entity_FreeCamera(Camera* pstCamera);
void Entity_FreeSprite(Sprite* pstSprite);

int Entity_Init(
    const double dPosX,
    const double dPosY,
    const Uint16 u16Width,
    const Uint16 u16Height,
    Entity**     pstEntity);

int Entity_InitCamera(Camera** pstCamera);

int Entity_InitSprite(
    const char*   pacFileName,
    const Uint16  u16Width,
    const Uint16  u16Height,
    const Uint16  u16ImageOffsetX,
    const Uint16  u16ImageOffsetY,
    Sprite**      pstSprite,
    SDL_Renderer* pstRenderer);

SDL_bool Entity_IsCameraLocked(const Camera* pstCamera);
SDL_bool Entity_IsMoving(const Entity* pstEntity);
SDL_bool Entity_IsRising(const Entity* pstEntity);
void     Entity_Jump(const double dForce, Entity* pstEntity);
void     Entity_LockCamera(Camera* pstCamera);
void     Entity_Move(Entity* pstEntity);

void Entity_MoveFull(
    const Direction bOrientation,
    const double    dAcceleration,
    const double    dMaxVelocityX,
    const Uint8     u8AnimStart,
    const Uint8     u8AnimEnd,
    const double    dAnimSpeed,
    const Uint8     u8FrameOffsetY,
    Entity*         pstEntity);

void Entity_Reset(Entity* pstEntity);
void Entity_ResetToSpawnPosition(Entity* pstEntity);

int Entity_SetCameraBoundariesToMapSize(
    const Sint32 s32LogicalWindowWidth,
    const Sint32 s32LogicalWindowHeight,
    const Uint16 u16MapWidth,
    const Uint16 u16MapHeight,
    Camera*      pstCamera);

void Entity_SetCameraTarget(
    const Sint32  s32LogicalWindowWidth,
    const Sint32  s32LogicalWindowHeight,
    const Entity* pstEntity,
    Camera*       pstCamera);

void Entity_SetAnimation(
    const Uint8  u8AnimStart,
    const Uint8  u8AnimEnd,
    const double dAnimSpeed,
    Entity*      pstEntity);

void Entity_SetDirection(const Direction eDirection, Entity* pstEntity);
void Entity_SetFrameOffset(const Uint8 u8OffsetX, const Uint8 u8OffsetY, Entity* pstEntity);
void Entity_SetPosition(const double dPosX, const double dPosY, Entity* pstEntity);
void Entity_SetSpawnPosition(const double dPosX, const double dPosY, Entity* pstEntity);
void Entity_SetSpeed(const double dAcceleration, const double dMaxVelocityX, Entity* pstEntity);
void Entity_Stop(Entity* pstEntity);
void Entity_UnlockCamera(Camera* pstCamera);

void Entity_Update(
    const double dDeltaTime,
    const double dGravitation,
    const Uint8  u8MeterInPixel,
    Entity*      pstEntity);

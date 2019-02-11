/**
 * @file Entity.c
 * @ingroup Entity
 * @defgroup Entity
 * @author Michael Fitzmayer
 * @copyright "THE BEER-WARE LICENCE" (Revision 42)
 */

#include <SDL.h>
#include <SDL_image.h>
#include "AABB.h"
#include "Constants.h"
#include "Entity.h"
#include "Utils.h"

typedef enum Flags_t
{
    IS_ANIMATED   = 0x0,
    IS_DEAD       = 0x1,
    IS_IN_MID_AIR = 0x2,
    IS_LOCKED     = 0x3,
    IS_MOVING     = 0x4
} Flags;

void AnimateEntity(SDL_bool bAnimate, Entity *pstEntity)
{
    if (bAnimate)
    {
        SetFlag(IS_ANIMATED, &pstEntity->u16Flags);
    }
    else
    {
        ClearFlag(IS_ANIMATED, &pstEntity->u16Flags);
    }
}

void ConnectHorizontalMapEndsForEntity(const Uint16 u16MapWidth, Entity *pstEntity)
{
    double dWidth = (double)pstEntity->u16Width;

    if (pstEntity->dPosX < 0.f - dWidth)
    {
        pstEntity->dPosX = u16MapWidth + dWidth;
    }
    else if (pstEntity->dPosX > u16MapWidth + dWidth)
    {
        pstEntity->dPosX = 0 - dWidth;
    }
}

void ConnectMapEndsForEntity(const Uint16 u16MapWidth, const Uint16 u16MapHeight, Entity *pstEntity)
{
    ConnectHorizontalMapEndsForEntity(u16MapWidth, pstEntity);
    ConnectVerticalMapEndsForEntity(u16MapHeight, pstEntity);
}

void ConnectVerticalMapEndsForEntity(const Uint16 u16MapHeight, Entity *pstEntity)
{
    double dHeight = (double)pstEntity->u16Height;

    if (pstEntity->dPosY < 0 - dHeight)
    {
        pstEntity->dPosY = u16MapHeight + dHeight;
    }
    else if (pstEntity->dPosY > u16MapHeight + dHeight)
    {
        pstEntity->dPosY = 0 - dHeight;
    }
}

int CreateBullet(const double dPosX, const double dPosY, Bullet *pstBullet)
{
    (void)dPosX;
    (void)dPosY;
    (void)pstBullet;

    return 0;
}

int DrawEntity(const Entity *pstEntity, const Camera *pstCamera, const Sprite *pstSprite, SDL_Renderer *pstRenderer)
{
    double           dPosX  = pstEntity->dPosX - pstCamera->dPosX;
    double           dPosY  = pstEntity->dPosY - pstCamera->dPosY;
    SDL_RendererFlip s8Flip = SDL_FLIP_NONE;
    SDL_Rect         stDst;
    SDL_Rect         stSrc;

    if (LEFT == pstEntity->eDirection)
    {
        s8Flip = SDL_FLIP_HORIZONTAL;
    }

    stSrc.x  = pstSprite->u16ImageOffsetX;
    stSrc.x += pstEntity->u8FrameOffsetX * pstEntity->u16Width;
    stSrc.x += pstEntity->u8AnimFrame * pstEntity->u16Width;
    stSrc.y  = pstSprite->u16ImageOffsetY;
    stSrc.y += pstEntity->u8FrameOffsetY * pstEntity->u16Height;
    stSrc.w  = pstEntity->u16Width;
    stSrc.h  = pstEntity->u16Height;
    stDst.x  = dPosX - (pstEntity->u16Width  / 2);
    stDst.y  = dPosY - (pstEntity->u16Height / 2);
    stDst.w  = pstEntity->u16Width;
    stDst.h  = pstEntity->u16Height;

    if (0 != SDL_RenderCopyEx(
            pstRenderer,
            pstSprite->pstTexture,
            &stSrc,
            &stDst,
            0,
            NULL,
            s8Flip))
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "%s\n", SDL_GetError());
        return -1;
    }
    return 0;
}

void DropEntity(Entity *pstEntity)
{
    SetFlag(IS_IN_MID_AIR, &pstEntity->u16Flags);
}

void FreeCamera(Camera *pstCamera)
{
    SDL_free(pstCamera);
}

void FreeEntity(Entity *pstEntity)
{
    SDL_free(pstEntity);
}

void FreeSprite(Sprite *pstSprite)
{
    if (pstSprite)
    {
        SDL_DestroyTexture(pstSprite->pstTexture);
        SDL_free(pstSprite);
        SDL_Log("Unload sprite image file.\n");
    }
}

int InitCamera(Camera **pstCamera)
{
    *pstCamera = SDL_calloc(sizeof(struct Camera_t), sizeof(Sint8));
    if (! *pstCamera)
    {
        return -1;
    }

    SDL_Log("Initialise camera.\n");
    return 0;
}

int InitEntity(
    const double dPosX,
    const double dPosY,
    const Uint16 u16Width,
    const Uint16 u16Height,
    Entity     **pstEntity)
{
    *pstEntity = SDL_calloc(sizeof(struct Entity_t), sizeof(Sint8));
    if (! *pstEntity)
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "InitEntity(): error allocating memory.\n");
        return -1;
    }

    (*pstEntity)->dPosX          = dPosX;
    (*pstEntity)->dPosY          = dPosY;
    (*pstEntity)->eDirection     = RIGHT;
    (*pstEntity)->dAcceleration  = 8.f;
    (*pstEntity)->dMaxVelocityX  = 4.5f;
    (*pstEntity)->u16Width       = u16Width;
    (*pstEntity)->u16Height      = u16Height;
    (*pstEntity)->dAnimSpeed     = 12.f;

    return 0;
}

int InitSprite(
    const char   *pacFileName,
    const Uint16  u16Width,
    const Uint16  u16Height,
    const Uint16  u16ImageOffsetX,
    const Uint16  u16ImageOffsetY,
    Sprite      **pstSprite,
    SDL_Renderer *pstRenderer)
{
    *pstSprite = SDL_calloc(sizeof(struct Sprite_t), sizeof(Sint8));
    if (! *pstSprite)
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "InitSprite(): error allocating memory.\n");
        return -1;
    }

    (*pstSprite)->pstTexture = IMG_LoadTexture(pstRenderer, pacFileName);
    if (! (*pstSprite)->pstTexture)
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "%s\n", IMG_GetError());
        return -1;
    }

    (*pstSprite)->u16Width        = u16Width;
    (*pstSprite)->u16Height       = u16Height;
    (*pstSprite)->u16ImageOffsetX = u16ImageOffsetX;
    (*pstSprite)->u16ImageOffsetY = u16ImageOffsetY;

    SDL_Log("Load sprite image file: %s.\n", pacFileName);

    return 0;
}

SDL_bool IsCameraLocked(const Camera *pstCamera)
{
    if (IsFlagSet(IS_LOCKED, pstCamera->u16Flags))
    {
        return SDL_TRUE;
    }
    else
    {
        return SDL_FALSE;
    }
}

SDL_bool IsEntityMoving(const Entity *pstEntity)
{
    if (IsFlagSet(IS_MOVING, pstEntity->u16Flags))
    {
        return SDL_TRUE;
    }
    else
    {
        return SDL_FALSE;
    }
}

SDL_bool IsEntityRising(const Entity *pstEntity)
{
    if (0 > pstEntity->dVelocityY)
    {
        return SDL_TRUE;
    }
    else
    {
        return SDL_FALSE;
    }
}

void JumpEntity(const double dForce, Entity *pstEntity)
{
    if (! pstEntity->bIsJumping)
    {
        // Prevent jumping while falling down.
        if (0 >= pstEntity->dVelocityY)
        {
            // Initial lift-up; may need adjustment (estimated value).
            pstEntity->dPosY -= pstEntity->u16Height / 8.0;
            pstEntity->dVelocityY = -dForce; // Apply force.
            pstEntity->bIsJumping = 1;
        }
    }
}

void LockCamera(Camera *pstCamera)
{
    SetFlag(IS_LOCKED, &pstCamera->u16Flags);
}

void MoveEntity(Entity *pstEntity)
{
    SetFlag(IS_MOVING, &pstEntity->u16Flags);
}

void MoveEntityFull(
    const Direction eDirection,
    const double    dAcceleration,
    const double    dMaxVelocityX,
    const Uint8     u8AnimStart,
    const Uint8     u8AnimEnd,
    const double    dAnimSpeed,
    const Uint8     u8FrameOffsetY,
    Entity         *pstEntity)
{
    SetFlag(IS_MOVING, &pstEntity->u16Flags);
    SetFrameOffset(0, u8FrameOffsetY, pstEntity);
    SetSpeed(dAcceleration, dMaxVelocityX, pstEntity);
    SetDirection(eDirection, pstEntity);
    SetAnimation(u8AnimStart, u8AnimEnd, dAnimSpeed, pstEntity);
}

void ResetEntity(Entity *pstEntity)
{
    pstEntity->u16Flags = 0;
}

void ResetEntityToSpawnPosition(Entity *pstEntity)
{
    pstEntity->dPosX = pstEntity->dSpawnPosX;
    pstEntity->dPosY = pstEntity->dSpawnPosY;
}

void SetCameraTargetEntity(
    const Sint32  s32LogicalWindowWidth,
    const Sint32  s32LogicalWindowHeight,
    const Entity *pstEntity,
    Camera       *pstCamera)
{
    if (IsFlagSet(IS_LOCKED, pstCamera->u16Flags))
    {
        pstCamera->dPosX  = pstEntity->dPosX;
        pstCamera->dPosX -= s32LogicalWindowWidth  / 2.0;
        pstCamera->dPosY  = pstEntity->dPosY;
        pstCamera->dPosY -= s32LogicalWindowHeight / 2.0;

        if (pstCamera->dPosX < 0)
        {
            pstCamera->dPosX = 0;
        }
    }
}

void SetAnimation(const Uint8 u8AnimStart, const Uint8 u8AnimEnd, const double  dAnimSpeed, Entity *pstEntity)
{
    pstEntity->dAnimSpeed = dAnimSpeed;

    if (u8AnimStart <= u8AnimEnd)
    {
        pstEntity->u8AnimStart = u8AnimStart;
        pstEntity->u8AnimEnd   = u8AnimEnd;
    }
    else
    {
        pstEntity->u8AnimStart = u8AnimEnd;
        pstEntity->u8AnimEnd   = u8AnimStart;
    }
}

int SetCameraBoundariesToMapSize(
    const Sint32  s32LogicalWindowWidth,
    const Sint32  s32LogicalWindowHeight,
    const Uint16 u16MapWidth,
    const Uint16 u16MapHeight,
    Camera        *pstCamera)
{
    SDL_bool bReturnValue = 0;
    pstCamera->s32MaxPosX = u16MapWidth  - s32LogicalWindowWidth;
    pstCamera->s32MaxPosY = u16MapHeight - s32LogicalWindowHeight;

    #ifdef DEBUG
    SDL_Log(
        "Set camera boundaries to map size (x: %d y: %d).\n",
        pstCamera->s32MaxPosX,
        pstCamera->s32MaxPosY);
    #endif

    if (pstCamera->dPosX <= 0)
    {
        pstCamera->dPosX = 0;
        bReturnValue = 1;
    }

    if (pstCamera->dPosY <= 0)
    {
        pstCamera->dPosY = 0;
    }

    if (pstCamera->dPosX > pstCamera->s32MaxPosX)
    {
        pstCamera->dPosX = pstCamera->s32MaxPosX;
        bReturnValue = 1;
    }

    if (pstCamera->dPosY > pstCamera->s32MaxPosY)
    {
        pstCamera->dPosY = pstCamera->s32MaxPosY;
    }

    return bReturnValue;
}

void SetDirection(const Direction eDirection, Entity *pstEntity)
{
    if (RIGHT == eDirection)
    {
        pstEntity->eDirection = RIGHT;
    }
    else
    {
        pstEntity->eDirection = LEFT;
    }
}

void SetFrameOffset(const Uint8 u8OffsetX, const Uint8 u8OffsetY, Entity *pstEntity)
{
    pstEntity->u8FrameOffsetX = u8OffsetX;
    pstEntity->u8FrameOffsetY = u8OffsetY;
}

void SetPosition(const double dPosX, const double dPosY, Entity *pstEntity)
{
    pstEntity->dPosX = dPosX;
    pstEntity->dPosY = dPosY;
}

void SetSpawnPosition(const double dPosX, const double dPosY, Entity *pstEntity)
{
    pstEntity->dSpawnPosX = dPosX;
    pstEntity->dSpawnPosY = dPosY;
}

void SetSpeed(const double dAcceleration, const double dMaxVelocityX, Entity *pstEntity)
{
    pstEntity->dAcceleration = dAcceleration;
    pstEntity->dMaxVelocityX = dMaxVelocityX;
}

void StopEntity(Entity *pstEntity)
{
    ClearFlag(IS_MOVING, &pstEntity->u16Flags);
}

void UnlockCamera(Camera *pstCamera)
{
    ClearFlag(IS_LOCKED, &pstCamera->u16Flags);
}

void UpdateEntity(
    const double dGravitation,
    const Uint8  u8MeterInPixel,
    Entity      *pstEntity)
{
    double dDeltaTime = (double)APPROX_TIME_PER_FRAME / (double)TIME_FACTOR;
    double dPosX      = pstEntity->dPosX;
    double dPosY      = pstEntity->dPosY;

    // Apply gravitation.
    if (0 != dGravitation)
    {
        if (IsEntityRising(pstEntity))
        {
            SetFlag(IS_IN_MID_AIR, &pstEntity->u16Flags);
        }

        if (IsFlagSet(IS_IN_MID_AIR, pstEntity->u16Flags))
        {
            double dG              = dGravitation * u8MeterInPixel;
            double dDistanceY      = dG * dDeltaTime * dDeltaTime;
            pstEntity->dVelocityY += dDistanceY;
            dPosY                 += pstEntity->dVelocityY;
        }
        else
        {
            pstEntity->bIsJumping = 0;
            // Correct position along the y-axis.
            pstEntity->dVelocityY = 0.f;
            dPosY = (16.f * round(dPosY / 16.f));
        }
    }

    // Calculate horizontal velocity.
    if (IsFlagSet(IS_MOVING, pstEntity->u16Flags))
    {
        double dAccel          = pstEntity->dAcceleration * (double)u8MeterInPixel;
        double dDistanceX      = dAccel * dDeltaTime * dDeltaTime;
        pstEntity->dVelocityX += dDistanceX;
    }
    else
    {
        pstEntity->dVelocityX -= pstEntity->dAcceleration * dDeltaTime;
    }

    // Set horizontal velocity limits.
    if (pstEntity->dVelocityX >= pstEntity->dMaxVelocityX)
    {
        pstEntity->dVelocityX = pstEntity->dMaxVelocityX;
    }
    if (pstEntity->dVelocityX < 0)
    {
        pstEntity->dVelocityX = 0;
    }

    // Set horizontal position.
    if (pstEntity->dVelocityX > 0)
    {
        if (RIGHT == pstEntity->eDirection)
        {
            dPosX += pstEntity->dVelocityX;
        }
        else
        {
            dPosX -= pstEntity->dVelocityX;
        }
    }

    // Update position.
    SetPosition(dPosX, dPosY, pstEntity);

    // Update axis-aligned bounding box.
    pstEntity->stBB.dBottom = dPosY + (pstEntity->u16Height / 2.0);
    pstEntity->stBB.dLeft   = dPosX - (pstEntity->u16Width  / 2.0);
    pstEntity->stBB.dRight  = dPosX + (pstEntity->u16Width  / 2.0);
    pstEntity->stBB.dTop    = dPosY - (pstEntity->u16Height / 2.0);

    // Update animation frame.
    if (IsFlagSet(IS_ANIMATED, pstEntity->u16Flags))
    {
        pstEntity->dAnimDelay += dDeltaTime;

        if (pstEntity->u8AnimFrame < pstEntity->u8AnimStart)
        {
            pstEntity->u8AnimFrame = pstEntity->u8AnimStart;
        }

        if (pstEntity->dAnimDelay > 1.f / pstEntity->dAnimSpeed)
        {
            pstEntity->u8AnimFrame++;
            pstEntity->dAnimDelay = 0.f;
        }
        // Loop animation.
        if (pstEntity->u8AnimFrame >= pstEntity->u8AnimEnd)
        {
            pstEntity->u8AnimFrame = pstEntity->u8AnimStart;
        }
    }
    else
    {
        pstEntity->u8AnimFrame = pstEntity->u8AnimStart;
    }
}

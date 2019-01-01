/**
 * @file Entity.c
 * @ingroup Entity
 * @defgroup Entity
 * @author Michael Fitzmayer
 * @copyright "THE BEER-WARE LICENCE" (Revision 42)
 */

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <stdbool.h>
#include <stdlib.h>
#include "AABB.h"
#include "Config.h"
#include "Entity.h"
#include "Macros.h"

void ConnectHorizontalMapEndsForEntity(
    const uint16_t  u16MapWidth,
    Entity        **pstEntity)
{
    if ((*pstEntity)->dPosX < 0)
    {
        (*pstEntity)->dPosX = u16MapWidth;
    }
    else if ((*pstEntity)->dPosX > u16MapWidth)
    {
        (*pstEntity)->dPosX = 0;
    }
}

void ConnectMapEndsForEntity(
    const uint16_t  u16MapWidth,
    const uint16_t  u16MapHeight,
    Entity        **pstEntity)
{
    ConnectHorizontalMapEndsForEntity(u16MapWidth, pstEntity);
    ConnectVerticalMapEndsForEntity(u16MapHeight, pstEntity);
}

void ConnectVerticalMapEndsForEntity(
    const uint16_t  u16MapHeight,
    Entity        **pstEntity)
{
    if ((*pstEntity)->dPosY < 0)
    {
        (*pstEntity)->dPosY = u16MapHeight;
    }
    else if ((*pstEntity)->dPosY > u16MapHeight)
    {
        (*pstEntity)->dPosY = 0;
    }
}

int DrawEntity(
    Entity       **pstEntity,
    Camera       **pstCamera,
    Sprite       **pstSprite,
    SDL_Renderer **pstRenderer)
{
    double           dPosX  = (*pstEntity)->dPosX - (*pstCamera)->dPosX;
    double           dPosY  = (*pstEntity)->dPosY - (*pstCamera)->dPosY;
    SDL_RendererFlip s8Flip = SDL_FLIP_NONE;
    SDL_Rect         stDst;
    SDL_Rect         stSrc;

    if (LEFT == (*pstEntity)->bOrientation)
    {
        s8Flip = SDL_FLIP_HORIZONTAL;
    }

    stSrc.x  = (*pstSprite)->u16ImageOffsetX;
    stSrc.x += (*pstEntity)->u8FrameOffsetX * (*pstEntity)->u8Width;
    stSrc.x += (*pstEntity)->u8AnimFrame * (*pstEntity)->u8Width;
    stSrc.y  = (*pstSprite)->u16ImageOffsetY;
    stSrc.y += (*pstEntity)->u8FrameOffsetY * (*pstEntity)->u8Height;
    stSrc.w  = (*pstEntity)->u8Width;
    stSrc.h  = (*pstEntity)->u8Height;
    stDst.x  = dPosX - ((*pstEntity)->u8Width  / 2);
    stDst.y  = dPosY - ((*pstEntity)->u8Height / 2);
    stDst.w  = (*pstEntity)->u8Width;
    stDst.h  = (*pstEntity)->u8Height;

    if (0 != SDL_RenderCopyEx(
            (*pstRenderer),
            (*pstSprite)->pstTexture,
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

void FreeCamera(Camera **pstCamera)
{
    free(*pstCamera);
}

void FreeEntity(Entity **pstEntity)
{
    free(*pstEntity);
}

void FreeSprite(Sprite **pstSprite)
{
    if (NULL != (*pstSprite))
    {
        SDL_DestroyTexture((*pstSprite)->pstTexture);
    }
    free(*pstSprite);
    SDL_Log("Unload sprite image file.\n");
}

int InitCamera(Camera **pstCamera)
{
    *pstCamera = NULL;
    *pstCamera = malloc(sizeof(struct Camera_t));
    if (NULL == *pstCamera)
    {
        SDL_LogError(
            SDL_LOG_CATEGORY_APPLICATION,
            "InitCamera(): error allocating memory.\n");
        return -1;
    }

    (*pstCamera)->u16Flags   = 0;
    (*pstCamera)->dPosX      = 0.f;
    (*pstCamera)->dPosY      = 0.f;
    (*pstCamera)->s32MaxPosX = 0;
    (*pstCamera)->s32MaxPosY = 0;

    SDL_Log("Initialise camera.\n");

    return 0;
}

int InitEntity(
    const double dPosX,
    const double dPosY,
    Entity     **pstEntity)
{
    *pstEntity = NULL;
    *pstEntity = malloc(sizeof(struct Entity_t));
    if (NULL == *pstEntity)
    {
        SDL_LogError(
            SDL_LOG_CATEGORY_APPLICATION,
            "InitEntity(): error allocating memory.\n");
        return -1;
    }

    SetPosition(dPosX, dPosY, &(*pstEntity));
    SetSpeed(8.0, 4.5, &(*pstEntity));
    SetOrientation(RIGHT, &(*pstEntity));

    (*pstEntity)->dVelocityX     =  0.f;
    (*pstEntity)->dVelocityY     =  0.f;
    (*pstEntity)->stBB.dBottom   =  0;
    (*pstEntity)->stBB.dLeft     =  0;
    (*pstEntity)->stBB.dRight    =  0;
    (*pstEntity)->stBB.dTop      =  0;
    (*pstEntity)->u16Flags       =  0;
    (*pstEntity)->u8Width        = 32;
    (*pstEntity)->u8Height       = 32;
    (*pstEntity)->u8FrameOffsetX =  0;
    (*pstEntity)->u8AnimFrame    =  0;
    (*pstEntity)->u8AnimStart    =  0;
    (*pstEntity)->u8AnimEnd      =  0;
    (*pstEntity)->dAnimDelay     =  0.f;

    return 0;
}

int InitSprite(
    const char    *pacFileName,
    const uint16_t u16Width,
    const uint16_t u16Height,
    const uint16_t u16ImageOffsetX,
    const uint16_t u16ImageOffsetY,
    Sprite       **pstSprite,
    SDL_Renderer **pstRenderer)
{
    *pstSprite = NULL;
    *pstSprite = malloc(sizeof(struct Sprite_t));
    if (NULL == *pstSprite)
    {
        SDL_LogError(
            SDL_LOG_CATEGORY_APPLICATION,
            "InitSprite(): error allocating memory.\n");
        return -1;
    }

    if (NULL != (*pstSprite)->pstTexture)
    {
        SDL_DestroyTexture((*pstSprite)->pstTexture);
    }

    (*pstSprite)->pstTexture = IMG_LoadTexture((*pstRenderer), pacFileName);
    if (NULL == (*pstSprite)->pstTexture)
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "%s\n", IMG_GetError());
        free(*pstSprite);
        return -1;
    }

    (*pstSprite)->u16Width        = u16Width;
    (*pstSprite)->u16Height       = u16Height;
    (*pstSprite)->u16ImageOffsetX = u16ImageOffsetX;
    (*pstSprite)->u16ImageOffsetY = u16ImageOffsetY;

    SDL_Log("Load sprite image file: %s.\n", pacFileName);

    return 0;
}

void SetCameraTargetEntity(
    const int32_t s32WindowWidth,
    const int32_t s32WindowHeight,
    const double  dZoomLevel,
    Camera       **pstCamera,
    Entity       **pstEntity)
{
    if (IS_NOT_SET((*pstCamera)->u16Flags, IS_LOCKED))
    {
        (*pstCamera)->dPosX  = (*pstEntity)->dPosX;
        (*pstCamera)->dPosX -= (s32WindowWidth  / 2) / dZoomLevel;
        (*pstCamera)->dPosY  = (*pstEntity)->dPosY;
        (*pstCamera)->dPosY -= (s32WindowHeight / 2) / dZoomLevel;

        if ((*pstCamera)->dPosX < 0)
        {
            (*pstCamera)->dPosX = 0;
        }
    }
}

void SetAnimation(
    const uint8_t  u8AnimStart,
    const uint8_t  u8AnimEnd,
    Entity       **pstEntity)
{
    if (u8AnimStart <= u8AnimEnd)
    {
        (*pstEntity)->u8AnimStart = u8AnimStart;
        (*pstEntity)->u8AnimEnd   = u8AnimEnd;
    }
    else
    {
        (*pstEntity)->u8AnimStart = u8AnimEnd;
        (*pstEntity)->u8AnimEnd   = u8AnimStart;
    }
}

void SetCameraBoundariesToMapSize(
    const int32_t   s32WindowWidth,
    const int32_t   s32WindowHeight,
    const double    dZoomLevel,
    const uint16_t  u16MapWidth,
    const uint16_t  u16MapHeight,
    Camera        **pstCamera)
{
    if (0 == (*pstCamera)->s32MaxPosX || 0 == (*pstCamera)->s32MaxPosY)
    {
        (*pstCamera)->s32MaxPosX = u16MapWidth  - s32WindowWidth  / dZoomLevel;
        (*pstCamera)->s32MaxPosY = u16MapHeight - s32WindowHeight / dZoomLevel;

        SDL_Log(
            "Set camera boundaries to map size (x: %d y: %d).\n",
            (*pstCamera)->s32MaxPosX,
            (*pstCamera)->s32MaxPosY);
    }

    if ((*pstCamera)->dPosX < 0)
    {
        (*pstCamera)->dPosX = 0;
    }

    if ((*pstCamera)->dPosY < 0)
    {
        (*pstCamera)->dPosY = 0;
    }

    if ((*pstCamera)->dPosX > (*pstCamera)->s32MaxPosX)
    {
        (*pstCamera)->dPosX = (*pstCamera)->s32MaxPosX;
    }

    if ((*pstCamera)->dPosY > (*pstCamera)->s32MaxPosY)
    {
        (*pstCamera)->dPosY = (*pstCamera)->s32MaxPosY;
    }
}

void SetFrameOffset(
    const uint8_t  u8OffsetX,
    const uint8_t  u8OffsetY,
    Entity       **pstEntity)
{
    (*pstEntity)->u8FrameOffsetX = u8OffsetX;
    (*pstEntity)->u8FrameOffsetY = u8OffsetY;
}

void SetOrientation(const bool bOrientation, Entity **pstEntity)
{
    if (RIGHT == bOrientation)
    {
        (*pstEntity)->bOrientation = RIGHT;
    }
    else
    {
        (*pstEntity)->bOrientation = LEFT;
    }
}

void SetPosition(
    const double  dPosX,
    const double  dPosY,
    Entity      **pstEntity)
{
    (*pstEntity)->dPosX = dPosX;
    (*pstEntity)->dPosY = dPosY;
}

void SetSpeed(
    const double  dAcceleration,
    const double  dMaxVelocityX,
    Entity      **pstEntity)
{
    (*pstEntity)->dAcceleration = dAcceleration;
    (*pstEntity)->dMaxVelocityX = dMaxVelocityX;
}

void UpdateEntity(
    const double dDeltaTime,
    const double dGravitation,
    Entity **pstEntity)
{
    double dPosX = (*pstEntity)->dPosX;
    double dPosY = (*pstEntity)->dPosY;

    // Apply gravitation.
    if (IS_SET((*pstEntity)->u16Flags, IS_IN_MID_AIR))
    {
        double dG                 = dGravitation * METER_IN_PIXEL;
        double dDistanceY         = dG * dDeltaTime * dDeltaTime;
        (*pstEntity)->dVelocityY += dDistanceY;
        dPosY                    += (*pstEntity)->dVelocityY;
    }
    else
    {
        (*pstEntity)->dVelocityY = 0.f;
        dPosY                    = (*pstEntity)->dPosY;
    }

    // Calculate horizontal velocity.
    if (IS_SET((*pstEntity)->u16Flags, IS_WALKING))
    {
        double dAccel             = (*pstEntity)->dAcceleration * METER_IN_PIXEL;
        double dDistanceX         = dAccel * dDeltaTime * dDeltaTime;
        (*pstEntity)->dVelocityX += dDistanceX;
        SET((*pstEntity)->u16Flags, IS_MOVING);
    }
    else
    {
        CLEAR((*pstEntity)->u16Flags, IS_MOVING);
        (*pstEntity)->dVelocityX -= (*pstEntity)->dAcceleration * dDeltaTime;
    }

    // Set horizontal velocity limits.
    if ((*pstEntity)->dVelocityX >= (*pstEntity)->dMaxVelocityX)
    {
        (*pstEntity)->dVelocityX = (*pstEntity)->dMaxVelocityX;
    }
    if ((*pstEntity)->dVelocityX < 0)
    {
        (*pstEntity)->dVelocityX = 0;
    }

    // Set horizontal position.
    if ((*pstEntity)->dVelocityX > 0)
    {
        if (RIGHT == (*pstEntity)->bOrientation)
        {
            dPosX += (*pstEntity)->dVelocityX;
        }
        else
        {
            dPosX -= (*pstEntity)->dVelocityX;
        }
    }

    // Update position.
    SetPosition(dPosX, dPosY, &(*pstEntity));

    // Update bounding box.
    (*pstEntity)->stBB.dBottom = dPosY + (*pstEntity)->u8Height;
    (*pstEntity)->stBB.dLeft   = dPosX;
    (*pstEntity)->stBB.dRight  = dPosX + (*pstEntity)->u8Width;
    (*pstEntity)->stBB.dTop    = dPosY;

    // Update animation frame.
    if (IS_SET((*pstEntity)->u16Flags, IS_MOVING))
    {
        (*pstEntity)->dAnimDelay += dDeltaTime;

        if ((*pstEntity)->u8AnimFrame < (*pstEntity)->u8AnimStart)
        {
            (*pstEntity)->u8AnimFrame = (*pstEntity)->u8AnimStart;
        }

        if ((*pstEntity)->dAnimDelay > 1.0 / 6.25)
        {
            (*pstEntity)->u8AnimFrame++;
            (*pstEntity)->dAnimDelay = 0.0;
        }
        // Loop animation.
        if ((*pstEntity)->u8AnimFrame >= (*pstEntity)->u8AnimEnd)
        {
            (*pstEntity)->u8AnimFrame = (*pstEntity)->u8AnimStart;
        }
    }
    else
    {
        (*pstEntity)->u8AnimFrame = (*pstEntity)->u8AnimStart;
    }
}

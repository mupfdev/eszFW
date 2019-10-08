/**
 * @file      Entity.c
 * @brief     Entity handler source
 * @ingroup   Entity
 * @defgroup  Entity Game entity handler
 * @author    Michael Fitzmayer
 * @copyright "THE BEER-WARE LICENCE" (Revision 42)
 */

#include <SDL.h>
#include <SDL_image.h>
#include "AABB.h"
#include "Constants.h"
#include "Entity.h"
#include "Utils.h"

/**
 * @typedef Flags
 * @brief   Entity flags type
 * @enum    Flags_t
 * @brief   Entity flags data
 * @remark  These flags are only used internally
 */
typedef enum Flags_t
{
    IS_ANIMATED   = 0x00,  ///< Entity is animated
    IS_DEAD       = 0x01,  ///< Entity is dead
    IS_IN_MID_AIR = 0x02,  ///< Entity is in mid-air
    IS_LOCKED     = 0x03,  ///< Camera is locked
    IS_MOVING     = 0x04   ///< Entity is moving

} Flags;

/**
 * @brief   Animate entity
 * @details Sets or clears the entity's IS_ANIMATED flag
 * @param   bAnimate
 *          SDL_FALSE = Do not animate, SDL_TRUE = Animate
 * @param   pstEntity
 *          Pointer to entity handle
 */
void Entity_Animate(SDL_bool bAnimate, Entity* pstEntity)
{
    if (bAnimate)
    {
        Utils_SetFlag(IS_ANIMATED, &pstEntity->u16Flags);
    }
    else
    {
        Utils_ClearFlag(IS_ANIMATED, &pstEntity->u16Flags);
    }
}

/**
 * @brief   Connect horizontal map ends for entity
 * @details Connects the horizontal map ends for an entity so it can
 *          travel from one side to the other by leaving the map
 * @param   u16MapWidth
 *          Map width
 * @param   pstEntity
 *          Pointer to entity handle
 */
void Entity_ConnectHorizontalMapEnds(const Uint16 u16MapWidth, Entity* pstEntity)
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

/**
 * @brief   Connect all map ends for entity
 * @details Connects horizontal and vertical map ends for an entity so
 *          it can travel from one side to the other by leaving the map
 * @param   u16MapWidth
 *          Map width
 * @param   u16MapHeight
 *          Map height
 * @param   pstEntity
 *          Pointer to entity handle
 */
void Entity_ConnectMapEnds(const Uint16 u16MapWidth, const Uint16 u16MapHeight, Entity* pstEntity)
{
    Entity_ConnectHorizontalMapEnds(u16MapWidth, pstEntity);
    Entity_ConnectVerticalMapEnds(u16MapHeight, pstEntity);
}

/**
 * @brief   Connect vertical map ends for entity
 * @details Connects the vertical map ends for an entity so it can
 *          travel from one side to the other by leaving the map
 * @param   u16MapHeight
 *          Map height
 * @param   pstEntity
 *          Pointer to entity handle
 */
void Entity_ConnectVerticalMapEnds(const Uint16 u16MapHeight, Entity* pstEntity)
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

/**
 * @brief   Create bullet/projectile
 * @details Creates a special bullet/projectile entity
 * @param   dPosX
 *          Position along the x-axis
 * @param   dPosY
 *          Position along the y-axis
 * @param   pstBullet
 *          Pointer to bullet handle
 * @remark  Does nothing yet, function needs to be implemented
 * @return  Always 0 at this point
 * @todo    Implement function
 */
int Entity_CreateBullet(const double dPosX, const double dPosY, Bullet* pstBullet)
{
    (void)dPosX;
    (void)dPosY;
    (void)pstBullet;

    return 0;
}

/**
 * @brief   Draw entity
 * @details Draws an entity on screen
 * @param   pstEntity
 *          Pointer to entity handle
 * @param   pstCamera
 *          Pointer to camera handle
 * @param   pstSprite
 *          Pointer to sprite handle
 * @param   pstRenderer
 *          Pointer to SDL rendering context
 * @return  Error code
 * @retval  0:  OK
 * @retval  -1: Error
 */
int Entity_Draw(
    const Entity* pstEntity,
    const Camera* pstCamera,
    const Sprite* pstSprite,
    SDL_Renderer* pstRenderer)
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

    stSrc.x = pstSprite->u16ImageOffsetX;
    stSrc.x += pstEntity->u8FrameOffsetX * pstEntity->u16Width;
    stSrc.x += pstEntity->u8AnimFrame * pstEntity->u16Width;
    stSrc.y = pstSprite->u16ImageOffsetY;
    stSrc.y += pstEntity->u8FrameOffsetY * pstEntity->u16Height;
    stSrc.w = pstEntity->u16Width;
    stSrc.h = pstEntity->u16Height;
    stDst.x = dPosX - (pstEntity->u16Width / 2);
    stDst.y = dPosY - (pstEntity->u16Height / 2);
    stDst.w = pstEntity->u16Width;
    stDst.h = pstEntity->u16Height;

    if (0 != SDL_RenderCopyEx(pstRenderer, pstSprite->pstTexture, &stSrc, &stDst, 0, NULL, s8Flip))
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "%s\n", SDL_GetError());
        return -1;
    }
    return 0;
}

/**
 * @brief   Drop entity
 * @details Sets the entity's IS_IN_MID_AIR flag
 * @param   pstEntity
 *          Pointer to entity handle
 */
void Entity_Drop(Entity* pstEntity)
{
    Utils_SetFlag(IS_IN_MID_AIR, &pstEntity->u16Flags);
}

/**
 * @brief   Free entity
 * @details Frees up allocated memory and unloads entity
 * @param   pstEntity
 *          Pointer to entity handle
 */
void Entity_Free(Entity* pstEntity)
{
    SDL_free(pstEntity);
}

/**
 * @brief   Free camera
 * @details Frees up allocated memory and unloads camera
 * @param   pstCamera
 *          Pointer to camera handle
 */
void Entity_FreeCamera(Camera* pstCamera)
{
    SDL_free(pstCamera);
}

/**
 * @brief   Free Sprite
 * @details Frees up allocated memory and unloads sprite
 * @param   pstSprite
 *          Pointer to sprite handle
 */
void Entity_FreeSprite(Sprite* pstSprite)
{
    if (pstSprite)
    {
        SDL_DestroyTexture(pstSprite->pstTexture);
        SDL_free(pstSprite);
        SDL_Log("Unload sprite image file.\n");
    }
}

/**
 * @brief   Initialise entity
 * @details Initialises entity
 * @param   dPosX
 *          Initial position along the x-axis
 * @param   dPosY
 *          Initial position along the y-axis
 * @param   u16Width
 *          Entity width in pixel
 * @param   u16Height
 *          Entity height in pixel
 * @param   pstEntity
 *          Pointer to entity handle
 * @return  Error code
 * @retval  0:  OK
 * @retval  -1: Error
 */
int Entity_Init(
    const double dPosX,
    const double dPosY,
    const Uint16 u16Width,
    const Uint16 u16Height,
    Entity**     pstEntity)
{
    *pstEntity = SDL_calloc(sizeof(struct Entity_t), sizeof(Sint8));
    if (!*pstEntity)
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "InitEntity(): error allocating memory.\n");
        return -1;
    }

    (*pstEntity)->dPosX         = dPosX;
    (*pstEntity)->dPosY         = dPosY;
    (*pstEntity)->eDirection    = RIGHT;
    (*pstEntity)->dAcceleration = 8.f;
    (*pstEntity)->dMaxVelocityX = 4.5f;
    (*pstEntity)->u16Width      = u16Width;
    (*pstEntity)->u16Height     = u16Height;
    (*pstEntity)->dAnimSpeed    = 12.f;

    return 0;
}

/**
 * @brief   Initialise camera
 * @details Initialises the camera
 * @param   pstCamera
 *          Pointer to camera handle
 * @return  Error code
 * @retval  0:  OK
 * @retval  -1: Error
 */
int Entity_InitCamera(Camera** pstCamera)
{
    *pstCamera = SDL_calloc(sizeof(struct Camera_t), sizeof(Sint8));
    if (!*pstCamera)
    {
        return -1;
    }

    SDL_Log("Initialise camera.\n");
    return 0;
}

/**
 * @brief   Initialise sprite
 * @details Initialises sprite image
 * @param   pacFileName
 *          Path and filename of the image file to load
 * @param   u16Width
 *          Sprite width in pixel
 * @param   u16Height
 *          Sprite height in pixel
 * @param   u16ImageOffsetX
 *          Image pixel offset along the x-axis in case a partial image
 *          should be loaded
 * @param   u16ImageOffsetY
 *          Image pixel offset along the y-axis in case a partial image
 *          should be loaded
 * @param   pstSprite
 *          Pointer to sprite handle
 * @param   pstRenderer
 *          Pointer to SDL rendering context
 * @return  Error code
 * @retval  0:  OK
 * @retval  -1: Error
 */
int Entity_InitSprite(
    const char*   pacFileName,
    const Uint16  u16Width,
    const Uint16  u16Height,
    const Uint16  u16ImageOffsetX,
    const Uint16  u16ImageOffsetY,
    Sprite**      pstSprite,
    SDL_Renderer* pstRenderer)
{
    *pstSprite = SDL_calloc(sizeof(struct Sprite_t), sizeof(Sint8));
    if (!*pstSprite)
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "InitSprite(): error allocating memory.\n");
        return -1;
    }

    (*pstSprite)->pstTexture = IMG_LoadTexture(pstRenderer, pacFileName);
    if (!(*pstSprite)->pstTexture)
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

/**
 * @brief   Check if camera is locked
 * @details Check whether the camera's IS_LOCKED flag is set or not
 * @param   pstCamera
 *          Pointer to camera handle
 * @return  Current lock state of the camera
 * @retval  SDL_TRUE:  Camera is locked
 * @retval  SDL_FALSE: Camera is not locked
 */
SDL_bool Entity_IsCameraLocked(const Camera* pstCamera)
{
    if (Utils_IsFlagSet(IS_LOCKED, pstCamera->u16Flags))
    {
        return SDL_TRUE;
    }
    else
    {
        return SDL_FALSE;
    }
}

/**
 * @brief   Check if entity is moving
 * @details Check whether the entity's IS_MOVING flag is set or not
 * @param   pstEntity
 *          Pointer to entity handle
 * @return  Current moving state of the entity
 * @retval  SDL_TRUE: Entity is moving
 * @retval  SDL_FALSE: Entity is not moving
 */
SDL_bool Entity_IsMoving(const Entity* pstEntity)
{
    if (Utils_IsFlagSet(IS_MOVING, pstEntity->u16Flags))
    {
        return SDL_TRUE;
    }
    else
    {
        return SDL_FALSE;
    }
}

/**
 * @brief   Check if a entity is rising
 * @details Check if a entity accelerates up along the y-axis
 * @param   pstEntity
 *          Pointer to entity handle
 * @return  Current rising state of the entity
 * @retval  SDL_TRUE: Entity is rising
 * @retval  SDL_FALSE: Entity is not rising
 */
SDL_bool Entity_IsRising(const Entity* pstEntity)
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

/**
 * @brief   Jump with entity
 * @details Initiates a jump by applying force
 * @param   dForce
 *          Initial jump force
 * @param   pstEntity
 *          Pointer to entity handle
 */
void Entity_Jump(const double dForce, Entity* pstEntity)
{
    if (! pstEntity->bIsJumping)
    {
        // Prevent jumping while falling down.
        if (0 >= pstEntity->dVelocityY)
        {
            // Initial lift-up; may need adjustment (estimated value).
            pstEntity->dPosY -= pstEntity->u16Height / 8.0;
            pstEntity->dVelocityY = -dForce;  // Apply force.
            pstEntity->bIsJumping = 1;
        }
    }
}

/**
 * @brief   Lock camera
 * @details Sets the camera's IS_LOCKED flag
 * @param   pstCamera
 *          Pointer to camera handle
 */
void Entity_LockCamera(Camera* pstCamera)
{
    Utils_SetFlag(IS_LOCKED, &pstCamera->u16Flags);
}

/**
 * @brief   Set entity in motion
 * @details Sets the entity's IS_MOVING flag
 * @param   pstEntity
 *          Pointer to entity handle
 */
void Entity_Move(Entity* pstEntity)
{
    Utils_SetFlag(IS_MOVING, &pstEntity->u16Flags);
}

/**
 * @brief   Move entity with parameters
 * @details Same as Entity_Move() but all parameters can be set at once
 * @param   eDirection
 *          Direction
 * @param   dAcceleration
 *          Acceleration
 * @param   dMaxVelocityX
 *          Maximum velocity along the y-axis
 * @param   u8AnimStart
 *          Start frame
 * @param   u8AnimEnd
 *          Last frame
 * @param   dAnimSpeed
 *          Animation speed
 * @param   u8FrameOffsetY
 *          Frame offset in frames along the y-axis
 * @param   pstEntity
 *          Pointer to entity handle
 */
void Entity_MoveFull(
    const Direction eDirection,
    const double    dAcceleration,
    const double    dMaxVelocityX,
    const Uint8     u8AnimStart,
    const Uint8     u8AnimEnd,
    const double    dAnimSpeed,
    const Uint8     u8FrameOffsetY,
    Entity*         pstEntity)
{
    Utils_SetFlag(IS_MOVING, &pstEntity->u16Flags);
    Entity_SetFrameOffset(0, u8FrameOffsetY, pstEntity);
    Entity_SetSpeed(dAcceleration, dMaxVelocityX, pstEntity);
    Entity_SetDirection(eDirection, pstEntity);
    Entity_SetAnimation(u8AnimStart, u8AnimEnd, dAnimSpeed, pstEntity);
}

/**
 * @brief   Reset entity flags
 * @details Resets all flags of an entity
 * @param   pstEntity
 *          Pointer to entity handle
 */
void Entity_Reset(Entity* pstEntity)
{
    pstEntity->u16Flags = 0;
}

/**
 * @brief   Reset to spawn position
 * @details Moves an entity to its configured spawn position
 * @param   pstEntity
 *          Pointer to entity handle
 */
void Entity_ResetToSpawnPosition(Entity* pstEntity)
{
    pstEntity->dPosX = pstEntity->dSpawnPosX;
    pstEntity->dPosY = pstEntity->dSpawnPosY;
}

/**
 * @brief   Set camera target
 * @details Sets the camera's target entity that should be tracked
 * @param   s32LogicalWindowWidth
 *          Logical window width in pixel
 * @param   s32LogicalWindowHeight
 *          Locical window height in pixel
 * @param   pstEntity
 *          Pointer to entity handle
 * @param   pstCamera
 *          Pointer to camera handle
 */
void Entity_SetCameraTarget(
    const Sint32  s32LogicalWindowWidth,
    const Sint32  s32LogicalWindowHeight,
    const Entity* pstEntity,
    Camera*       pstCamera)
{
    if (Utils_IsFlagSet(IS_LOCKED, pstCamera->u16Flags))
    {
        pstCamera->dPosX = pstEntity->dPosX;
        pstCamera->dPosX -= s32LogicalWindowWidth / 2.0;
        pstCamera->dPosY = pstEntity->dPosY;
        pstCamera->dPosY -= s32LogicalWindowHeight / 2.0;

        if (pstCamera->dPosX < 0)
        {
            pstCamera->dPosX = 0;
        }
    }
}

/**
 * @brief   Set animation for entity
 * @details Sets animation start, end and speed for entity
 * @param   u8AnimStart
 *          Start frame
 * @param   u8AnimEnd
 *          Last frame
 * @param   dAnimSpeed
 *          Animation speed
 * @param   pstEntity
 *          Pointer to entity handle
 */
void Entity_SetAnimation(
    const Uint8  u8AnimStart,
    const Uint8  u8AnimEnd,
    const double dAnimSpeed,
    Entity*      pstEntity)
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

/**
 * @brief   Set camera boundaries to map size
 * @details Sets the camera's boundaries to size of the map
 * @param   s32LogicalWindowWidth
 *          Logical window width in pixel
 * @param   s32LogicalWindowHeight
 *          Logical window height in pixel
 * @param   u16MapWidth
 *          Map width in pixel
 * @param   u16MapHeight
 *          Map height in pixel
 * @param   pstCamera
 *          Pointer to camera handle
 * @return  Current state of camera
 * @retval  0: Camera has reached set boundaries
 * @retval  1: Camera hasn't reached set boundaries
 */
int Entity_SetCameraBoundariesToMapSize(
    const Sint32 s32LogicalWindowWidth,
    const Sint32 s32LogicalWindowHeight,
    const Uint16 u16MapWidth,
    const Uint16 u16MapHeight,
    Camera*      pstCamera)
{
    SDL_bool bReturnValue = 0;
    pstCamera->s32MaxPosX = u16MapWidth - s32LogicalWindowWidth;
    pstCamera->s32MaxPosY = u16MapHeight - s32LogicalWindowHeight;

    if (pstCamera->dPosX <= 0)
    {
        pstCamera->dPosX = 0;
        bReturnValue     = 1;
    }

    if (pstCamera->dPosY <= 0)
    {
        pstCamera->dPosY = 0;
    }

    if (pstCamera->dPosX > pstCamera->s32MaxPosX)
    {
        pstCamera->dPosX = pstCamera->s32MaxPosX;
        bReturnValue     = 1;
    }

    if (pstCamera->dPosY > pstCamera->s32MaxPosY)
    {
        pstCamera->dPosY = pstCamera->s32MaxPosY;
    }

    return bReturnValue;
}

/**
 * @brief   Set entity direction
 * @details Sets the direction of an entity
 * @param   eDirection
 *          Direction
 * @param   pstEntity
 *          Pointer to entity handle
 */
void Entity_SetDirection(const Direction eDirection, Entity* pstEntity)
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

/**
 * @brief   Set frame offset
 * @details Sets a frame offset in frames
 * @param   u8OffsetX
 *          Offset in frames along the x-axis
 * @param   u8OffsetY
 *          Offset in frames along the y-axis
 * @param   pstEntity
 *          Pointer to entity handle
 * @todo    Describe this function in more detail
 */
void Entity_SetFrameOffset(const Uint8 u8OffsetX, const Uint8 u8OffsetY, Entity* pstEntity)
{
    pstEntity->u8FrameOffsetX = u8OffsetX;
    pstEntity->u8FrameOffsetY = u8OffsetY;
}

/**
 * @brief   Set entity position
 * @details Sets the absolute position for an entity
 * @param   dPosX
 *          Position along the x-axis
 * @param   dPosY
 *          Position along the y-axis
 * @param   pstEntity
 *          Pointer to entity handle
 */
void Entity_SetPosition(const double dPosX, const double dPosY, Entity* pstEntity)
{
    pstEntity->dPosX = dPosX;
    pstEntity->dPosY = dPosY;
}

/**
 * @brief   Set entity spawn position
 * @details Sets absolute (re)spawn position for an entity
 * @param   dPosX
 *          Position along the x-axis
 * @param   dPosY
 *          Position along the y-axis
 * @param   pstEntity
 *          Pointer to entity handle
 */
void Entity_SetSpawnPosition(const double dPosX, const double dPosY, Entity* pstEntity)
{
    pstEntity->dSpawnPosX = dPosX;
    pstEntity->dSpawnPosY = dPosY;
}

/**
 * @brief   Set entity speed
 * @details Sets a entity's acceleration and maximum velocity along the
 *          x-axis
 * @param   dAcceleration
 *          Acceleration
 * @param   dMaxVelocityX
 *          Maximum velocity along the y-axis
 * @param   pstEntity
 *          Pointer to entity handle
 */
void Entity_SetSpeed(const double dAcceleration, const double dMaxVelocityX, Entity* pstEntity)
{
    pstEntity->dAcceleration = dAcceleration;
    pstEntity->dMaxVelocityX = dMaxVelocityX;
}

/**
 * @brief   Stop entity
 * @details Clears the IS_MOVING flag of an entity
 * @param   pstEntity
 *          Pointer to entity handle
 */
void Entity_Stop(Entity* pstEntity)
{
    Utils_ClearFlag(IS_MOVING, &pstEntity->u16Flags);
}

/**
 * @brief   Unlock camera
 * @details Clears the IS_LOCKED flag of a camera entity
 * @param   pstCamera
 *          Pointer to camera handle
 */
void Entity_UnlockCamera(Camera* pstCamera)
{
    Utils_ClearFlag(IS_LOCKED, &pstCamera->u16Flags);
}

/**
 * @brief   Update entity
 * @details Updates the current state of an entity
 * @remark  This function is usually called once per frame
 * @param   dDeltaTime
 *          Delta time since last call
 * @param   dGravitation
 *          Gravitational constant of entity
 * @param   u8MeterInPixel
 *          Definition of meter in pixel
 * @param   pstEntity
 *          Pointer to entity handle
 */
void Entity_Update(
    const double dDeltaTime,
    const double dGravitation,
    const Uint8  u8MeterInPixel,
    Entity*      pstEntity)
{
    double dPosX = pstEntity->dPosX;
    double dPosY = pstEntity->dPosY;

    // Apply gravitation.
    if (0 != dGravitation)
    {
        if (Entity_IsRising(pstEntity))
        {
            Utils_SetFlag(IS_IN_MID_AIR, &pstEntity->u16Flags);
        }

        if (Utils_IsFlagSet(IS_IN_MID_AIR, pstEntity->u16Flags))
        {
            double dG         = dGravitation * u8MeterInPixel;
            double dDistanceY = dG * DELTA_TIME * DELTA_TIME;
            pstEntity->dVelocityY += dDistanceY;
            dPosY += pstEntity->dVelocityY;
        }
        else
        {
            pstEntity->bIsJumping = 0;
            // Correct position along the y-axis.
            pstEntity->dVelocityY = 0.f;
            dPosY                 = (16.f * Utils_Round(dPosY / 16.f));
        }
    }

    // Calculate horizontal velocity.
    if (Utils_IsFlagSet(IS_MOVING, pstEntity->u16Flags))
    {
        double dAccel     = pstEntity->dAcceleration * (double)u8MeterInPixel;
        double dDistanceX = dAccel * DELTA_TIME * DELTA_TIME;
        pstEntity->dVelocityX += dDistanceX;
    }
    else
    {
        pstEntity->dVelocityX -= pstEntity->dAcceleration * DELTA_TIME;
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
    Entity_SetPosition(dPosX, dPosY, pstEntity);

    // Update axis-aligned bounding box.
    pstEntity->stBB.dBottom = dPosY + (double)(pstEntity->u16Height / 2.f);
    pstEntity->stBB.dLeft   = dPosX - (double)(pstEntity->u16Width / 2.f);
    pstEntity->stBB.dRight  = dPosX + (double)(pstEntity->u16Width / 2.f);
    pstEntity->stBB.dTop    = dPosY - (double)(pstEntity->u16Height / 2.f);

    if (pstEntity->stBB.dLeft <= 0)
    {
        pstEntity->stBB.dLeft = 0;
    }

    if (pstEntity->stBB.dTop <= 0)
    {
        pstEntity->stBB.dTop = 0;
    }

    // Update animation frame.
    if (Utils_IsFlagSet(IS_ANIMATED, pstEntity->u16Flags))
    {
        pstEntity->dAnimDelay += dDeltaTime;

        if (pstEntity->u8AnimFrame < pstEntity->u8AnimStart)
        {
            pstEntity->u8AnimFrame = pstEntity->u8AnimStart;
        }

        if (pstEntity->dAnimDelay > (1.f / pstEntity->dAnimSpeed - dDeltaTime))
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

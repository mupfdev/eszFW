/**
 * @file Main.c
 * @ingroup Main
 * @defgroup Main
 * @author Michael Fitzmayer
 * @copyright "THE BEER-WARE LICENCE" (Revision 42)
 */

#include <SDL.h>
#include <eszfw.h>
#include <math.h>
#include <stdbool.h>
#include <stdint.h>

static Audio      *pstAudio;
static Background *pstBg;
static Camera     *pstCamera;
static Entity     *pstEntity[1];
static Font       *pstFont;
static Object     *pstObject[1];
static Map        *pstMap;
static Music      *pstMusic;
static Sprite     *pstSprite;
static Video      *pstVideo;

static double dBgVelocityX;
static double dDeltaTime = 0.f;

static int  Init();
static int  Render();
static void Quit();

#ifndef __ANDROID__
int main(int sArgC, char *pacArgV[])
#else
int SDL_main(int sArgC, char *pacArgV[])
#endif
{
    int        sReturnValue  = 0;
    bool       bIsRunning    = true;
    double     dTimeA        = 0.f;
    double     dTimeB        = 0.f;
    bool       bIsOnPlatform = true;
    bool       bOrientation  = LEFT;
    bool       bIsMoving     = false;
    SDL_Event  stEvent;

    (void)sArgC;
    (void)pacArgV;

    sReturnValue = Init();
    sReturnValue = Render();
    if (-1 == sReturnValue)
    {
        bIsRunning = false;
    }

    #ifdef __ANDROID__
    double  dZoomLevel;
    double  dZoomLevelMin;
    double  dZoomLevelMax;
    int32_t s32TouchPosX;
    int32_t s32WindowW;

    if (bIsRunning)
    {
        dZoomLevel    = 0.f;
        dZoomLevelMin = (double)pstVideo->s32WindowHeight / (double)pstMap->u16Height;
        dZoomLevelMax = pstVideo->dInitialZoomLevel;
        s32TouchPosX  = 0;
        s32WindowW    = pstVideo->s32LogicalWindowWidth;
    }
    #endif

    InitFPSLimiter(&dTimeA, &dTimeB, &dDeltaTime);
    while (bIsRunning)
    {
        // Limit framerate.
        LimitFramerate(60, &dTimeA, &dTimeB, &dDeltaTime);

        // Render scene.
        sReturnValue = Render();
        RenderScene(pstVideo->pstRenderer);

        if (-1 == sReturnValue)
        {
            bIsRunning = false;
            continue;
        }

        // Reset entity flags.
        ResetEntity(pstEntity[0]);

        // Handle events.
        while(SDL_PollEvent(&stEvent) != 0)
        {
            if (stEvent.type == SDL_QUIT)
            {
                bIsRunning = false;
            }
            #ifndef __ANDROID__
            else if (SDL_KEYDOWN == stEvent.type)
            {
                switch(stEvent.key.keysym.sym)
                {
                  case SDLK_q:
                      bIsRunning = false;
                      break;
                  case SDLK_LEFT:
                      bOrientation = LEFT;
                      bIsMoving    = true;
                      break;
                  case SDLK_RIGHT:
                      bOrientation = RIGHT;
                      bIsMoving    = true;
                      break;
                  case SDLK_SPACE:
                      JumpEntity(2.5f + (0.75f * (*pstEntity)->dVelocityX), pstEntity[0]);
                      break;
                  case SDLK_LSHIFT:
                      UnlockCamera(pstCamera);
                      break;
                }
            }
            else if (SDL_KEYUP == stEvent.type)
            {
                switch(stEvent.key.keysym.sym)
                {
                  case SDLK_LEFT:
                      bIsMoving = false;
                      break;
                  case SDLK_RIGHT:
                      bIsMoving = false;
                      break;
                  case SDLK_LSHIFT:
                      LockCamera(pstCamera);
                      break;
                }
            }
            #else // __ANDROID__
            else if (SDL_KEYDOWN == stEvent.type)
            {
                if (stEvent.key.keysym.sym == SDLK_AC_BACK)
                {
                    bIsRunning = false;
                }
            }
            else if (SDL_FINGERDOWN == stEvent.type)
            {
                s32TouchPosX = (int32_t)round(stEvent.tfinger.x * s32WindowW);

                if (s32TouchPosX < (s32WindowW / 2))
                {
                    bOrientation = LEFT;
                    bIsMoving    = true;
                }
                else
                {
                    bOrientation = RIGHT;
                    bIsMoving    = true;
                }
            }
            else if (SDL_FINGERUP == stEvent.type)
            {
                bIsMoving = false;
            }
            else if (SDL_FINGERMOTION == stEvent.type)
            {
                if (0.05 < fabs(stEvent.tfinger.dy))
                {
                    JumpEntity(2.5f + (0.75f * (*pstEntity)->dVelocityX), pstEntity[0]);
                }
            }
            else if (SDL_MULTIGESTURE == stEvent.type)
            {
                bIsMoving  = false;
                dZoomLevel = pstVideo->dZoomLevel;
                if (0.002 < fabs(stEvent.mgesture.dDist))
                {
                    if (0 < stEvent.mgesture.dDist)
                    {
                        // Pinch open.
                        dZoomLevel += 5.f * dDeltaTime;
                    }
                    else
                    {
                        // Pinch close.
                        dZoomLevel -= 5.f * dDeltaTime;
                    }

                    if (dZoomLevel <= dZoomLevelMin)
                    {
                        dZoomLevel = dZoomLevelMin;
                    }
                    if (dZoomLevel >= dZoomLevelMax)
                    {
                        dZoomLevel = dZoomLevelMax;
                    }

                    SetZoomLevel(dZoomLevel, pstVideo);
                }
            }
            #endif // __ANDROID__
        }

        // Set the player's idle animation.
        if (! IsEntityMoving(pstEntity[0]))
        {
            AnimateEntity(true, pstEntity[0]);
            SetFrameOffset(0, 0, pstEntity[0]);
            SetAnimation(
                0, 11,
                pstEntity[0]->dAnimSpeed,
                pstEntity[0]);
        }

        // Move player entity.
        if (bIsMoving)
        {
            AnimateEntity(true, pstEntity[0]);
            MoveEntityFull(
                bOrientation, 6.0, 3.0, 1, 7,
                pstEntity[0]->dAnimSpeed,
                1,
                pstEntity[0]);
        }

        // Update game logic.
        // Set up collision detection.
        if (IsOnTileOfType(
                "Platform", pstEntity[0]->dPosX, pstEntity[0]->dPosY,
                pstEntity[0]->u16Height, pstMap))
        {
            bIsOnPlatform = true;
        }
        else
        {
            bIsOnPlatform = false;
        }

        if (pstEntity[0]->dPosX < 0.f || pstEntity[0]->dPosX > pstMap->u16Width)
        {
            bIsOnPlatform = true;
        }

        if (! bIsOnPlatform)
        {
            AnimateEntity(false, pstEntity[0]);
            if (IsEntityRising(pstEntity[0]))
            {
                SetFrameOffset(0, 0, pstEntity[0]);
            }
            else
            {
                SetFrameOffset(0, 1, pstEntity[0]);
            }
            SetAnimation(14, 14, pstEntity[0]->dAnimSpeed, pstEntity[0]);
            DropEntity(pstEntity[0]);
        }

        UpdateEntity(dDeltaTime, pstMap->dGravitation, pstMap->u8MeterInPixel, pstEntity[0]);

        // Follow player entity and set camera boudnaries to map size.
        SetCameraTargetEntity(
            pstVideo->s32LogicalWindowWidth,
            pstVideo->s32LogicalWindowHeight,
            pstCamera,
            pstEntity[0]);

        if (1 == SetCameraBoundariesToMapSize(
            pstVideo->s32LogicalWindowWidth,
            pstVideo->s32LogicalWindowHeight,
            pstMap->u16Width,
            pstMap->u16Height,
            pstCamera))
        {
            // Do not move background when camera hits boundaries.
            dBgVelocityX = 0.f;
        }
        else
        {
            dBgVelocityX = pstEntity[0]->dVelocityX;
        }

        // Set zoom level dynamically in relation to vertical velocity.
        #ifndef __ANDROID__
        if (0.0 < pstEntity[0]->dVelocityY)
        {
            pstVideo->dZoomLevel -= dDeltaTime / 3.5f;
            if (1.0 > pstVideo->dZoomLevel)
            {
                pstVideo->dZoomLevel = 1;
            }
        }
        else
        {
            pstVideo->dZoomLevel += dDeltaTime / 1.75f;
            if (pstVideo->dZoomLevel > pstVideo->dInitialZoomLevel)
            {
                pstVideo->dZoomLevel = pstVideo->dInitialZoomLevel;
            }
        }
        SetZoomLevel(pstVideo->dZoomLevel, pstVideo);
        #endif // __ANDROID__

        ConnectHorizontalMapEndsForEntity(
            pstMap->u16Width,
            pstEntity[0]);

        if (pstEntity[0]->dPosY > (pstMap->u16Height + pstEntity[0]->u16Height))
        {
            ResetEntityToSpawnPosition(pstEntity[0]);
        }
    }

    Quit();

    if (-1 == sReturnValue)
    {
        return EXIT_FAILURE;
    }
    else
    {
        return EXIT_SUCCESS;
    }
}

static int Init()
{
    int sReturnValue = 0;

    const char *pacBgFileNames[4] = {
        "res/backgrounds/sky.png",
        "res/backgrounds/clouds.png",
        "res/backgrounds/sea.png",
        "res/backgrounds/far-grounds.png",
    };

    sReturnValue = InitVideo(
        "Rainbow Joe", 640, 360, 384, 216,
        false, &pstVideo);
    RETURN_ON_ERROR(sReturnValue);

    sReturnValue = InitCamera(&pstCamera);
    RETURN_ON_ERROR(sReturnValue);

    sReturnValue = InitEntity(0, 0, 24, 40, &pstEntity[0]);
    RETURN_ON_ERROR(sReturnValue);

    sReturnValue = InitMap("res/maps/Demo.tmx", "res/tilesets/tileset.png", 32, &pstMap);
    RETURN_ON_ERROR(sReturnValue);

    sReturnValue = InitObject(&pstObject[0]);
    RETURN_ON_ERROR(sReturnValue);

    sReturnValue = InitFont("res/ttf/FifteenNarrow.ttf", &pstFont);
    RETURN_ON_ERROR(sReturnValue);

    sReturnValue = InitBackground(
        4, pacBgFileNames, pstVideo->s32WindowWidth, BOTTOM,
        pstVideo->pstRenderer, &pstBg);
    RETURN_ON_ERROR(sReturnValue);

    /*sReturnValue = InitSprite(
        "res/sprites/characters_7.png", 736, 128, 0, 0,
        &pstSprite, &pstVideo->pstRenderer);
    RETURN_ON_ERROR(sReturnValue);*/

    sReturnValue = InitSprite(
        "res/sprites/player.png", 360, 80, 0, 0,
        &pstSprite, pstVideo->pstRenderer);
    RETURN_ON_ERROR(sReturnValue);

    sReturnValue = InitAudio(&pstAudio);
    RETURN_ON_ERROR(sReturnValue);

    sReturnValue = InitMusic(
        "res/music/LeftRightExcluded.ogg", -1,
        &pstMusic);
    RETURN_ON_ERROR(sReturnValue);

    LockCamera(pstCamera);
    PlayMusic(3000, pstMusic);

    GetSingleObjectByName("Player", pstMap, &pstObject[0]);
    SetPosition(pstObject[0]->dPosX, pstObject[0]->dPosY, pstEntity[0]);
    SetSpawnPosition(pstObject[0]->dPosX, pstObject[0]->dPosY, pstEntity[0]);
    SetFrameOffset(0, 0, pstEntity[0]);
    SetFontColour(0xfe, 0x95, 0x14, pstFont);

    return sReturnValue;
}

static int Render()
{
    int sReturnValue = 0;

    sReturnValue = DrawBackground(
        pstEntity[0]->bOrientation,
        pstVideo->s32LogicalWindowHeight,
        pstCamera->dPosY,
        dBgVelocityX,
        pstVideo->pstRenderer,
        pstBg);

    sReturnValue = DrawMap(
        0, false, true, "BG",
        pstCamera->dPosX,
        pstCamera->dPosY,
        dDeltaTime,
        pstMap,
        pstVideo->pstRenderer);

    sReturnValue = DrawEntity(
        pstEntity[0],
        pstCamera,
        pstSprite,
        pstVideo->pstRenderer);

    sReturnValue = DrawMap(
        1, false, false, "FG",
        pstCamera->dPosX,
        pstCamera->dPosY,
        dDeltaTime,
        pstMap,
        pstVideo->pstRenderer);

    return sReturnValue;
}

static void Quit()
{
    FreeMusic(pstMusic);
    FreeAudio(pstAudio);
    FreeSprite(pstSprite);
    FreeBackground(pstBg);
    FreeFont(pstFont);
    FreeObject(pstObject[0]);
    FreeMap(pstMap);
    FreeEntity(pstEntity[0]);
    FreeCamera(pstCamera);
    FreeVideo(pstVideo);
    SDL_Quit();
}

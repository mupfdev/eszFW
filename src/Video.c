/**
 * @file Video.c
 * @ingroup Video
 * @defgroup Video
 * @author Michael Fitzmayer
 * @copyright "THE BEER-WARE LICENCE" (Revision 42)
 */

#include <SDL.h>
#include <SDL_image.h>
#include "Video.h"
#include "Constants.h"

void FreeVideo(Video *pstVideo)
{
    IMG_Quit();
    if (pstVideo)
    {
        if (pstVideo->pstRenderer)
        {
            SDL_DestroyRenderer(pstVideo->pstRenderer);
        }
        if (pstVideo->pstWindow)
        {
            SDL_DestroyWindow(pstVideo->pstWindow);
        }
        SDL_free(pstVideo);
        SDL_Log("Terminate window.\n");
    }
}

Sint8 InitVideo(
    const char    *pacWindowTitle,
    const Sint32   s32WindowWidth,
    const Sint32   s32WindowHeight,
    const Sint32   s32LogicalWindowWidth,
    const Sint32   s32LogicalWindowHeight,
    const SDL_bool bFullscreen,
    Video        **pstVideo)
{
    SDL_DisplayMode stDisplayMode;
    Uint32          u32Flags = 0;

    *pstVideo = SDL_malloc(sizeof(struct Video_t));
    if (! *pstVideo)
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "InitVideo(): error allocating memory.\n");
        return -1;
    }

    (*pstVideo)->pstRenderer            = NULL;
    (*pstVideo)->pstWindow              = NULL;
    (*pstVideo)->s32WindowHeight        = s32WindowHeight;
    (*pstVideo)->s32WindowWidth         = s32WindowWidth;
    (*pstVideo)->s32LogicalWindowWidth  = s32LogicalWindowWidth;
    (*pstVideo)->s32LogicalWindowHeight = s32LogicalWindowHeight;
    (*pstVideo)->u8RefreshRate          = 60;
    (*pstVideo)->dTimeA                 = SDL_GetTicks();
    (*pstVideo)->dTimeB                 = SDL_GetTicks();
    (*pstVideo)->dDeltaTime             = ((*pstVideo)->dTimeB - (*pstVideo)->dTimeA) / 1000.f;

    if (0 > SDL_Init(SDL_INIT_VIDEO))
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "%s\n", SDL_GetError());
        return -1;
    }

    SDL_GetCurrentDisplayMode(0, &stDisplayMode);
    if (stDisplayMode.refresh_rate != 0)
    {
        (*pstVideo)->u8RefreshRate = stDisplayMode.refresh_rate;
    }

    if (IMG_INIT_PNG != IMG_Init(IMG_INIT_PNG))
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "%s\n", IMG_GetError());
        return -1;
    }

    if (bFullscreen)
    {
        u32Flags = u32Flags | SDL_WINDOW_FULLSCREEN_DESKTOP;
    }

    #ifdef __ANDROID__
    u32Flags = 0;
    if (0 == SDL_GetCurrentDisplayMode(0, &stDisplayMode))
    {
        (*pstVideo)->s32WindowWidth  = stDisplayMode.w;
        (*pstVideo)->s32WindowHeight = stDisplayMode.h;
    }
    #endif

    (*pstVideo)->pstWindow = SDL_CreateWindow(
        pacWindowTitle,
        SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED,
        (*pstVideo)->s32WindowWidth,
        (*pstVideo)->s32WindowHeight,
        u32Flags);

    if (! (*pstVideo)->pstWindow)
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "%s\n", SDL_GetError());
        return -1;
    }

    if (0 > SDL_ShowCursor(SDL_DISABLE))
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "%s\n", SDL_GetError());
        return -1;
    }

    SDL_GetWindowSize(
        (*pstVideo)->pstWindow,
        &(*pstVideo)->s32WindowWidth,
        &(*pstVideo)->s32WindowHeight);

    (*pstVideo)->dZoomLevel =
        (double)(*pstVideo)->s32WindowHeight / (double)s32LogicalWindowHeight;
    (*pstVideo)->dInitialZoomLevel = (*pstVideo)->dZoomLevel;

    (*pstVideo)->pstRenderer = SDL_CreateRenderer(
        (*pstVideo)->pstWindow,
        -1,
        SDL_RENDERER_ACCELERATED | SDL_RENDERER_TARGETTEXTURE);

    if (! (*pstVideo)->pstRenderer)
    {
        SDL_DestroyWindow((*pstVideo)->pstWindow);
        return -1;
    }

    SDL_Log(
        "Setting up window at resolution %dx%d @ %d FPS.\n",
        (*pstVideo)->s32WindowWidth, (*pstVideo)->s32WindowHeight, (*pstVideo)->u8RefreshRate);

    SetZoomLevel((*pstVideo)->dZoomLevel, *pstVideo);
    SDL_Log("Set initial zoom-level to factor %f.\n", (*pstVideo)->dZoomLevel);

    return 0;
}

void RenderScene(Video *pstVideo)
{
    double dTime = (double)APPROX_TIME_PER_FRAME / (double)TIME_FACTOR;

    pstVideo->dTimeB     = SDL_GetTicks();
    pstVideo->dDeltaTime = (pstVideo->dTimeB - pstVideo->dTimeA) / 1000.f;
    pstVideo->dTimeA     = pstVideo->dTimeB;

    if (pstVideo->dDeltaTime >= dTime)
    {
        pstVideo->dDeltaTime = dTime;
    }

    SDL_RenderPresent(pstVideo->pstRenderer);
    SDL_Delay(1000.f / (double)pstVideo->u8RefreshRate - pstVideo->dDeltaTime);
    SDL_RenderClear(pstVideo->pstRenderer);
}

Sint8 SetZoomLevel(const double dZoomLevel, Video *pstVideo)
{
    pstVideo->dZoomLevel             = dZoomLevel;
    pstVideo->s32LogicalWindowWidth  = pstVideo->s32WindowWidth  / dZoomLevel;
    pstVideo->s32LogicalWindowHeight = pstVideo->s32WindowHeight / dZoomLevel;

    if (0 != SDL_RenderSetLogicalSize(
            pstVideo->pstRenderer,
            pstVideo->s32LogicalWindowWidth,
            pstVideo->s32LogicalWindowHeight))
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "%s\n", SDL_GetError());
        return -1;
    }

    #ifdef DEBUG
    if (dZoomLevel != (*pstVideo)->dZoomLevel)
    {
        SDL_Log("Set zoom-level to factor %f.\n", dZoomLevel);
    }
    #endif

    return 0;
}

Sint8 ToggleFullscreen(Video *pstVideo)
{
    Sint8  s8ReturnValue;
    Uint32 u32WindowFlags;

    u32WindowFlags = SDL_GetWindowFlags(pstVideo->pstWindow);

    if (u32WindowFlags & SDL_WINDOW_FULLSCREEN_DESKTOP)
    {
        SDL_SetWindowPosition(pstVideo->pstWindow, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
        s8ReturnValue = SDL_SetWindowFullscreen(pstVideo->pstWindow, 0);
        SDL_Log("Set window to windowed mode.\n");
    }
    else
    {
        s8ReturnValue = SDL_SetWindowFullscreen(pstVideo->pstWindow, SDL_WINDOW_FULLSCREEN_DESKTOP);
        SDL_Log("Set window to fullscreen mode.\n");
    }

    if (0 != s8ReturnValue)
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "%s\n", SDL_GetError());
    }

    return s8ReturnValue;
}

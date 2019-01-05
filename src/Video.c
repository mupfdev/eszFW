/**
 * @file Video.c
 * @ingroup Video
 * @defgroup Video
 * @author Michael Fitzmayer
 * @copyright "THE BEER-WARE LICENCE" (Revision 42)
 */

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <stdbool.h>
#include <stdint.h>
#include "Video.h"

void FreeVideo(Video **pstVideo)
{
    IMG_Quit();
    SDL_DestroyRenderer((*pstVideo)->pstRenderer);
    SDL_DestroyWindow((*pstVideo)->pstWindow);
    free(*pstVideo);
    SDL_Log("Terminate window.\n");
}

int InitVideo(
    const char   *pacWindowTitle,
    const int32_t s32WindowWidth,
    const int32_t s32WindowHeight,
    const int32_t s32LogicalWindowWidth,
    const int32_t s32LogicalWindowHeight,
    const bool    bFullscreen,
    Video       **pstVideo)
{
    uint32_t  u32Flags = 0;

    *pstVideo = NULL;
    *pstVideo = malloc(sizeof(struct Video_t));
    if (NULL == *pstVideo)
    {
        SDL_LogError(
            SDL_LOG_CATEGORY_APPLICATION,
            "InitVideo(): error allocating memory.\n");
        return -1;
    }

    (*pstVideo)->pstRenderer            = NULL;
    (*pstVideo)->pstWindow              = NULL;
    (*pstVideo)->s32WindowHeight        = s32WindowHeight;
    (*pstVideo)->s32WindowWidth         = s32WindowWidth;
    (*pstVideo)->s32LogicalWindowWidth  = s32LogicalWindowWidth;
    (*pstVideo)->s32LogicalWindowHeight = s32LogicalWindowHeight;

    if (0 > SDL_Init(SDL_INIT_VIDEO))
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "%s\n", SDL_GetError());
        free(pstVideo);
        return -1;
    }

    if (IMG_INIT_PNG != IMG_Init(IMG_INIT_PNG))
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "%s\n", IMG_GetError());
        free(pstVideo);
        return -1;
    }

    if (bFullscreen)
    {
        u32Flags = SDL_WINDOW_FULLSCREEN_DESKTOP;
    }

    (*pstVideo)->pstWindow = SDL_CreateWindow(
        pacWindowTitle,
        SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED,
        (*pstVideo)->s32WindowWidth,
        (*pstVideo)->s32WindowHeight,
        u32Flags);

    if (NULL == (*pstVideo)->pstWindow)
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "%s\n", SDL_GetError());
        free(pstVideo);
        return -1;
    }

    if (0 > SDL_ShowCursor(SDL_DISABLE))
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "%s\n", SDL_GetError());
        free(pstVideo);
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

    if (NULL == (*pstVideo)->pstRenderer)
    {
        SDL_DestroyWindow((*pstVideo)->pstWindow);
        free(pstVideo);
        return -1;
    }

    SDL_Log(
        "Setting up window at resolution %dx%d.\n",
        (*pstVideo)->s32WindowWidth, (*pstVideo)->s32WindowHeight);

    SetZoomLevel((*pstVideo)->dZoomLevel, &(*pstVideo));
    SDL_Log("Set initial zoom-level to factor %f.\n", (*pstVideo)->dZoomLevel);

    return 0;
}

void RenderScene(SDL_Renderer **pstRenderer)
{
    SDL_RenderPresent(*pstRenderer);
    SDL_RenderClear(*pstRenderer);
}

int SetZoomLevel(const double dZoomLevel, Video **pstVideo)
{
    (*pstVideo)->dZoomLevel             = dZoomLevel;
    (*pstVideo)->s32LogicalWindowWidth  = (*pstVideo)->s32WindowWidth  / dZoomLevel;
    (*pstVideo)->s32LogicalWindowHeight = (*pstVideo)->s32WindowHeight / dZoomLevel;

    if (0 != SDL_RenderSetLogicalSize(
            (*pstVideo)->pstRenderer,
            (*pstVideo)->s32LogicalWindowWidth,
            (*pstVideo)->s32LogicalWindowHeight))
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

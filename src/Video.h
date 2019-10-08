/**
 * @file    Video.h
 * @ingroup Video
 */
#pragma once

#include <SDL.h>

/**
 * @typedef Video
 * @brief   Video handle type
 * @struct  Video_t
 * @brief   Video handle data
 */
typedef struct Video_t
{
    SDL_Renderer* pstRenderer;
    SDL_Window*   pstWindow;
    Sint32        s32WindowWidth;
    Sint32        s32WindowHeight;
    Sint32        s32LogicalWindowWidth;
    Sint32        s32LogicalWindowHeight;
    Uint8         u8RefreshRate;
    double        dZoomLevel;
    double        dInitialZoomLevel;
    double        dTimeA;
    double        dTimeB;
    double        dDeltaTime;
} Video;

void Video_Free(Video* pstVideo);

Sint8 Video_Init(
    const char*    pacWindowTitle,
    const Sint32   s32WindowWidth,
    const Sint32   s32WindowHeight,
    const Sint32   s32LogicalWindowWidth,
    const Sint32   s32LogicalWindowHeight,
    const SDL_bool bFullscreen,
    Video**        pstVideo);

void  Video_RenderScene(Video* pstVideo);
Sint8 Video_SetFullscreen(const SDL_bool dFullscreen, Video* pstVideo);
Sint8 Video_SetZoomLevel(const double dZoomLevel, Video* pstVideo);
Sint8 Video_ToggleFullscreen(Video* pstVideo);

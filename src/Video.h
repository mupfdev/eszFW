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
    SDL_Renderer* pstRenderer;             ///< Pointer to SDL2 rendering context
    SDL_Window*   pstWindow;               ///< SDL2 window handle
    Sint32        s32WindowWidth;          ///< Window width in pixel
    Sint32        s32WindowHeight;         ///< Window height in pixel
    Sint32        s32LogicalWindowWidth;   ///< Logical window width in pixel
    Sint32        s32LogicalWindowHeight;  ///< Logical window height in pixel
    Uint8         u8RefreshRate;           ///< Refresh rate
    double        dZoomLevel;              ///< Zoom-level
    double        dInitialZoomLevel;       ///< Initial zoom-level
    double        dTimeA;                  ///< Point in time A
    double        dTimeB;                  ///< Point in time B
    double        dDeltaTime;              ///< Delta time
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
Sint8 Video_SetZoomLevel(const double dZoomLevel, Video* pstVideo);
Sint8 Video_ToggleFullscreen(Video* pstVideo);

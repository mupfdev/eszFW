/**
 * @file Video.h
 * @ingroup Video
 */

#ifndef _VIDEO_H_
#define _VIDEO_H_

#include <SDL.h>

typedef struct Video_t
{
    SDL_Renderer *pstRenderer;
    SDL_Window   *pstWindow;
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

void FreeVideo(Video *pstVideo);

Sint8 InitVideo(
    const char    *pacWindowTitle,
    const Sint32   s32WindowWidth,
    const Sint32   s32WindowHeight,
    const Sint32   s32LogicalWindowWidth,
    const Sint32   s32LogicalWindowHeight,
    const SDL_bool bFullscreen,
    Video        **pstVideo);

void LimitFramerate(Video *pstVideo);
void RenderScene(Video *pstVideo);
Sint8 SetFullscreen(const SDL_bool dFullscreen, Video *pstVideo);
Sint8 SetZoomLevel(const double dZoomLevel, Video *pstVideo);
Sint8 ToggleFullscreen(Video *pstVideo);

#endif // _VIDEO_H_

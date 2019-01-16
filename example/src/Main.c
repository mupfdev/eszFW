/**
 * @file Main.c
 * @author Michael Fitzmayer
 * @copyright "THE BEER-WARE LICENCE" (Revision 42)
 */

#ifdef __ANDROID__
#include <SDL.h>
#else
#include <SDL2/SDL.h>
#endif
#include <eszFW.h>
#include <stdbool.h>
#include <stdlib.h>
#include "RainbowJoe/Main.h"

int main()
{
    int    sExecStatus = EXIT_SUCCESS;
    bool   bFullscreen = false;
    Video *pstVideo    = NULL;

    #ifdef __ANDROID__
    bFullscreen = true;
    #endif

    if (-1 == InitVideo(
            "eszFW Demo", 640, 360, 384, 216,
            bFullscreen, &pstVideo))
    {
        sExecStatus = EXIT_FAILURE;
        goto exit;
    }

    if (-1 == RainbowJoe(&pstVideo))
    {
        sExecStatus = EXIT_FAILURE;
        goto exit;
    }

exit:
    FreeVideo(&pstVideo);
    SDL_Quit();
    return sExecStatus;
}

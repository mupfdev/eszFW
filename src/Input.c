/**
 * @file Input.c
 * @ingroup Input
 * @defgroup Input
 * @author Michael Fitzmayer
 * @copyright "THE BEER-WARE LICENCE" (Revision 42)
 */

#include <SDL2/SDL.h>
#include <stdint.h>
#include "Input.h"

int ReadInput(const uint8_t **pu8KeyState)
{
    SDL_PumpEvents();
    *pu8KeyState = SDL_GetKeyboardState(NULL);

    if (0 < SDL_PeepEvents(0, 0, SDL_PEEKEVENT, SDL_QUIT, SDL_QUIT))
    {
        return -1;
    }

    return 0;
}

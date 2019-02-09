/**
 * @file Utils.c
 * @ingroup Utils
 * @defgroup Utils
 * @author Michael Fitzmayer
 * @copyright "THE BEER-WARE LICENCE" (Revision 42)
 */

#include <SDL.h>
#include "Utils.h"

void ClearFlag(const Uint8 u8Bit, Uint16 *u16Flags)
{
    *u16Flags &= ~(1 << u8Bit);
}

SDL_bool IsFlagSet(const Uint8 u8Bit, Uint16 u16Flags)
{
    if ((u16Flags >> u8Bit) & 1)
    {
        return SDL_TRUE;
    }
    else
    {
        return SDL_FALSE;
    }
}

void SetFlag(const Uint8 u8Bit, Uint16 *u16Flags)
{
    *u16Flags |= 1 << u8Bit;
}

void ToggleFlag(const Uint8 u8Bit, Uint16 *u16Flags)
{
    *u16Flags ^= 1 << u8Bit;
}

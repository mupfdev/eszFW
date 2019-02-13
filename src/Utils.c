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

// The state word must be initialized to non-zero.
Uint32 Xorshift(Uint32 *pu32State)
{
    Uint32 u32Val = *pu32State;
    u32Val ^= u32Val << 13;
    u32Val ^= u32Val >> 17;
    u32Val ^= u32Val << 5;
    *pu32State = u32Val;

    return u32Val;
}

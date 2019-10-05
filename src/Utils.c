/**
 * @file      Utils.c
 * @ingroup   Utils
 * @defgroup  Utils Various utilities used across the framework
 * @author    Michael Fitzmayer
 * @copyright "THE BEER-WARE LICENCE" (Revision 42)
 */

#include <SDL.h>
#include "Utils.h"

void Utils_ClearFlag(const Uint8 u8Bit, Uint16* u16Flags)
{
    *u16Flags &= ~(1 << u8Bit);
}

SDL_bool Utils_IsFlagSet(const Uint8 u8Bit, Uint16 u16Flags)
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

void Utils_SetFlag(const Uint8 u8Bit, Uint16* u16Flags)
{
    *u16Flags |= 1 << u8Bit;
}

void Utils_ToggleFlag(const Uint8 u8Bit, Uint16* u16Flags)
{
    *u16Flags ^= 1 << u8Bit;
}

double Utils_Round(double dValue)
{
    double dDecimalPlace = dValue - SDL_floor(dValue);
    if (dDecimalPlace >= 0.5)
    {
        return SDL_ceil(dValue);
    }
    else
    {
        return SDL_floor(dValue);
    }
}

// The state word must be initialized to non-zero.
Uint32 Utils_Xorshift(Uint32* pu32State)
{
    Uint32 u32Val = *pu32State;
    u32Val ^= u32Val << 13;
    u32Val ^= u32Val >> 17;
    u32Val ^= u32Val << 5;
    *pu32State = u32Val;

    return u32Val;
}

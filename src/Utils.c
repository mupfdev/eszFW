// SPDX-License-Identifier: Beerware
/**
 * @file      Utils.c
 * @ingroup   Utils
 * @defgroup  Utils Various utilities used across the framework
 * @author    Michael Fitzmayer
 * @copyright "THE BEER-WARE LICENCE" (Revision 42)
 */

#include <SDL.h>
#include "Utils.h"

/**
 * @brief   Clear flag
 * @details Clears specific flag in bit/flag field
 * @param   u8Bit
 *          Flag/bit to clear
 * @param   pu16Flags
 *          Pointer to flag/bit field
 */
void Utils_ClearFlag(const Uint8 u8Bit, Uint16* pu16Flags)
{
    *pu16Flags &= ~(1 << u8Bit);
}

/**
 * @brief   Check if flag is set
 * @details Checks whether a specific flag is set or not
 * @param   u8Bit
 *          Flag/bit to check
 * @param   u16Flags
 *          Flag/bit field
 * @return  Current state of the flag
 * @retval  SDL_TRUE:  Flag is set
 * @retval  SDL_FALSE: Flag is not set
 */
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

/**
 * @brief   Set flag
 * @details Sets specific flag in bit/flag field
 * @param   u8Bit
 *          Flag/bit to set
 * @param   pu16Flags
 *          Pointer to flag/bit field
 */
void Utils_SetFlag(const Uint8 u8Bit, Uint16* pu16Flags)
{
    *pu16Flags |= 1 << u8Bit;
}

/**
 * @brief   Toggle flag
 * @details Toggles specific flag in bit/flag field
 * @param   u8Bit
 *          Flag/bit to toggle
 * @param   pu16Flags
 *          Pointer to flag/bit field
 */
void Utils_ToggleFlag(const Uint8 u8Bit, Uint16* pu16Flags)
{
    *pu16Flags ^= 1 << u8Bit;
}

/**
 * @brief   Round to integral value
 * @details Round to integral value, regardless of rounding direction
 * @param   dValue
 *          The value to round
 * @return  Rounded integral value
 */
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

/**
 * @brief    Pseudo-random number generator
 * @details  Pseudo-random number generator based on linear-feedback
 *           shift registers
 * @param    pu32State
 *           Current state/seed
 * @return   32-bit pseudo-random number
 * @remark   The state word must be initialized to non-zero.
 */
Uint32 Utils_Xorshift(Uint32* pu32State)
{
    Uint32 u32Val = *pu32State;
    u32Val ^= u32Val << 13;
    u32Val ^= u32Val >> 17;
    u32Val ^= u32Val << 5;
    *pu32State = u32Val;

    return u32Val;
}

/**
 * @file Utils.c
 * @ingroup Utils
 * @defgroup Utils
 * @author Michael Fitzmayer
 * @copyright "THE BEER-WARE LICENCE" (Revision 42)
 */

#include <stdbool.h>
#include <stdint.h>
#include "Utils.h"

void ClearFlag(const uint8_t u8Bit, uint16_t *u16Flags)
{
    *u16Flags &= ~(1 << u8Bit);
}

bool IsFlagSet(const uint8_t u8Bit, uint16_t u16Flags)
{
    if ((u16Flags >> u8Bit) & 1)
    {
        return true;
    }
    else
    {
        return false;
    }
}

void SetFlag(const uint8_t u8Bit, uint16_t *u16Flags)
{
    *u16Flags |= 1 << u8Bit;
}

void ToggleFlag(const uint8_t u8Bit, uint16_t *u16Flags)
{
    *u16Flags ^= 1 << u8Bit;
}

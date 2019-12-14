// SPDX-License-Identifier: Beerware
/**
 * @file      Utils.h
 * @ingroup   Utils
 * @author    Michael Fitzmayer
 * @copyright "THE BEER-WARE LICENCE" (Revision 42)
 */
#pragma once

#include <SDL.h>

/**
 * @def    RETURN_ON_ERROR
 * @brief  Macro used for error checking
 */
#define RETURN_ON_ERROR(value) if (-1 == value) { return value; }

void     Utils_ClearFlag(const Uint8 u8Bit, Uint16* pu16Flags);
SDL_bool Utils_IsFlagSet(const Uint8 u8Bit, Uint16 u16Flags);
void     Utils_SetFlag(const Uint8 u8Bit, Uint16* pu16Flags);
void     Utils_ToggleFlag(const Uint8 u8Bit, Uint16* pu16Flags);
double   Utils_Round(double dValue);
Uint32   Utils_Xorshift(Uint32* pu32State);

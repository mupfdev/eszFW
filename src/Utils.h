/**
 * @file      Utils.h
 * @ingroup   Utils
 * @author    Michael Fitzmayer
 * @copyright "THE BEER-WARE LICENCE" (Revision 42)
 */
#pragma once

#include <SDL.h>

#define RETURN_ON_ERROR(value) if (-1 == value) { return value; }

void     ClearFlag(const Uint8 u8Bit, Uint16* u16Flags);
SDL_bool IsFlagSet(const Uint8 u8Bit, Uint16 u16Flags);
void     SetFlag(const Uint8 u8Bit, Uint16* u16Flags);
void     ToggleFlag(const Uint8 u8Bit, Uint16* u16Flags);
double   Round(double dValue);
Uint32   Xorshift(Uint32* pu32State);

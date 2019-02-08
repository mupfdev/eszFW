/**
 * @file      Utils.h
 * @ingroup   Utils
 * @defgroup  Utils
 * @author    Michael Fitzmayer
 * @copyright "THE BEER-WARE LICENCE" (Revision 42)
 */

#ifndef _UTILS_H_
#define _UTILS_H_

#include <stdbool.h>

#define RETURN_ON_ERROR(value) if (-1 == value) { return value; }

void ClearFlag(const uint8_t u8Bit, uint16_t *u16Flags);
bool IsFlagSet(const uint8_t u8Bit, uint16_t u16Flags);
void SetFlag(const uint8_t u8Bit, uint16_t *u16Flags);
void ToggleFlag(const uint8_t u8Bit, uint16_t *u16Flags);

#endif // _UTILS_H_

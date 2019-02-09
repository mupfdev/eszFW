/**
 * @file      Constants.h
 * @ingroup   Constants
 * @defgroup  Constants
 * @author    Michael Fitzmayer
 * @copyright "THE BEER-WARE LICENCE" (Revision 42)
 */

#ifndef _CONSTANTS_H_
#define _CONSTANTS_H_

typedef enum Alignment_t
{
    BOTTOM = 0,
    TOP    = 1
} Alignment;

typedef enum Direction_t
{
    RIGHT  = 0,
    LEFT   = 1
} Direction;

typedef enum TimeConstant_t
{
    APPROX_TIME_PER_FRAME = 17,
    TIME_FACTOR           = 1000
} TimeConstant;

#endif // _CONSTANTS_H_

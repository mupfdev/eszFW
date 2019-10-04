/**
 * @file      Constants.h
 * @ingroup   Constants
 * @defgroup  Constants Constants used across the framework
 * @author    Michael Fitzmayer
 * @copyright "THE BEER-WARE LICENCE" (Revision 42)
 */
#pragma once

/**
 * @typedef Alignment
 * @brief   Alignment constants type
 * @enum    Alignment_t
 * @brief   Alignment constants enumeration
 */
typedef enum Alignment_t
{
    BOTTOM = 0,
    TOP    = 1

} Alignment;

/**
 * @typedef Direction
 * @brief   Directional constants type
 * @enum    Direction_t
 * @brief   Directional constants enumeration
 */
typedef enum Direction_t
{
    RIGHT = 0,
    LEFT  = 1

} Direction;

/**
 * @typedef TimeConstant
 * @brief   Time constants type
 * @enum    TimeConstant_t
 * @brief   Time constants enumeration
 */
typedef enum TimeConstant_t
{
    APPROX_TIME_PER_FRAME = 17,
    TIME_FACTOR           = 1000

} TimeConstant;

/**
 * @def   DELTA_TIME
 * @brief Delta time calculation formula
 */
#define DELTA_TIME (double)APPROX_TIME_PER_FRAME / (double)TIME_FACTOR

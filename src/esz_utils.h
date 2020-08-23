// SPDX-License-Identifier: MIT
/**
 * @file    esz_utils.h
 * @brief   eszFW utilities
 * @details Various helper functions and utilities.
 * @remark  For internal use only!
 */

#ifndef ESZ_UTILS_H
#define ESZ_UTILS_H

#include <stdbool.h>
#include <stdint.h>

#include "esz_compat.h"
#include "esz_types.h"

bool    get_boolean_property(const uint64_t name_hash, esz_tiled_property_t* properties, int32_t property_count, esz_core_t* core);
double  get_decimal_property(const uint64_t name_hash, esz_tiled_property_t* properties, int32_t property_count, esz_core_t* core);
int32_t get_integer_property(const uint64_t name_hash, esz_tiled_property_t* tiled_properties, int32_t property_count, esz_core_t* core);

#ifdef ESZ_UTILS_IMPLEMENTATION

bool get_boolean_property(const uint64_t name_hash, esz_tiled_property_t* properties, int32_t property_count, esz_core_t* core)
{
    core->map->boolean_property = false;
    load_property(name_hash, properties, property_count, core);
    return core->map->boolean_property;
}

double get_decimal_property(const uint64_t name_hash, esz_tiled_property_t* properties, int32_t property_count, esz_core_t* core)
{
    core->map->decimal_property = 0.0;
    load_property(name_hash, properties, property_count, core);
    return core->map->decimal_property;
}

int32_t get_integer_property(const uint64_t name_hash, esz_tiled_property_t* tiled_properties, int32_t property_count, esz_core_t* core)
{
    core->map->integer_property = 0;
    load_property(name_hash, tiled_properties, property_count, core);
    return core->map->integer_property;
}

#endif // ESZ_UTILS_IMPLEMENTATION
#endif // ESZ_UTILS_H

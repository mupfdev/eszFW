// SPDX-License-Identifier: MIT
/**
 * @file    esz_utils.h
 * @brief   eszFW utilities
 * @details Various helper functions and utilities
 */

#ifndef ESZ_UTILS_H
#define ESZ_UTILS_H

#include <stdbool.h>
#include <stdint.h>

#include "esz_types.h"

bool        get_boolean_property(const uint64_t name_hash, esz_tiled_property_t* properties, int32_t property_count, esz_core_t* core);
double      get_decimal_property(const uint64_t name_hash, esz_tiled_property_t* properties, int32_t property_count, esz_core_t* core);
int32_t     get_integer_property(const uint64_t name_hash, esz_tiled_property_t* tiled_properties, int32_t property_count, esz_core_t* core);
const char* get_string_property(const uint64_t name_hash, esz_tiled_property_t* tiled_properties, int32_t property_count, esz_core_t* core);
bool        is_camera_at_horizontal_boundary(esz_core_t* core);
void        move_camera_to_target(esz_window_t* window, esz_core_t* core);
void        poll_events(esz_window_t* window, esz_core_t* core);
void        set_camera_boundaries_to_map_size(esz_window_t* window, esz_core_t* core);
void        update_bounding_box(esz_entity_t* entity);
void        update_entities(esz_window_t* window, esz_core_t* core);

#endif // ESZ_UTILS_H

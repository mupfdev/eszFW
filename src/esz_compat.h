// SPDX-License-Identifier: MIT
/**
 * @file    esz_compat.h
 * @brief   eszFW compatibility layer
 * @details Functions used to maintain compatibility between libTMX and
 *          cute_tiled.
 */

#ifndef ESZ_COMPAT_H
#define ESZ_COMPAT_H

#include <stdbool.h>
#include <stdint.h>

#include "esz_types.h"

int32_t              get_first_gid(esz_tiled_map_t* tiled_map);
esz_tiled_layer_t*   get_head_layer(esz_tiled_map_t* tiled_map);
esz_tiled_object_t*  get_head_object(esz_tiled_layer_t* tiled_layer, esz_core_t* core);
esz_tiled_tileset_t* get_head_tileset(esz_tiled_map_t* tiled_map);
int32_t*             get_layer_content(esz_tiled_layer_t* tiled_layer);
const char*          get_layer_name(esz_tiled_layer_t* tiled_layer);
int32_t              get_layer_property_count(esz_tiled_layer_t* tiled_layer);
int32_t              get_local_id(int32_t gid, esz_tiled_map_t* tiled_map);
int32_t              get_map_property_count(esz_tiled_map_t* tiled_map);
int32_t              get_next_animated_tile_id(int32_t gid, int32_t current_frame, esz_tiled_map_t* tiled_map);
const char*          get_object_name(esz_tiled_object_t* tiled_object);
int32_t              get_object_property_count(esz_tiled_object_t* tiled_object);
const char*          get_object_type_name(esz_tiled_object_t* tiled_object);
int32_t              get_tile_height(esz_tiled_map_t* tiled_map);
void                 get_tile_position(int32_t gid, int32_t* pos_x, int32_t* pos_y, esz_tiled_map_t* tiled_map);
int32_t              get_tile_width(esz_tiled_map_t* tiled_map);
bool                 is_gid_valid(int32_t gid, esz_tiled_map_t* tiled_map);
bool                 is_tile_animated(int32_t gid, int32_t* animation_length, int32_t* id, esz_tiled_map_t* tiled_map);
bool                 is_tiled_layer_of_type(const esz_tiled_layer_type tiled_type, esz_tiled_layer_t* tiled_layer, esz_core_t* core);
void                 load_property(const uint64_t name_hash, esz_tiled_property_t* properties, int32_t property_count, esz_core_t* core);
esz_status           load_tiled_map(const char* map_file_name, esz_core_t* core);
int32_t              remove_gid_flip_bits(int32_t gid);
bool                 tile_has_properties(int32_t gid, esz_tiled_tile_t** tile, esz_tiled_map_t* tiled_map);
void                 unload_tiled_map(esz_core_t* core);

#endif // ESZ_COMPAT_H

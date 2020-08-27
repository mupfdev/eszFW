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

#include "esz_compat.h"
#include "esz_types.h"

bool    get_boolean_property(const uint64_t name_hash, esz_tiled_property_t* properties, int32_t property_count, esz_core_t* core);
double  get_decimal_property(const uint64_t name_hash, esz_tiled_property_t* properties, int32_t property_count, esz_core_t* core);
int32_t get_integer_property(const uint64_t name_hash, esz_tiled_property_t* tiled_properties, int32_t property_count, esz_core_t* core);
bool    is_camera_at_horizontal_boundary(esz_core_t* core);
void    move_camera_to_target(esz_window_t* window, esz_core_t* core);
void    set_camera_boundaries_to_map_size(esz_window_t* window, esz_core_t* core);

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

bool is_camera_at_horizontal_boundary(esz_core_t* core)
{
    return core->camera.is_at_horizontal_boundary;
}

void move_camera_to_target(esz_window_t* window, esz_core_t* core)
{
    if (esz_is_camera_locked(core))
    {
        if (core->map->entity)
        {
            if (core->map->entity[core->camera.target_actor_id].actor)
            {
                esz_entity_t* target = &core->map->entity[core->camera.target_actor_id];

                core->camera.pos_x = target->pos_x;
                core->camera.pos_x -= (double)window->logical_width / 2.0;
                core->camera.pos_y = target->pos_y;
                core->camera.pos_y -= (double)window->logical_height / 2.0;
            }

            if (0 > core->camera.pos_x)
            {
                core->camera.pos_x = 0;
            }

            set_camera_boundaries_to_map_size(window, core);
        }
    }
}

void set_camera_boundaries_to_map_size(esz_window_t* window, esz_core_t* core)
{
    core->camera.is_at_horizontal_boundary = false;
    core->camera.max_pos_x                 = (int32_t)core->map->width  - window->logical_width;
    core->camera.max_pos_y                 = (int32_t)core->map->height - window->logical_height;

    if (0 >= core->camera.pos_x)
    {
        core->camera.pos_x                     = 0;
        core->camera.is_at_horizontal_boundary = true;
    }

    if (0 >= core->camera.pos_y)
    {
        core->camera.pos_y = 0;
    }

    if (core->camera.pos_x >= core->camera.max_pos_x)
    {
        core->camera.pos_x                     = core->camera.max_pos_x;
        core->camera.is_at_horizontal_boundary = true;
    }

    if (core->camera.pos_y >= core->camera.max_pos_y)
    {
        core->camera.pos_y = core->camera.max_pos_y;
    }
}

#endif // ESZ_UTILS_IMPLEMENTATION
#endif // ESZ_UTILS_H

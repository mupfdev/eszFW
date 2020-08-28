// SPDX-License-Identifier: MIT
/**
 * @file    esz_utils.c
 * @brief   eszFW utilities
 */

#include <stdbool.h>
#include <stdint.h>
#include <SDL.h>

#include "esz_compat.h"
#include "esz_hash.h"
#include "esz_macros.h"
#include "esz_types.h"
#include "esz_utils.h"

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

const char* get_string_property(const uint64_t name_hash, esz_tiled_property_t* tiled_properties, int32_t property_count, esz_core_t* core)
{
    core->map->string_property = NULL;
    load_property(name_hash, tiled_properties, property_count, core);
    return core->map->string_property;
}

bool is_camera_at_horizontal_boundary(esz_core_t* core)
{
    return core->camera.is_at_horizontal_boundary;
}

void move_camera_to_target(esz_window_t* window, esz_core_t* core)
{
    if (core->camera.is_locked)
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

void poll_events(esz_window_t* window, esz_core_t* core)
{
    while (0 != SDL_PollEvent(&core->event.handle))
    {
        switch (core->event.handle.type)
        {
            case SDL_QUIT:
                core->is_active = false;
                return;
            case SDL_FINGERDOWN:
                if (core->event.finger_down_cb)
                {
                    core->event.finger_down_cb(window, core);
                }
                break;
            case SDL_FINGERUP:
                if (core->event.finger_up_cb)
                {
                    core->event.finger_up_cb(window, core);
                }
                break;
            case SDL_FINGERMOTION:
                if (core->event.finger_motion_cb)
                {
                    core->event.finger_motion_cb(window, core);
                }
                break;
            case SDL_KEYDOWN:
                if (core->event.key_down_cb)
                {
                    core->event.key_down_cb(window, core);
                }
                break;
            case SDL_KEYUP:
                if (core->event.key_up_cb)
                {
                    core->event.key_up_cb(window, core);
                }
                break;
            case SDL_MULTIGESTURE:
                if (core->event.multi_gesture_cb)
                {
                    core->event.multi_gesture_cb(window, core);
                }
                break;
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

void update_bounding_box(esz_entity_t* entity)
{
    entity->bounding_box.top    = entity->pos_y - (double)(entity->height / 2.0);
    entity->bounding_box.bottom = entity->pos_y + (double)(entity->height / 2.0);
    entity->bounding_box.left   = entity->pos_x - (double)(entity->width  / 2.0);
    entity->bounding_box.right  = entity->pos_x + (double)(entity->width  / 2.0);

    if (0 >= entity->bounding_box.left)
    {
        entity->bounding_box.left = 0.0;
    }

    if (0 >= entity->bounding_box.top)
    {
        entity->bounding_box.top = 0.0;
    }
}

void update_entities(esz_window_t* window, esz_core_t* core)
{
    esz_tiled_layer_t* layer;
    int32_t            index = 0;

    if (! core->is_map_loaded)
    {
        return;
    }

    layer = get_head_layer(core->map->handle);

    while (layer)
    {
        if (is_tiled_layer_of_type(ESZ_OBJECT_GROUP, layer, core))
        {
            esz_tiled_object_t* tiled_object = get_head_object(layer, core);
            while (tiled_object)
            {
                uint64_t      type_hash = generate_hash((const unsigned char*)get_object_type_name(tiled_object));
                esz_entity_t* entity    = &core->map->entity[index];

                switch (type_hash)
                {
                    case H_actor:
                    {
                        esz_actor_t** actor                 = &entity->actor;
                        uint32_t*     state                 = &(*actor)->state;
                        double        acceleration_x        = (*actor)->acceleration    * core->map->meter_in_pixel;
                        double        acceleration_y        = core->map->meter_in_pixel * core->map->meter_in_pixel;
                        double        time_since_last_frame = window->time_since_last_frame;
                        double        distance_x            = acceleration_x * time_since_last_frame * time_since_last_frame;
                        double        distance_y            = acceleration_y * time_since_last_frame * time_since_last_frame;

                        // Vertical movement and gravity
                        // ----------------------------------------------------

                        if (IS_STATE_SET(*state, STATE_GRAVITATIONAL))
                        {
                            CLR_STATE((*actor)->state, STATE_FLOATING);

                            if (0 > (*actor)->velocity_y)
                            {
                                SET_STATE((*actor)->state, STATE_RISING);
                            }
                            else
                            {
                                CLR_STATE((*actor)->state, STATE_RISING);
                            }

                            if (IS_STATE_SET((*actor)->state, STATE_RISING))
                            {
                                SET_STATE((*actor)->state, STATE_IN_MID_AIR);
                            }

                            // tbd. check ground collision here
                        }
                        else
                        {
                            SET_STATE((*actor)->state, STATE_FLOATING);
                            CLR_STATE((*actor)->state, STATE_IN_MID_AIR);
                            CLR_STATE((*actor)->state, STATE_JUMPING);
                            CLR_STATE((*actor)->state, STATE_RISING);
                        }

                        if (0 < core->map->gravitation)
                        {
                            if (IS_STATE_SET((*actor)->state, STATE_IN_MID_AIR))
                            {
                                (*actor)->velocity_y += distance_y;
                                entity->pos_y += (*actor)->velocity_y;
                            }
                            else
                            {
                                int32_t tile_height = get_tile_height(core->map->handle);

                                CLR_STATE((*actor)->action, ACTION_JUMP);
                                (*actor)->velocity_y = 0.0;
                                // Correct actor position along the y-axis:
                                entity->pos_y = ((double)tile_height * round(entity->pos_y / (double)tile_height));
                            }
                        }
                        else
                        {
                            if (IS_STATE_SET((*actor)->state, STATE_MOVING))
                            {
                                (*actor)->velocity_y += distance_y;
                            }
                            else
                            {
                                (*actor)->velocity_y -= distance_y;
                            }

                            if (0.0 < (*actor)->velocity_y)
                            {
                                if (IS_STATE_SET((*actor)->state, STATE_GOING_UP))
                                {
                                    entity->pos_y -= (*actor)->velocity_y;
                                }
                                else if (IS_STATE_SET((*actor)->state, STATE_GOING_DOWN))
                                {
                                    entity->pos_y += (*actor)->velocity_y;
                                }
                            }

                            /* Since the velocity in free fall is
                             * normally not limited, the maximum
                             * horizontal velocity is used in this case.
                             */
                            if ((*actor)->max_velocity_x <= (*actor)->velocity_y)
                            {
                                (*actor)->velocity_y = (*actor)->max_velocity_x;
                            }
                            else if (0.0 > (*actor)->velocity_x)
                            {
                                (*actor)->velocity_y = 0.0;
                            }
                        }

                        // Horizontal movement
                        // ----------------------------------------------------

                        if (IS_STATE_SET((*actor)->state, STATE_MOVING))
                        {
                            (*actor)->velocity_x += distance_x;
                        }
                        else
                        {
                            // tbd. friction
                            (*actor)->velocity_x -= distance_x * 2.0;
                        }

                        if (0.0 < (*actor)->velocity_x)
                        {
                            if (IS_STATE_SET((*actor)->state, STATE_GOING_LEFT))
                            {
                                entity->pos_x -= (*actor)->velocity_x;
                            }
                            else if (IS_STATE_SET((*actor)->state, STATE_GOING_RIGHT))
                            {
                                entity->pos_x += (*actor)->velocity_x;
                            }
                        }

                        if ((*actor)->max_velocity_x <= (*actor)->velocity_x)
                        {
                            (*actor)->velocity_x = (*actor)->max_velocity_x;
                        }
                        else if (0.0 > (*actor)->velocity_x)
                        {
                            (*actor)->velocity_x = 0.0;
                        }

                        // Connect map ends
                        // ----------------------------------------------------

                        if ((*actor)->connect_horizontal_map_ends)
                        {
                            if (0.0 - entity->width > entity->pos_x)
                            {
                                entity->pos_x = core->map->width + entity->width;
                            }
                            else if (core->map->width + entity->width < entity->pos_x)
                            {
                                entity->pos_x = 0.0 - entity->width;
                            }
                        }
                        else
                        {
                            if ((double)(entity->width / 4) > entity->pos_x)
                            {
                                entity->pos_x = (double)(entity->width / 4);
                            }
                            // tbd.
                        }

                        if ((*actor)->connect_vertical_map_ends)
                        {
                            if (0.0 - entity->height > entity->pos_y)
                            {
                                entity->pos_y = core->map->height + entity->width;
                            }
                            else if (core->map->height + entity->height < entity->pos_y)
                            {
                                entity->pos_y = 0.0 - entity->height;
                            }
                        }
                        else
                        {
                            // tbd.
                        }

                       break;
                    }
                }

                // Update axis-aligned bounding box
                // ------------------------------------------------------------

                update_bounding_box(entity);

                index        += 1;
                tiled_object  = tiled_object->next;
            }
        }
        layer = layer->next;
    }
}

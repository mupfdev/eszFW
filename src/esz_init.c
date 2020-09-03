// SPDX-License-Identifier: MIT
/**
 * @file  esz_init.c
 * @brief eszFW initialisation routines
 */

#include <picolog.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <SDL.h>

#include "esz_macros.h"

DISABLE_WARNING_PUSH
DISABLE_WARNING_PADDING
DISABLE_WARNING_SPECTRE_MITIGATION

#ifndef STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#endif
#include <stb_image.h>
#include <stb_sprintf.h>

DISABLE_WARNING_SYMBOL_NOT_DEFINED

#include <SDL.h>
#include <cwalk.h>

DISABLE_WARNING_POP

#include "esz_compat.h"
#include "esz_hash.h"
#include "esz_init.h"
#include "esz_types.h"
#include "esz_utils.h"

static esz_status load_background_layer(int32_t index, esz_window_t* window, esz_core_t* core);

esz_status load_animated_tiles(esz_core_t* core)
{
    esz_tiled_layer_t* layer               = get_head_layer(core->map->handle);
    int32_t            animated_tile_count = 0;

    while (layer)
    {
        if (is_tiled_layer_of_type(ESZ_TILE_LAYER, layer, core) && layer->visible)
        {
            for (int32_t index_height = 0; index_height < (int32_t)core->map->handle->height; index_height += 1)
            {
                for (int32_t index_width = 0; index_width < (int32_t)core->map->handle->width; index_width += 1)
                {
                    int32_t* layer_content = get_layer_content(layer);
                    int32_t  gid           = remove_gid_flip_bits((int32_t)layer_content[(index_height * (int32_t)core->map->handle->width) + index_width]);

                    if (is_tile_animated(gid, NULL, NULL, core->map->handle))
                    {
                        animated_tile_count += 1;
                    }
                }
            }
        }
        layer = layer->next;
    }

    if (0 >= animated_tile_count)
    {
        return ESZ_OK;
    }
    else
    {
        core->map->animated_tile = (esz_animated_tile_t*)calloc((size_t)animated_tile_count, sizeof(struct esz_animated_tile));
        if (!core->map->animated_tile)
        {
            plog_error("%s: error allocating memory.", __func__);
            return ESZ_ERROR_CRITICAL;
        }
    }

    plog_info("Load %u animated tile(s).", animated_tile_count);
    return ESZ_OK;
}

esz_status load_background(esz_window_t* window, esz_core_t* core)
{
    char    property_name[21] = { 0 };
    bool    search_is_running = true;
    int32_t prop_cnt          = get_map_property_count(core->map->handle);

    core->map->background.layer_shift = get_decimal_property(H_background_layer_shift, core->map->handle->properties, prop_cnt, core);
    core->map->background.velocity    = get_decimal_property(H_background_constant_velocity, core->map->handle->properties, prop_cnt, core);

    if (0.0 < core->map->background.velocity)
    {
        core->map->background.velocity_is_constant = true;
    }

    if (get_boolean_property(H_background_is_top_aligned, core->map->handle->properties, prop_cnt, core))
    {
        core->map->background.alignment = ESZ_TOP;
    }
    else
    {
        core->map->background.alignment = ESZ_BOT;
    }

    core->map->background.layer_count = 0;
    while (search_is_running)
    {
        stbsp_snprintf(property_name, (size_t)21, "background_layer_%u", core->map->background.layer_count + 1);

        if (get_string_property(generate_hash((const unsigned char*)property_name), core->map->handle->properties, prop_cnt, core))
        {
            core->map->background.layer_count += 1;
        }
        else
        {
            search_is_running = false;
            break;
        }
    }

    if (0 == core->map->background.layer_count)
    {
        return ESZ_OK;
    }

    core->map->background.layer = (esz_background_layer_t*)calloc((size_t)core->map->background.layer_count, sizeof(struct esz_background_layer));
    if (! core->map->background.layer)
    {
        plog_error("%s: error allocating memory.", __func__);
        return ESZ_ERROR_CRITICAL;
    }

    if (0 < core->map->background.layer_count)
    {
        for (int32_t index = 0; index < core->map->background.layer_count; index += 1)
        {
            load_background_layer(index, window, core);
        }
    }
    else
    {
        return ESZ_OK;
    }

    plog_info("Load parallax-scrolling background with %u layers.", core->map->background.layer_count);

    return ESZ_OK;
}

esz_status load_entities(esz_core_t* core)
{
    esz_tiled_layer_t*  layer         = get_head_layer(core->map->handle);
    esz_tiled_object_t* tiled_object  = NULL;
    bool                player_found  = false;

    if (core->map->entity_count)
    {
        return ESZ_OK;
    }

    while (layer)
    {
        if (is_tiled_layer_of_type(ESZ_OBJECT_GROUP, layer, core))
        {
            tiled_object = get_head_object(layer, core);
            while (tiled_object)
            {
                core->map->entity_count += 1;
                tiled_object             = tiled_object->next;
            }
        }
        layer = layer->next;
    }

    if (core->map->entity_count)
    {
        core->map->entity = (esz_entity_t*)calloc((size_t)core->map->entity_count, sizeof(struct esz_entity));
        if (! core->map->entity)
        {
            plog_error("%s: error allocating memory.", __func__);
            return ESZ_ERROR_CRITICAL;
        }
    }

    plog_info("Load %u entities:", core->map->entity_count);

    layer = get_head_layer(core->map->handle);
    while (layer)
    {
        if (is_tiled_layer_of_type(ESZ_OBJECT_GROUP, layer, core))
        {
            int32_t index = 0;
            tiled_object  = get_head_object(layer, core);
            while (tiled_object)
            {
                uint64_t              type_hash  = generate_hash((const unsigned char*)get_object_type_name(tiled_object));
                esz_entity_t*         entity     = &core->map->entity[index];
                esz_tiled_property_t* properties = tiled_object->properties;
                int32_t               prop_cnt   = get_object_property_count(tiled_object);

                entity->pos_x = (double)tiled_object->x;
                entity->pos_y = (double)tiled_object->y;

                switch (type_hash)
                {
                    case H_actor:
                    {
                        esz_actor_t** actor = &entity->actor;

                        (*actor) = (esz_actor_t*)calloc(1, sizeof(struct esz_actor));
                        if (! (*actor))
                        {
                            plog_error("%s: error allocating memory for actor.", __func__);
                            return ESZ_ERROR_CRITICAL;
                        }

                        (*actor)->current_animation           = 1;

                        (*actor)->acceleration                = get_decimal_property(H_acceleration, properties, prop_cnt, core);
                        (*actor)->jumping_power               = get_decimal_property(H_jumping_power, properties, prop_cnt, core);
                        (*actor)->max_velocity_x              = get_decimal_property(H_max_velocity_x, properties, prop_cnt, core);

                        (*actor)->sprite_sheet_id             = get_integer_property(H_sprite_sheet_id, properties, prop_cnt, core);

                        (*actor)->connect_horizontal_map_ends = get_boolean_property(H_connect_horizontal_map_ends, properties, prop_cnt, core);
                        (*actor)->connect_vertical_map_ends   = get_boolean_property(H_connect_vertical_map_ends, properties, prop_cnt, core);

                        (*actor)->spawn_pos_x                 = core->map->entity[index].pos_x;
                        (*actor)->spawn_pos_y                 = core->map->entity[index].pos_y;

                        if (get_boolean_property(H_is_affected_by_gravity, properties, prop_cnt, core))
                        {
                            SET_STATE((*actor)->state, STATE_GRAVITATIONAL);
                        }
                        else
                        {
                            SET_STATE((*actor)->state, STATE_FLOATING);
                        }

                        if (get_boolean_property(H_is_animated, properties, prop_cnt, core))
                        {
                            SET_STATE((*actor)->state, STATE_ANIMATED);
                        }

                        if (get_boolean_property(H_is_in_midground, properties, prop_cnt, core))
                        {
                            SET_STATE((*actor)->state, STATE_IN_MIDGROUND);
                        }
                        else if (get_boolean_property(H_is_in_background, properties, prop_cnt, core))
                        {
                            SET_STATE((*actor)->state, STATE_IN_BACKGROUND);
                        }
                        else
                        {
                            SET_STATE((*actor)->state, STATE_IN_FOREGROUND);
                        }

                        if (get_boolean_property(H_is_left_oriented, properties, prop_cnt, core))
                        {
                            SET_STATE((*actor)->state, STATE_GOING_LEFT);
                            SET_STATE((*actor)->state, STATE_LOOKING_LEFT);
                        }
                        else
                        {
                            SET_STATE((*actor)->state, STATE_GOING_RIGHT);
                            SET_STATE((*actor)->state, STATE_LOOKING_RIGHT);
                        }

                        if (get_boolean_property(H_is_moving, properties, prop_cnt, core))
                        {
                            SET_STATE((*actor)->state, STATE_MOVING);
                        }

                        if (get_boolean_property(H_is_player, properties, prop_cnt, core) && ! player_found)
                        {
                            player_found                      = true;
                            core->camera.is_locked            = true;
                            core->map->active_player_actor_id = index;
                            core->camera.target_actor_id      = index;

                            plog_info("  %d %s *", index, get_object_name(tiled_object));
                        }
                        else
                        {
                            plog_info("  %d %s", index, get_object_name(tiled_object));
                        }

                        if (IS_STATE_SET((*actor)->state, STATE_ANIMATED))
                        {
                            char property_name[14] = { 0 };
                            bool search_is_running = true;

                            (*actor)->animation_count = 0;
                            while (search_is_running)
                            {
                                stbsp_snprintf(property_name, (size_t)14, "animation_%u", (*actor)->animation_count + 1);

                                if (get_boolean_property(generate_hash((const unsigned char*)property_name), properties, prop_cnt, core))
                                {
                                    (*actor)->animation_count += 1;
                                }
                                else
                                {
                                    search_is_running = false;
                                    break;
                                }
                            }
                        }

                        if (0 < (*actor)->animation_count)
                        {
                            char property_name[26] = { 0 };

                            (*actor)->animation = (esz_animation_t*)calloc((size_t)(*actor)->animation_count, sizeof(struct esz_animation));
                            if (! (*actor)->animation)
                            {
                                plog_error("%s: error allocating memory.", __func__);
                                return ESZ_ERROR_CRITICAL;
                            }

                            for (int32_t index = 0; index < (*actor)->animation_count; index += 1)
                            {
                                stbsp_snprintf(property_name, 26, "animation_%u_first_frame", index + 1);
                                (*actor)->animation[index].first_frame =
                                    get_integer_property(generate_hash((const unsigned char*)property_name), properties, prop_cnt, core);

                                if (0 == (*actor)->animation[index].first_frame)
                                {
                                    (*actor)->animation[index].first_frame = 1;
                                }

                                stbsp_snprintf(property_name, 26, "animation_%u_fps", index + 1);
                                (*actor)->animation[index].fps =
                                    get_integer_property(generate_hash((const unsigned char*)property_name), properties, prop_cnt, core);

                                stbsp_snprintf(property_name, 26, "animation_%u_length", index + 1);
                                (*actor)->animation[index].length =
                                    get_integer_property(generate_hash((const unsigned char*)property_name), properties, prop_cnt, core);

                                stbsp_snprintf(property_name, 26, "animation_%u_offset_y", index + 1);
                                (*actor)->animation[index].offset_y =
                                    get_integer_property(generate_hash((const unsigned char*)property_name), properties, prop_cnt, core);
                            }
                        }
                    }
                    break;
                }

                entity->width  = get_integer_property(H_width, properties, prop_cnt, core);
                entity->height = get_integer_property(H_height, properties, prop_cnt, core);

                if (0 >= entity->width)
                {
                    entity->width = get_tile_width(core->map->handle);
                }

                if (0 >= entity->height)
                {
                    entity->width = get_tile_height(core->map->handle);
                }

                update_bounding_box(entity);

                index        += 1;
                tiled_object  = tiled_object->next;
            }
        }
        layer = layer->next;
    }

    if (! player_found)
    {
        plog_warn("  No player actor found.");
    }

    return ESZ_OK;
}

esz_status load_map_path(const char* map_file_name, esz_core_t* core)
{
    core->map->path = (char*)calloc(1, (size_t)(strnlen(map_file_name, 64) + 1));
    if (! core->map->path)
    {
        plog_error("%s: error allocating memory.", __func__);
        return ESZ_ERROR_CRITICAL;
    }

    cwk_path_get_dirname(map_file_name, (size_t*)&(core->map->path_length));
    SDL_strlcpy(core->map->path, map_file_name, core->map->path_length + 1);

    return ESZ_OK;
}

esz_status load_sprites(esz_window_t* window, esz_core_t* core)
{
    char    property_name[17] = { 0 };
    bool    search_is_running = true;
    int32_t prop_cnt          = get_map_property_count(core->map->handle);

    core->map->sprite_sheet_count = 0;

    while (search_is_running)
    {
        stbsp_snprintf(property_name, 17, "sprite_sheet_%u", core->map->sprite_sheet_count + 1);

        if (get_string_property(generate_hash((const unsigned char*)property_name), core->map->handle->properties, prop_cnt, core))
        {
            core->map->sprite_sheet_count += 1;
        }
        else
        {
            search_is_running = false;
        }
    }

    if (0 == core->map->sprite_sheet_count)
    {
        return ESZ_OK;
    }

    core->map->sprite = (esz_sprite_t*)calloc((size_t)core->map->sprite_sheet_count, sizeof(struct esz_sprite));
    if (! core->map->sprite)
    {
        plog_error("%s: error allocating memory.", __func__);
        return ESZ_ERROR_CRITICAL;
    }

    for (int32_t index = 0; index < core->map->sprite_sheet_count; index += 1)
    {
        stbsp_snprintf(property_name, 17, "sprite_sheet_%u", index + 1);

        const char* file_name = get_string_property(generate_hash((const unsigned char*)property_name), core->map->handle->properties, prop_cnt, core);

        if (file_name)
        {
            int32_t source_length             = (int32_t)(strnlen(core->map->path, 64) + strnlen(file_name, 64) + 1);
            char*   sprite_sheet_image_source = (char*)calloc(1, source_length);
            if (! sprite_sheet_image_source)
            {
                plog_error("%s: error allocating memory.", __func__);
                return ESZ_ERROR_CRITICAL;
            }

            stbsp_snprintf(sprite_sheet_image_source, source_length, "%s%s", core->map->path, file_name);

            core->map->sprite[index].id = index + 1;

            if (ESZ_ERROR_CRITICAL == load_texture_from_file(sprite_sheet_image_source, &core->map->sprite[index].texture, window))
            {
                free(sprite_sheet_image_source);
                return ESZ_ERROR_CRITICAL;
            }

            free(sprite_sheet_image_source);
        }
    }

    return ESZ_OK;
}

esz_status load_tile_properties(esz_core_t* core)
{
    esz_tiled_layer_t* layer      = get_head_layer(core->map->handle);
    int32_t            tile_count = (int32_t)(core->map->handle->height * core->map->handle->width);

    core->map->tile_properties = (uint32_t*)calloc((size_t)tile_count, sizeof(uint32_t));
    if (! core->map->tile_properties)
    {
        plog_error("%s: error allocating memory.", __func__);
        return ESZ_WARNING;
    }

    while (layer)
    {
        if (is_tiled_layer_of_type(ESZ_TILE_LAYER, layer, core))
        {
            for (int32_t index_height = 0; index_height < (int32_t)core->map->handle->height; index_height += 1)
            {
                for (int32_t index_width = 0; index_width < (int32_t)core->map->handle->width; index_width += 1)
                {
                    esz_tiled_tileset_t* tileset = get_head_tileset(core->map->handle);
                    esz_tiled_tile_t*    tile = tileset->tiles;
                    int32_t*             layer_content = get_layer_content(layer);
                    int32_t              gid = remove_gid_flip_bits((int32_t)layer_content[(index_height * (int32_t)core->map->handle->width) + index_width]);
                    int32_t              tile_index = (index_width + 1) * (index_height + 1);

                    if (tile_has_properties(gid, &tile, core->map->handle))
                    {
                        // tbd.

                        #ifndef USE_LIBTMX
                        if (get_boolean_property(H_climbable, tile->properties, tile->property_count, core))
                        {
                            SET_STATE(core->map->tile_properties[tile_index], TILE_CLIMBABLE);
                        }

                        if (get_boolean_property(H_solid_above, tile->properties, tile->property_count, core))
                        {
                            SET_STATE(core->map->tile_properties[tile_index], TILE_SOLID_ABOVE);
                        }

                        if (get_boolean_property(H_solid_below, tile->properties, tile->property_count, core))
                        {
                            SET_STATE(core->map->tile_properties[tile_index], TILE_SOLID_BELOW);
                        }

                        if (get_boolean_property(H_solid_left, tile->properties, tile->property_count, core))
                        {
                            SET_STATE(core->map->tile_properties[tile_index], TILE_SOLID_LEFT);
                        }

                        if (get_boolean_property(H_solid_right, tile->properties, tile->property_count, core))
                        {
                            SET_STATE(core->map->tile_properties[tile_index], TILE_SOLID_RIGHT);
                        }
                        #endif
                    }
                }
            }
        }
        layer = layer->next;
    }

    return ESZ_OK;
}

esz_status load_tileset(esz_window_t* window, esz_core_t* core)
{
    esz_status status      = ESZ_OK;
    char*      image_path  = NULL;
    int32_t    path_length = get_tileset_path_length(core);

    image_path = calloc(1, path_length);
    if (! image_path)
    {
        plog_error("%s: error allocating memory.", __func__);
        return ESZ_ERROR_CRITICAL;
    }

    set_tileset_path(image_path, path_length, core);

    if (ESZ_OK != load_texture_from_file(image_path, &core->map->tileset_texture, window))
    {
        plog_error("%s: Error loading image '%s'.", __func__, image_path);
        status = ESZ_ERROR_CRITICAL;
    }

    free(image_path);
    return status;
}

/* Based on
 * https://wiki.libsdl.org/SDL_CreateRGBSurfaceWithFormatFrom#Code_Examples
 */
esz_status load_texture_from_file(const char* file_name, SDL_Texture** texture, esz_window_t* window)
{
    SDL_Surface*   surface;
    int            width;
    int            height;
    int            orig_format;
    int            req_format = STBI_rgb_alpha;
    int            depth;
    int            pitch;
    uint32_t       pixel_format;
    unsigned char* data;

    if (! file_name)
    {
        return ESZ_WARNING;
    }

    data = stbi_load(file_name, &width, &height, &orig_format, req_format);

    if (NULL == data)
    {
        plog_error("%s: %s.", __func__, stbi_failure_reason());
        return ESZ_ERROR_CRITICAL;
    }

    if (STBI_rgb == req_format)
    {
        depth        = 24;
        pitch        = 3 * width; // 3 bytes per pixel * pixels per row
        pixel_format = SDL_PIXELFORMAT_RGB24;
    }
    else
    {
        // STBI_rgb_alpha (RGBA)
        depth        = 32;
        pitch        = 4 * width;
        pixel_format = SDL_PIXELFORMAT_RGBA32;
    }

    surface = SDL_CreateRGBSurfaceWithFormatFrom((void*)data, width, height, depth, pitch, pixel_format);

    if (NULL == surface)
    {
        plog_error("%s: %s.", __func__, SDL_GetError());
        stbi_image_free(data);
        return ESZ_ERROR_CRITICAL;
    }

    *texture = SDL_CreateTextureFromSurface(window->renderer, surface);

    SDL_FreeSurface(surface);
    stbi_image_free(data);

    plog_info("Loading image from file: %s.", file_name);
    return ESZ_OK;
}

esz_status load_texture_from_memory(const unsigned char* buffer, const int length, SDL_Texture** texture, esz_window_t* window)
{
    SDL_Surface*   surface;
    int            width;
    int            height;
    int            orig_format;
    int            req_format = STBI_rgb_alpha;
    int            depth;
    int            pitch;
    uint32_t       pixel_format;
    unsigned char* data;

    if (! buffer)
    {
        return ESZ_WARNING;
    }

    data = stbi_load_from_memory(buffer, length, &width, &height, &orig_format, req_format);

    if (NULL == data)
    {
        plog_error("%s: %s.", __func__, stbi_failure_reason());
        return ESZ_ERROR_CRITICAL;
    }

    if (STBI_rgb == req_format)
    {
        depth        = 24;
        pitch        = 3 * width; // 3 bytes per pixel * pixels per row
        pixel_format = SDL_PIXELFORMAT_RGB24;
    }
    else
    {
        // STBI_rgb_alpha (RGBA)
        depth        = 32;
        pitch        = 4 * width;
        pixel_format = SDL_PIXELFORMAT_RGBA32;
    }

    surface = SDL_CreateRGBSurfaceWithFormatFrom((void*)data, width, height, depth, pitch, pixel_format);

    if (NULL == surface)
    {
        plog_error("%s: %s.", __func__, SDL_GetError());
        stbi_image_free(data);
        return ESZ_ERROR_CRITICAL;
    }

    *texture = SDL_CreateTextureFromSurface(window->renderer, surface);

    SDL_FreeSurface(surface);
    stbi_image_free(data);

    plog_info("Loading image from memory.");
    return ESZ_OK;
}

static esz_status load_background_layer(int32_t index, esz_window_t* window, esz_core_t* core)
{
    esz_status   status        = ESZ_OK;
    SDL_Texture* image_texture = NULL;
    SDL_Rect     dst;
    int32_t      image_width;
    int32_t      image_height;
    int32_t      prop_cnt          = get_map_property_count(core->map->handle);
    int32_t      source_length     = 0;
    double       layer_width_factor;
    char         property_name[21] = { 0 };
    char*        background_layer_image_source;

    stbsp_snprintf(property_name, 21, "background_layer_%u", index + 1);

    const char* file_name = get_string_property(generate_hash((const unsigned char*)property_name), core->map->handle->properties, prop_cnt, core);
    source_length = (int32_t)(strnlen(core->map->path, 64) + strnlen(file_name, 64) + 1);

    background_layer_image_source = (char*)calloc(1, source_length);
    if (! background_layer_image_source)
    {
        plog_error("%s: error allocating memory.", __func__);
        return ESZ_ERROR_CRITICAL;
    }

    stbsp_snprintf(background_layer_image_source, source_length, "%s%s", core->map->path, file_name);

    if (ESZ_ERROR_CRITICAL == load_texture_from_file(background_layer_image_source, &image_texture, window))
    {
        status = ESZ_ERROR_CRITICAL;
        goto exit;
    }

    free(background_layer_image_source);

    if (0 > SDL_QueryTexture(image_texture, NULL, NULL, &image_width, &image_height))
    {
        plog_error("%s: %s.", __func__, SDL_GetError());
        status = ESZ_ERROR_CRITICAL;
        goto exit;
    }

    layer_width_factor = ceil((double)window->width / (double)image_width);

    core->map->background.layer[index].width  = image_width * (int32_t)layer_width_factor;
    core->map->background.layer[index].height = image_height;

    core->map->background.layer[index].texture = SDL_CreateTexture(
        window->renderer,
        SDL_PIXELFORMAT_ARGB8888,
        SDL_TEXTUREACCESS_TARGET,
        core->map->background.layer[index].width,
        core->map->background.layer[index].height);

    if (! core->map->background.layer[index].texture)
    {
        plog_error("%s: %s.", __func__, SDL_GetError());
        status = ESZ_ERROR_CRITICAL;
        goto exit;
    }

    if (0 != SDL_SetRenderTarget(window->renderer, core->map->background.layer[index].texture))
    {
        plog_error("%s: %s.", __func__, SDL_GetError());
        status = ESZ_ERROR_CRITICAL;
        goto exit;
    }

    dst.x = 0;
    int32_t pass;
    for (pass = 0; pass < layer_width_factor; pass += 1)
    {
        dst.y = 0;
        dst.w = image_width;
        dst.h = image_height;

        if (0 > SDL_RenderCopy(window->renderer, image_texture, NULL, &dst))
        {
            plog_error("%s: %s.", __func__, SDL_GetError());
            status = ESZ_ERROR_CRITICAL;
            goto exit;
        }

        dst.x += image_width;
    }

    if (0 > SDL_SetTextureBlendMode(core->map->background.layer[index].texture, SDL_BLENDMODE_BLEND))
    {
        plog_error("%s: %s.", __func__, SDL_GetError());
        status = ESZ_ERROR_CRITICAL;
        goto exit;
    }

exit:
    if (image_texture)
    {
        SDL_DestroyTexture(image_texture);
    }

    plog_info("Load background layer %d.", index + 1);
    return status;
}

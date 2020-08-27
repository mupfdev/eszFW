// SPDX-License-Identifier: MIT
/**
 * @file    esz_render.h
 * @brief   eszFW rendering and scene drawing
 */

#ifndef ESZ_RENDER_H
#define ESZ_RENDER_H

#include <stdbool.h>
#include <stdint.h>

#include "esz_compat.h"
#include "esz_types.h"
#include "esz_utils.h"

esz_status create_and_set_render_target(SDL_Texture** target, esz_window_t* window);
esz_status draw_scene(esz_window_t* window, esz_core_t* core);
esz_status render_actors(int32_t level, esz_window_t* window, esz_core_t* core);
esz_status render_background(esz_window_t* window, esz_core_t* core);
esz_status render_background_layer(int32_t index, esz_window_t* window, esz_core_t* core);
esz_status render_map(int32_t level, esz_window_t* window, esz_core_t* core);
esz_status render_scene(esz_window_t* window, esz_core_t* core);

#ifdef ESZ_RENDER_IMPLEMENTATION

esz_status create_and_set_render_target(SDL_Texture** target, esz_window_t* window)
{
    if (! (*target))
    {
        (*target) = SDL_CreateTexture(
            window->renderer,
            SDL_PIXELFORMAT_ARGB8888,
            SDL_TEXTUREACCESS_TARGET,
            window->width,
            window->height);
    }

    if (! (*target))
    {
        plog_error("%s: %s.", __func__, SDL_GetError());
        return ESZ_ERROR_CRITICAL;
    }
    else
    {
        if (0 > SDL_SetTextureBlendMode((*target), SDL_BLENDMODE_BLEND))
        {
            plog_error("%s: %s.", __func__, SDL_GetError());
            SDL_DestroyTexture((*target));
            return ESZ_ERROR_CRITICAL;
        }
    }

    if (0 > SDL_SetRenderTarget(window->renderer, (*target)))
    {
        plog_error("%s: %s.", __func__, SDL_GetError());
        SDL_DestroyTexture((*target));
        return ESZ_ERROR_CRITICAL;
    }

    SDL_RenderClear(window->renderer);

    return ESZ_OK;
}

esz_status draw_scene(esz_window_t* window, esz_core_t* core)
{
    SDL_Rect dst;

    if (0 > SDL_SetRenderTarget(window->renderer, NULL))
    {
        plog_error("%s: %s.", __func__, SDL_GetError());
    }

    if (! esz_is_map_loaded(core))
    {
        // Display logo in the lower-right corder.
        dst.x = (window->logical_width)  - 53;
        dst.y = (window->logical_height) - 19;
        dst.w = 48;
        dst.h = 14;

        SDL_SetRenderDrawColor(window->renderer, 0xa9, 0x20, 0x3e, SDL_ALPHA_OPAQUE);

        if (0 > SDL_RenderCopy(window->renderer, window->esz_logo, NULL, &dst))
        {
            plog_error("%s: %s.", __func__, SDL_GetError());
            return ESZ_ERROR_CRITICAL;
        }

        SDL_RenderPresent(window->renderer);
        SDL_RenderClear(window->renderer);

        return ESZ_OK;
    }

    dst.x = 0;
    dst.y = 0;
    dst.w = window->width;
    dst.h = window->height;

    for (int32_t index = 0; index < ESZ_RENDER_LAYER_MAX; index += 1)
    {
        if (IS_STATE_SET(core->debug, index))
        {
            continue;
        }

        if (0 > SDL_RenderCopy(window->renderer, core->map->render_target[index], NULL, &dst))
        {
            plog_error("%s: %s.", __func__, SDL_GetError());
            return ESZ_ERROR_CRITICAL;
        }
    }

    SDL_RenderPresent(window->renderer);

    SDL_RenderClear(window->renderer);
    return ESZ_OK;
}

esz_status render_actors(int32_t level, esz_window_t* window, esz_core_t* core)
{
    esz_tiled_layer_t* layer;
    esz_render_layer   render_layer = ESZ_ACTOR_FG;
    int32_t            index        = 0;

    if (! esz_is_map_loaded(core))
    {
        return ESZ_OK;
    }

    layer = get_head_layer(core->map->handle);

    if (level >= ESZ_ACTOR_LAYER_LEVEL_MAX)
    {
        plog_error("%s: invalid layer level selected.", __func__);
        return ESZ_ERROR_CRITICAL;
    }

    if (ESZ_ACTOR_LAYER_BG == level)
    {
        render_layer = ESZ_ACTOR_BG;
    }
    else if (ESZ_ACTOR_LAYER_MG == level)
    {
        render_layer = ESZ_ACTOR_MG;
    }

    if (ESZ_OK != create_and_set_render_target(&core->map->render_target[render_layer], window))
    {
        return ESZ_ERROR_CRITICAL;
    }

    while (layer)
    {
        if (is_tiled_layer_of_type(ESZ_OBJECT_GROUP, layer, core))
        {
            esz_tiled_object_t* tiled_object = get_head_object(layer, core);
            while (tiled_object)
            {
                uint64_t      type_hash = esz_hash((const unsigned char*)get_object_type_name(tiled_object));
                esz_entity_t* object    = &core->map->entity[index];

                switch (type_hash)
                {
                    case H_actor:
                    {
                        esz_actor_t**    actor = &object->actor;
                        double           pos_x = object->pos_x - core->camera.pos_x;
                        double           pos_y = object->pos_y - core->camera.pos_y;
                        SDL_RendererFlip flip  = SDL_FLIP_NONE;
                        SDL_Rect         dst   = { 0 };
                        SDL_Rect         src   = { 0 };

                        if (ESZ_ACTOR_LAYER_BG == level && ! IS_STATE_SET((*actor)->state, STATE_IN_BACKGROUND))
                        {
                            break;
                        }

                        if (ESZ_ACTOR_LAYER_MG == level && ! IS_STATE_SET((*actor)->state, STATE_IN_MIDGROUND))
                        {
                            break;
                        }

                        if (ESZ_ACTOR_LAYER_FG == level && ! IS_STATE_SET((*actor)->state, STATE_IN_FOREGROUND))
                        {
                                break;
                        }

                        if (IS_STATE_SET((*actor)->state, STATE_LOOKING_LEFT))
                        {
                            flip = SDL_FLIP_HORIZONTAL;
                        }

                        // Update animation frame
                        // -----------------------------------------------------

                        if (IS_STATE_SET((*actor)->state, STATE_ANIMATED) && (*actor)->animation)
                        {
                            int32_t current_animation = (*actor)->current_animation;

                            (*actor)->time_since_last_anim_frame += window->time_since_last_frame;

                            if ((*actor)->time_since_last_anim_frame >= 1.0 / (double)((*actor)->animation[current_animation - 1].fps))
                            {
                                (*actor)->time_since_last_anim_frame = 0.0;

                                (*actor)->current_frame += 1;

                                if ((*actor)->current_frame >= (*actor)->animation[current_animation - 1].length)
                                {
                                    (*actor)->current_frame = 0;
                                }
                            }

                            src.x  = ((*actor)->animation[current_animation - 1].first_frame - 1) * object->width;
                            src.x += (*actor)->current_frame                                      * object->width;
                            src.y  = (*actor)->animation[current_animation - 1].offset_y          * object->height;
                        }

                        src.w  = object->width;
                        src.h  = object->height;
                        dst.x  = (int32_t)pos_x - (object->width  / 2);
                        dst.y  = (int32_t)pos_y - (object->height / 2);
                        dst.w  = object->width;
                        dst.h  = object->height;

                        if (0 > SDL_RenderCopyEx(window->renderer, core->map->sprite[(*actor)->sprite_sheet_id].texture, &src, &dst, 0, NULL, flip))
                        {
                            plog_error("%s: %s.", __func__, SDL_GetError());
                            return ESZ_ERROR_CRITICAL;
                        }
                        break;
                    }
                }
                index        += 1;
                tiled_object  = tiled_object->next;
            }
        }
        layer = layer->next;
    }

    return ESZ_OK;
}

esz_status render_background(esz_window_t* window, esz_core_t* core)
{
    esz_status       status       = ESZ_OK;
    esz_render_layer render_layer = ESZ_BACKGROUND;
    double           factor;

    if (!esz_is_map_loaded(core))
    {
        return ESZ_OK;
    }

    factor = (double)core->map->background.layer_count + 1.0;

    if (! core->map->render_target[render_layer])
    {
        core->map->render_target[render_layer] = SDL_CreateTexture(
            window->renderer,
            SDL_PIXELFORMAT_ARGB8888,
            SDL_TEXTUREACCESS_TARGET,
            window->width,
            window->height);
    }

    if (! core->map->render_target[render_layer])
    {
        plog_error("%s: %s.", __func__, SDL_GetError());
        return ESZ_ERROR_CRITICAL;
    }

    if (is_camera_at_horizontal_boundary(core))
    {
        if (! core->map->background.velocity_is_constant)
        {
            core->map->background.velocity = 0.0;
        }
    }
    else
    {
        if (core->map->entity)
        {
            if (core->map->entity[core->camera.target_actor_id].actor)
            {
                core->map->background.velocity = core->map->entity[core->camera.target_actor_id].actor->velocity_x;
            }
        }
        else
        {
            core->map->background.velocity = 0.0;
        }
    }

    if (! esz_is_camera_locked(core))
    {
        core->map->background.velocity = 0.0;
    }

    for (int32_t index = 0; index < core->map->background.layer_count; index += 1)
    {
        core->map->background.layer[index].velocity = core->map->background.velocity / factor;
        factor -= core->map->background.layer_shift;

        status = render_background_layer(index, window, core);

        if (ESZ_OK != status)
        {
            break;
        }
    }

    return status;
}

esz_status render_background_layer(int32_t index, esz_window_t* window, esz_core_t* core)
{
    esz_render_layer render_layer = ESZ_BACKGROUND;
    int32_t          width        = 0;
    SDL_Rect         dst;
    double           pos_x_a;
    double           pos_x_b;

    if (0 > SDL_QueryTexture(core->map->background.layer[index].texture, NULL, NULL, &width, NULL))
    {
        plog_error("%s: %s.", __func__, SDL_GetError());
        return ESZ_ERROR_CRITICAL;
    }

    if (core->map->background.layer[index].pos_x < -width)
    {
        core->map->background.layer[index].pos_x = +width;
    }

    if (core->map->background.layer[index].pos_x > +width)
    {
        core->map->background.layer[index].pos_x = -width;
    }

    pos_x_a = core->map->background.layer[index].pos_x;
    if (0 < pos_x_a)
    {
        pos_x_b = pos_x_a - width;
    }
    else
    {
        pos_x_b = pos_x_a + width;
    }

    if (0 < core->map->background.layer[index].velocity)
    {
        if (ESZ_RIGHT == core->map->background.direction)
        {
            core->map->background.layer[index].pos_x -= core->map->background.layer[index].velocity;
        }
        else
        {
            core->map->background.layer[index].pos_x += core->map->background.layer[index].velocity;
        }
    }

    if (ESZ_TOP == core->map->background.alignment)
    {
        dst.y = (int32_t)(core->map->background.layer[index].pos_y - core->camera.pos_y);
    }
    else
    {
        dst.y = (int32_t)(core->map->background.layer[index].pos_y + (window->logical_height - core->map->background.layer[index].height));
    }

    if (0 > SDL_SetRenderTarget(window->renderer, core->map->render_target[render_layer]))
    {
        plog_error("%s: %s.", __func__, SDL_GetError());
        return ESZ_ERROR_CRITICAL;
    }

    if (0 == index)
    {
        SDL_SetRenderDrawColor(
            window->renderer,
            (core->map->handle->backgroundcolor >> 16) & 0xFF,
            (core->map->handle->backgroundcolor >> 8)  & 0xFF,
            (core->map->handle->backgroundcolor)       & 0xFF,
            0);

        SDL_RenderClear(window->renderer);
    }

    dst.x = (int32_t)pos_x_a;
    dst.w = width;
    dst.h = core->map->background.layer[index].height;

    if (0 > SDL_RenderCopyEx(window->renderer, core->map->background.layer[index].texture, NULL, &dst, 0, NULL, SDL_FLIP_NONE))
    {
        plog_error("%s: %s.", __func__, SDL_GetError());
        return ESZ_ERROR_CRITICAL;
    }

    dst.x = (int32_t)pos_x_b;
    if (0 > SDL_RenderCopyEx(window->renderer, core->map->background.layer[index].texture, NULL, &dst, 0, NULL, SDL_FLIP_NONE))
    {
        plog_error("%s: %s.", __func__, SDL_GetError());
        return ESZ_ERROR_CRITICAL;
    }

    return ESZ_OK;
}

esz_status render_map(int32_t level, esz_window_t* window, esz_core_t* core)
{
    esz_tiled_layer_t* layer;
    bool               render_animated_tiles = false;
    esz_render_layer   render_layer          = ESZ_MAP_FG;

    if (! esz_is_map_loaded(core))
    {
        return ESZ_OK;
    }

    layer = get_head_layer(core->map->handle);

    if (level >= ESZ_MAP_LAYER_LEVEL_MAX)
    {
        plog_error("%s: invalid layer level selected.", __func__);
        return ESZ_ERROR_CRITICAL;
    }

    if (ESZ_MAP_LAYER_BG == level)
    {
        render_layer = ESZ_MAP_BG;

        if (0 < core->map->animated_tile_fps)
        {
            render_animated_tiles = true;
        }
    }

    if (ESZ_OK != create_and_set_render_target(&core->map->render_target[render_layer], window))
    {
        return ESZ_ERROR_CRITICAL;
    }

    // Update and render animated tiles.
    core->map->time_since_last_anim_frame += window->time_since_last_frame;

    if (0 < core->map->animated_tile_index &&
        core->map->time_since_last_anim_frame >= 1.0 / (double)(core->map->animated_tile_fps) && render_animated_tiles)
    {
        core->map->time_since_last_anim_frame = 0.0;

        /* Remark: animated tiles are always rendered in the background
         * layer.
         */
        if (! core->map->animated_tile_texture)
        {
            core->map->animated_tile_texture = SDL_CreateTexture(
                window->renderer,
                SDL_PIXELFORMAT_ARGB8888,
                SDL_TEXTUREACCESS_TARGET,
                (int32_t)(core->map->width),
                (int32_t)(core->map->height));
        }

        if (! core->map->animated_tile_texture)
        {
            plog_error("%s: %s.", __func__, SDL_GetError());
            return ESZ_ERROR_CRITICAL;
        }

        if (0 > SDL_SetRenderTarget(window->renderer, core->map->animated_tile_texture))
        {
            plog_error("%s: %s.", __func__, SDL_GetError());
            return ESZ_ERROR_CRITICAL;
        }
        SDL_RenderClear(window->renderer);

        for (int32_t index = 0; core->map->animated_tile_index > index; index += 1)
        {
            esz_tiled_tileset_t* tileset;
            int32_t              gid          = core->map->animated_tile[index].gid;
            int32_t              next_tile_id = 0;
            int32_t              local_id;
            SDL_Rect             dst;
            SDL_Rect             src;

            local_id = core->map->animated_tile[index].id + 1;
            tileset  = get_head_tileset(core->map->handle);
            src.w    = dst.w = get_tile_width(core->map->handle);
            src.h    = dst.h = get_tile_height(core->map->handle);
            dst.x    = (int32_t)core->map->animated_tile[index].dst_x;
            dst.y    = (int32_t)core->map->animated_tile[index].dst_y;

            get_tile_position(local_id, &src.x, &src.y, core->map->handle);

            if (0 > SDL_RenderCopy(window->renderer, core->map->tileset_texture, &src, &dst))
            {
                plog_error("%s: %s.", __func__, SDL_GetError());
                return ESZ_ERROR_CRITICAL;
            }

            core->map->animated_tile[index].current_frame += 1;

            if (core->map->animated_tile[index].current_frame >= core->map->animated_tile[index].animation_length)
            {
                core->map->animated_tile[index].current_frame = 0;
            }

            next_tile_id = get_next_animated_tile_id(gid, core->map->animated_tile[index].current_frame, core->map->handle);

            core->map->animated_tile[index].id = next_tile_id;
        }

        if (0 > SDL_SetRenderTarget(window->renderer, core->map->render_target[render_layer]))
        {
            plog_error("%s: %s.", __func__, SDL_GetError());
            return ESZ_ERROR_CRITICAL;
        }
        SDL_RenderClear(window->renderer);

        if (0 > SDL_SetTextureBlendMode(core->map->animated_tile_texture, SDL_BLENDMODE_BLEND))
        {
            plog_error("%s: %s.", __func__, SDL_GetError());
            return ESZ_ERROR_CRITICAL;
        }
    }

    // Texture has already been rendered.
    if (core->map->layer_texture[level])
    {
        double   render_pos_x = core->map->pos_x - core->camera.pos_x;
        double   render_pos_y = core->map->pos_y - core->camera.pos_y;
        SDL_Rect dst          = {
            (int32_t)render_pos_x,
            (int32_t)render_pos_y,
            (int32_t)core->map->width,
            (int32_t)core->map->height
        };

        if (0 > SDL_RenderCopyEx(window->renderer, core->map->layer_texture[level], NULL, &dst, 0, NULL, SDL_FLIP_NONE))
        {
            plog_error("%s: %s.", __func__, SDL_GetError());
            return ESZ_ERROR_CRITICAL;
        }

        if (render_animated_tiles)
        {
            if (core->map->animated_tile_texture)
            {
                if (0 > SDL_RenderCopyEx(window->renderer, core->map->animated_tile_texture, NULL, &dst, 0, NULL, SDL_FLIP_NONE))
                {
                    plog_error("%s: %s.", __func__, SDL_GetError());
                    return ESZ_ERROR_CRITICAL;
                }
            }
        }

        return ESZ_OK;
    }

    // Texture does not yet exist. Render it!
    core->map->layer_texture[level] = SDL_CreateTexture(
        window->renderer,
        SDL_PIXELFORMAT_ARGB8888,
        SDL_TEXTUREACCESS_TARGET,
        (int32_t)core->map->width,
        (int32_t)core->map->height);

    if (! core->map->layer_texture[level])
    {
        plog_error("%s: %s.", __func__, SDL_GetError());
        return ESZ_ERROR_CRITICAL;
    }

    if (0 > SDL_SetRenderTarget(window->renderer, core->map->layer_texture[level]))
    {
        plog_error("%s: %s.", __func__, SDL_GetError());
        return ESZ_ERROR_CRITICAL;
    }
    SDL_RenderClear(window->renderer);

    while (layer)
    {
        SDL_Rect dst;
        SDL_Rect src;

        if (is_tiled_layer_of_type(ESZ_TILE_LAYER, layer, core))
        {
            bool    is_in_foreground  = false;
            bool    is_layer_rendered = false;
            int32_t prop_cnt = get_layer_property_count(layer);

            is_in_foreground = get_boolean_property(H_is_in_foreground, layer->properties, prop_cnt, core);

            if (ESZ_MAP_LAYER_BG == level && false == is_in_foreground)
            {
                is_layer_rendered = true;
            }
            else if (ESZ_MAP_LAYER_FG == level && is_in_foreground)
            {
                is_layer_rendered = true;
            }

            if (layer->visible && is_layer_rendered)
            {
                for (int32_t index_height = 0; index_height < (int32_t)core->map->handle->height; index_height += 1)
                {
                    for (int32_t index_width = 0; index_width < (int32_t)core->map->handle->width; index_width += 1)
                    {
                        int32_t* layer_content = get_layer_content(layer);
                        int32_t  gid           = remove_gid_flip_bits((int32_t)layer_content[(index_height * (int32_t)core->map->handle->width) + index_width]);
                        int32_t  local_id      = gid - get_first_gid(core->map->handle);

                        if (is_gid_valid(gid, core->map->handle))
                        {
                            esz_tiled_tileset_t* tileset = get_head_tileset(core->map->handle);

                            src.w = dst.w = get_tile_width(core->map->handle);
                            src.h = dst.h = get_tile_height(core->map->handle);
                            dst.x = (int32_t)(index_width  * get_tile_width(core->map->handle));
                            dst.y = (int32_t)(index_height * get_tile_height(core->map->handle));

                            get_tile_position(gid, &src.x, &src.y, core->map->handle);
                            SDL_RenderCopy(window->renderer, core->map->tileset_texture, &src, &dst);

                            if (render_animated_tiles)
                            {
                                int32_t animation_length = 0;
                                int32_t id               = 0;

                                if (is_tile_animated(gid, &animation_length, &id, core->map->handle))
                                {
                                    core->map->animated_tile[core->map->animated_tile_index].gid              = get_local_id(gid, core->map->handle);
                                    core->map->animated_tile[core->map->animated_tile_index].id               = id;
                                    core->map->animated_tile[core->map->animated_tile_index].dst_x            = dst.x;
                                    core->map->animated_tile[core->map->animated_tile_index].dst_y            = dst.y;
                                    core->map->animated_tile[core->map->animated_tile_index].current_frame    = 0;
                                    core->map->animated_tile[core->map->animated_tile_index].animation_length = animation_length;

                                    core->map->animated_tile_index += 1;
                                }
                            }
                        }
                    }
                }

                {
                    const char* layer_name = get_layer_name(layer);
                    plog_info("Render map layer: %s", layer_name);
                }
            }
        }
        layer = layer->next;
    }

    if (0 > SDL_SetRenderTarget(window->renderer, core->map->render_target[render_layer]))
    {
        plog_error("%s: %s.", __func__, SDL_GetError());
        return ESZ_ERROR_CRITICAL;
    }

    if (0 > SDL_SetTextureBlendMode(core->map->layer_texture[level], SDL_BLENDMODE_BLEND))
    {
        plog_error("%s: %s.", __func__, SDL_GetError());
        return ESZ_ERROR_CRITICAL;
    }

    return ESZ_OK;
}

esz_status render_scene(esz_window_t* window, esz_core_t* core)
{
    esz_status status = ESZ_OK;

    status = render_background(window, core);
    if (ESZ_OK != status)
    {
        return status;
    }

    for (int32_t index = 0; index < ESZ_MAP_LAYER_LEVEL_MAX; index  += 1)
    {
        status = render_map(index, window, core);
        if (ESZ_OK != status)
        {
            return status;
        }
    }

    for (int32_t index = 0; index < ESZ_ACTOR_LAYER_LEVEL_MAX; index += 1)
    {
        status = render_actors(index, window, core);
        if (ESZ_OK != status)
        {
            return status;
        }
    }

    return status;
}

#endif // ESZ_RENDER_IMPLEMENTATION
#endif // ESZ_RENDER_H

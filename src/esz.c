// Spdx-License-Identifier: MIT
/**
 * @file    esz.c
 * @brief   eszFW game engine
 * @details A cross-platform game engine written in C
 */

#include "esz_macros.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <picolog.h>

DISABLE_WARNING_PUSH
DISABLE_WARNING_PADDING
DISABLE_WARNING_SPECTRE_MITIGATION
DISABLE_WARNING_SYMBOL_NOT_DEFINED

#include <cwalk.h>
#include <SDL.h>

#ifdef USE_LIBTMX
    #include <tmx.h>
#else // (cute_tiled.h)
    #define CUTE_TILED_IMPLEMENTATION
    #include <cute_tiled.h>
#endif

DISABLE_WARNING_POP

DISABLE_WARNING_PUSH
DISABLE_WARNING_PADDING
DISABLE_WARNING_SPECTRE_MITIGATION

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#define STB_SPRINTF_IMPLEMENTATION
#include <stb_sprintf.h>

DISABLE_WARNING_POP

#include "esz.h"
#include "esz_hashes.h"
#include "esz_types.h"

#define ESZ_COMPAT_IMPLEMENTATION
#include "esz_compat.h"

#define ESZ_UTILS_IMPLEMENTATION
#include "esz_utils.h"

#define ESZ_INIT_IMPLEMENTATION
#include "esz_init.h"

#define ESZ_RENDER_IMPLEMENTATION
#include "esz_render.h"

DISABLE_WARNING_PUSH
DISABLE_WARNING_SPECTRE_MITIGATION

bool esz_bounding_boxes_do_intersect(const esz_aabb_t bb_a, const esz_aabb_t bb_b)
{
    double bb_a_x = bb_b.left - bb_a.right;
    double bb_a_y = bb_b.top  - bb_a.bottom;
    double bb_b_x = bb_a.left - bb_b.right;
    double bb_b_y = bb_a.top  - bb_b.bottom;

    if (0.0 < bb_a_x || 0.0 < bb_a_y)
    {
        return false;
    }

    if (0.0 < bb_b_x || 0.0 < bb_b_y)
    {
        return false;
    }

    return true;
}

void esz_clear_player_state(esz_state state, esz_core_t* core)
{
    if (! esz_is_map_loaded(core))
    {
        return;
    }

    if (core->map->entity)
    {
        if (core->map->entity[core->camera.target_actor_id].actor)
        {
            esz_actor_t* actor = core->map->entity[core->map->active_player_actor_id].actor;
            CLR_STATE(actor->state, state);
        }
    }
}

esz_status esz_create_window(const char* window_title, esz_window_config_t* config, esz_window_t** window)
{
    esz_status      status = ESZ_OK;
    SDL_DisplayMode display_mode;
    uint32_t        renderer_flags = SDL_RENDERER_ACCELERATED | SDL_RENDERER_TARGETTEXTURE;
    plog_id_t       logger_id;

    const unsigned char* esz_logo;
    const unsigned char  esz_logo_pxdata[228] = {
        0x89, 0x50, 0x4e, 0x47, 0x0d, 0x0a, 0x1a, 0x0a, 0x00, 0x00, 0x00, 0x0d,
        0x49, 0x48, 0x44, 0x52, 0x00, 0x00, 0x00, 0x18, 0x00, 0x00, 0x00, 0x07,
        0x08, 0x02, 0x00, 0x00, 0x00, 0x9d, 0x95, 0x1a, 0xe1, 0x00, 0x00, 0x00,
        0xab, 0x49, 0x44, 0x41, 0x54, 0x18, 0xd3, 0x7d, 0x91, 0xa1, 0x19, 0xc2,
        0x30, 0x10, 0x85, 0x5f, 0xf8, 0xaa, 0x4f, 0x54, 0x44, 0x44, 0x31, 0x42,
        0x27, 0x40, 0x22, 0x11, 0xd9, 0x81, 0x15, 0x3a, 0x02, 0xa2, 0x2b, 0x74,
        0x02, 0x4c, 0x45, 0x24, 0x13, 0x44, 0x22, 0x3b, 0x01, 0x22, 0x02, 0x71,
        0x13, 0x20, 0xae, 0x5c, 0xcb, 0x11, 0x78, 0xf2, 0xcf, 0xbb, 0x97, 0x77,
        0xdf, 0xb9, 0xeb, 0xfe, 0x70, 0x7e, 0xde, 0xf1, 0xa9, 0xb1, 0xed, 0x00,
        0x54, 0xf9, 0x37, 0x14, 0x35, 0xf2, 0x50, 0x6e, 0x0c, 0x31, 0x64, 0xf8,
        0x89, 0xd4, 0x6d, 0xf8, 0x0a, 0x01, 0xe5, 0x00, 0xfc, 0x44, 0x8d, 0xba,
        0x7d, 0x4f, 0x00, 0x4a, 0xe4, 0x14, 0xf8, 0xf4, 0x20, 0xc3, 0xb7, 0xf2,
        0xc7, 0x77, 0x62, 0x64, 0x49, 0x01, 0xb0, 0xd3, 0xec, 0x12, 0xb9, 0x44,
        0xce, 0xd9, 0x0e, 0x08, 0x4f, 0x81, 0x65, 0x59, 0xd9, 0xda, 0xa4, 0x8c,
        0x6d, 0xe7, 0x88, 0xa8, 0x0c, 0xbc, 0x6d, 0x5e, 0x55, 0x0a, 0x9c, 0x67,
        0x5c, 0xa8, 0xd2, 0x45, 0xe4, 0x88, 0x96, 0x6f, 0xa5, 0x4b, 0x9e, 0x01,
        0x2c, 0x03, 0xe2, 0x56, 0x6e, 0x52, 0x8c, 0xdf, 0x55, 0xaf, 0xf6, 0x4b,
        0x7f, 0xae, 0xf6, 0x02, 0x39, 0xaf, 0x64, 0xb2, 0x37, 0xc2, 0xf0, 0x45,
        0x00, 0x00, 0x00, 0x00, 0x49, 0x45, 0x4e, 0x44, 0xae, 0x42, 0x60, 0x82
    };
    esz_logo = esz_logo_pxdata;

    logger_id = plog_add_stream(stdout, PLOG_LEVEL_INFO);
    plog_set_time_fmt(logger_id, "%H:%M:%S");
    plog_timestamp_on(logger_id);

    #ifndef __WIN32__
    plog_colors_on(logger_id);
    #endif

    *window = (esz_window_t*)calloc(1, sizeof(struct esz_window));
    if (! *window)
    {
        plog_error("%s: error allocating memory.", __func__);
        status = ESZ_ERROR_CRITICAL;
        goto exit;
    }

    (*window)->width          = config->width;
    (*window)->height         = config->height;
    (*window)->logical_width  = config->logical_width;
    (*window)->logical_height = config->logical_height;
    (*window)->vsync_enabled  = config->enable_vsync;

    if (config->enable_fullscreen)
    {
        (*window)->flags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
    }

    if (0 > SDL_Init(SDL_INIT_VIDEO))
    {
        plog_error("%s: %s.", __func__, SDL_GetError());
        status = ESZ_ERROR_CRITICAL;
        goto exit;
    }

    if (0 != SDL_GetCurrentDisplayMode(0, &display_mode))
    {
        plog_error("%s: %s.", __func__, SDL_GetError());
        status = ESZ_ERROR_CRITICAL;
        goto exit;
    }

    #ifdef __ANDROID__
    (*window)->flags  = 0;
    (*window)->width  = display_mode.w;
    (*window)->height = display_mode.h;
    #endif

    (*window)->refresh_rate  = display_mode.refresh_rate;

    if (0 == (*window)->refresh_rate)
    {
        (*window)->refresh_rate = 60;
        if ((*window)->vsync_enabled)
        {
            plog_warn("Couldn't determine the monitor's refresh rate: VSync disabled.");
            (*window)->vsync_enabled = false;
        }
    }

    if ((*window)->vsync_enabled)
    {
        renderer_flags |= SDL_RENDERER_PRESENTVSYNC;
    }

    (*window)->window = SDL_CreateWindow(
        window_title,
        SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED,
        (*window)->width,
        (*window)->height,
        (*window)->flags);

    if (! (*window)->window)
    {
        plog_error("%s: %s.", __func__, SDL_GetError());
        status = ESZ_ERROR_CRITICAL;
        goto exit;
    }

    SDL_GetWindowSize((*window)->window, &(*window)->width, &(*window)->height);

    (*window)->zoom_level         = (double)(*window)->height / (double)(*window)->logical_height;
    (*window)->initial_zoom_level = (*window)->zoom_level;

    /* Get index of opengl rendering driver and create 2D rendering
     * context.
     */
    for (int32_t index = 0; index < SDL_GetNumRenderDrivers(); index += 1)
    {
        SDL_RendererInfo renderer_info = { 0 };
        SDL_GetRenderDriverInfo(index, &renderer_info);

        if (H_opengl != esz_hash((const unsigned char*)renderer_info.name))
        {
            continue;
        }
        else
        {
            (*window)->renderer = SDL_CreateRenderer((*window)->window, index, renderer_flags);

            if (! (*window)->renderer)
            {
                plog_error("%s: %s.", __func__, SDL_GetError());
                status = ESZ_ERROR_CRITICAL;
                goto exit;
            }
            break;
        }
    }

    /* Use default rendering driver if opengl isn't available.
     */
    if (! (*window)->renderer)
    {
        SDL_RendererInfo renderer_info = { 0 };
        SDL_GetRenderDriverInfo(0, &renderer_info);

        plog_warn("opengl not found: use default rendering driver: %s.", renderer_info.name);

        (*window)->renderer = SDL_CreateRenderer((*window)->window, -1, renderer_flags);

        if (! (*window)->renderer)
        {
            plog_error("%s: %s.", __func__, SDL_GetError());
            status = ESZ_ERROR_CRITICAL;
            goto exit;
        }
    }

    plog_info(
        "Setting up window at resolution %dx%d @ %d Hz.",
        (*window)->width,
        (*window)->height,
        (*window)->refresh_rate);

    esz_set_zoom_level((*window)->zoom_level, *window);
    plog_info("Set initial zoom-level to factor %f.", (*window)->zoom_level);

    if (ESZ_OK != load_texture_from_memory(esz_logo, 228, &(*window)->esz_logo, *window))
    {
        status = ESZ_ERROR_CRITICAL;
    }

exit:
    return status;
}

void esz_deactivate_core(esz_core_t* core)
{
    core->is_active = false;
}

void esz_destroy_core(esz_core_t* core)
{
    free(core);
    plog_info("Destroy engine core.");
}

void esz_destroy_window(esz_window_t* window)
{
    if (window->esz_logo)
    {
        SDL_DestroyTexture(window->esz_logo);
    }

    if (window->renderer)
    {
        SDL_DestroyRenderer(window->renderer);
    }

    if (window->window)
    {
        SDL_DestroyWindow(window->window);
    }

    free(window);
    plog_info("Quitting.");

    SDL_Quit();
}

const uint8_t* esz_get_keyboard_state(void)
{
    return SDL_GetKeyboardState(NULL);
}

uint64_t esz_hash(const unsigned char* name)
{
    uint64_t hash = 5381;
    uint32_t c;

    while ((c = *name++))
    {
        hash = ((hash << 5) + hash) + c;
    }

    return hash;
}

int32_t esz_get_keycode(esz_core_t* core)
{
    return core->event.handle.key.keysym.sym;
}

bool esz_get_boolean_map_property(const uint64_t name_hash, esz_core_t* core)
{
    int32_t prop_cnt = get_map_property_count(core->map->handle);

    core->map->boolean_property = false;
    load_property(name_hash, core->map->handle->properties, prop_cnt, core);
    return core->map->boolean_property;
}

double esz_get_decimal_map_property(const uint64_t name_hash, esz_core_t* core)
{
    int32_t prop_cnt = get_map_property_count(core->map->handle);

    core->map->decimal_property = 0.0;
    load_property(name_hash, core->map->handle->properties, prop_cnt, core);
    return core->map->decimal_property;
}

int32_t esz_get_integer_map_property(const uint64_t name_hash, esz_core_t* core)
{
    int32_t prop_cnt = get_map_property_count(core->map->handle);

    core->map->integer_property = 0;
    load_property(name_hash, core->map->handle->properties, prop_cnt, core);
    return core->map->integer_property;
}

const char* esz_get_string_map_property(const uint64_t name_hash, esz_core_t* core)
{
    int32_t prop_cnt = get_map_property_count(core->map->handle);

    core->map->string_property = NULL;
    load_property(name_hash, core->map->handle->properties, prop_cnt, core);
    return core->map->string_property;
}

double esz_get_time_since_last_frame(esz_window_t* window)
{
    return window->time_since_last_frame;
}

void esz_hide_render_layer(esz_render_layer layer, esz_core_t* core)
{
    if (ESZ_RENDER_LAYER_MAX != layer)
    {
        SET_STATE(core->debug, (uint32_t)layer);
    }
}

esz_status esz_init_core(esz_core_t** core)
{
    *core = (esz_core_t*)calloc(1, sizeof(struct esz_core));
    if (! *core)
    {
        plog_error("%s: error allocating memory.", __func__);
        return ESZ_ERROR_CRITICAL;
    }

    (*core)->is_active = true;

    return ESZ_OK;
}

bool esz_is_camera_locked(esz_core_t* core)
{
    return core->camera.is_locked;
}

bool esz_is_core_active(esz_core_t* core)
{
    return core->is_active;
}

bool esz_is_map_loaded(esz_core_t* core)
{
    if (core->is_map_loaded)
    {
        return true;
    }

    return false;
}

bool esz_is_player_moving(esz_core_t* core)
{
    if (! esz_is_map_loaded(core))
    {
        return false;
    }

    if (core->map->entity)
    {
        if (core->map->entity[core->camera.target_actor_id].actor)
        {
            esz_actor_t* actor = core->map->entity[core->map->active_player_actor_id].actor;

            if (IS_STATE_SET(actor->state, STATE_MOVING))
            {
                return true;
            }
        }
    }

    return false;
}

esz_status esz_load_map(const char* map_file_name, esz_window_t* window, esz_core_t* core)
{
    char* tileset_image_source = NULL;

    if (esz_is_map_loaded(core))
    {
        plog_warn("A map has already been loaded: unload map first.");
        return ESZ_WARNING;
    }

    // Load map file and allocate required memory
    // ------------------------------------------------------------------------

    // 1. Map
    // ------------------------------------------------------------------------

    core->map = (esz_map_t*)calloc(1, sizeof(struct esz_map));
    if (! core->map)
    {
        plog_error("%s: error allocating memory.", __func__);
        return ESZ_WARNING;
    }

    // 2. Tiled map
    // ------------------------------------------------------------------------

    if (ESZ_OK != load_tiled_map(map_file_name, core))
    {
        goto warning;
    }
    core->is_map_loaded = true;

    // 3. Tile properties
    // ------------------------------------------------------------------------

    {
        esz_tiled_layer_t* layer      = get_head_layer(core->map->handle);
        int32_t            tile_count = (int32_t)(core->map->handle->height * core->map->handle->width);

        core->map->tile_properties = (uint32_t*)calloc((size_t)tile_count, sizeof(uint32_t));
        if (!core->map->tile_properties)
        {
            plog_error("%s: error allocating memory.", __func__);
            goto warning;
        }

        while (layer)
        {
            if (is_tiled_layer_of_type(ESZ_TILE_LAYER, layer, core))
            {
                for (int32_t index_height = 0; index_height < (int32_t)core->map->handle->height; index_height += 1)
                {
                    for (int32_t index_width = 0; index_width < (int32_t)core->map->handle->width; index_width += 1)
                    {
                        esz_tiled_tileset_t* tileset       = get_head_tileset(core->map->handle);
                        esz_tiled_tile_t*    tile          = tileset->tiles;
                        int32_t*             layer_content = get_layer_content(layer);
                        int32_t              gid           = remove_gid_flip_bits((int32_t)layer_content[(index_height * (int32_t)core->map->handle->width) + index_width]);
                        int32_t              tile_index    = (index_width + 1) * (index_height + 1);

                        if (tile_has_properties(gid, &tile, core->map->handle))
                        {
                            /*
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
                            */
                        }
                    }
                }
            }
            layer = layer->next;
        }
    }

    // 4. Paths and file locations
    // ------------------------------------------------------------------------

    core->map->path = (char*)calloc(1, (size_t)(strnlen(map_file_name, 64) + 1));
    if (! core->map->path)
    {
        plog_error("%s: error allocating memory.", __func__);
        goto warning;
    }

    cwk_path_get_dirname(map_file_name, (size_t*)&(core->map->path_length));
    SDL_strlcpy(core->map->path, map_file_name, core->map->path_length + 1);


    //tbd.


    {
        int64_t source_length;

        #ifdef USE_LIBTMX
        int32_t first_gid = get_first_gid(core->map->handle);
        int64_t ts_path_length;
        char*   ts_path;

        cwk_path_get_dirname(core->map->handle->ts_head->source, (size_t*)&ts_path_length);

        ts_path = calloc(1, ts_path_length + 1);
        if (! ts_path)
        {
            plog_error("%s: error allocating memory.", __func__);
            goto warning;
        }

        source_length  = strnlen(core->map->path, 64);
        source_length += strnlen(core->map->handle->tiles[first_gid]->tileset->image->source, 64);
        source_length += ts_path_length + 1;

        tileset_image_source = calloc(1, source_length);
        if (! tileset_image_source)
        {
            plog_error("%s: error allocating memory.", __func__);
            free(ts_path);
            goto warning;
        }

        /* The tileset image source is stored relatively to the tileset
         * file but because we only know the location of the tileset
         * file relatively to the map file, we need to adjust the path
         * accordingly.  It's a hack, but it works.
         */

        SDL_strlcpy(ts_path, core->map->handle->ts_head->source, ts_path_length + 1);
        stbsp_snprintf(tileset_image_source, (int32_t)source_length, "%s%s%s",
                     core->map->path,
                     ts_path,
                     core->map->handle->tiles[first_gid]->tileset->image->source);

        free(ts_path);

        #else // (cute_tiled.h)
        if (! core->map->handle->tilesets)
        {
            plog_error("%s: no embedded tileset found.", __func__);
            goto warning;
        }

        source_length  = strnlen(core->map->path, 64);
        source_length += strnlen(core->map->handle->tilesets->image.ptr, 64);
        source_length += 1;

        tileset_image_source = (char*)calloc(1, source_length);
        if (! tileset_image_source)
        {
            plog_error("%s: error allocating memory.", __func__);
            esz_unload_map(window, core);
            return ESZ_WARNING;
        }

        stbsp_snprintf(tileset_image_source, (int32_t)source_length, "%s%s",
                     core->map->path,
                     core->map->handle->tilesets->image.ptr);

        #endif
    }

    // 5. Entities
    // ------------------------------------------------------------------------

    if (ESZ_OK != init_entities(core))
    {
        free(tileset_image_source);
        goto warning;
    }

    // 6. Tileset
    // ------------------------------------------------------------------------

    if (tileset_image_source)
    {
        if (ESZ_OK != load_texture_from_file(tileset_image_source, &core->map->tileset_texture, window))
        {
            plog_error("%s: Error loading image '%s'.", __func__, tileset_image_source);
            free(tileset_image_source);
            goto warning;
        }
    }
    else
    {
        plog_error("%s: Could not determine location of tileset image.", __func__);
        goto warning;
    }

    free(tileset_image_source);

    // 7. Sprites
    // ------------------------------------------------------------------------

    if (ESZ_OK != init_sprites(window, core))
    {
        goto warning;
    }

    // 8. Animated tiles
    // ------------------------------------------------------------------------

    if (ESZ_OK != init_animated_tiles(core))
    {
        goto warning;
    }

    // 9. Background
    // ------------------------------------------------------------------------

    if (ESZ_OK != init_background(window, core))
    {
        goto warning;
    }

    plog_info(
        "Set gravitational constant to %f (g*%dpx/s^2).",
        core->map->gravitation, core->map->meter_in_pixel);

    core->map->animated_tile_fps = esz_get_integer_map_property(H_animated_tile_fps, core);
    if (core->map->animated_tile_fps > window->refresh_rate)
    {
        // It can't update faster anyway.
        core->map->animated_tile_fps = window->refresh_rate;
    }

    if (core->event.map_loaded_cb)
    {
        core->event.map_loaded_cb(window, core);
    }

    // ------------------------------------------------------------------------

    core->map->height = (int32_t)((int32_t)core->map->handle->height * get_tile_height(core->map->handle));
    core->map->width = (int32_t)((int32_t)core->map->handle->width * get_tile_width(core->map->handle));
    core->map->gravitation = esz_get_decimal_map_property(H_gravitation, core);
    core->map->meter_in_pixel = esz_get_integer_map_property(H_meter_in_pixel, core);

    plog_info(
        "Load map file: %s containing %d entities(s).",
        map_file_name, core->map->entity_count);

    return ESZ_OK;
warning:
    esz_unload_map(window, core);
    return ESZ_WARNING;
}

void esz_lock_camera(esz_core_t* core)
{
    core->camera.is_locked = true;
}

void esz_register_event_callback(const esz_event_type event_type, esz_event_callback event_callback, esz_core_t* core)
{
    switch (event_type)
    {
        case EVENT_FINGERDOWN:
            core->event.finger_down_cb   = event_callback;
            break;
        case EVENT_FINGERUP:
            core->event.finger_up_cb     = event_callback;
            break;
        case EVENT_FINGERMOTION:
            core->event.finger_motion_cb = event_callback;
            break;
        case EVENT_KEYDOWN:
            core->event.key_down_cb      = event_callback;
            break;
        case EVENT_KEYUP:
            core->event.key_up_cb        = event_callback;
            break;
        case EVENT_MAP_LOADED:
            core->event.map_loaded_cb    = event_callback;
            break;
        case EVENT_MAP_UNLOADED:
            core->event.map_unloaded_cb  = event_callback;
            break;
        case EVENT_MULTIGESTURE:
            core->event.multi_gesture_cb = event_callback;
            break;
    }
}

void esz_set_active_player_actor(int32_t id, esz_core_t* core)
{
    if (! esz_is_map_loaded(core))
    {
        return;
    }

    core->map->active_player_actor_id = id;
}

void esz_set_next_player_animation(esz_core_t* core)
{
    if (! esz_is_map_loaded(core))
    {
        return;
    }

    if (core->map->entity)
    {
        if (core->map->entity[core->camera.target_actor_id].actor)
        {
            esz_actor_t* actor = core->map->entity[core->map->active_player_actor_id].actor;
            int32_t      id    = actor->current_animation + 1;

            if (id > actor->animation_count)
            {
                id = 1;
            }

            esz_set_player_animation(id, core);
        }
    }
}

void esz_set_player_animation(int32_t id, esz_core_t* core)
{
    if (!esz_is_map_loaded(core))
    {
        return;
    }

    if (core->map->entity)
    {
        if (core->map->entity[core->camera.target_actor_id].actor)
        {
            esz_actor_t* actor = core->map->entity[core->map->active_player_actor_id].actor;

            if (0 >= id || actor->animation_count < id)
            {
                return;
            }

            if (actor->current_animation != id)
            {
                actor->current_frame = 0;
                actor->current_animation = id;
            }
        }
    }
}

void esz_set_player_state(esz_state state, esz_core_t* core)
{
    if (!esz_is_map_loaded(core))
    {
        return;
    }

    if (core->map->entity)
    {
        if (core->map->entity[core->camera.target_actor_id].actor)
        {
            esz_actor_t* actor = core->map->entity[core->map->active_player_actor_id].actor;
            SET_STATE(actor->state, state);
        }
    }
}

void esz_set_camera_position(const double pos_x, const double pos_y, bool pos_is_relative, esz_window_t* window, esz_core_t* core)
{
    if (! esz_is_camera_locked(core))
    {
        if (pos_is_relative)
        {
            double time_factor = (window->time_since_last_frame * 1000.0);

            core->camera.pos_x += pos_x * time_factor;
            core->camera.pos_y += pos_y * time_factor;
        }
        else
        {
            core->camera.pos_x = pos_x;
            core->camera.pos_y = pos_y;
        }

        set_camera_boundaries_to_map_size(window, core);
    }
}

void esz_set_camera_target(const int32_t id, esz_core_t* core)
{
    core->camera.target_actor_id = id;
}

esz_status esz_set_zoom_level(const double factor, esz_window_t* window)
{
    window->zoom_level     = factor;
    window->logical_width  = (int32_t)((double)window->width  / factor);
    window->logical_height = (int32_t)((double)window->height / factor);

    if (0 > SDL_RenderSetLogicalSize(window->renderer, window->logical_width, window->logical_height))
    {
        plog_error("%s: %s.", __func__, SDL_GetError());
        return ESZ_WARNING;
    }
    else
    {
        return ESZ_OK;
    }
}

void esz_show_render_layer(esz_render_layer layer, esz_core_t* core)
{
    if (ESZ_RENDER_LAYER_MAX != layer)
    {
        CLR_STATE(core->debug, (uint32_t)layer);
    }
}

esz_status esz_show_scene(esz_window_t* window, esz_core_t* core)
{
    esz_status status;

    status = render_scene(window, core);
    if (ESZ_OK != status)
    {
        goto exit;
    }

    status = draw_scene(window, core);

exit:
    return status;
}

esz_status esz_toggle_fullscreen(esz_window_t* window)
{
    esz_status status = ESZ_OK;

    window->flags = SDL_GetWindowFlags(window->window);

    if (window->flags & SDL_WINDOW_FULLSCREEN_DESKTOP)
    {
        if (0 > SDL_SetWindowFullscreen(window->window, 0))
        {
            status = ESZ_WARNING;
        }
        SDL_SetWindowPosition(window->window, window->pos_x, window->pos_y);
    }
    else
    {
        SDL_GetWindowPosition(window->window, &window->pos_x, &window->pos_y);

        if (0 > SDL_SetWindowFullscreen(window->window, SDL_WINDOW_FULLSCREEN_DESKTOP))
        {
            status = ESZ_WARNING;
        }
    }

    if (ESZ_OK != status)
    {
        plog_error("%s: %s.", __func__, SDL_GetError());
    }

    return status;
}

void esz_trigger_player_action(esz_action action, esz_core_t* core)
{
    if (!esz_is_map_loaded(core))
    {
        return;
    }

    if (core->map->entity)
    {
        if (core->map->entity[core->camera.target_actor_id].actor)
        {
            esz_actor_t* actor = core->map->entity[core->map->active_player_actor_id].actor;
            SET_STATE(actor->action, action);
        }
    }
}

void esz_unload_map(esz_window_t* window, esz_core_t* core)
{
    esz_tiled_layer_t*  layer;
    esz_tiled_object_t* tiled_object = NULL;

    if (! esz_is_map_loaded(core))
    {
        plog_warn("No map has been loaded.");
        return;
    }
    core->is_map_loaded           = false;
    core->camera.target_actor_id = 0;

    for (int32_t index = 0; index < ESZ_MAP_LAYER_LEVEL_MAX; index += 1)
    {
        if (core->map->layer_texture[index])
        {
            SDL_DestroyTexture(core->map->layer_texture[index]);
            core->map->layer_texture[index] = NULL;
        }
    }

    for (int32_t index = 0; index < ESZ_RENDER_LAYER_MAX; index += 1)
    {
        if (core->map->render_target[index])
        {
            SDL_DestroyTexture(core->map->render_target[index]);
            core->map->render_target[index] = NULL;
        }
    }

    if (core->map->animated_tile_texture)
    {
        SDL_DestroyTexture(core->map->animated_tile_texture);
        core->map->animated_tile_texture = NULL;
    }

    // Free up allocated memory in reverse order
    // ------------------------------------------------------------------------

    // 9. Background
    // ------------------------------------------------------------------------

    if (0 < core->map->background.layer_count)
    {
        for (int32_t index = 0; index < core->map->background.layer_count; index += 1)
        {
            if (core->map->background.layer[index].texture)
            {
                SDL_DestroyTexture(core->map->background.layer[index].texture);
                core->map->background.layer[index].texture = NULL;
            }
        }
    }

    free(core->map->background.layer);

    // 8. Animated tiles
    // ------------------------------------------------------------------------

    free(core->map->animated_tile);

    // 7. Sprites
    // ------------------------------------------------------------------------

    if (0 < core->map->sprite_sheet_count)
    {
        for (int32_t index = 0; index < core->map->sprite_sheet_count; index += 1)
        {
            core->map->sprite[index].id = 0;

            if (core->map->sprite[index].texture)
            {
                SDL_DestroyTexture(core->map->sprite[index].texture);
                core->map->sprite[index].texture = NULL;
            }
        }
    }

    free(core->map->sprite);

    // 6. Tileset
    // ------------------------------------------------------------------------

    if (core->map->tileset_texture)
    {
        SDL_DestroyTexture(core->map->tileset_texture);
        core->map->tileset_texture = NULL;
    }

    // 5. Entities
    // ------------------------------------------------------------------------

    layer = get_head_layer(core->map->handle);
    while (layer)
    {
        if (is_tiled_layer_of_type(ESZ_OBJECT_GROUP, layer, core))
        {
            int32_t index = 0;
            tiled_object  = get_head_object(layer, core);

            while (tiled_object)
            {
                uint64_t      type_hash = esz_hash((const unsigned char*)get_object_type_name(tiled_object));
                esz_entity_t* entity    = &core->map->entity[index];

                switch (type_hash)
                {
                    case H_actor:
                    {
                        esz_actor_t** actor = &entity->actor;
                        free((*actor)->animation);
                        free((*actor));
                    }
                    break;
                }

                index        += 1;
                tiled_object = tiled_object->next;
            }
        }
        layer = layer->next;
    }
    free(core->map->entity);

    // 4. Paths and file locations
    // ------------------------------------------------------------------------

    free(core->map->path);

    // 3. Tile properties
    // ------------------------------------------------------------------------

    free(core->map->tile_properties);

    // 2. Tiled map
    // ------------------------------------------------------------------------

    unload_tiled_map(core);

    // 1. Map
    // ------------------------------------------------------------------------

    free(core->map);

    if (core->event.map_unloaded_cb)
    {
        core->event.map_unloaded_cb(window, core);
    }

    plog_info("Unload map.");
}

void esz_unlock_camera(esz_core_t* core)
{
    core->camera.is_locked = false;
}

void esz_update_core(esz_window_t* window, esz_core_t* core)
{
    double delta_time = 0.0;

    poll_events(window, core);

    window->time_b = window->time_a;
    window->time_a = SDL_GetTicks();

    if (window->time_a > window->time_b)
    {
        window->time_a = window->time_b;
    }

    delta_time                     = (double)(window->time_b - window->time_a) / 1000.0;
    window->time_since_last_frame  = 1000.0 / (double)window->refresh_rate - delta_time;

    if (! window->vsync_enabled)
    {
        double delay = floor(window->time_since_last_frame);
        SDL_Delay((uint32_t)delay);
    }

    window->time_since_last_frame /= 1000.0;

    if (! esz_is_map_loaded(core))
    {
        return;
    }

    move_camera_to_target(window, core);
    update_entities(window, core);
}

DISABLE_WARNING_POP

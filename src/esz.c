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

DISABLE_WARNING_PUSH
DISABLE_WARNING_SPECTRE_MITIGATION

// Private function prototypes
// ----------------------------------------------------------------------------

static esz_status create_and_set_render_target(SDL_Texture** target, esz_window_t* window);
static esz_status draw_scene(esz_window_t* window, esz_core_t* core);
static esz_status init_animated_tiles(esz_core_t* core);
static esz_status init_background(esz_window_t* window, esz_core_t* core);
static esz_status init_entities(esz_core_t* core);
static esz_status init_sprites(esz_window_t* window, esz_core_t* core);
static bool       is_camera_at_horizontal_boundary(esz_core_t* core);
static esz_status load_background_layer(int32_t index, esz_window_t* window, esz_core_t* core);
static esz_status load_texture_from_file(const char* file_name, SDL_Texture** texture, esz_window_t* window);
static esz_status load_texture_from_memory(const unsigned char* buffer, const int length, SDL_Texture** texture, esz_window_t* window);
static void       move_camera_to_target(esz_window_t* window, esz_core_t* core);
static void       poll_events(esz_window_t* window, esz_core_t* core);
static esz_status render_actors(int32_t level, esz_window_t* window, esz_core_t* core);
static esz_status render_background(esz_window_t* window, esz_core_t* core);
static esz_status render_background_layer(int32_t index, esz_window_t* window, esz_core_t* core);
static esz_status render_map(int32_t level, esz_window_t* window, esz_core_t* core);
static esz_status render_scene(esz_window_t* window, esz_core_t* core);
static void       set_camera_boundaries_to_map_size(esz_window_t* window, esz_core_t* core);
static void       update_bounding_box(esz_entity_t* entity);
static void       update_entities(esz_window_t* window, esz_core_t* core);

// Public functions
// ----------------------------------------------------------------------------

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
    if (!esz_is_map_loaded(core))
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
    int32_t         index;
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
    for (index = 0; index < SDL_GetNumRenderDrivers(); index += 1)
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
    int32_t             index;
    esz_tiled_layer_t*  layer;
    esz_tiled_object_t* tiled_object = NULL;

    if (! esz_is_map_loaded(core))
    {
        plog_warn("No map has been loaded.");
        return;
    }
    core->is_map_loaded           = false;
    core->camera.target_actor_id = 0;

    for (index = 0; index < ESZ_MAP_LAYER_LEVEL_MAX; index += 1)
    {
        if (core->map->layer_texture[index])
        {
            SDL_DestroyTexture(core->map->layer_texture[index]);
            core->map->layer_texture[index] = NULL;
        }
    }

    for (index = 0; index < ESZ_RENDER_LAYER_MAX; index += 1)
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
        for (index = 0; index < core->map->background.layer_count; index += 1)
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
        for (index = 0; index < core->map->sprite_sheet_count; index += 1)
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

    index = 0;
    layer = get_head_layer(core->map->handle);
    while (layer)
    {
        if (is_tiled_layer_of_type(ESZ_OBJECT_GROUP, layer, core))
        {
            tiled_object = get_head_object(layer, core);

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

// Private functions (for internal use only)
// ----------------------------------------------------------------------------

static esz_status create_and_set_render_target(SDL_Texture** target, esz_window_t* window)
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

static esz_status draw_scene(esz_window_t* window, esz_core_t* core)
{
    SDL_Rect dst;
    int32_t  index;

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

    for (index = 0; index < ESZ_RENDER_LAYER_MAX; index += 1)
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

static esz_status init_animated_tiles(esz_core_t* core)
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

                    // tbd.


                    #ifdef USE_LIBTMX
                    if (core->map->handle->tiles[gid])
                    {
                        if (core->map->handle->tiles[gid]->animation)
                        {
                            animated_tile_count += 1;
                        }
                    }

                    #else // (cute_tiled.h)
                    {
                        cute_tiled_tileset_t*         tileset   = core->map->handle->tilesets;
                        cute_tiled_tile_descriptor_t* tile      = tileset->tiles;
                        int32_t                       first_gid = get_first_gid(core->map->handle);
                        int32_t                       local_id  = gid - first_gid;

                        if (gid)
                        {
                            while (tile)
                            {
                                if (tile->tile_index == local_id)
                                {
                                    if (tile->animation)
                                    {
                                        animated_tile_count += 1;
                                        break;
                                    }
                                }
                                tile = tile->next;
                            }
                        }
                    }
                    #endif
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

    plog_info("Initialise %u animated tile(s).", animated_tile_count);
    return ESZ_OK;
}

static esz_status init_background(esz_window_t* window, esz_core_t* core)
{
    char property_name[21] = { 0 };
    bool search_is_running = true;

    core->map->background.layer_shift = esz_get_decimal_map_property(H_background_layer_shift, core);
    core->map->background.velocity    = esz_get_decimal_map_property(H_background_constant_velocity, core);

    if (0.0 < core->map->background.velocity)
    {
        core->map->background.velocity_is_constant = true;
    }

    if (esz_get_boolean_map_property(H_background_is_top_aligned, core))
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

        if (esz_get_string_map_property(esz_hash((const unsigned char*)property_name), core))
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

    plog_info("Initialise parallax-scrolling background with %u layers.", core->map->background.layer_count);

    return ESZ_OK;
}

static esz_status init_entities(esz_core_t* core)
{
    esz_tiled_layer_t*  layer         = get_head_layer(core->map->handle);
    esz_tiled_object_t* tiled_object  = NULL;
    int32_t             index         = 0;
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

    plog_info("Initialise %u entities:", core->map->entity_count);

    layer = get_head_layer(core->map->handle);
    while (layer)
    {
        if (is_tiled_layer_of_type(ESZ_OBJECT_GROUP, layer, core))
        {
            tiled_object = get_head_object(layer, core);
            while (tiled_object)
            {
                uint64_t              type_hash  = esz_hash((const unsigned char*)get_object_type_name(tiled_object));
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
                            player_found = true;
                            esz_lock_camera(core);
                            esz_set_active_player_actor(index, core);
                            esz_set_camera_target(index, core);

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

                                if (get_boolean_property(esz_hash((const unsigned char*)property_name), properties, prop_cnt, core))
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

                            for (index = 0; index < (*actor)->animation_count; index += 1)
                            {
                                stbsp_snprintf(property_name, 26, "animation_%u_first_frame", index + 1);
                                (*actor)->animation[index].first_frame =
                                    get_integer_property(esz_hash((const unsigned char*)property_name), properties, prop_cnt, core);

                                if (0 == (*actor)->animation[index].first_frame)
                                {
                                    (*actor)->animation[index].first_frame = 1;
                                }

                                stbsp_snprintf(property_name, 26, "animation_%u_fps", index + 1);
                                (*actor)->animation[index].fps =
                                    get_integer_property(esz_hash((const unsigned char*)property_name), properties, prop_cnt, core);

                                stbsp_snprintf(property_name, 26, "animation_%u_length", index + 1);
                                (*actor)->animation[index].length =
                                    get_integer_property(esz_hash((const unsigned char*)property_name), properties, prop_cnt, core);

                                stbsp_snprintf(property_name, 26, "animation_%u_offset_y", index + 1);
                                (*actor)->animation[index].offset_y =
                                    get_integer_property(esz_hash((const unsigned char*)property_name), properties, prop_cnt, core);
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

static esz_status init_sprites(esz_window_t* window, esz_core_t* core)
{
    char    property_name[17] = { 0 };
    bool    search_is_running = true;
    int32_t index;

    core->map->sprite_sheet_count = 0;

    while (search_is_running)
    {
        stbsp_snprintf(property_name, 17, "sprite_sheet_%u", core->map->sprite_sheet_count + 1);

        if (esz_get_string_map_property(esz_hash((const unsigned char*)property_name), core))
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

    for (index = 0; index < core->map->sprite_sheet_count; index += 1)
    {
        stbsp_snprintf(property_name, 17, "sprite_sheet_%u", index + 1);

        const char* file_name = esz_get_string_map_property(esz_hash((const unsigned char*)property_name), core);

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

static bool is_camera_at_horizontal_boundary(esz_core_t* core)
{
    return core->camera.is_at_horizontal_boundary;
}

static esz_status load_background_layer(int32_t index, esz_window_t* window, esz_core_t* core)
{
    esz_status   status        = ESZ_OK;
    SDL_Texture* image_texture = NULL;
    SDL_Rect     dst;
    int32_t      image_width;
    int32_t      image_height;
    int32_t      source_length = 0;
    double       layer_width_factor;
    char         property_name[21] = { 0 };
    char*        background_layer_image_source;

    stbsp_snprintf(property_name, 21, "background_layer_%u", index + 1);

    const char* file_name = esz_get_string_map_property(esz_hash((const unsigned char*)property_name), core);
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

/* Based on
 * https://wiki.libsdl.org/SDL_CreateRGBSurfaceWithFormatFrom#Code_Examples
 */
static esz_status load_texture_from_file(const char* file_name, SDL_Texture** texture, esz_window_t* window)
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

static esz_status load_texture_from_memory(const unsigned char* buffer, const int length, SDL_Texture** texture, esz_window_t* window)
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

static void move_camera_to_target(esz_window_t* window, esz_core_t* core)
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

static void poll_events(esz_window_t* window, esz_core_t* core)
{
    while (0 != SDL_PollEvent(&core->event.handle))
    {
        switch (core->event.handle.type)
        {
            case SDL_QUIT:
                esz_deactivate_core(core);
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

static esz_status render_actors(int32_t level, esz_window_t* window, esz_core_t* core)
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

static esz_status render_background(esz_window_t* window, esz_core_t* core)
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

static esz_status render_background_layer(int32_t index, esz_window_t* window, esz_core_t* core)
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

static esz_status render_map(int32_t level, esz_window_t* window, esz_core_t* core)
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
                                set_animated_tile_position(gid, dst.x, dst.y, core->map);
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

static esz_status render_scene(esz_window_t* window, esz_core_t* core)
{
    esz_status status = ESZ_OK;
    int32_t    index;

    status = render_background(window, core);
    if (ESZ_OK != status)
    {
        return status;
    }

    for (index = 0; index < ESZ_MAP_LAYER_LEVEL_MAX; index  += 1)
    {
        status = render_map(index, window, core);
        if (ESZ_OK != status)
        {
            return status;
        }
    }

    for (index = 0; index < ESZ_ACTOR_LAYER_LEVEL_MAX; index += 1)
    {
        status = render_actors(index, window, core);
        if (ESZ_OK != status)
        {
            return status;
        }
    }

    return status;
}

static void set_camera_boundaries_to_map_size(esz_window_t* window, esz_core_t* core)
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

static void update_bounding_box(esz_entity_t* entity)
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

static void update_entities(esz_window_t* window, esz_core_t* core)
{
    esz_tiled_layer_t* layer;
    int32_t            index = 0;

    if (! esz_is_map_loaded(core))
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
                uint64_t      type_hash = esz_hash((const unsigned char*)get_object_type_name(tiled_object));
                esz_entity_t* entity    = &core->map->entity[index];

                switch (type_hash)
                {
                    case H_actor:
                    {
                        esz_actor_t** actor                 = &entity->actor;
                        uint32_t*     state                 = &(*actor)->state;
                        double        acceleration_x        = (*actor)->acceleration    * core->map->meter_in_pixel;
                        double        acceleration_y        = core->map->meter_in_pixel * core->map->meter_in_pixel;
                        double        time_since_last_frame = esz_get_time_since_last_frame(window);
                        double        distance_x            = acceleration_x * time_since_last_frame * time_since_last_frame;
                        double        distance_y            = acceleration_y * time_since_last_frame * time_since_last_frame;

                        // Adjust mutually exclusive states
                        // ----------------------------------------------------
                        if (IS_STATE_SET(*state, STATE_IN_MIDGROUND))
                        {
                            CLR_STATE((*actor)->state, STATE_IN_BACKGROUND);
                            CLR_STATE((*actor)->state, STATE_IN_FOREGROUND);
                        }
                        else if (IS_STATE_SET(*state, STATE_IN_FOREGROUND))
                        {
                            CLR_STATE((*actor)->state, STATE_IN_BACKGROUND);
                            CLR_STATE((*actor)->state, STATE_IN_MIDGROUND);
                        }
                        else if (IS_STATE_SET(*state, STATE_IN_BACKGROUND))
                        {
                            CLR_STATE((*actor)->state, STATE_IN_MIDGROUND);
                            CLR_STATE((*actor)->state, STATE_IN_FOREGROUND);
                        }
                        else
                        {
                            SET_STATE((*actor)->state, STATE_IN_MIDGROUND);
                        }

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

DISABLE_WARNING_POP

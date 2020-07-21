// Spdx-License-Identifier: MIT
/**
 * @file    esz.c
 * @brief   eszFW game engine
 * @details A cross-platform game engine written in C
 */

#include <stdbool.h>
#include <stdint.h>
#include <SDL.h>
#include <cwalk.h>
#include "esz.h"
#include "esz_types.h"

#ifdef USE_LIBTMX
    #include <tmx.h>
#elif  USE_CUTE_TILED
    #define CUTE_TILED_IMPLEMENTATION
    #include <cute_tiled.h>
#endif

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

enum hash_table {
    H_animated_tile_fps            = 0xde2debdd,
    H_background_is_top_aligned    = 0x00773f85,
    H_background_layer_count       = 0xc1378329,
    H_background_layer_shift       = 0xc255007e,
    H_background_constant_velocity = 0x5ad8b7fc,
    H_gravitation                  = 0x12404d2d,
    H_is_in_foreground             = 0xd43eb8f1,
    H_meter_in_pixel               = 0x4a407cf9,
    H_objectgroup                  = 0x970be349,
    H_opengl                       = 0x12ef9eea,
    H_sprite_sheet_count           = 0xcc08d6fc,
    H_tilelayer                    = 0x0e844fb0,

};

/**********************
 * Private Prototypes *
 **********************/

static void          count_animated_tiles(int32_t* animated_tile_count, esz_core_t* core);
static void          count_objects(int32_t* object_count, esz_core_t* core);
static void          count_tile_layers(int32_t* layer_count, esz_core_t* core);
static esz_status    draw_background(esz_window_t* window, esz_core_t* core);
static esz_status    draw_logo(esz_window_t *window);
static esz_status    draw_map(const esz_layer_type layer_type, esz_window_t *window, esz_core_t *core);
static bool          get_boolean_property(const unsigned long name_hash, esz_property_handle_t* properties, esz_core_t* core);
static uint16_t      get_camera_target(esz_core_t* core);
static esz_status    init_animated_tiles(esz_core_t* core);
static esz_status    init_background(esz_window_t* window, esz_core_t* core);
static esz_status    init_objects(esz_core_t* core);
static esz_status    init_sprites(esz_window_t* window, esz_core_t* core);
static bool          is_camera_locked(esz_core_t* core);
static bool          is_camera_at_horizontal_boundary(esz_core_t* core);
static esz_status    load_background_layer(int32_t index, esz_window_t* window, esz_core_t* core);
static void          load_property(const unsigned long name_hash, esz_property_handle_t* properties, esz_core_t* core);
static esz_status    load_texture_from_file(const char* file_name, SDL_Texture** texture, esz_window_t* window);
static esz_status    load_texture_from_memory(const unsigned char* buffer, const int length, SDL_Texture** texture, esz_window_t* window);
static void          move_camera_to_target(esz_core_t* core);
static void          poll_events(esz_window_t* window, esz_core_t* core);
static int           remove_gid_flip_bits(int gid);
static esz_status    render_background(esz_window_t* window, esz_core_t* core);
static esz_status    render_background_layer(int32_t index, esz_window_t* window, esz_core_t* core);
static esz_status    render_map(const esz_layer_type layer_type, esz_window_t *window, esz_core_t* core);
static double        round_(double number);
static void          set_camera_boundaries_to_map_size(esz_window_t* window, esz_core_t* core);
static void          set_camera_target(const uint16_t target_entity_id, esz_core_t* core);

#ifdef USE_LIBTMX
static void          tmxlib_store_property(tmx_property* property, void* core);
#endif

/********************
 * Public functions *
 ********************/

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

esz_status esz_create_window(const char* window_title, esz_window_config_t* config, esz_window_t** window)
{
    esz_status      status = ESZ_OK;
    SDL_DisplayMode display_mode;
    uint32_t        renderer_flags = SDL_RENDERER_ACCELERATED | SDL_RENDERER_TARGETTEXTURE;

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

    *window = SDL_calloc(1, sizeof(struct esz_window));
    if (! *window)
    {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "%s: error allocating memory.\n", __func__);
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

    /*******************************
     * Initialise Video sub-system *
     *******************************/

    if (0 > SDL_Init(SDL_INIT_VIDEO))
    {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "%s: %s.\n", __func__, SDL_GetError());
        status = ESZ_ERROR_CRITICAL;
        goto exit;
    }

    if (0 != SDL_GetCurrentDisplayMode(0, &display_mode))
    {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "%s: %s.\n", __func__, SDL_GetError());
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
            SDL_Log("Couldn't determine the monitor's refresh rate: VSync disabled.\n");
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
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "%s: %s.\n", __func__, SDL_GetError());
        status = ESZ_ERROR_CRITICAL;
        goto exit;
    }

    SDL_GetWindowSize((*window)->window, &(*window)->width, &(*window)->height);

    (*window)->zoom_level         = (double)(*window)->height / (double)(*window)->logical_height;
    (*window)->initial_zoom_level = (*window)->zoom_level;

    /* Get index of opengl rendering driver and create 2D rendering
     * context.
     */
    for (int driver_index = 0; driver_index < SDL_GetNumRenderDrivers(); driver_index += 1)
    {
        SDL_RendererInfo renderer_info = { 0 };
        SDL_GetRenderDriverInfo(driver_index, &renderer_info);

        if (H_opengl != esz_hash((const unsigned char*)renderer_info.name))
        {
            continue;
        }
        else
        {
            (*window)->renderer = SDL_CreateRenderer((*window)->window, driver_index, renderer_flags);

            if (! (*window)->renderer)
            {
                SDL_LogError(SDL_LOG_CATEGORY_ERROR, "%s: %s.\n", __func__, SDL_GetError());
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

        SDL_Log("opengl not found: use default rendering driver: %s.\n", renderer_info.name);

        (*window)->renderer = SDL_CreateRenderer((*window)->window, -1, renderer_flags);

        if (! (*window)->renderer)
        {
            SDL_LogError(SDL_LOG_CATEGORY_ERROR, "%s: %s.\n", __func__, SDL_GetError());
            status = ESZ_ERROR_CRITICAL;
            goto exit;
        }
    }

    SDL_Log(
        "Setting up window at resolution %dx%d @ %d Hz.\n",
        (*window)->width,
        (*window)->height,
        (*window)->refresh_rate);

    esz_set_zoom_level((*window)->zoom_level, *window);
    SDL_Log("Set initial zoom-level to factor %f.\n", (*window)->zoom_level);

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
    SDL_free(core);
    SDL_Log("Destroy engine core.\n");
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

    SDL_free(window);
    SDL_Log("Quitting.\n");

    SDL_Quit();
}

esz_status esz_draw_frame(esz_window_t* window, esz_core_t* core)
{
    esz_status status;
    double     delta_time = 0.0;
    double     delay;

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
        delay = SDL_floor(window->time_since_last_frame);
        SDL_Delay((uint32_t)delay);
    }

    window->time_since_last_frame /= 1000.0;

    status = render_background(window, core);
    if (ESZ_OK != status)
    {
        goto exit;
    }

    status = render_map(ESZ_BG, window, core);
    if (ESZ_OK != status)
    {
        goto exit;
    }

    status = render_map(ESZ_FG, window, core);
    if (ESZ_OK != status)
    {
        goto exit;
    }

    if (0 > SDL_SetRenderTarget(window->renderer, NULL))
    {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "%s: %s.\n", __func__, SDL_GetError());
    }

    if (core->map.is_loaded)
    {
        draw_background(window, core);
        draw_map(ESZ_BG, window, core);
        // tbd.
        draw_map(ESZ_FG, window, core);
    }
    else
    {
        draw_logo(window);
    }

    SDL_RenderPresent(window->renderer);
    SDL_RenderClear(window->renderer);

exit:
    return status;
}

bool esz_get_boolean_map_property(const unsigned long name_hash, esz_core_t* core)
{
    load_property(name_hash, core->map.handle->properties, core);
    return core->map.boolean_property;
}

double esz_get_decimal_map_property(const unsigned long name_hash, esz_core_t* core)
{
    load_property(name_hash, core->map.handle->properties, core);
    return core->map.decimal_property;
}

int32_t esz_get_integer_map_property(const unsigned long name_hash, esz_core_t* core)
{
    load_property(name_hash, core->map.handle->properties, core);
    return core->map.integer_property;
}

const char* esz_get_string_map_property(const unsigned long name_hash, esz_core_t* core)
{
    load_property(name_hash, core->map.handle->properties, core);
    return core->map.string_property;
}

const uint8_t* esz_get_keyboard_state(void)
{
    return SDL_GetKeyboardState(NULL);
}

const unsigned long esz_hash(const unsigned char* name)
{
    unsigned long hash = 5381;
    int c;

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

double esz_get_time_since_last_frame(esz_window_t* window)
{
    return window->time_since_last_frame;
}

esz_status esz_init_core(esz_core_t** core)
{
    *core = SDL_calloc(1, sizeof(struct esz_core));
    if (! *core)
    {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "%s: error allocating memory.\n", __func__);
        return ESZ_ERROR_CRITICAL;
    }

    (*core)->is_active = true;

    return ESZ_OK;
}

bool esz_is_core_active(esz_core_t* core)
{
    return core->is_active;
}

bool esz_is_map_loaded(esz_core_t* core)
{
    return core->map.is_loaded;
}

esz_status esz_load_map(const char* map_file_name, esz_window_t* window, esz_core_t* core)
{
    char* tileset_image_source;

    if (esz_is_map_loaded(core))
    {
        SDL_Log("A map has already been loaded: unload map first.\n");
        return ESZ_WARNING;
    }
    core->map.is_loaded = true;

    /******************************************
     * Load map and allocate required memory: *
     ******************************************/

    /**************
     * 1 Map file *
     **************/

    #ifdef USE_LIBTMX

    core->map.handle = (esz_map_handle_t*)tmx_load(map_file_name);
    if (! core->map.handle)
    {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "%s: %s.\n", __func__, tmx_strerr());
        return ESZ_WARNING;
    }

    #elif USE_CUTE_TILED

    core->map.handle = (esz_map_handle_t*)cute_tiled_load_map_from_file(map_file_name, NULL);
    if (! core->map.handle)
    {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "%s: %s.\n", __func__, cute_tiled_error_reason);
        return ESZ_WARNING;
    }

    {
        cute_tiled_layer_t* layer = core->map.handle->layers;
        while (layer)
        {
            if (H_tilelayer == esz_hash((const unsigned char*)layer->type.ptr) && ! core->map.hash_id_tilelayer)
            {
                core->map.hash_id_tilelayer = layer->type.hash_id;
                SDL_Log("Set hash ID for tile layer: %llu\n", core->map.hash_id_tilelayer);
            }
            else if (H_objectgroup == esz_hash((const unsigned char*)layer->type.ptr) && ! core->map.hash_id_objectgroup)
            {
                core->map.hash_id_objectgroup = layer->type.hash_id;
                SDL_Log("Set hash ID for object group: %llu\n", core->map.hash_id_objectgroup);
            }

            layer = layer->next;
        }
    }
    #endif

    /******************************
     * 2 Paths and file locations *
     ******************************/

    // Map file path.
    core->map.path = SDL_calloc(1, (size_t)(SDL_strlen(map_file_name) + 1));
    if (! core->map.path)
    {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "%s: error allocating memory.\n", __func__);
        esz_unload_map(window, core);
        return ESZ_WARNING;
    }

    cwk_path_get_dirname(map_file_name, &core->map.path_length);
    SDL_strlcpy(core->map.path, map_file_name, core->map.path_length + 1);

    // Tileset image source.
    {
        size_t source_length;

        #ifdef USE_LIBTMX
        int    first_gid = core->map.handle->ts_head->firstgid;
        char*  ts_path;
        size_t ts_path_length;

        cwk_path_get_dirname(core->map.handle->ts_head->source, &ts_path_length);

        ts_path = SDL_calloc(1, ts_path_length + 1);
        if (! ts_path)
        {
            SDL_LogError(SDL_LOG_CATEGORY_ERROR, "%s: error allocating memory.\n", __func__);
            esz_unload_map(window, core);
            return ESZ_WARNING;
        }

        source_length  = SDL_strlen(core->map.path);
        source_length += SDL_strlen(core->map.handle->tiles[first_gid]->tileset->image->source);
        source_length += ts_path_length + 1;

        tileset_image_source = SDL_calloc(1, source_length);
        if (! tileset_image_source)
        {
            SDL_LogError(SDL_LOG_CATEGORY_ERROR, "%s: error allocating memory.\n", __func__);
            SDL_free(ts_path);
            esz_unload_map(window, core);
            return ESZ_WARNING;
        }

        /* The tileset image source is stored relatively to the tileset
         * file but because we only know the location of the tileset
         * file relatively to the map file, we need to adjust the path
         * accordingly.  It's a hack, but it works.
         */

        SDL_strlcpy(ts_path, core->map.handle->ts_head->source, ts_path_length + 1);
        SDL_snprintf(tileset_image_source, source_length, "%s%s%s",
                     core->map.path,
                     ts_path,
                     core->map.handle->tiles[first_gid]->tileset->image->source);

        SDL_free(ts_path);

        #elif USE_CUTE_TILED
        if (! core->map.handle->tilesets)
        {
            SDL_LogError(SDL_LOG_CATEGORY_ERROR, "%s: no embedded tileset found.\n", __func__);
            SDL_free(tileset_image_source);
            esz_unload_map(window, core);
            return ESZ_WARNING;
        }

        source_length  = SDL_strlen(core->map.path);
        source_length += SDL_strlen(core->map.handle->tilesets->image.ptr);
        source_length += 1;

        tileset_image_source = SDL_calloc(1, source_length);
        if (! tileset_image_source)
        {
            SDL_LogError(SDL_LOG_CATEGORY_ERROR, "%s: error allocating memory.\n", __func__);
            SDL_free(tileset_image_source);
            esz_unload_map(window, core);
            return ESZ_WARNING;
        }

        SDL_snprintf(tileset_image_source, source_length, "%s%s",
                     core->map.path,
                     core->map.handle->tilesets->image.ptr);

        #endif // USE_CUTE_TILED


    }

    /*************
     * 3 Objects *
     *************/

    if (ESZ_OK != init_objects(core))
    {
        SDL_free(tileset_image_source);
        esz_unload_map(window, core);
        return ESZ_WARNING;
    }

    /**************
     * 4 Entities *
     **************/

    // tbd.

    /*************
     * 5 Tileset *
     *************/

    if (tileset_image_source)
    {
        if (ESZ_OK != load_texture_from_file(tileset_image_source, &core->map.tileset_texture, window))
        {
            SDL_LogError(SDL_LOG_CATEGORY_ERROR, "%s: Error loading image '%s'.\n", __func__, tileset_image_source);
            SDL_free(tileset_image_source);
            esz_unload_map(window, core);
            return ESZ_WARNING;
        }
    }
    else
    {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "%s: Could not determine location of tileset image.\n", __func__);
        esz_unload_map(window, core);
        return ESZ_WARNING;
    }

    SDL_free(tileset_image_source);

    /*************
     * 6 Sprites *
     *************/

    if (ESZ_OK != init_sprites(window, core))
    {
        esz_unload_map(window, core);
        return ESZ_WARNING;
    }

    /********************
     * 7 Animated tiles *
     ********************/

    if (ESZ_OK != init_animated_tiles(core))
    {
        esz_unload_map(window, core);
        return ESZ_WARNING;
    }

    /*****************
     * 8 Backgrounds *
     *****************/

    if (ESZ_OK != init_background(window, core))
    {
        esz_unload_map(window, core);
        return ESZ_WARNING;
    }

    /***********************
     * Set base attributes *
     ***********************/

    core->map.animated_tile_index = 0;

    #ifdef USE_LIBTMX
    core->map.height = core->map.handle->height * core->map.handle->tile_height;
    core->map.width  = core->map.handle->width  * core->map.handle->tile_width;

    #elif  USE_CUTE_TILED
    core->map.height = core->map.handle->height * core->map.handle->tileheight;
    core->map.width  = core->map.handle->width  * core->map.handle->tilewidth;

    #endif

    core->map.gravitation    = esz_get_decimal_map_property(H_gravitation, core);
    core->map.meter_in_pixel = esz_get_integer_map_property(H_meter_in_pixel, core);

    SDL_Log(
        "Set gravitational constant to %f (g*%dpx/s^2).\n",
        core->map.gravitation, core->map.meter_in_pixel);

    core->map.animated_tile_fps = esz_get_integer_map_property(H_animated_tile_fps, core);
    if (core->map.animated_tile_fps > window->refresh_rate)
    {
        // It can't update faster anyway.
        core->map.animated_tile_fps = window->refresh_rate;
    }

    if (core->event.map_loaded_cb)
    {
        core->event.map_loaded_cb(window, core);
    }

    SDL_Log(
        "Load map file: %s containing %d object(s).\n",
        map_file_name, core->map.object_count);

    return ESZ_OK;
}

void esz_lock_camera(esz_core_t* core)
{
    SDL_Log("Enable camera lock.\n");
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

void esz_set_camera_position(const double pos_x, const double pos_y, bool pos_is_relative, esz_window_t* window, esz_core_t* core)
{
    if (! is_camera_locked(core))
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

esz_status esz_set_zoom_level(const double factor, esz_window_t* window)
{
    window->zoom_level     = factor;
    window->logical_width  = (int32_t)((double)window->width  / factor);
    window->logical_height = (int32_t)((double)window->height / factor);

    if (0 > SDL_RenderSetLogicalSize(window->renderer, window->logical_width, window->logical_height))
    {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "%s: %s.\n", __func__, SDL_GetError());
        return ESZ_WARNING;
    }
    else
    {
        return ESZ_OK;
    }
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
        SDL_Log("Set window to windowed mode.\n");
    }
    else
    {
        SDL_GetWindowPosition(window->window, &window->pos_x, &window->pos_y);

        if (0 > SDL_SetWindowFullscreen(window->window, SDL_WINDOW_FULLSCREEN_DESKTOP))
        {
            status = ESZ_WARNING;
        }
        SDL_Log("Set window to fullscreen mode.\n");
    }

    if (ESZ_OK != status)
    {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "%s: %s.\n", __func__, SDL_GetError());
    }

    return status;
}

void esz_unload_map(esz_window_t* window, esz_core_t* core)
{
    if (! esz_is_map_loaded(core))
    {
        SDL_Log("No map has been loaded.\n");
        return;
    }

    /*************************
     * Reset base attributes *
     *************************/

    core->map.animated_tile_fps               = 0;
    core->map.animated_tile_index             = 0;
    core->map.background.alignment            = ESZ_BOT;
    core->map.background.direction            = ESZ_LEFT;
    core->map.background.layer_count          = 0;
    core->map.background.layer_shift          = 0.0;
    core->map.background.velocity             = 0.0;
    core->map.background.velocity_is_constant = false;
    core->map.boolean_property                = false;
    core->map.decimal_property                = 0.0;
    core->map.gravitation                     = 0.0;

    #ifndef USE_LIBTMX
    core->map.hash_id_objectgroup             = 0;
    core->map.hash_id_tilelayer               = 0;
    #endif

    core->map.height                          = 0;
    core->map.integer_property                = 0;
    core->map.is_loaded                       = false;
    core->map.meter_in_pixel                  = 0;
    core->map.object_count                    = 0;
    core->map.pos_x                           = 0.0;
    core->map.pos_y                           = 0.0;
    core->map.sprite_sheet_count              = 0;
    core->map.string_property                 = NULL;
    core->map.time_since_last_anim_frame      = 0.0;
    core->map.width                           = 0;

    /*******************************************************
     * Destroy textures that where created in render_map() *
     *******************************************************/

    for (int layer = 0; ESZ_LAYER_MAX > layer; layer += 1)
    {
        if (core->map.map_layer[layer])
        {
            SDL_DestroyTexture(core->map.map_layer[layer]);
            core->map.map_layer[layer] = NULL;
        }

        if (core->map.render_target[layer])
        {
            SDL_DestroyTexture(core->map.render_target[layer]);
            core->map.render_target[layer] = NULL;
        }
    }

    #ifdef USE_LIBTMX
    if (core->map.animated_tile_texture)
    {
        SDL_DestroyTexture(core->map.animated_tile_texture);
        core->map.animated_tile_texture = NULL;
    }
    #endif

    /**********************************************
     * Free up allocated memory in reverse order: *
     **********************************************/

    /*****************
     * 8 Backgrounds *
     *****************/

    if (0 < core->map.background.layer_count)
    {
        for (int32_t index = 0; index < core->map.background.layer_count; index += 1)
        {
            if (core->map.background.layer[index].render_target)
            {
                SDL_DestroyTexture(core->map.background.layer[index].render_target);
                core->map.background.layer[index].render_target = NULL;
            }
        }
    }

    SDL_free(core->map.background.layer);

    if (core->map.background.render_target)
    {
        SDL_DestroyTexture(core->map.background.render_target);
        core->map.background.render_target = NULL;
    }

    /********************
     * 7 Animated tiles *
     ********************/

    SDL_free(core->map.animated_tile);

    /*************
     * 6 Sprites *
     *************/

    if (0 < core->map.sprite_sheet_count)
    {
        for (int32_t index = 0; index < core->map.sprite_sheet_count; index += 1)
        {
            if (core->map.sprite[index].render_target)
            {
                SDL_DestroyTexture(core->map.sprite[index].render_target);
                core->map.sprite[index].render_target = NULL;
            }
        }
    }

    SDL_free(core->map.sprite);

    /*************
     * 5 Tileset *
     *************/

    if (core->map.tileset_texture)
    {
        SDL_DestroyTexture(core->map.tileset_texture);
        core->map.tileset_texture = NULL;
    }

    /**************
     * 4 Entities *
     **************/

    // tbd.

    /*************
     * 3 Objects *
     *************/

    SDL_free(core->map.object);

    /******************************
     * 2 Paths and file locations *
     ******************************/

    SDL_free(core->map.path);

    /**************
     * 1 Map file *
     **************/

    #ifdef USE_LIBTMX
    if (core->map.handle)
    {
        tmx_map_free(core->map.handle);
    }

    #elif  USE_CUTE_TILED
    if (core->map.handle)
    {
        cute_tiled_free_map(core->map.handle);
    }
    #endif

    if (core->event.map_unloaded_cb)
    {
        core->event.map_unloaded_cb(window, core);
    }

    SDL_Log("Unload map.\n");
}

void esz_unlock_camera(esz_core_t* core)
{
    SDL_Log("Disable camera lock.\n");
    core->camera.is_locked = false;
}

void esz_update_core(esz_window_t* window, esz_core_t* core)
{
    poll_events(window, core);
}

/*********************
 * Private Functions *
 *********************/

static void count_animated_tiles(int32_t* animated_tile_count, esz_core_t* core)
{
    #ifdef USE_LIBTMX
    tmx_layer* layer = core->map.handle->ly_head;

    #elif  USE_CUTE_TILED
    cute_tiled_layer_t* layer = core->map.handle->layers;

    #endif

    *animated_tile_count = 0;

    while (layer)
    {
        bool is_tile_layer = false;

        #ifdef USE_LIBTMX
        if (L_LAYER == layer->type)
        {
            is_tile_layer = true;
        }
        #elif  USE_CUTE_TILED
        if (core->map.hash_id_tilelayer == layer->type.hash_id)
        {
            is_tile_layer = true;
        }
        #endif

        if (is_tile_layer && layer->visible)
        {
            for (int index_height = 0; index_height < core->map.handle->height; index_height += 1)
            {
                for (int index_width = 0; index_width < core->map.handle->width; index_width += 1)
                {
                    int gid;

                    #ifdef USE_LIBTMX
                    uint32_t* layer_content = layer->content.gids;
                    #elif  USE_CUTE_TILED
                    int*      layer_content = layer->data;
                    #endif

                    gid = remove_gid_flip_bits(layer_content[(index_height * core->map.handle->width) + index_width]);

                    #ifdef USE_LIBTMX
                    if (core->map.handle->tiles[gid])
                    {
                        if (core->map.handle->tiles[gid]->animation)
                        {
                            *animated_tile_count += 1;
                        }
                    }

                    #elif USE_CUTE_TILED
                    {
                        cute_tiled_tileset_t*         tileset   = core->map.handle->tilesets;
                        cute_tiled_tile_descriptor_t* tile      = tileset->tiles;
                        int                           first_gid = core->map.handle->tilesets->firstgid;
                        int                           local_id  = gid - first_gid;

                        if (gid)
                        {
                            while (tile)
                            {
                                if (tile->tile_index == local_id)
                                {
                                    if (tile->animation)
                                    {
                                        *animated_tile_count += 1;
                                        break;
                                    }
                                }
                                tile = tile->next;
                            }
                        }
                    }
                    #endif // USE_CUTE_TILED
                }
            }
        }
        layer = layer->next;
    }
}

static void count_tile_layers(int32_t* tile_layer_count, esz_core_t* core)
{
    #ifdef USE_LIBTMX
    tmx_layer* layer = core->map.handle->ly_head;

    #elif  USE_CUTE_TILED
    cute_tiled_layer_t* layer = core->map.handle->layers;

    #endif

    *tile_layer_count = 0;

    while (layer)
    {
        bool is_tile_layer = false;

        #ifdef USE_LIBTMX
        if (L_LAYER == layer->type)
        {
            is_tile_layer = true;
        }

        #elif  USE_CUTE_TILED
        if (core->map.hash_id_tilelayer == layer->type.hash_id)
        {
            is_tile_layer = true;
        }

        #endif

        if (is_tile_layer && layer->visible)
        {
            *tile_layer_count += 1;
        }
        layer = layer->next;
    }
}

static void count_objects(int32_t* object_count, esz_core_t* core)
{
    #ifdef USE_LIBTMX
    tmx_layer*  layer = core->map.handle->ly_head;
    tmx_object* object;

    #elif  USE_CUTE_TILED
    cute_tiled_layer_t*  layer = core->map.handle->layers;
    cute_tiled_object_t* object;

    #endif

    while (layer)
    {
        bool is_object_layer = false;

        #ifdef USE_LIBTMX
        if (L_OBJGR == layer->type)
        {
            is_object_layer = true;
            object          = layer->content.objgr->head;
        }

        #elif USE_CUTE_TILED
        if (core->map.hash_id_objectgroup == layer->type.hash_id)
        {
            is_object_layer = true;
            object          = layer->objects;
        }

        #endif

        if (is_object_layer)
        {
            while (object)
            {
                (*object_count) += 1;
                object = object->next;
            }
        }

        layer = layer->next;
    }
}

static esz_status draw_background(esz_window_t* window, esz_core_t* core)
{
    SDL_Rect dst;

    dst.x = 0;
    dst.y = 0;
    dst.w = window->width;
    dst.h = window->height;

    if (0 > SDL_RenderCopy(window->renderer, core->map.background.render_target, NULL, &dst))
    {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "%s: %s.\n", __func__, SDL_GetError());
        return ESZ_WARNING;
    }

    if (is_camera_at_horizontal_boundary(core))
    {
        /*****************************************************************
         * Do not move background when camera is at horizontal boundary. *
         *****************************************************************/
        if (! core->map.background.velocity_is_constant)
        {
            core->map.background.velocity = 0.0;
        }
    }
    else
    {
        /**************************************************************
         * Synchronize horizontal velocity of the background with the *
         * horizontal velocity of the tracked entity.                 *
         **************************************************************/

        // tbd.
        //core->map.background.velocity = core->map.entity[core->camera.target_entity_id].velocity_x;
    }

    /************************************************
     * Do not move background when camera is locked *
     ************************************************/

    if (is_camera_locked(core))
    {
        if (! core->map.background.velocity_is_constant)
        {
            core->map.background.velocity = 0.0;
        }
    }

    return ESZ_OK;
}

static esz_status draw_logo(esz_window_t *window)
{
    SDL_Rect dst;

    /******************************************
     * Display logo in the lower-right corner *
     ******************************************/

    dst.x = (window->logical_width)  - 53;
    dst.y = (window->logical_height) - 19;
    dst.w = 48;
    dst.h = 14;

    SDL_SetRenderDrawColor(window->renderer, 0xa9, 0x20, 0x3e, SDL_ALPHA_OPAQUE);

    if (0 > SDL_RenderCopy(window->renderer, window->esz_logo, NULL, &dst))
    {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "%s: %s.\n", __func__, SDL_GetError());
        return ESZ_ERROR_CRITICAL;
    }

    return ESZ_OK;
}

static esz_status draw_map(const esz_layer_type layer_type, esz_window_t *window, esz_core_t *core)
{
    SDL_Rect dst;

    dst.x = 0;
    dst.y = 0;
    dst.w = window->width;
    dst.h = window->height;

    if (0 > SDL_RenderCopy(window->renderer, core->map.render_target[layer_type], NULL, &dst))
    {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "%s: %s.\n", __func__, SDL_GetError());
        return ESZ_ERROR_CRITICAL;
    }

    return ESZ_OK;
}

static bool get_boolean_property(const unsigned long name_hash, esz_property_handle_t* properties, esz_core_t* core)
{
    load_property(name_hash, properties, core);
    return core->map.boolean_property;
}

static uint16_t get_camera_target(esz_core_t* core)
{
    return core->camera.target_entity_id;
}

static esz_status init_animated_tiles(esz_core_t* core)
{
    count_animated_tiles(&core->map.animated_tile_count, core);

    if (0 == core->map.animated_tile_count)
    {
        return ESZ_OK;
    }

    /**************************************
     * Allocate memory for animated tiles *
     **************************************/

    if (0 < core->map.animated_tile_count)
    {
        core->map.animated_tile = SDL_calloc((size_t)&core->map.animated_tile_count, sizeof(struct esz_animated_tile));
        if (! core->map.animated_tile)
        {
            SDL_LogError(SDL_LOG_CATEGORY_ERROR, "%s: error allocating memory.\n", __func__);
            return ESZ_ERROR_CRITICAL;
        }
    }

    SDL_Log("Initialise %u animated tile(s).\n", core->map.animated_tile_count);
    return ESZ_OK;
}

static esz_status init_background(esz_window_t* window, esz_core_t* core)
{
    core->map.background.layer_count = esz_get_integer_map_property(H_background_layer_count, core);
    core->map.background.layer_shift = esz_get_decimal_map_property(H_background_layer_shift, core);
    core->map.background.velocity    = esz_get_decimal_map_property(H_background_constant_velocity, core);

    if (0.0 < core->map.background.velocity)
    {
        core->map.background.velocity_is_constant = true;
    }

    if (esz_get_boolean_map_property(H_background_is_top_aligned, core))
    {
        core->map.background.alignment = ESZ_TOP;
    }
    else
    {
        core->map.background.alignment = ESZ_BOT;
    }

    core->map.background.layer = SDL_calloc((size_t)core->map.background.layer_count, sizeof(struct esz_background_layer));
    if (! core->map.background.layer)
    {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "%s: error allocating memory.\n", __func__);
        return ESZ_ERROR_CRITICAL;
    }

    if (0 < core->map.background.layer_count)
    {
        for (int32_t index = 0; index < core->map.background.layer_count; index += 1)
        {
            load_background_layer(index, window, core);
        }
    }
    else
    {
        return ESZ_OK;
    }

    SDL_Log("Initialise parallax-scrolling background with %u layers.\n", core->map.background.layer_count);

    return ESZ_OK;
}

static esz_status init_objects(esz_core_t* core)
{
    /*****************
     * Count objects *
     *****************/

    if (core->map.object_count)
    {
        return ESZ_OK;
    }

    count_objects(&core->map.object_count, core);

    /*******************************
     * Allocate memory for objects *
     *******************************/

    if (core->map.object_count)
    {
        core->map.object = SDL_calloc((size_t)core->map.object_count, sizeof(struct esz_object));
        if (! core->map.object)
        {
            SDL_LogError(SDL_LOG_CATEGORY_ERROR, "%s: error allocating memory.\n", __func__);
            return ESZ_ERROR_CRITICAL;
        }
    }

    SDL_Log("Initialise %u object(s).\n", core->map.object_count);
    return ESZ_OK;
}

static esz_status init_sprites(esz_window_t* window, esz_core_t* core)
{
    core->map.sprite_sheet_count = esz_get_integer_map_property(H_sprite_sheet_count, core);

    if (0 == core->map.sprite_sheet_count)
    {
        return ESZ_OK;
    }

    core->map.sprite = SDL_calloc((size_t)core->map.sprite_sheet_count, sizeof(struct esz_sprite));
    if (! core->map.sprite)
    {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "%s: error allocating memory.\n", __func__);
        return ESZ_ERROR_CRITICAL;
    }

    if (0 < core->map.sprite_sheet_count)
    {
        char property_name[17] = { 0 };

        for (int32_t index = 0; index < core->map.sprite_sheet_count; index += 1)
        {
            char*  sprite_sheet_image_source;
            size_t source_length;

            SDL_snprintf(property_name, 17, "sprite_sheet_%u", index);

            const char* file_name = esz_get_string_map_property(esz_hash((const unsigned char*)property_name), core);
            // Todo: error handling.
            source_length         = SDL_strlen(core->map.path) + SDL_strlen(file_name) + 1;

            sprite_sheet_image_source = SDL_calloc(1, source_length);
            if (! sprite_sheet_image_source)
            {
                SDL_LogError(SDL_LOG_CATEGORY_ERROR, "%s: error allocating memory.\n", __func__);
                return ESZ_ERROR_CRITICAL;
            }

            SDL_snprintf(sprite_sheet_image_source, source_length, "%s%s", core->map.path, file_name);
            core->map.sprite[index].id = index;

            if (ESZ_OK != load_texture_from_file(sprite_sheet_image_source, &core->map.sprite[index].render_target, window))
            {
                SDL_free(sprite_sheet_image_source);
                return ESZ_ERROR_CRITICAL;
            }

            SDL_free(sprite_sheet_image_source);
        }
    }

    return ESZ_OK;
}

static bool is_camera_locked(esz_core_t* core)
{
    return core->camera.is_locked;
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
    double       layer_width_factor;
    char         property_name[21] = { 0 };
    char*        background_layer_image_source;
    size_t       source_length = 0;

    SDL_snprintf(property_name, 21, "background_layer_%u", index);

    const char* file_name = esz_get_string_map_property(esz_hash((const unsigned char*)property_name), core);
    source_length = SDL_strlen(core->map.path) + SDL_strlen(file_name) + 1;

    background_layer_image_source = SDL_calloc(1, source_length);
    if (! background_layer_image_source)
    {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "%s: error allocating memory.\n", __func__);
        return ESZ_ERROR_CRITICAL;
    }

    SDL_snprintf(background_layer_image_source, source_length, "%s%s", core->map.path, file_name);

    if (ESZ_OK != load_texture_from_file(background_layer_image_source, &image_texture, window))
    {
        status = ESZ_ERROR_CRITICAL;
        goto exit;
    }

    SDL_free(background_layer_image_source);

    if (0 > SDL_QueryTexture(image_texture, NULL, NULL, &image_width, &image_height))
    {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "%s: %s.\n", __func__, SDL_GetError());
        status = ESZ_ERROR_CRITICAL;
        goto exit;
    }

    layer_width_factor = SDL_ceil((double)window->width / (double)image_width);

    core->map.background.layer[index].width  = image_width * (int32_t)layer_width_factor;
    core->map.background.layer[index].height = image_height;

    core->map.background.layer[index].render_target = SDL_CreateTexture(
        window->renderer,
        SDL_PIXELFORMAT_ARGB8888,
        SDL_TEXTUREACCESS_TARGET,
        core->map.background.layer[index].width,
        core->map.background.layer[index].height);

    if (! core->map.background.layer[index].render_target)
    {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "%s: %s.\n", __func__, SDL_GetError());
        status = ESZ_ERROR_CRITICAL;
        goto exit;
    }

    if (0 != SDL_SetRenderTarget(window->renderer, core->map.background.layer[index].render_target))
    {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "%s: %s.\n", __func__, SDL_GetError());
        status = ESZ_ERROR_CRITICAL;
        goto exit;
    }

    dst.x = 0;
    for (int32_t pass = 0; pass < layer_width_factor; pass += 1)
    {
        dst.y = 0;
        dst.w = image_width;
        dst.h = image_height;

        if (0 > SDL_RenderCopy(window->renderer, image_texture, NULL, &dst))
        {
            SDL_LogError(SDL_LOG_CATEGORY_ERROR, "%s: %s.\n", __func__, SDL_GetError());
            status = ESZ_ERROR_CRITICAL;
            goto exit;
        }

        dst.x += image_width;
    }

    if (0 > SDL_SetTextureBlendMode(core->map.background.layer[index].render_target, SDL_BLENDMODE_BLEND))
    {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "%s: %s.\n", __func__, SDL_GetError());
        status = ESZ_ERROR_CRITICAL;
        goto exit;
    }

exit:
    if (image_texture)
    {
        SDL_DestroyTexture(image_texture);
    }

    SDL_Log("Load background layer %d.\n", index);
    return status;
}

// This function should not be used directly.
static void load_property(const unsigned long name_hash, esz_property_handle_t* properties, esz_core_t* core)
{
    core->map.boolean_property = false;
    core->map.decimal_property = 0.0;
    core->map.integer_property = 0;
    core->map.string_property  = NULL;

    #ifdef USE_LIBTMX
    core->map.hash_query = name_hash;
    tmx_property_foreach(properties, tmxlib_store_property, (void*)core);

    #elif  USE_CUTE_TILED
    int index = 0;

    while (properties[index].name.ptr)
    {
        if (name_hash == esz_hash((const unsigned char*)properties[index].name.ptr))
        {
            break;
        }

        index += 1;
    }

    switch (properties[index].type)
    {
        case CUTE_TILED_PROPERTY_COLOR:
        case CUTE_TILED_PROPERTY_FILE:
	case CUTE_TILED_PROPERTY_NONE:
            // tbd.
            break;
	case CUTE_TILED_PROPERTY_INT:
            SDL_LogVerbose(SDL_LOG_CATEGORY_APPLICATION,
                           "Loading integer property '%s': %d\n", properties[index].name.ptr, properties[index].data.integer);
            core->map.integer_property = properties[index].data.integer;
            break;
	case CUTE_TILED_PROPERTY_BOOL:
            SDL_LogVerbose(SDL_LOG_CATEGORY_APPLICATION,
                           "Loading boolean property '%s': %u\n", properties[index].name.ptr, properties[index].data.boolean);
            core->map.boolean_property = (bool)properties[index].data.boolean;
            break;
	case CUTE_TILED_PROPERTY_FLOAT:
            SDL_LogVerbose(SDL_LOG_CATEGORY_APPLICATION,
                           "Loading decimal property '%s': %f\n", properties[index].name.ptr, (double)properties[index].data.floating);
            core->map.decimal_property = (double)properties[index].data.floating;
            break;
	case CUTE_TILED_PROPERTY_STRING:
            SDL_LogVerbose(SDL_LOG_CATEGORY_APPLICATION,
                           "Loading string property '%s': %s\n", properties[index].name.ptr, properties[index].data.string.ptr);
            core->map.string_property  = properties[index].data.string.ptr;
            break;
    }
    #endif // USE_CUTE_TILED
}

// Based on https://wiki.libsdl.org/SDL_CreateRGBSurfaceWithFormatFrom#Code_Examples
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

    data = stbi_load(file_name, &width, &height, &orig_format, req_format);

    if (NULL == data)
    {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "%s: %s.\n", __func__, stbi_failure_reason());
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
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "%s: %s.\n", __func__, SDL_GetError());
        stbi_image_free(data);
        return ESZ_ERROR_CRITICAL;
    }

    *texture = SDL_CreateTextureFromSurface(window->renderer, surface);

    SDL_FreeSurface(surface);
    stbi_image_free(data);

    SDL_Log("Loading image from file: %s.\n", file_name);
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

    data = stbi_load_from_memory(buffer, length, &width, &height, &orig_format, req_format);

    if (NULL == data)
    {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "%s: %s.\n", __func__, stbi_failure_reason());
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
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "%s: %s.\n", __func__, SDL_GetError());
        stbi_image_free(data);
        return ESZ_ERROR_CRITICAL;
    }

    *texture = SDL_CreateTextureFromSurface(window->renderer, surface);

    SDL_FreeSurface(surface);
    stbi_image_free(data);

    SDL_Log("Loading image from memory.\n");
    return ESZ_OK;
}

static void move_camera_to_target(esz_core_t* core)
{
    if (is_camera_locked(core))
    {
        /*
        Entity* hEntity = &heszFW.hMap->hEntity[heszFW.hCamera->u16TargetEntityID];

        heszFW.hCamera->dPosX  = hEntity->dPosX;
        heszFW.hCamera->dPosX -= heszFW.hVideo->s32LogicalWindowWidth  / 2.0;
        heszFW.hCamera->dPosY  = hEntity->dPosY;
        heszFW.hCamera->dPosY -= heszFW.hVideo->s32LogicalWindowHeight / 2.0;

        if (heszFW.hCamera->dPosX < 0)
        {
            heszFW.hCamera->dPosX = 0;
        }
        */
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
                    core->event.finger_down_cb((void*)window, (void*)core);
                }
                break;
            case SDL_FINGERUP:
                if (core->event.finger_up_cb)
                {
                    core->event.finger_up_cb((void*)window, (void*)core);
                }
                break;
            case SDL_FINGERMOTION:
                if (core->event.finger_motion_cb)
                {
                    core->event.finger_motion_cb((void*)window, (void*)core);
                }
                break;
            case SDL_KEYDOWN:
                if (core->event.key_down_cb)
                {
                    core->event.key_down_cb((void*)window, (void*)core);
                }
                break;
            case SDL_KEYUP:
                if (core->event.key_up_cb)
                {
                    core->event.key_up_cb((void*)window, (void*)core);
                }
                break;
            case SDL_MULTIGESTURE:
                if (core->event.multi_gesture_cb)
                {
                    core->event.multi_gesture_cb((void*)window, (void*)core);
                }
                break;
        }
    }
}

static int remove_gid_flip_bits(int gid)
{
    #ifdef USE_LIBTMX
    return gid & TMX_FLIP_BITS_REMOVAL;

    #elif  USE_CUTE_TILED
    return cute_tiled_unset_flags(gid);

    #endif
}

static esz_status render_background(esz_window_t* window, esz_core_t* core)
{
    esz_status status = ESZ_OK;
    double     factor = (double)core->map.background.layer_count + 1.0;

    if (! core->map.background.render_target)
    {
        core->map.background.render_target = SDL_CreateTexture(
            window->renderer,
            SDL_PIXELFORMAT_ARGB8888,
            SDL_TEXTUREACCESS_TARGET,
            window->width,
            window->height);
    }

    if (! core->map.background.render_target)
    {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "%s: %s.\n", __func__, SDL_GetError());
        return ESZ_ERROR_CRITICAL;
    }

    for (int32_t index = 0; index < core->map.background.layer_count; index += 1)
    {
        core->map.background.layer[index].velocity = core->map.background.velocity / factor;
        factor -= core->map.background.layer_shift;

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
    SDL_Rect dst;
    double   pos_x_a;
    double   pos_x_b;
    int32_t  width = 0;

    if (0 > SDL_QueryTexture(core->map.background.layer[index].render_target, NULL, NULL, &width, NULL))
    {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "%s: %s.\n", __func__, SDL_GetError());
        return ESZ_ERROR_CRITICAL;
    }

    if (core->map.background.layer[index].pos_x < -width)
    {
        core->map.background.layer[index].pos_x = +width;
    }

    if (core->map.background.layer[index].pos_x > +width)
    {
        core->map.background.layer[index].pos_x = -width;
    }

    pos_x_a = core->map.background.layer[index].pos_x;
    if (0 < pos_x_a)
    {
        pos_x_b = pos_x_a - width;
    }
    else
    {
        pos_x_b = pos_x_a + width;
    }

    if (0 < core->map.background.layer[index].velocity)
    {
        if (ESZ_RIGHT == core->map.background.direction)
        {
            core->map.background.layer[index].pos_x -= core->map.background.layer[index].velocity;
        }
        else
        {
            core->map.background.layer[index].pos_x += core->map.background.layer[index].velocity;
        }
    }

    if (ESZ_TOP == core->map.background.alignment)
    {
        dst.y = (int)(core->map.background.layer[index].pos_y - core->camera.pos_y);
    }
    else
    {
        dst.y = (int)(core->map.background.layer[index].pos_y + (window->logical_height - core->map.background.layer[index].height));
    }

    if (0 > SDL_SetRenderTarget(window->renderer, core->map.background.render_target))
    {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "%s: %s.\n", __func__, SDL_GetError());
        return ESZ_ERROR_CRITICAL;
    }

    if (0 == index)
    {
        /************************
         * Set background color *
         ************************/

        SDL_SetRenderDrawColor(
            window->renderer,
            (core->map.handle->backgroundcolor >> 16) & 0xFF,
            (core->map.handle->backgroundcolor >> 8)  & 0xFF,
            (core->map.handle->backgroundcolor)       & 0xFF,
            0);

        SDL_RenderClear(window->renderer);
    }

    dst.x = (int)pos_x_a;
    dst.w = width;
    dst.h = core->map.background.layer[index].height;

    if (0 > SDL_RenderCopyEx(window->renderer, core->map.background.layer[index].render_target, NULL, &dst, 0, NULL, SDL_FLIP_NONE))
    {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "%s: %s.\n", __func__, SDL_GetError());
        return ESZ_ERROR_CRITICAL;
    }

    dst.x = (int)pos_x_b;
    if (0 > SDL_RenderCopyEx(window->renderer, core->map.background.layer[index].render_target, NULL, &dst, 0, NULL, SDL_FLIP_NONE))
    {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "%s: %s.\n", __func__, SDL_GetError());
        return ESZ_ERROR_CRITICAL;
    }

    return ESZ_OK;
}

static esz_status render_map(const esz_layer_type layer_type, esz_window_t *window, esz_core_t* core)
{
    bool render_animated_tiles;

    #ifdef USE_LIBTMX
    tmx_layer* layer          = core->map.handle->ly_head;
    #elif  USE_CUTE_TILED
    cute_tiled_layer_t* layer = core->map.handle->layers;
    #endif

    if (! core->map.is_loaded)
    {
        return ESZ_OK;
    }

    if (ESZ_LAYER_MAX == layer_type)
    {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "%s: Layer type out of range.\n", __func__);
        return ESZ_ERROR_CRITICAL;
    }

    if (ESZ_BG == layer_type)
    {
        render_animated_tiles = true;
    }

    if (! core->map.render_target[layer_type])
    {
        core->map.render_target[layer_type] = SDL_CreateTexture(
            window->renderer,
            SDL_PIXELFORMAT_ARGB8888,
            SDL_TEXTUREACCESS_TARGET,
            window->width,
            window->height);
    }

    if (! core->map.render_target[layer_type])
    {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "%s: %s.\n", __func__, SDL_GetError());
        return ESZ_ERROR_CRITICAL;
    }
    else
    {
        if (0 > SDL_SetTextureBlendMode(core->map.render_target[layer_type], SDL_BLENDMODE_BLEND))
        {
            SDL_LogError(SDL_LOG_CATEGORY_ERROR, "%s: %s.\n", __func__, SDL_GetError());
            return ESZ_ERROR_CRITICAL;
        }
    }

    if (0 > SDL_SetRenderTarget(window->renderer, core->map.render_target[layer_type]))
    {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "%s: %s.\n", __func__, SDL_GetError());
        return ESZ_ERROR_CRITICAL;
    }

    SDL_RenderClear(window->renderer);

    /************************************
     * Update and render animated tiles *
     ************************************/

    core->map.time_since_last_anim_frame += window->time_since_last_frame;

    if (0 < core->map.animated_tile_index &&
        core->map.time_since_last_anim_frame >= 1.0 / (double)(core->map.animated_tile_fps) && render_animated_tiles)
    {
        core->map.time_since_last_anim_frame = 0.0;

        /* Remark: animated tiles are always rendered in the background
         * layer.
         */
        if (! core->map.animated_tile_texture)
        {
            core->map.animated_tile_texture = SDL_CreateTexture(
                window->renderer,
                SDL_PIXELFORMAT_ARGB8888,
                SDL_TEXTUREACCESS_TARGET,
                (int)(core->map.width),
                (int)(core->map.height));
        }

        if (! core->map.animated_tile_texture)
        {
            SDL_LogError(SDL_LOG_CATEGORY_ERROR, "%s: %s.\n", __func__, SDL_GetError());
            return ESZ_ERROR_CRITICAL;
        }

        if (0 > SDL_SetRenderTarget(window->renderer, core->map.animated_tile_texture))
        {
            SDL_LogError(SDL_LOG_CATEGORY_ERROR, "%s: %s.\n", __func__, SDL_GetError());
            return ESZ_ERROR_CRITICAL;
        }

        for (int32_t index = 0; core->map.animated_tile_index > index; index += 1)
        {
            int      gid          = core->map.animated_tile[index].gid;
            uint32_t tile_id;
            uint32_t next_tile_id = 0;
            SDL_Rect dst;
            SDL_Rect src;

            #ifdef USE_LIBTMX
            tmx_tileset* tileset;
            int          first_gid = core->map.handle->ts_head->firstgid;

            tile_id  = core->map.animated_tile[index].id + 1;
            tileset  = core->map.handle->tiles[first_gid]->tileset;
            src.x    = (int)core->map.handle->tiles[tile_id]->ul_x;
            src.y    = (int)core->map.handle->tiles[tile_id]->ul_y;
            src.w    = dst.w = (int)tileset->tile_width;
            src.h    = dst.h = (int)tileset->tile_height;
            dst.x             = (int)core->map.animated_tile[index].dst_x;
            dst.y             = (int)core->map.animated_tile[index].dst_y;

            #elif  USE_CUTE_TILED
            cute_tiled_tileset_t*         tileset = core->map.handle->tilesets;
            cute_tiled_tile_descriptor_t* tile    = tileset->tiles;

            tile_id  = core->map.animated_tile[index].id;
            src.w    = dst.w = (int)tileset->tilewidth;
            src.h    = dst.h = (int)tileset->tileheight;
            dst.x             = (int)core->map.animated_tile[index].dst_x;
            dst.y             = (int)core->map.animated_tile[index].dst_y;

            while (tile)
            {
                if (tile->tile_index == gid)
                {
                    src.x  = (tile_id % tileset->columns) * tileset->tilewidth;
                    src.y  = (tile_id / tileset->columns) * tileset->tileheight;
                    break;
                }
                tile = tile->next;
            }

            #endif // USE_CUTE_TILED

            if (0 > SDL_RenderCopy(window->renderer, core->map.tileset_texture, &src, &dst))
            {
                SDL_LogError(SDL_LOG_CATEGORY_ERROR, "%s: %s.\n", __func__, SDL_GetError());
                return ESZ_ERROR_CRITICAL;
            }

            core->map.animated_tile[index].current_frame += 1;

            if (core->map.animated_tile[index].current_frame >= core->map.animated_tile[index].animation_length)
            {
                core->map.animated_tile[index].current_frame = 0;
            }

            #ifdef USE_LIBTMX
            next_tile_id = core->map.handle->tiles[gid]->animation[core->map.animated_tile[index].current_frame].tile_id;

            #elif  USE_CUTE_TILED
            tile = tileset->tiles;
            while (tile)
            {
                if (tile->tile_index == gid)
                {
                    next_tile_id = tile->animation[core->map.animated_tile[index].current_frame].tileid;
                    break;
                }
                tile = tile->next;
            }

            #endif // USE_CUTE_TILED

            core->map.animated_tile[index].id = next_tile_id;
        }

        if (0 > SDL_SetRenderTarget(window->renderer, core->map.map_layer[layer_type]))
        {
            SDL_LogError(SDL_LOG_CATEGORY_ERROR, "%s: %s.\n", __func__, SDL_GetError());
            return ESZ_ERROR_CRITICAL;
        }

        if (0 > SDL_SetRenderTarget(window->renderer, core->map.render_target[layer_type]))
        {
            SDL_LogError(SDL_LOG_CATEGORY_ERROR, "%s: %s.\n", __func__, SDL_GetError());
            return ESZ_ERROR_CRITICAL;
        }

        if (0 > SDL_SetTextureBlendMode(core->map.animated_tile_texture, SDL_BLENDMODE_BLEND))
        {
            SDL_LogError(SDL_LOG_CATEGORY_ERROR, "%s: %s.\n", __func__, SDL_GetError());
            return ESZ_ERROR_CRITICAL;
        }
    }

    /**************************************
     * Texture has already been rendered. *
     **************************************/

    if (core->map.map_layer[layer_type])
    {
        double   render_pos_x = core->map.pos_x - core->camera.pos_x;
        double   render_pos_y = core->map.pos_y - core->camera.pos_y;
        SDL_Rect dst          = {
            (int)render_pos_x,
            (int)render_pos_y,
            (int)core->map.width,
            (int)core->map.height
        };

        if (0 > SDL_RenderCopyEx(window->renderer, core->map.map_layer[layer_type], NULL, &dst, 0, NULL, SDL_FLIP_NONE))
        {
            SDL_LogError(SDL_LOG_CATEGORY_ERROR, "%s: %s.\n", __func__, SDL_GetError());
            return ESZ_ERROR_CRITICAL;
        }

        /******************
         * Animated tiles *
         ******************/

        if (render_animated_tiles)
        {
            if (core->map.animated_tile_texture)
            {
                if (0 > SDL_RenderCopyEx(window->renderer, core->map.animated_tile_texture, NULL, &dst, 0, NULL, SDL_FLIP_NONE))
                {
                    SDL_LogError(SDL_LOG_CATEGORY_ERROR, "%s: %s.\n", __func__, SDL_GetError());
                    return ESZ_ERROR_CRITICAL;
                }
            }
        }

        return ESZ_OK;
    }

    /******************************************
     * Texture does not yet exist. Render it! *
     ******************************************/

    core->map.map_layer[layer_type] = SDL_CreateTexture(
        window->renderer,
        SDL_PIXELFORMAT_ARGB8888,
        SDL_TEXTUREACCESS_TARGET,
        (int)core->map.width,
        (int)core->map.height);

    if (! core->map.map_layer[layer_type])
    {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "%s: %s.\n", __func__, SDL_GetError());
        return ESZ_ERROR_CRITICAL;
    }

    if (0 > SDL_SetRenderTarget(window->renderer, core->map.map_layer[layer_type]))
    {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "%s: %s.\n", __func__, SDL_GetError());
        return ESZ_ERROR_CRITICAL;
    }

    while (layer)
    {
        bool     is_tile_layer = false;
        SDL_Rect dst;
        SDL_Rect src;

        #ifdef USE_LIBTMX
        if (L_LAYER == layer->type)
        {
            is_tile_layer = true;
        }
        #elif  USE_CUTE_TILED
        if (core->map.hash_id_tilelayer == layer->type.hash_id)
        {
            is_tile_layer = true;
        }
        #endif

        if(is_tile_layer)
        {
            bool is_in_foreground = false;
            bool render_layer     = false;

            is_in_foreground = get_boolean_property(H_is_in_foreground, layer->properties, core);

            if (ESZ_BG == layer_type && false == is_in_foreground)
            {
                render_layer = true;
            }
            else if (ESZ_FG == layer_type && true == is_in_foreground)
            {
                render_layer = true;
            }

            if (layer->visible && render_layer)
            {
                for (int index_height = 0; index_height < (int)core->map.handle->height; index_height += 1)
                {
                    for (int index_width = 0; index_width < (int)core->map.handle->width; index_width += 1)
                    {
                        int  gid;

                        #ifdef USE_LIBTMX
                        uint32_t* layer_content = layer->content.gids;
                        #elif  USE_CUTE_TILED
                        int*      layer_content = layer->data;
                        #endif

                        gid = remove_gid_flip_bits(layer_content[(index_height * core->map.handle->width) + index_width]);

                        #ifdef USE_LIBTMX
                        if (core->map.handle->tiles[gid])
                        {
                            int          first_gid = core->map.handle->ts_head->firstgid;
                            tmx_tileset* tileset;

                            tileset = core->map.handle->tiles[first_gid]->tileset;
                            src.x   = (int)core->map.handle->tiles[gid]->ul_x;
                            src.y   = (int)core->map.handle->tiles[gid]->ul_y;
                            src.w   = dst.w = (int)tileset->tile_width;
                            src.h   = dst.h = (int)tileset->tile_height;
                            dst.x           = (int)(index_width  * tileset->tile_width);
                            dst.y           = (int)(index_height * tileset->tile_height);

                            SDL_RenderCopy(window->renderer, core->map.tileset_texture, &src, &dst);

                            if (render_animated_tiles && core->map.handle->tiles[gid]->animation)
                            {
                                uint32_t animation_length = core->map.handle->tiles[gid]->animation_len;
                                uint32_t id               = core->map.handle->tiles[gid]->animation[0].tile_id;

                                core->map.animated_tile[core->map.animated_tile_index].gid              = gid;
                                core->map.animated_tile[core->map.animated_tile_index].id               = id;
                                core->map.animated_tile[core->map.animated_tile_index].dst_x            = dst.x;
                                core->map.animated_tile[core->map.animated_tile_index].dst_y            = dst.y;
                                core->map.animated_tile[core->map.animated_tile_index].current_frame    = 0;
                                core->map.animated_tile[core->map.animated_tile_index].animation_length = animation_length;

                                core->map.animated_tile_index += 1;
                            }
                        }

                        #elif USE_CUTE_TILED
                        if (gid)
                        {
                            cute_tiled_tileset_t* tileset;
                            int                   first_gid = core->map.handle->tilesets->firstgid;
                            int                   local_id  = gid - first_gid;

                            tileset = core->map.handle->tilesets;
                            src.x   = (local_id % tileset->columns) * tileset->tilewidth;
                            src.y   = (gid / tileset->columns)      * tileset->tileheight;
                            src.w   = dst.w = (int)tileset->tilewidth;
                            src.h   = dst.h = (int)tileset->tileheight;
                            dst.x           = (int)(index_width  * tileset->tilewidth);
                            dst.y           = (int)(index_height * tileset->tileheight);

                            SDL_RenderCopy(window->renderer, core->map.tileset_texture, &src, &dst);

                            if (render_animated_tiles)
                            {
                                cute_tiled_tileset_t*         tileset   = core->map.handle->tilesets;
                                cute_tiled_tile_descriptor_t* tile      = tileset->tiles;

                                while (tile)
                                {
                                    if (tile->tile_index == local_id)
                                    {
                                        if (tile->animation)
                                        {
                                            uint32_t animation_length = tile->frame_count;
                                            uint32_t id               = tile->animation->tileid;
                                            int      first_gid        = core->map.handle->tilesets->firstgid;
                                            int      local_id         = gid - first_gid;

                                            core->map.animated_tile[core->map.animated_tile_index].gid              = local_id;
                                            core->map.animated_tile[core->map.animated_tile_index].id               = id;
                                            core->map.animated_tile[core->map.animated_tile_index].dst_x            = dst.x;
                                            core->map.animated_tile[core->map.animated_tile_index].dst_y            = dst.y;
                                            core->map.animated_tile[core->map.animated_tile_index].current_frame    = 0;
                                            core->map.animated_tile[core->map.animated_tile_index].animation_length = animation_length;

                                            core->map.animated_tile_index += 1;

                                            break;
                                        }
                                    }
                                    tile = tile->next;
                                }
                            }
                        }
                        #endif // USE_CUTE_TILED
                    }
                }

                {
                    const char* layer_name;
                    #ifdef USE_LIBTMX
                    layer_name = layer->name;

                    #elif  USE_CUTE_TILED
                    layer_name = layer->name.ptr;

                    #endif

                    SDL_Log("Render map layer: %s\n", layer_name);
                }
            }
        }
        layer = layer->next;
    }

    if (0 > SDL_SetRenderTarget(window->renderer, core->map.render_target[layer_type]))
    {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "%s: %s.\n", __func__, SDL_GetError());
        return ESZ_ERROR_CRITICAL;
    }

    if (0 > SDL_SetTextureBlendMode(core->map.map_layer[layer_type], SDL_BLENDMODE_BLEND))
    {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "%s: %s.\n", __func__, SDL_GetError());
        return ESZ_ERROR_CRITICAL;
    }

    return ESZ_OK;
}

static double round_(double number)
{
    double decimal_place = number - SDL_floor(number);

    if (decimal_place >= 0.5)
    {
        return SDL_ceil(number);
    }
    else
    {
        return SDL_floor(number);
    }
}

static void set_camera_boundaries_to_map_size(esz_window_t* window, esz_core_t* core)
{
    core->camera.is_at_horizontal_boundary = false;
    core->camera.max_pos_x                 = (int32_t)core->map.width  - window->logical_width;
    core->camera.max_pos_y                 = (int32_t)core->map.height - window->logical_height;

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

static void set_camera_target(const uint16_t target_entity_id, esz_core_t* core)
{
    core->camera.target_entity_id = target_entity_id;
}

#ifdef USE_LIBTMX
static void tmxlib_store_property(tmx_property* property, void* core)
{
    esz_core_t* core_ptr = core;

    if (core_ptr->map.hash_query == esz_hash((const unsigned char*)property->name))
    {
        switch (property->type)
        {
            case PT_COLOR:
            case PT_NONE:
                // tbd.
                break;
            case PT_BOOL:
                SDL_LogVerbose(SDL_LOG_CATEGORY_APPLICATION,
                               "Loading boolean property '%s': %u\n", property->name, property->value.boolean);
                core_ptr->map.boolean_property = (bool)property->value.boolean;
                break;
            case PT_FILE:
                SDL_LogVerbose(SDL_LOG_CATEGORY_APPLICATION,
                               "Loading string property '%s': %s\n", property->name, property->value.file);
                core_ptr->map.string_property  = property->value.file;
                break;
            case PT_FLOAT:
                SDL_LogVerbose(SDL_LOG_CATEGORY_APPLICATION,
                               "Loading decimal property '%s': %f\n", property->name, (double)property->value.decimal);
                core_ptr->map.decimal_property = (double)property->value.decimal;
                break;
            case PT_INT:
                SDL_LogVerbose(SDL_LOG_CATEGORY_APPLICATION,
                               "Loading integer property '%s': %d\n", property->name, property->value.integer);
                core_ptr->map.integer_property = property->value.integer;
                break;
            case PT_STRING:
                SDL_LogVerbose(SDL_LOG_CATEGORY_APPLICATION,
                               "Loading string property '%s': %s\n", property->name, property->value.string);
                core_ptr->map.string_property  = property->value.string;
                break;
        }
    }
}
#endif

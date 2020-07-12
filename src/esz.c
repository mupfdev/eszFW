// Spdx-License-Identifier: MIT
/**
 * @file    esz.c
 * @brief   eszFW game engine
 * @details A cross-platform game engine written in C
 */

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#include <emscripten/html5.h>
#endif
#include <SDL.h>
#include <cwalk.h>
#include <tmx.h>
#include "esz.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

/**********************
 * Private Prototypes *
 **********************/

static void         count_map_layers(tmx_layer* layer, Uint16* layer_count);
static void         count_objects(tmx_object* tmx_object, Uint16** object_count);
static esz_status   draw_background(esz_window_t* window, esz_core_t* core);
static esz_status   draw_map(const esz_layer_type layer_type, esz_window_t *window, esz_core_t *core);
static Uint16       get_camera_target(esz_core_t* core);
static esz_status   init_animated_tiles(esz_core_t* core);
static esz_status   init_background(esz_window_t* window, esz_core_t* core);
static esz_status   init_objects(esz_core_t* core);
static esz_status   init_sprites(esz_window_t* window, esz_core_t* core);
static SDL_bool     is_camera_locked(esz_core_t* core);
static SDL_bool     is_camera_at_horizontal_boundary(esz_core_t* core);
static void         load_property_by_name(const char* property_name, tmx_property* property, esz_core_t* core);
static esz_status   load_texture_from_file(const char* file_name, SDL_Texture** texture, esz_window_t* window);
static void         move_camera_to_target(esz_core_t* core);
static void         poll_events(esz_window_t* window, esz_core_t* core);
static Uint32       remove_gid_flip_bits(Uint32 gid);
static esz_status   render_background(esz_window_t* window, esz_core_t* core);
static esz_status   render_background_layer(Sint32 index, esz_window_t* window, esz_core_t* core);
static esz_status   render_map(const esz_layer_type layer_type, esz_window_t *window, esz_core_t* core);
static double       round_(double number);
static void         set_camera_boundaries_to_map_size(esz_window_t* window, esz_core_t* core);
static void         set_camera_target(const Uint16 target_entity_id, esz_core_t* core);
static void         store_property(tmx_property* property, void* core);

/********************
 * Public functions *
 ********************/

SDL_bool esz_bounding_boxes_do_intersect(const esz_aabb bb_a, const esz_aabb bb_b)
{
    double bb_a_x = bb_b.left - bb_a.right;
    double bb_a_y = bb_b.top  - bb_a.bottom;
    double bb_b_x = bb_a.left - bb_b.right;
    double bb_b_y = bb_a.top  - bb_b.bottom;

    if (0.0 < bb_a_x || 0.0 < bb_a_y)
    {
        return SDL_FALSE;
    }

    if (0.0 < bb_b_x || 0.0 < bb_b_y)
    {
        return SDL_FALSE;
    }

    return SDL_TRUE;
}

esz_status esz_create_window(const char* window_title, esz_window_config_t* config, esz_window_t** window)
{
    esz_status      status = ESZ_OK;
    SDL_DisplayMode display_mode;
    Uint32          renderer_flags = SDL_RENDERER_ACCELERATED | SDL_RENDERER_TARGETTEXTURE;

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
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "%s: %s\n", __func__, SDL_GetError());
        status = ESZ_ERROR_CRITICAL;
        goto exit;
    }

    if (0 != SDL_GetCurrentDisplayMode(0, &display_mode))
    {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "%s: %s\n", __func__, SDL_GetError());
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
            (*window)->vsync_enabled = SDL_FALSE;
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
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "%s: %s\n", __func__, SDL_GetError());
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

        if (! SDL_strstr(renderer_info.name, "opengl"))
        {
            continue;
        }
        else
        {
            (*window)->renderer = SDL_CreateRenderer((*window)->window, driver_index, renderer_flags);

            if (! (*window)->renderer)
            {
                SDL_LogError(SDL_LOG_CATEGORY_ERROR, "%s: %s\n", __func__, SDL_GetError());
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
            SDL_LogError(SDL_LOG_CATEGORY_ERROR, "%s: %s\n", __func__, SDL_GetError());
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

exit:
    return status;
}

void esz_deactivate_core(esz_core_t* core)
{
    core->is_active = SDL_FALSE;
}

void esz_destroy_core(esz_core_t* core)
{
    SDL_free(core);
    SDL_Log("Destroy engine core.\n");
}

void esz_destroy_window(esz_window_t* window)
{
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
        SDL_Delay((Uint32)delay);
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
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "%s: %s\n", __func__, SDL_GetError());
    }

    if (core->map.is_loaded)
    {
        draw_background(window, core);
        draw_map(ESZ_BG, window, core);
        // tbd.
        draw_map(ESZ_FG, window, core);
    }

    SDL_RenderPresent(window->renderer);
    SDL_RenderClear(window->renderer);

exit:
    return status;
}

const Uint8* esz_get_keyboard_state(void)
{
    return SDL_GetKeyboardState(NULL);
}

Sint32 esz_get_keycode(esz_core_t* core)
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

    (*core)->is_active = SDL_TRUE;

    return ESZ_OK;
}

SDL_bool esz_is_core_active(esz_core_t* core)
{
    return core->is_active;
}

SDL_bool esz_is_map_loaded(esz_core_t* core)
{
    return core->map.is_loaded;
}

static esz_status load_background_layer(Uint16 index, esz_window_t* window, esz_core_t* core)
{
    esz_status   status        = ESZ_OK;
    SDL_Texture* image_texture = NULL;
    SDL_Rect     dst;
    Sint32       image_width;
    Sint32       image_height;
    double       layer_width_factor;
    char         property_name[21]                        = { 0 };
    char         background_layer_image[ESZ_MAX_PATH_LEN] = { 0 };

    SDL_snprintf(property_name, 21, "background_layer_%u", index);
    load_property_by_name(property_name, core->map.tmx_map->properties, core);
    SDL_snprintf(background_layer_image, ESZ_MAX_PATH_LEN, "%s%s", core->map.resource_path, core->map.string_property);

    //if (ESZ_OK != load_texture(background_layer_image, &image_texture, window))
    if (ESZ_OK != load_texture_from_file(background_layer_image, &image_texture, window))
    {
        status = ESZ_ERROR_CRITICAL;
        goto exit;
    }

    if (0 > SDL_QueryTexture(image_texture, NULL, NULL, &image_width, &image_height))
    {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "%s: %s\n", __func__, SDL_GetError());
        status = ESZ_ERROR_CRITICAL;
        goto exit;
    }

    layer_width_factor = SDL_ceil((double)window->width / (double)image_width);

    core->map.background.layer[index].width  = image_width * (Sint32)layer_width_factor;
    core->map.background.layer[index].height = image_height;

    core->map.background.layer[index].render_target = SDL_CreateTexture(
        window->renderer,
        SDL_PIXELFORMAT_ARGB8888,
        SDL_TEXTUREACCESS_TARGET,
        core->map.background.layer[index].width,
        core->map.background.layer[index].height);

    if (! core->map.background.layer[index].render_target)
    {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "%s: %s\n", __func__, SDL_GetError());
        status = ESZ_ERROR_CRITICAL;
        goto exit;
    }

    if (0 != SDL_SetRenderTarget(window->renderer, core->map.background.layer[index].render_target))
    {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "%s: %s\n", __func__, SDL_GetError());
        status = ESZ_ERROR_CRITICAL;
        goto exit;
    }

    dst.x = 0;
    for (Sint32 pass = 0; pass < layer_width_factor; pass += 1)
    {
        dst.y = 0;
        dst.w = image_width;
        dst.h = image_height;

        if (0 > SDL_RenderCopy(window->renderer, image_texture, NULL, &dst))
        {
            SDL_LogError(SDL_LOG_CATEGORY_ERROR, "%s: %s\n", __func__, SDL_GetError());
            status = ESZ_ERROR_CRITICAL;
            goto exit;
        }

        dst.x += image_width;
    }

    if (0 > SDL_SetTextureBlendMode(core->map.background.layer[index].render_target, SDL_BLENDMODE_BLEND))
    {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "%s: %s\n", __func__, SDL_GetError());
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

void esz_load_map(const char* map_file_name, esz_window_t* window, esz_core_t* core)
{
    if (esz_is_map_loaded(core))
    {
        SDL_Log("A map has already been loaded: unload map first.\n");
        return;
    }
    core->map.is_loaded = SDL_TRUE;

    /******************************************
     * Load map and allocate required memory: *
     ******************************************/

    /**************
     * 1 Map file *
     **************/

    core->map.tmx_map = tmx_load(map_file_name);
    if (! core->map.tmx_map)
    {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "%s: %s\n", __func__, tmx_strerr());
        return;
    }

    /*************
     * 2 Objects *
     *************/

    if (ESZ_OK != init_objects(core))
    {
        esz_unload_map(window, core);
        return;
    }

    /**************
     * 3 Entities *
     **************/

    // tbd.

    /*************
     * 4 Tileset *
     *************/

    load_property_by_name("tileset_image", core->map.tmx_map->properties, core);
    if (core->map.string_property)
    {
        SDL_strlcpy(core->map.resource_path, map_file_name, ESZ_MAX_PATH_LEN - 1);

        cwk_path_get_dirname(core->map.resource_path, &core->map.resource_path_length);

        SDL_snprintf(
            core->map.resource_path, (size_t)(core->map.resource_path_length + 1), "%.*s", (int)(core->map.resource_path_length + 1), map_file_name);

        SDL_strlcpy(core->map.tileset_image, core->map.resource_path,   core->map.resource_path_length + 1);
        SDL_strlcat(core->map.tileset_image, core->map.string_property, ESZ_MAX_PATH_LEN - core->map.resource_path_length - 1);

        if (ESZ_OK != load_texture_from_file(core->map.tileset_image, &core->map.tileset_texture, window))
        {
            SDL_LogError(SDL_LOG_CATEGORY_ERROR, "%s: Error loading image '%s'.\n", __func__, core->map.tileset_image);
            esz_unload_map(window, core);
            return;
        }

        SDL_Log("Loading tileset image: %s.\n", core->map.tileset_image);
    }
    else
    {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "%s: property 'tileset_image' isn't specified.\n", __func__);
        esz_unload_map(window, core);
        return;
    }

    /*************
     * 5 Sprites *
     *************/

    if (ESZ_OK != init_sprites(window, core))
    {
        esz_unload_map(window, core);
        return;
    }

    /********************
     * 6 Animated tiles *
     ********************/

    if (ESZ_OK != init_animated_tiles(core))
    {
        esz_unload_map(window, core);
        return;
    }

    /*****************
     * 7 Backgrounds *
     *****************/

    if (ESZ_OK != init_background(window, core))
    {
        esz_unload_map(window, core);
        return;
    }

    /***********************
     * Set base attributes *
     ***********************/

    core->map.animated_tile_count = 0;

    core->map.height = core->map.tmx_map->height * core->map.tmx_map->tile_height;
    core->map.width  = core->map.tmx_map->width  * core->map.tmx_map->tile_width;

    load_property_by_name("gravitation", core->map.tmx_map->properties, core);
    if (0 < core->map.decimal_property)
    {
        core->map.gravitation = (double)core->map.decimal_property;
    }

    load_property_by_name("meter_in_pixel", core->map.tmx_map->properties, core);
    if (core->map.integer_property)
    {
        if (0 <= core->map.integer_property)
        {
            core->map.meter_in_pixel = core->map.integer_property;
        }
    }

    SDL_Log(
        "Set gravitational constant to %f (g*%dpx/s^2).\n",
        core->map.gravitation, core->map.meter_in_pixel);

    load_property_by_name("animated_tile_fps", core->map.tmx_map->properties, core);
    if (core->map.integer_property)
    {
        if (0 <= core->map.integer_property)
        {
            core->map.animated_tile_fps = core->map.integer_property;
        }
    }
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
}

void esz_lock_camera(esz_core_t* core)
{
    SDL_Log("Enable camera lock.\n");
    core->camera.is_locked = SDL_TRUE;
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

void esz_set_camera_position(const double pos_x, const double pos_y, SDL_bool pos_is_relative, esz_window_t* window, esz_core_t* core)
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
    window->logical_width  = (Sint32)((double)window->width  / factor);
    window->logical_height = (Sint32)((double)window->height / factor);

    if (0 > SDL_RenderSetLogicalSize(window->renderer, window->logical_width, window->logical_height))
    {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "%s: %s\n", __func__, SDL_GetError());
        return ESZ_ERROR_WARNING;
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
            status = ESZ_ERROR_WARNING;
        }
        SDL_SetWindowPosition(window->window, window->pos_x, window->pos_y);
        SDL_Log("Set window to windowed mode.\n");
    }
    else
    {
        SDL_GetWindowPosition(window->window, &window->pos_x, &window->pos_y);

        if (0 > SDL_SetWindowFullscreen(window->window, SDL_WINDOW_FULLSCREEN_DESKTOP))
        {
            status = ESZ_ERROR_WARNING;
        }
        SDL_Log("Set window to fullscreen mode.\n");
    }

    if (ESZ_OK != status)
    {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "%s: %s\n", __func__, SDL_GetError());
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

    SDL_SetRenderDrawColor(window->renderer, 0xa9, 0x20, 0x3e, SDL_ALPHA_OPAQUE);
    SDL_RenderClear(window->renderer);
    SDL_RenderPresent(window->renderer);

    /*************************
     * Reset base attributes *
     *************************/

    core->map.animated_tile_count        = 0;
    core->map.animated_tile_fps          = 0;
    core->map.background.alignment       = ESZ_BOT;
    core->map.background.direction       = ESZ_LEFT;
    core->map.background.layer_count     = 0;
    core->map.background.layer_shift     = 0.0;
    core->map.background.velocity        = 0.0;
    core->map.boolean_property           = SDL_FALSE;
    core->map.decimal_property           = 0.0;
    core->map.gravitation                = 0.0;
    core->map.height                     = 0;
    core->map.integer_property           = 0;
    core->map.is_loaded                  = SDL_FALSE;
    core->map.meter_in_pixel             = 0;
    core->map.object_count               = 0;
    core->map.pos_x                      = 0.0;
    core->map.pos_y                      = 0.0;
    core->map.sprite_sheet_count         = 0;
    core->map.string_property            = NULL;
    core->map.time_since_last_anim_frame = 0.0;
    core->map.width                      = 0;

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

    if (core->map.animated_tile_texture)
    {
        SDL_DestroyTexture(core->map.animated_tile_texture);
        core->map.animated_tile_texture = NULL;
    }

    /**********************************************
     * Free up allocated memory in reverse order: *
     **********************************************/

    /*****************
     * 7 Backgrounds *
     *****************/

    if (0 < core->map.background.layer_count)
    {
        for (Uint16 index = 0; index < core->map.background.layer_count; index += 1)
        {
            if (core->map.background.layer[index].render_target)
            {
                SDL_DestroyTexture(core->map.background.layer[index].render_target);
                core->map.background.layer[index].render_target = NULL;
            }
        }
    }

    if (core->map.background.layer)
    {
        SDL_free(core->map.background.layer);
    }

    if (core->map.background.render_target)
    {
        SDL_DestroyTexture(core->map.background.render_target);
        core->map.background.render_target = NULL;
    }

    /********************
     * 6 Animated tiles *
     ********************/

    if (core->map.animated_tile)
    {
        SDL_free(core->map.animated_tile);
    }

    /*************
     * 5 Sprites *
     *************/

    if (0 < core->map.sprite_sheet_count)
    {
        for (Uint16 index = 0; index < core->map.sprite_sheet_count; index += 1)
        {
            if (core->map.sprite[index].render_target)
            {
                SDL_DestroyTexture(core->map.sprite[index].render_target);
                core->map.sprite[index].render_target = NULL;
            }
        }
    }

    if (core->map.sprite)
    {
        SDL_free(core->map.sprite);
    }

    /*************
     * 4 Tileset *
     *************/

    if (core->map.tileset_texture)
    {
        SDL_DestroyTexture(core->map.tileset_texture);
        core->map.tileset_texture = NULL;
    }

    /**************
     * 3 Entities *
     **************/

    // tbd.

    /*************
     * 2 Objects *
     *************/

    if (core->map.object)
    {
        SDL_free(core->map.object);
    }

    /**************
     * 1 Map file *
     **************/

    if (core->map.tmx_map)
    {
        tmx_map_free(core->map.tmx_map);
    }

    if (core->event.map_unloaded_cb)
    {
        core->event.map_unloaded_cb(window, core);
    }

    SDL_Log("Unload map.\n");
}

void esz_unlock_camera(esz_core_t* core)
{
    SDL_Log("Disable camera lock.\n");
    core->camera.is_locked = SDL_FALSE;
}

void esz_update_core(esz_window_t* window, esz_core_t* core)
{
    poll_events(window, core);
}

/*********************
 * Private Functions *
 *********************/

static void count_map_layers(tmx_layer* layer, Uint16* layer_count)
{
    *layer_count = 0;

    while (layer)
    {
        if (L_LAYER == layer->type && layer->visible)
        {
            *layer_count += 1;
        }
        layer = layer->next;
    }
}

static void count_objects(tmx_object* tmx_object, Uint16** object_count)
{
    if (tmx_object)
    {
        (**object_count) += 1;
    }

    if (tmx_object && tmx_object->next)
    {
        count_objects(tmx_object->next, object_count);
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
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "%s: %s\n", __func__, SDL_GetError());
        return ESZ_ERROR_WARNING;
    }

    if (is_camera_at_horizontal_boundary(core))
    {
        /*****************************************************************
         * Do not move background when camera is at horizontal boundary. *
         *****************************************************************/
        core->map.background.velocity = 0.0;
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
        core->map.background.velocity = 0.0;
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
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "%s: %s\n", __func__, SDL_GetError());
        return ESZ_ERROR_CRITICAL;
    }

    return ESZ_OK;
}

static Uint16 get_camera_target(esz_core_t* core)
{
    return core->camera.target_entity_id;
}

static esz_status init_animated_tiles(esz_core_t* core)
{
    tmx_layer* layer               = core->map.tmx_map->ly_head;
    Uint32     animated_tile_count = 0;

    /************************
     * Count animated tiles *
     ************************/

    while (layer)
    {
        Uint32 gid;

        if (L_LAYER == layer->type)
        {
            if (layer->visible)
            {
                for (Uint16 index_height = 0; index_height < core->map.tmx_map->height; index_height += 1)
                {
                    for (Uint32 index_width = 0; index_width < core->map.tmx_map->width; index_width += 1)
                    {
                        gid = remove_gid_flip_bits(
                            layer->content.gids[(index_height * core->map.tmx_map->width) + index_width]);

                        if (core->map.tmx_map->tiles[gid])
                        {
                            if (core->map.tmx_map->tiles[gid]->animation)
                            {
                                animated_tile_count += 1;
                            }
                        }
                    }
                }
            }
        }
        layer = layer->next;
    }

    if (0 == animated_tile_count)
    {
        SDL_Log("No animated tiles found.\n");
        return ESZ_OK;
    }

    /**************************************
     * Allocate memory for animated tiles *
     **************************************/

    if (0 < animated_tile_count)
    {
        core->map.animated_tile = SDL_calloc((size_t)animated_tile_count, sizeof(struct esz_animated_tile));

        if (! core->map.animated_tile)
        {
            SDL_LogError(SDL_LOG_CATEGORY_ERROR, "%s: error allocating memory.\n", __func__);
            return ESZ_ERROR_CRITICAL;
        }
    }

    SDL_Log("Initialise %u animated tile(s).\n", animated_tile_count);
    return ESZ_OK;
}

static esz_status init_background(esz_window_t* window, esz_core_t* core)
{
    load_property_by_name("background_layer_count", core->map.tmx_map->properties, core);

    if (core->map.integer_property)
    {
        core->map.background.layer_count = core->map.integer_property;
    }

    load_property_by_name("background_layer_shift", core->map.tmx_map->properties, core);

    if (0 < core->map.decimal_property)
    {
        core->map.background.layer_shift = (double)core->map.decimal_property;
    }

    load_property_by_name("background_is_top_aligned", core->map.tmx_map->properties, core);

    if (core->map.boolean_property)
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
        for (Uint16 index = 0; index < core->map.background.layer_count; index += 1)
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
    Uint16*    object_count = &core->map.object_count;
    tmx_layer* layer;

    /*****************
     * Count objects *
     *****************/

    if (core->map.object_count)
    {
        return ESZ_OK;
    }

    layer = core->map.tmx_map->ly_head;

    while (layer)
    {
        if (L_OBJGR == layer->type)
        {
            count_objects(layer->content.objgr->head, &object_count);
        }

        layer = layer->next;
    }

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
    load_property_by_name("sprite_sheet_count", core->map.tmx_map->properties, core);
    if (core->map.integer_property)
    {
        if (0 <= core->map.integer_property)
        {
            core->map.sprite_sheet_count = core->map.integer_property;
        }
    }

    core->map.sprite = SDL_calloc((size_t)core->map.sprite_sheet_count, sizeof(struct esz_sprite));

    if (! core->map.sprite)
    {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "%s: error allocating memory.\n", __func__);
        return ESZ_ERROR_CRITICAL;
    }

    if (0 < core->map.sprite_sheet_count)
    {
        char property_name[17]                    = { 0 };
        char sprite_sheet_image[ESZ_MAX_PATH_LEN] = { 0 };

        for (Uint16 index = 0; index < core->map.sprite_sheet_count; index += 1)
        {
            SDL_snprintf(property_name, 17, "sprite_sheet_%u", index);
            load_property_by_name(property_name, core->map.tmx_map->properties, core);
            SDL_snprintf(sprite_sheet_image, ESZ_MAX_PATH_LEN, "%s%s", core->map.resource_path, core->map.string_property);

            core->map.sprite[index].id = index;

            //if (ESZ_OK != load_texture(sprite_sheet_image, &core->map.sprite[index].render_target, window))
            if (ESZ_OK != load_texture_from_file(sprite_sheet_image, &core->map.sprite[index].render_target, window))
            {
                return ESZ_ERROR_CRITICAL;
            }
        }
    }

    return ESZ_OK;
}

static SDL_bool is_camera_locked(esz_core_t* core)
{
    return core->camera.is_locked;
}

static SDL_bool is_camera_at_horizontal_boundary(esz_core_t* core)
{
    return core->camera.is_at_horizontal_boundary;
}

static void load_property_by_name(const char* property_name, tmx_property* property, esz_core_t* core)
{
    core->map.boolean_property = SDL_FALSE;
    core->map.decimal_property = 0.0;
    core->map.integer_property = 0;
    core->map.string_property  = NULL;

    SDL_strlcpy(core->map.search_pattern, property_name, ESZ_MAX_PATTERN_LEN);

    tmx_property_foreach(property, store_property, (void*)core);
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
    Uint32         pixel_format;
    unsigned char* data;

    data = stbi_load(file_name, &width, &height, &orig_format, req_format);

    if (NULL == data)
    {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "%s: %s\n", __func__, stbi_failure_reason());
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
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "%s: %s\n", __func__, SDL_GetError());
        stbi_image_free(data);
        return ESZ_ERROR_CRITICAL;
    }

    *texture = SDL_CreateTextureFromSurface(window->renderer, surface);

    SDL_FreeSurface(surface);
    stbi_image_free(data);

    SDL_Log("Load image file: %s.\n", file_name);
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

static Uint32 remove_gid_flip_bits(Uint32 gid)
{
    return gid & TMX_FLIP_BITS_REMOVAL;
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
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "%s: %s\n", __func__, SDL_GetError());
        return ESZ_ERROR_CRITICAL;
    }

    for (Sint32 index = 0; index < core->map.background.layer_count; index += 1)
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

static esz_status render_background_layer(Sint32 index, esz_window_t* window, esz_core_t* core)
{
    SDL_Rect dst;
    double   pos_x_a;
    double   pos_x_b;
    Sint32   width = 0;

    if (0 > SDL_QueryTexture(core->map.background.layer[index].render_target, NULL, NULL, &width, NULL))
    {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "%s: %s\n", __func__, SDL_GetError());
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
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "%s: %s\n", __func__, SDL_GetError());
        return ESZ_ERROR_CRITICAL;
    }

    if (0 == index)
    {
        /************************
         * Set background color *
         ************************/

        SDL_SetRenderDrawColor(
            window->renderer,
            (core->map.tmx_map->backgroundcolor >> 16) & 0xFF,
            (core->map.tmx_map->backgroundcolor >> 8)  & 0xFF,
            (core->map.tmx_map->backgroundcolor)       & 0xFF,
            0);

        SDL_RenderClear(window->renderer);
    }

    dst.x = (int)pos_x_a;
    dst.w = width;
    dst.h = core->map.background.layer[index].height;

    if (0 > SDL_RenderCopyEx(window->renderer, core->map.background.layer[index].render_target, NULL, &dst, 0, NULL, SDL_FLIP_NONE))
    {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "%s: %s\n", __func__, SDL_GetError());
        return ESZ_ERROR_CRITICAL;
    }

    dst.x = (int)pos_x_b;
    if (0 > SDL_RenderCopyEx(window->renderer, core->map.background.layer[index].render_target, NULL, &dst, 0, NULL, SDL_FLIP_NONE))
    {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "%s: %s\n", __func__, SDL_GetError());
        return ESZ_ERROR_CRITICAL;
    }

    return ESZ_OK;
}

static esz_status render_map(const esz_layer_type layer_type, esz_window_t *window, esz_core_t* core)
{
    tmx_layer* layer                 = core->map.tmx_map->ly_head;
    SDL_bool   render_animated_tiles = SDL_FALSE;

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
        render_animated_tiles = SDL_TRUE;
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
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "%s: %s\n", __func__, SDL_GetError());
        return ESZ_ERROR_CRITICAL;
    }
    else
    {
        if (0 > SDL_SetTextureBlendMode(core->map.render_target[layer_type], SDL_BLENDMODE_BLEND))
        {
            SDL_LogError(SDL_LOG_CATEGORY_ERROR, "%s: %s\n", __func__, SDL_GetError());
            return ESZ_ERROR_CRITICAL;
        }
    }

    if (0 > SDL_SetRenderTarget(window->renderer, core->map.render_target[layer_type]))
    {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "%s: %s\n", __func__, SDL_GetError());
        return ESZ_ERROR_CRITICAL;
    }

    SDL_RenderClear(window->renderer);

    /************************************
     * Update and render animated tiles *
     ************************************/

    core->map.time_since_last_anim_frame += window->time_since_last_frame;

    if (0 < core->map.animated_tile_count &&
        core->map.time_since_last_anim_frame >= 1.0 / (double)(core->map.animated_tile_fps) &&
        render_animated_tiles)
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
                (int)(core->map.tmx_map->width  * core->map.tmx_map->tile_width),
                (int)(core->map.tmx_map->height * core->map.tmx_map->tile_height));
        }

        if (! core->map.animated_tile_texture)
        {
            SDL_LogError(SDL_LOG_CATEGORY_ERROR, "%s: %s\n", __func__, SDL_GetError());
            return ESZ_ERROR_CRITICAL;
        }

        if (0 > SDL_SetRenderTarget(window->renderer, core->map.animated_tile_texture))
        {
            SDL_LogError(SDL_LOG_CATEGORY_ERROR, "%s: %s\n", __func__, SDL_GetError());
            return ESZ_ERROR_CRITICAL;
        }

        for (Uint16 index = 0; core->map.animated_tile_count > index; index += 1)
        {
            Uint32       gid          = core->map.animated_tile[index].gid;
            Uint32       tile_id      = core->map.animated_tile[index].id + 1;
            Uint32       next_tile_id = 0;
            SDL_Rect     dst;
            SDL_Rect     src;
            tmx_tileset* tileset;

            tileset = core->map.tmx_map->tiles[1]->tileset;
            src.x   = (int)core->map.tmx_map->tiles[tile_id]->ul_x;
            src.y   = (int)core->map.tmx_map->tiles[tile_id]->ul_y;
            src.w   = dst.w = (int)tileset->tile_width;
            src.h   = dst.h = (int)tileset->tile_height;
            dst.x           = (int)core->map.animated_tile[index].dst_x;
            dst.y           = (int)core->map.animated_tile[index].dst_y;

            if (0 > SDL_RenderCopy(window->renderer, core->map.tileset_texture, &src, &dst))
            {
                SDL_LogError(SDL_LOG_CATEGORY_ERROR, "%s: %s\n", __func__, SDL_GetError());
                return ESZ_ERROR_CRITICAL;
            }

            core->map.animated_tile[index].current_frame += 1;

            if (core->map.animated_tile[index].current_frame >= core->map.animated_tile[index].animation_length)
            {
                core->map.animated_tile[index].current_frame = 0;
            }

            next_tile_id = core->map.tmx_map->tiles[gid]->animation[core->map.animated_tile[index].current_frame].tile_id;
            core->map.animated_tile[index].id = next_tile_id;
        }

        if (0 > SDL_SetRenderTarget(window->renderer, core->map.map_layer[layer_type]))
        {
            SDL_LogError(SDL_LOG_CATEGORY_ERROR, "%s: %s\n", __func__, SDL_GetError());
            return ESZ_ERROR_CRITICAL;
        }

        if (0 > SDL_SetRenderTarget(window->renderer, core->map.render_target[layer_type]))
        {
            SDL_LogError(SDL_LOG_CATEGORY_ERROR, "%s: %s\n", __func__, SDL_GetError());
            return ESZ_ERROR_CRITICAL;
        }

        if (0 > SDL_SetTextureBlendMode(core->map.animated_tile_texture, SDL_BLENDMODE_BLEND))
        {
            SDL_LogError(SDL_LOG_CATEGORY_ERROR, "%s: %s\n", __func__, SDL_GetError());
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
            (int)(core->map.tmx_map->width  * core->map.tmx_map->tile_width),
            (int)(core->map.tmx_map->height * core->map.tmx_map->tile_height)
        };

        if (0 > SDL_RenderCopyEx(window->renderer, core->map.map_layer[layer_type], NULL, &dst, 0, NULL, SDL_FLIP_NONE))
        {
            SDL_LogError(SDL_LOG_CATEGORY_ERROR, "%s: %s\n", __func__, SDL_GetError());
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
                    SDL_LogError(SDL_LOG_CATEGORY_ERROR, "%s: %s\n", __func__, SDL_GetError());
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
        (int)(core->map.tmx_map->width  * core->map.tmx_map->tile_width),
        (int)(core->map.tmx_map->height * core->map.tmx_map->tile_height));

    if (! core->map.map_layer[layer_type])
    {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "%s: %s\n", __func__, SDL_GetError());
        return ESZ_ERROR_CRITICAL;
    }

    if (0 > SDL_SetRenderTarget(window->renderer, core->map.map_layer[layer_type]))
    {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "%s: %s\n", __func__, SDL_GetError());
        return ESZ_ERROR_CRITICAL;
    }

    while (layer)
    {
        SDL_bool     is_in_foreground = SDL_FALSE;
        SDL_bool     render_layer     = SDL_FALSE;
        Uint32       gid;
        SDL_Rect     dst;
        SDL_Rect     src;
        tmx_tileset* tileset;

        if (L_LAYER == layer->type)
        {
            load_property_by_name("is_in_foreground", layer->properties, core);

            is_in_foreground = core->map.boolean_property;

            if (ESZ_BG == layer_type && SDL_FALSE == is_in_foreground)
            {
                render_layer = SDL_TRUE;
            }
            else if (ESZ_FG == layer_type && SDL_TRUE == is_in_foreground)
            {
                render_layer = SDL_TRUE;
            }

            if (layer->visible && render_layer)
            {
                for (Uint32 index_height = 0; index_height < core->map.tmx_map->height; index_height += 1)
                {
                    for (Uint32 index_width = 0; index_width < core->map.tmx_map->width; index_width += 1)
                    {
                        gid = remove_gid_flip_bits(
                            layer->content.gids[(index_height * core->map.tmx_map->width) + index_width]);

                        if (core->map.tmx_map->tiles[gid])
                        {
                            tileset = core->map.tmx_map->tiles[1]->tileset;
                            src.x   = (int)core->map.tmx_map->tiles[gid]->ul_x;
                            src.y   = (int)core->map.tmx_map->tiles[gid]->ul_y;
                            src.w   = dst.w = (int)tileset->tile_width;
                            src.h   = dst.h = (int)tileset->tile_height;
                            dst.x           = (int)(index_width  * tileset->tile_width);
                            dst.y           = (int)(index_height * tileset->tile_height);

                            SDL_RenderCopy(window->renderer, core->map.tileset_texture, &src, &dst);

                            if (render_animated_tiles && core->map.tmx_map->tiles[gid]->animation)
                            {
                                Uint32 animation_length = core->map.tmx_map->tiles[gid]->animation_len;
                                Uint32 id               = core->map.tmx_map->tiles[gid]->animation[0].tile_id;

                                core->map.animated_tile[core->map.animated_tile_count].gid              = gid;
                                core->map.animated_tile[core->map.animated_tile_count].id               = id;
                                core->map.animated_tile[core->map.animated_tile_count].dst_x            = dst.x;
                                core->map.animated_tile[core->map.animated_tile_count].dst_y            = dst.y;
                                core->map.animated_tile[core->map.animated_tile_count].current_frame    = 0;
                                core->map.animated_tile[core->map.animated_tile_count].animation_length = animation_length;

                                core->map.animated_tile_count += 1;
                            }
                        }
                    }
                }
                SDL_Log("Render map layer: %s\n", layer->name);
            }
        }
        layer = layer->next;
    }

    if (0 > SDL_SetRenderTarget(window->renderer, core->map.render_target[layer_type]))
    {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "%s: %s\n", __func__, SDL_GetError());
        return ESZ_ERROR_CRITICAL;
    }

    if (0 > SDL_SetTextureBlendMode(core->map.map_layer[layer_type], SDL_BLENDMODE_BLEND))
    {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "%s: %s\n", __func__, SDL_GetError());
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
    core->camera.is_at_horizontal_boundary = SDL_FALSE;
    core->camera.max_pos_x                 = (Sint32)core->map.width  - window->logical_width;
    core->camera.max_pos_y                 = (Sint32)core->map.height - window->logical_height;

    if (0 >= core->camera.pos_x)
    {
        core->camera.pos_x                     = 0;
        core->camera.is_at_horizontal_boundary = SDL_TRUE;
    }

    if (0 >= core->camera.pos_y)
    {
        core->camera.pos_y = 0;
    }

    if (core->camera.pos_x >= core->camera.max_pos_x)
    {
        core->camera.pos_x                     = core->camera.max_pos_x;
        core->camera.is_at_horizontal_boundary = SDL_TRUE;
    }

    if (core->camera.pos_y >= core->camera.max_pos_y)
    {
        core->camera.pos_y = core->camera.max_pos_y;
    }
}

static void set_camera_target(const Uint16 target_entity_id, esz_core_t* core)
{
    core->camera.target_entity_id = target_entity_id;
}

static void store_property(tmx_property* property, void* core)
{
    esz_core_t* core_ptr = core;

    /* Shouldn't be called repeatedly to minimise performance cost:
     * intended to use for initialisation only.
     */
    if (0 == SDL_strncmp(property->name, core_ptr->map.search_pattern, SDL_strlen((char*)core_ptr->map.search_pattern)))
    {
        switch (property->type)
        {
            case PT_COLOR:
            case PT_NONE:
                // tbd.
                break;
            case PT_BOOL:
                SDL_Log("Loading boolean property '%s': %u\n", core_ptr->map.search_pattern, property->value.boolean);
                core_ptr->map.boolean_property = (SDL_bool)property->value.boolean;
                break;
            case PT_FILE:
                SDL_Log("Loading string property '%s': %s\n", core_ptr->map.search_pattern, property->value.file);
                core_ptr->map.string_property  = property->value.file;
                break;
            case PT_FLOAT:
                SDL_Log("Loading floating point property '%s': %f\n", core_ptr->map.search_pattern, (double)property->value.decimal);
                core_ptr->map.decimal_property = (double)property->value.decimal;
                break;
            case PT_INT:
                SDL_Log("Loading integer property '%s': %d\n", core_ptr->map.search_pattern, property->value.integer);
                core_ptr->map.integer_property = property->value.integer;
                break;
            case PT_STRING:
                SDL_Log("Loading string property '%s': %s\n", core_ptr->map.search_pattern, property->value.string);
                core_ptr->map.string_property  = property->value.string;
                break;
        }
    }
}

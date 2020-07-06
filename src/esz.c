// Spdx-License-Identifier: Beerware
/**
 * @file    esz.c
 * @brief   eszFW game engine
 * @details A cross-platform game engine written in C99
 */

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#include <emscripten/html5.h>
#endif
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <SDL_mixer.h>
#include <cwalk.h>
#include <tmx.h>
#include "esz.h"

/**********************
 * Private Prototypes *
 **********************/

static void       CountMapLayers(tmx_layer* layer, Uint16* layer_count);
static void       CountObjects(tmx_object* tmx_object, Uint16** object_count);
static esz_Status DrawMap(const esz_LayerType layer_type, esz_Window *window, esz_Core *core);
static Uint16     GetCameraTarget(esz_Core* core);
static esz_Status InitAnimatedTiles(esz_Core* core);
static esz_Status InitObjects(esz_Core* core);
static esz_Status InitSprites(esz_Window* window, esz_Core* core);
static SDL_bool   IsCameraLocked(esz_Core* core);
static void       LoadPropertyByName(const char* property_name, tmx_property* property, esz_Core* core);
static esz_Status LoadSprite(const char* sprite_sheet_image, esz_Sprite* sprite, esz_Window* window);
static void       MoveCameraToTarget(esz_Core* core);
static void       PollEvents(esz_Window* window, esz_Core* core);
static Uint16     RemoveGidFlipBits(Uint16 gid);
static double     RoundNumber(double number);
static SDL_bool   SetCameraBoundariesToMapSize(esz_Window* window, esz_Core* core);
static void       SetCameraTarget(const Uint16 target_entity_id, esz_Core* core);
static void       StoreProperty(tmx_property* property, void* core);
static void       UpdateCamera(esz_Window* window, esz_Core* core);

/*
static void      ConnectHorizontalMapEndsForEntity(Entity* hEntity);
static void      ConnectMapEndsForEntity(Entity* hEntity);
static void      ConnectVerticalMapEndsForEntity(Entity* hEntity);
static eszStatus DrawEntity(const Entity* hEntity);
static void      GetEntity(tmx_object* pstTiledObject, Uint16 u16Index, Object ahObject[]);
static SDL_bool  IsEntityMoving(const Entity* hEntity);
static SDL_bool  IsEntityRising(const Entity* hEntity);
static void      SetEntityPosition(const double dPosX, const double dPosY, Entity* hEntity);
static void      UpdateEntities(void);
static void      UpdateEntity(Entity* hEntity);
*/

/********************
 * Public functions *
 ********************/

/**
 * 
 */
SDL_bool esz_BoundingBoxesDoIntersect(const esz_AABB bb_a, const esz_AABB bb_b)
{
    double bb_a_x = bb_b.left - bb_a.right;
    double bb_a_y = bb_b.top  - bb_a.bottom;
    double bb_b_x = bb_a.left - bb_b.right;
    double bb_b_y = bb_a.top  - bb_b.bottom;

    if (0.0f < bb_a_x || 0.0f < bb_a_y)
    {
        return SDL_FALSE;
    }

    if (0.0f < bb_b_x || 0.0f < bb_b_y)
    {
        return SDL_FALSE;
    }

    return SDL_TRUE;
}

/**
 * 
 */
esz_Status esz_CreateWindow(const char* window_title, esz_WindowConfig* config, esz_Window** window)
{
    esz_Status      status;
    SDL_DisplayMode display_mode;

    *window = SDL_calloc(1, sizeof(struct esz_Window_t));

    if (! *window)
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "esz_CreateWindow(): error allocating memory.\n");
        status = ESZ_ERROR_CRITICAL;
        goto error;
    }

    (*window)->width          = config->width;
    (*window)->height         = config->height;
    (*window)->logical_width  = config->logical_width;
    (*window)->logical_height = config->logical_height;

    if (config->enable_fullscreen)
    {
        (*window)->flags = (*window)->flags | SDL_WINDOW_FULLSCREEN_DESKTOP;
    }

    /*******************************
     * Initialise Video sub-system *
     *******************************/

    if (0 > SDL_Init(SDL_INIT_VIDEO))
    {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "esz_CreateWindow(): %s\n", SDL_GetError());
        status = ESZ_ERROR_CRITICAL;
        goto error;
    }

    if (IMG_INIT_PNG != IMG_Init(IMG_INIT_PNG))
    {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "esz_CreateWindow(): %s\n", IMG_GetError());
        status = ESZ_ERROR_CRITICAL;
        goto error;
    }

    if (0 != SDL_GetCurrentDisplayMode(0, &display_mode))
    {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "esz_CreateWindow(): %s\n", SDL_GetError());
        status = ESZ_ERROR_CRITICAL;
        goto error;
    }

#ifdef __ANDROID__
    (*window)->flags  = 0;
    (*window)->width  = display_mode.w;
    (*window)->height = display_mode.h;
#endif
    (*window)->refresh_rate  = display_mode.refresh_rate;

    (*window)->window = SDL_CreateWindow(
        window_title,
        SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED,
        (*window)->width,
        (*window)->height,
        (*window)->flags);

    if (! (*window)->window)
    {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "esz_CreateWindow(): %s\n", SDL_GetError());
        status = ESZ_ERROR_CRITICAL;
        goto error;
    }

    SDL_GetWindowSize((*window)->window, &(*window)->width, &(*window)->height);

    (*window)->zoom_level         = (double)(*window)->height / (double)(*window)->logical_height;
    (*window)->initial_zoom_level = (*window)->zoom_level;

    /* Get index of OpenGL ES 2 rendering driver and create 2D rendering
     * context.
     */
    for (int driver_index = 0; driver_index < SDL_GetNumRenderDrivers(); driver_index += 1)
    {
        SDL_RendererInfo renderer_info = { 0 };
        SDL_GetRenderDriverInfo(driver_index, &renderer_info );

        if (! SDL_strstr(renderer_info.name, "opengles2"))
        {
            continue;
        }
        else
        {
            (*window)->renderer = SDL_CreateRenderer(
                (*window)->window, driver_index,
                SDL_RENDERER_ACCELERATED | SDL_RENDERER_TARGETTEXTURE | SDL_RENDERER_PRESENTVSYNC);

            if (! (*window)->renderer)
            {
                SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "esz_CreateWindow(): %s\n", SDL_GetError());
                status = ESZ_ERROR_CRITICAL;
                goto error;
            }
            break;
        }
    }

    if (! (*window)->renderer)
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "esz_CreateWindow(): opengles2 rendering driver not found.\n");
        status = ESZ_ERROR_CRITICAL;
        goto error;
    }

    SDL_Log(
        "Setting up window at resolution %dx%d @ %d Hz.\n",
        (*window)->width,
        (*window)->height,
        (*window)->refresh_rate);

    esz_SetZoomLevel((*window)->zoom_level, *window);
    SDL_Log("Set initial zoom-level to factor %f.\n", (*window)->zoom_level);

error:
    return status;
}

/**
 * @brief Deactivate engine core
 * @param core
 *        Engine core
 */
void esz_DeactivateCore(esz_Core* core)
{
    core->is_active = SDL_FALSE;
}

/**
 * @brief Destroy engine core
 * @param core
 *        Engine core
 */
void esz_DestroyCore(esz_Core* core)
{
    SDL_free(core);
    SDL_Log("Destroy engine core.\n");
}

/**
 * @brief Destroy window
 * @param window
 *        Window
 */
void esz_DestroyWindow(esz_Window* window)
{
    IMG_Quit();

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

void esz_DrawFrame(Uint32* time_a, Uint32* time_b, esz_Window* window, esz_Core* core)
{
    double   delta_time = 0.f;
    //SDL_Rect dst;

    DrawMap(ESZ_BACKGROUND, window, core);

    // tbd.

    DrawMap(ESZ_FOREGROUND, window, core);

    *time_b = *time_a;
    *time_a = SDL_GetTicks();

    if (*time_a > *time_b)
    {
        *time_a = *time_b;
    }

    delta_time                     = (double)(*time_b - *time_a) / 1000.f;
    window->time_since_last_frame  = 1000.f / (double)window->refresh_rate - delta_time;
    window->time_since_last_frame /= 1000.f;

    /*dst.x = 0;
    dst.y = 0;
    dst.w = window->width;
    dst.h = window->height;

    if (0 > SDL_SetRenderTarget(window->renderer, NULL))
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "DrawFrame(): %s\n", SDL_GetError());
    }

    if (core->map.is_loaded)
    {
        if (0 > SDL_RenderCopy(window->renderer, core->map.texture, NULL, &dst))
        {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "DrawFrame(): %s\n", SDL_GetError());
        }
    }*/

    SDL_RenderPresent(window->renderer);
    SDL_RenderClear(window->renderer);
}

/**
 * @brief   Get current keycode
 * @details This function can be used to determine the current keycode
 *          inside a event callback function
 * @param   core
 *          Engine core
 * @return  Current keycode
 */
Uint32 esz_GetKeycode(esz_Core* core)
{
    return core->event.event_handle.key.keysym.sym;
}

double esz_GetTimeSinceLastFrame(esz_Window* window)
{
    return window->time_since_last_frame;
}

/**
 * @brief Initialise engine core
 * @param core
 *        Engine core
 */
esz_Status esz_InitCore(esz_Core** core)
{
    *core = SDL_calloc(1, sizeof(struct esz_Core_t));

    if (! *core)
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "esz_InitCore(): error allocating memory.\n");
        return ESZ_ERROR_CRITICAL;
    }

    (*core)->is_active = SDL_TRUE;

    // DEBUG!
    (*core)->camera.pos_x = 128;
    (*core)->camera.pos_y = 500;
    // DEBUG!

    return ESZ_OK;
}

/**
 * @brief  Check if engine core is active
 * @param  core
 *         Engine core
 * @return Active state
 */
SDL_bool esz_IsCoreActive(esz_Core* core)
{
    return core->is_active;
}

/**
 * @brief Load map
 * @param map_file_name
 *        File name of the map to load
 * @param window
 *        window handle
 * @param core
 *        Engine core
 */
void esz_LoadMap(const char* map_file_name, esz_Window* window, esz_Core* core)
{
    if (core->map.is_loaded)
    {
        SDL_Log("A map has already been loaded: unload map first.\n");
        return;
    }

    /******************************************
     * Load map and allocate required memory: *
     ******************************************/

    /**************
     * 1 Map file *
     **************/

    core->map.tmx_map = tmx_load(map_file_name);
    if (! core->map.tmx_map)
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "esz_LoadMap(): %s\n", tmx_strerr());
        return;
    }

    /*************
     * 2 Objects *
     *************/

    if (ESZ_OK != InitObjects(core))
    {
        esz_UnloadMap(window, core);
        return;
    }

    /**************
     * 3 Entities *
     **************/

    // tbd.

    /*************
     * 4 Tileset *
     *************/

    LoadPropertyByName("tileset_image", core->map.tmx_map->properties, core);
    if (core->map.string_property)
    {
        SDL_strlcpy(core->map.resource_path, map_file_name, ESZ_MAX_PATH_LEN - 1);

        cwk_path_get_dirname(core->map.resource_path, &core->map.resource_path_length);

        SDL_snprintf(
            core->map.resource_path, (int)(core->map.resource_path_length + 1), "%.*s", (int)(core->map.resource_path_length + 1), map_file_name);

        SDL_strlcpy(core->map.tileset_image, core->map.resource_path,     core->map.resource_path_length + 1);
        SDL_strlcat(core->map.tileset_image, core->map.string_property,   ESZ_MAX_PATH_LEN - core->map.resource_path_length - 1);

        core->map.tileset_texture = IMG_LoadTexture(window->renderer, core->map.tileset_image);

        if (! core->map.tileset_texture)
        {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "esz_LoadMap(): %s\n", IMG_GetError());
            esz_UnloadMap(window, core);
            return;
        }

        SDL_Log("Loading tileset image: %s.\n", core->map.tileset_image);
    }
    else
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "esz_LoadMap(): property 'tileset_image' isn't specified.\n");
        esz_UnloadMap(window, core);
        return;
    }

    /*************
     * 5 Sprites *
     *************/

    if (ESZ_OK != InitSprites(window, core))
    {
        esz_UnloadMap(window, core);
        return;
    }

    /********************
     * 6 Animated tiles *
     ********************/

    if (ESZ_OK != InitAnimatedTiles(core))
    {
        esz_UnloadMap(window, core);
        return;
    }

    /***********************
     * Set base attributes *
     ***********************/

    core->map.animated_tile_count = 0;

    core->map.height = core->map.tmx_map->height * core->map.tmx_map->tile_height;
    core->map.width  = core->map.tmx_map->width  * core->map.tmx_map->tile_width;

    LoadPropertyByName("gravitation", core->map.tmx_map->properties, core);
    if (core->map.decimal_property)
    {
        core->map.gravitation = core->map.decimal_property;
    }

    LoadPropertyByName("meter_in_pixel", core->map.tmx_map->properties, core);
    if (core->map.integer_property)
    {
        core->map.meter_in_pixel = core->map.integer_property;
    }

    SDL_Log(
        "Set gravitational constant to %f (g*%dpx/s^2).\n",
        core->map.gravitation, core->map.meter_in_pixel);

    LoadPropertyByName("animated_tile_fps", core->map.tmx_map->properties, core);
    if (core->map.integer_property)
    {
        core->map.animated_tile_fps = core->map.integer_property;
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

    core->map.is_loaded = SDL_TRUE;
    SDL_Log(
        "Load map file: %s containing %d object(s).\n",
        map_file_name, core->map.object_count);
}

/**
 * @brief Enable camera lock
 */
void esz_LockCamera(esz_Core* core)
{
    SDL_Log("Enable camera lock.\n");
    core->camera.flags |= 1UL << CAMERA_IS_LOCKED;
}

/**
 * @brief  Register event callback
 * @param  event_type
 *         Event type
 * @param  func_callback
 *         Callback function or NULL
 * @param  core
 *         Engine core
 */
void esz_RegisterEventCallback(const esz_EventType event_type, void (*callback)(void* window, void* core), esz_Core* core)
{
    switch (event_type)
    {
        case EVENT_KEYDOWN:
            core->event.key_down_cb      = callback;
            break;
        case EVENT_KEYUP:
            core->event.key_up_cb        = callback;
            break;
        case EVENT_FINGERDOWN:
            core->event.finger_down_cb   = callback;
            break;
        case EVENT_FINGERUP:
            core->event.finger_up_cb     = callback;
            break;
        case EVENT_FINGERMOTION:
            core->event.finger_motion_cb = callback;
            break;
        case EVENT_MAP_LOADED:
            core->event.map_loaded_cb    = callback;
            break;
        case EVENT_MAP_UNLOADED:
            core->event.map_unloaded_cb  = callback;
            break;
        case EVENT_MULTIGESTURE:
            core->event.multi_gesture_cb = callback;
            break;
    }
}

/**
 * @brief  Set zoom level
 * @param  factor
 *         Zoom-level factor
 * @param  window
 *         Window handle
 * @return Status code
 * @retval ESZ_OK
 *         OK
 * @retval ESZ_ERROR_WARNING
 *         Zoom-level could not be changed
 */
esz_Status esz_SetZoomLevel(const double factor, esz_Window* window)
{
    window->zoom_level     = factor;
    window->logical_width  = window->width  / factor;
    window->logical_height = window->height / factor;

    if (0 > SDL_RenderSetLogicalSize(window->renderer, window->logical_width, window->logical_height))
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "esz_SetZoomLevel(): %s\n", SDL_GetError());
        return ESZ_ERROR_WARNING;
    }
    else
    {
        return ESZ_OK;
    }
}

/**
 * @brief Toggle between fullscreen and windowed mode
 */
esz_Status esz_ToggleFullscreen(esz_Window* window)
{
    esz_Status status = ESZ_OK;

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
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "esz_ToggleFullscreen(): %s\n", SDL_GetError());
    }

    return status;
}

/**
 *
 */
void esz_UnloadMap(esz_Window* window, esz_Core* core)
{
    if (! core->map.is_loaded)
    {
        SDL_Log("No map has been loaded.\n");
        return;
    }

    /*************************
     * Reset base attributes *
     *************************/

    core->map.decimal_property           = 0.f;
    core->map.gravitation                = 0.f;
    core->map.pos_x                      = 0.f;
    core->map.pos_y                      = 0.f;
    core->map.time_since_last_anim_frame = 0.f;
    core->map.string_property            = NULL;
    core->map.integer_property           = 0;
    core->map.animated_tile_count        = 0;
    core->map.height                     = 0;
    core->map.object_count               = 0;
    core->map.width                      = 0;
    core->map.animated_tile_fps          = 0;
    core->map.meter_in_pixel             = 0;
    core->map.sprite_sheet_count         = 0;
    core->map.boolean_property           = SDL_FALSE;
    core->map.is_loaded                  = SDL_FALSE;

    /****************************************************
     * Destroy textures that where created in DrawMap() *
     ****************************************************/

    for (Uint8 layer = 0; ESZ_LAYER_MAX > layer; layer += 1)
    {
        if (core->map.map_layer[layer])
        {
            SDL_DestroyTexture(core->map.map_layer[layer]);
        }
        core->map.map_layer[layer] = NULL;
    }

    if (core->map.animated_tile_texture)
    {
        SDL_DestroyTexture(core->map.animated_tile_texture);
        core->map.animated_tile_texture = NULL;
    }

    /*if (core->map.texture)
    {
        SDL_DestroyTexture(core->map.texture);
        core->map.texture = NULL;
    }*/

    /**********************************************
     * Free up allocated memory in reverse order: *
     **********************************************/

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
            if (core->map.sprite[index].texture)
            {
                SDL_DestroyTexture(core->map.sprite[index].texture);
                core->map.sprite[index].texture = NULL;
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

/**
 * @brief Disable camera lock
 */
void esz_UnlockCamera(esz_Core* core)
{
    SDL_Log("Disable camera lock.\n");
    core->camera.flags &= ~(1UL << CAMERA_IS_LOCKED);
}

/**
 * @brief Update engine core
 */
void esz_UpdateCore(esz_Window* window, esz_Core* core)
{
    PollEvents(window, core);
}

/*********************
 * Private Functions *
 *********************/

static void CountMapLayers(tmx_layer* layer, Uint16* layer_count)
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

static void CountObjects(tmx_object* tmx_object, Uint16** object_count)
{
    if (tmx_object)
    {
        (**object_count) += 1;
    }

    if (tmx_object && tmx_object->next)
    {
        CountObjects(tmx_object->next, object_count);
    }
}

static esz_Status DrawMap(const esz_LayerType layer_type, esz_Window *window, esz_Core *core)
{
    tmx_layer* layer                 = core->map.tmx_map->ly_head;
    SDL_bool   render_animated_tiles = SDL_FALSE;
    SDL_bool   render_bg_color       = SDL_FALSE;

    if (! core->map.is_loaded)
    {
        return ESZ_OK;
    }

    if (ESZ_LAYER_MAX == layer_type)
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "DrawMap(): Layer type out of range.\n");
        return ESZ_ERROR_CRITICAL;
    }

    if (ESZ_BACKGROUND == layer_type)
    {
        render_animated_tiles = SDL_TRUE;
        render_bg_color       = SDL_TRUE;
    }

    /*if (! core->map.texture)
    {
        core->map.texture = SDL_CreateTexture(
            window->renderer,
            SDL_PIXELFORMAT_ARGB8888,
            SDL_TEXTUREACCESS_TARGET,
            window->width,
            window->height);
    }

    if (! core->map.texture)
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "DrawMap(): %s\n", SDL_GetError());
        return ESZ_ERROR_CRITICAL;
    }

    if (0 > SDL_SetRenderTarget(window->renderer, core->map.texture))
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "DrawMap(): %s\n", SDL_GetError());
        return ESZ_ERROR_CRITICAL;
    }*/

    /************************************
     * Update and render animated tiles *
     ************************************/

    core->map.time_since_last_anim_frame += window->time_since_last_frame;

    if (0 < core->map.animated_tile_count &&
        core->map.time_since_last_anim_frame >= 1.f / (double)(core->map.animated_tile_fps) &&
        render_animated_tiles)
    {
        core->map.time_since_last_anim_frame = 0.f;

        /* Remark: animated tiles are always rendered in the background
         * layer.
         */
        if (! core->map.animated_tile_texture)
        {
            core->map.animated_tile_texture = SDL_CreateTexture(
                window->renderer,
                SDL_PIXELFORMAT_ARGB8888,
                SDL_TEXTUREACCESS_TARGET,
                core->map.tmx_map->width  * core->map.tmx_map->tile_width,
                core->map.tmx_map->height * core->map.tmx_map->tile_height);
        }

        if (! core->map.animated_tile_texture)
        {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "DrawMap(): %s\n", SDL_GetError());
            return ESZ_ERROR_CRITICAL;
        }

        if (0 > SDL_SetRenderTarget(window->renderer, core->map.animated_tile_texture))
        {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "DrawMap(): %s\n", SDL_GetError());
            return ESZ_ERROR_CRITICAL;
        }

        for (Uint16 index = 0; core->map.animated_tile_count > index; index += 1)
        {
            Uint16       gid          = core->map.animated_tile[index].gid;
            Uint16       tile_id      = core->map.animated_tile[index].id + 1;
            Uint16       next_tile_id = 0;
            SDL_Rect     dst;
            SDL_Rect     src;
            tmx_tileset* tileset;

            tileset = core->map.tmx_map->tiles[1]->tileset;
            src.x   = core->map.tmx_map->tiles[tile_id]->ul_x;
            src.y   = core->map.tmx_map->tiles[tile_id]->ul_y;
            src.w   = dst.w = tileset->tile_width;
            src.h   = dst.h = tileset->tile_height;
            dst.x           = core->map.animated_tile[index].dst_x;
            dst.y           = core->map.animated_tile[index].dst_y;

            SDL_RenderCopy(window->renderer, core->map.tileset_texture, &src, &dst);

            core->map.animated_tile[index].current_frame += 1;

            if (core->map.animated_tile[index].current_frame >= core->map.animated_tile[index].animation_length)
            {
                core->map.animated_tile[index].current_frame = 0;
            }

            next_tile_id = core->map.tmx_map->tiles[gid]->animation[core->map.animated_tile[index].current_frame].tile_id;
            core->map.animated_tile[index].id = next_tile_id;
        }

        // Switch back to default render target.
        //if (0 > SDL_SetRenderTarget(window->renderer, core->map.texture))
        if (0 > SDL_SetRenderTarget(window->renderer, NULL))
        {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "DrawMap(): %s\n", SDL_GetError());
            return ESZ_ERROR_CRITICAL;
        }

        if (0 > SDL_SetTextureBlendMode(core->map.animated_tile_texture, SDL_BLENDMODE_BLEND))
        {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "DrawMap(): %s\n", SDL_GetError());
            return ESZ_ERROR_CRITICAL;
        }
    }

    /***********************************************
     * Texture has already been rendered. Draw it! *
     ***********************************************/

    if (core->map.map_layer[layer_type])
    {
        double   render_pos_x = core->map.pos_x - core->camera.pos_x;
        double   render_pos_y = core->map.pos_y - core->camera.pos_y;
        SDL_Rect dst          = {
            render_pos_x,
            render_pos_y,
            core->map.tmx_map->width  * core->map.tmx_map->tile_width,
            core->map.tmx_map->height * core->map.tmx_map->tile_height
        };

        if (0 > SDL_RenderCopyEx(window->renderer, core->map.map_layer[layer_type], NULL, &dst, 0, NULL, SDL_FLIP_NONE))
        {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "DrawMap(): %s\n", SDL_GetError());
            return ESZ_ERROR_CRITICAL;
        }

        /***********************
         * Draw animated tiles *
         ***********************/

        if (render_animated_tiles)
        {
            if (core->map.animated_tile_texture)
            {
                if (0 > SDL_RenderCopyEx(window->renderer, core->map.animated_tile_texture, NULL, &dst, 0, NULL, SDL_FLIP_NONE))
                {
                    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "DrawMap(): %s\n", SDL_GetError());
                    return ESZ_ERROR_CRITICAL;
                }
            }
        }

        return ESZ_OK;
    }

    /******************************************
     * Texture does not yet exist. Render it: *
     ******************************************/

    core->map.map_layer[layer_type] = SDL_CreateTexture(
        window->renderer,
        SDL_PIXELFORMAT_ARGB8888,
        SDL_TEXTUREACCESS_TARGET,
        core->map.tmx_map->width  * core->map.tmx_map->tile_width,
        core->map.tmx_map->height * core->map.tmx_map->tile_height);

    if (! core->map.map_layer[layer_type])
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "DrawMap(): %s\n", SDL_GetError());
        return ESZ_ERROR_CRITICAL;
    }

    if (0 > SDL_SetRenderTarget(window->renderer, core->map.map_layer[layer_type]))
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "DrawMap(): %s\n", SDL_GetError());
        return ESZ_ERROR_CRITICAL;
    }

    if (render_bg_color)
    {
        SDL_SetRenderDrawColor(
            window->renderer,
            (core->map.tmx_map->backgroundcolor >> 16) & 0xFF,
            (core->map.tmx_map->backgroundcolor >> 8)  & 0xFF,
            (core->map.tmx_map->backgroundcolor)       & 0xFF,
            0);
    }

    while (layer)
    {
        SDL_bool     is_in_foreground = SDL_FALSE;
        SDL_bool     render_layer     = SDL_FALSE;
        Uint16       gid;
        SDL_Rect     dst;
        SDL_Rect     src;
        tmx_tileset* tileset;

        if (L_LAYER == layer->type)
        {
            LoadPropertyByName("is_in_foreground", layer->properties, core);
            if (core->map.boolean_property)
            {
                is_in_foreground = core->map.boolean_property;
            }

            if (ESZ_BACKGROUND == layer_type && SDL_FALSE == is_in_foreground)
            {
                render_layer = SDL_TRUE;
            }
            else if (ESZ_FOREGROUND == layer_type && SDL_TRUE == is_in_foreground)
            {
                render_layer = SDL_TRUE;
            }

            if (layer->visible && render_layer)
            {
                for (Uint16 index_height = 0; index_height < core->map.tmx_map->height; index_height += 1)
                {
                    for (Uint32 index_width = 0; index_width < core->map.tmx_map->width; index_width += 1)
                    {
                        gid = RemoveGidFlipBits(
                            layer->content.gids[(index_height * core->map.tmx_map->width) + index_width]);

                        if (core->map.tmx_map->tiles[gid])
                        {
                            tileset = core->map.tmx_map->tiles[1]->tileset;
                            src.x   = core->map.tmx_map->tiles[gid]->ul_x;
                            src.y   = core->map.tmx_map->tiles[gid]->ul_y;
                            src.w   = dst.w = tileset->tile_width;
                            src.h   = dst.h = tileset->tile_height;
                            dst.x           = index_width  * tileset->tile_width;
                            dst.y           = index_height * tileset->tile_height;

                            SDL_RenderCopy(window->renderer, core->map.tileset_texture, &src, &dst);

                            if (render_animated_tiles && core->map.tmx_map->tiles[gid]->animation)
                            {
                                Uint8  animation_length = core->map.tmx_map->tiles[gid]->animation_len;
                                Uint16 id               = core->map.tmx_map->tiles[gid]->animation[0].tile_id;

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
                if (ESZ_BACKGROUND == layer_type)
                {
                    SDL_Log("Render background layer: %s\n", layer->name);
                }
                else
                {
                    SDL_Log("Render foreground layer: %s\n", layer->name);
                }
            }
        }
        layer = layer->next;
    }

    // Switch back to default render target.
    //if (0 > SDL_SetRenderTarget(window->renderer, core->map.texture))
    if (0 > SDL_SetRenderTarget(window->renderer, NULL))
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "DrawMap(): %s\n", SDL_GetError());
        return ESZ_ERROR_CRITICAL;
    }

    if (0 > SDL_SetTextureBlendMode(core->map.map_layer[layer_type], SDL_BLENDMODE_BLEND))
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "DrawMap(): %s\n", SDL_GetError());
        return ESZ_ERROR_CRITICAL;
    }

    return ESZ_OK;
}

static Uint16 GetCameraTarget(esz_Core* core)
{
    return core->camera.target_entity_id;
}

static esz_Status InitAnimatedTiles(esz_Core* core)
{
    tmx_layer* layer               = core->map.tmx_map->ly_head;
    Uint16     animated_tile_count = 0;

    /************************
     * Count animated tiles *
     ************************/

    while (layer)
    {
        Uint16 gid;

        if (L_LAYER == layer->type)
        {
            if (layer->visible)
            {
                for (Uint16 index_height = 0; index_height < core->map.tmx_map->height; index_height += 1)
                {
                    for (Uint32 index_width = 0; index_width < core->map.tmx_map->width; index_width += 1)
                    {
                        gid = RemoveGidFlipBits(
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

    if (animated_tile_count)
    {
        core->map.animated_tile = SDL_calloc(animated_tile_count, sizeof(struct esz_AnimatedTile_t));

        if (! core->map.animated_tile)
        {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "InitAnimatedTiles(): error allocating memory.\n");
            return ESZ_ERROR_CRITICAL;
        }
    }

    SDL_Log("Initialise %u animated tile(s).\n", animated_tile_count);
    return ESZ_OK;
}

static esz_Status InitObjects(esz_Core* core)
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
            CountObjects(layer->content.objgr->head, &object_count);
        }

        layer = layer->next;
    }

    /*******************************
     * Allocate memory for objects *
     *******************************/

    if (core->map.object_count)
    {
        core->map.object = SDL_calloc(core->map.object_count, sizeof(struct esz_Object_t));

        if (! core->map.object)
        {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "InitObjects(): error allocating memory.\n");
            return ESZ_ERROR_CRITICAL;
        }
    }

    SDL_Log("Initialise %u object(s).\n", core->map.object_count);
    return ESZ_OK;
}

static esz_Status InitSprites(esz_Window* window, esz_Core* core)
{
    LoadPropertyByName("sprite_sheet_count", core->map.tmx_map->properties, core);
    if (core->map.integer_property)
    {
        core->map.sprite_sheet_count = core->map.integer_property;
    }

    core->map.sprite = SDL_calloc(core->map.sprite_sheet_count, sizeof(struct esz_Sprite_t));

    if (! core->map.sprite)
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "InitSprites(): error allocating memory.\n");
        return ESZ_ERROR_CRITICAL;
    }

    if (0 < core->map.sprite_sheet_count)
    {
        char property_name[17]                    = { 0 };
        char sprite_sheet_image[ESZ_MAX_PATH_LEN] = { 0 };

        for (Uint16 index = 0; index < core->map.sprite_sheet_count; index += 1)
        {
            SDL_snprintf(property_name, 17, "sprite_sheet_%u", index);

            LoadPropertyByName(property_name, core->map.tmx_map->properties, core);

            SDL_snprintf(sprite_sheet_image, ESZ_MAX_PATH_LEN, "%s%s", core->map.resource_path, core->map.string_property);

            core->map.sprite[index].id = index;

            if (ESZ_OK != LoadSprite(sprite_sheet_image, &core->map.sprite[index], window))
            {
                return ESZ_ERROR_CRITICAL;
            }
        }
    }

    return ESZ_OK;
}

static SDL_bool IsCameraLocked(esz_Core* core)
{
    if ((core->camera.flags >> CAMERA_IS_LOCKED) & 1U)
    {
        return SDL_TRUE;
    }
    else
    {
        return SDL_FALSE;
    }
}

static void LoadPropertyByName(const char* property_name, tmx_property* property, esz_Core* core)
{
    core->map.boolean_property = SDL_FALSE;
    core->map.decimal_property = 0.f;
    core->map.integer_property = 0;
    core->map.string_property  = NULL;

    SDL_strlcpy(core->map.search_pattern, property_name, ESZ_MAX_PATTERN_LEN);

    tmx_property_foreach(property, StoreProperty, (void*)core);
}

static esz_Status LoadSprite(const char* sprite_sheet_image, esz_Sprite* sprite, esz_Window* window)
{
    sprite->texture = IMG_LoadTexture(window->renderer, sprite_sheet_image);

    if (! sprite->texture)
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "LoadSprite(): %s\n", IMG_GetError());
        return ESZ_ERROR_CRITICAL;
    }

    SDL_Log("Load sprite image file: %s.\n", sprite_sheet_image);
    return ESZ_OK;
}

static void MoveCameraToTarget(esz_Core* core)
{
    if ((core->camera.flags >> CAMERA_IS_LOCKED) & 1U)
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

static void PollEvents(esz_Window* window, esz_Core* core)
{
    while (0 != SDL_PollEvent(&core->event.event_handle))
    {
        switch (core->event.event_handle.type)
        {
            case SDL_QUIT:
                esz_DeactivateCore(core);
                return;
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
            case SDL_MULTIGESTURE:
                if (core->event.multi_gesture_cb)
                {
                    core->event.multi_gesture_cb((void*)window, (void*)core);
                }
                break;
        }
    }
}

static Uint16 RemoveGidFlipBits(Uint16 gid)
{
    return gid & TMX_FLIP_BITS_REMOVAL;
}

static double RoundNumber(double number)
{
    double decimal_place = number - SDL_floor(number);

    if (decimal_place >= 0.5f)
    {
        return SDL_ceil(number);
    }
    else
    {
        return SDL_floor(number);
    }
}

static SDL_bool SetCameraBoundariesToMapSize(esz_Window* window, esz_Core* core)
{
    SDL_bool camera_reached_boundaries = SDL_FALSE;

    core->camera.max_pos_x = core->map.width  - window->logical_width;
    core->camera.max_pos_y = core->map.height - window->logical_height;

    if (0 >= core->camera.pos_x)
    {
        core->camera.pos_x        = 0;
        camera_reached_boundaries = 1;
    }

    if (0 <= core->camera.pos_y)
    {
        core->camera.pos_y = 0;
    }

    if (core->camera.pos_x > core->camera.max_pos_x)
    {
        core->camera.pos_x        = core->camera.max_pos_x;
        camera_reached_boundaries = 1;
    }

    if (core->camera.pos_y > core->camera.max_pos_y)
    {
        core->camera.pos_y = core->camera.max_pos_y;
    }

    return camera_reached_boundaries;
}

static void SetCameraTarget(const Uint16 target_entity_id, esz_Core* core)
{
    core->camera.target_entity_id = target_entity_id;
}

static void StoreProperty(tmx_property* property, void* core)
{
    esz_Core* core_ptr = core;

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
                SDL_LogVerbose(SDL_LOG_CATEGORY_APPLICATION, "Loading boolean property '%s': %u\n", core_ptr->map.search_pattern, property->value.boolean);
                core_ptr->map.boolean_property = property->value.boolean;
                break;
            case PT_FILE:
                SDL_LogVerbose(SDL_LOG_CATEGORY_APPLICATION, "Loading string property '%s': %s\n", core_ptr->map.search_pattern, property->value.file);
                core_ptr->map.string_property  = property->value.file;
                break;
            case PT_FLOAT:
                SDL_LogVerbose(SDL_LOG_CATEGORY_APPLICATION, "Loading floating point property '%s': %f\n", core_ptr->map.search_pattern, property->value.decimal);
                core_ptr->map.decimal_property = property->value.decimal;
                break;
            case PT_INT:
                SDL_LogVerbose(SDL_LOG_CATEGORY_APPLICATION, "Loading integer property '%s': %d\n", core_ptr->map.search_pattern, property->value.integer);
                core_ptr->map.integer_property = property->value.integer;
                break;
            case PT_STRING:
                SDL_LogVerbose(SDL_LOG_CATEGORY_APPLICATION, "Loading string property '%s': %s\n", core_ptr->map.search_pattern, property->value.string);
                core_ptr->map.string_property  = property->value.string;
                break;
        }
    }
}

static void UpdateCamera(esz_Window* window, esz_Core* core)
{
    //MoveCameraToTarget(core);

    if (SetCameraBoundariesToMapSize(window, core))
    {
        // Do not move background when camera hits boundaries.
        //dBgVelocityX = 0;
    }
    else
    {
        //Uint16 u16EntityID = GetCameraTarget(void);;
        //dBgVelocityX = pstRes->pstEntity[u16EntityID]->dVelocityX;
    }

    // Do not move background when camera is locked.
    if (! IsCameraLocked(core))
    {
        //dBgVelocityX = 0;
    }
}

/*


static void ConnectHorizontalMapEndsForEntity(Entity* hEntity)
{
    double dWidth = (double)hEntity->u16Width;

    if (hEntity->dPosX < 0.f - dWidth)
    {
        hEntity->dPosX = heszFW.hMap->u16Width + dWidth;
    }
    else if (hEntity->dPosX > heszFW.hMap->u16Width + dWidth)
    {
        hEntity->dPosX = 0 - dWidth;
    }
}

static void ConnectMapEndsForEntity(Entity* hEntity)
{
    ConnectHorizontalMapEndsForEntity(hEntity);
    ConnectVerticalMapEndsForEntity(hEntity);
}

static void ConnectVerticalMapEndsForEntity(Entity* hEntity)
{
    double dHeight = (double)hEntity->u16Height;

    if (hEntity->dPosY < 0 - dHeight)
    {
        hEntity->dPosY =heszFW.hMap->u16Height + dHeight;
    }
    else if (hEntity->dPosY > heszFW.hMap->u16Height + dHeight)
    {
        hEntity->dPosY = 0 - dHeight;
    }
}

static eszStatus DrawEntity(const Entity* hEntity)
{
    Uint16           u16SpriteID = 0;
    double           dPosX       = hEntity->dPosX - heszFW.hCamera->dPosX;
    double           dPosY       = hEntity->dPosY - heszFW.hCamera->dPosY;
    SDL_RendererFlip s8Flip      = SDL_FLIP_NONE;
    SDL_Rect         stDst;
    SDL_Rect         stSrc;

    // Todo: set u16Sprite

    if (LEFT == hEntity->eDirection)
    {
        s8Flip = SDL_FLIP_HORIZONTAL;
    }

    stSrc.x  = 0;
    stSrc.x += hEntity->u8FrameOffsetX * hEntity->u16Width;
    stSrc.x += hEntity->u8AnimFrame    * hEntity->u16Width;
    stSrc.y  = 0;
    stSrc.y += hEntity->u8FrameOffsetY * hEntity->u16Height;
    stSrc.w  = hEntity->u16Width;
    stSrc.h  = hEntity->u16Height;
    stDst.x  = dPosX - (hEntity->u16Width  / 2);
    stDst.y  = dPosY - (hEntity->u16Height / 2);
    stDst.w  = hEntity->u16Width;
    stDst.h  = hEntity->u16Height;

    if (0 != SDL_RenderCopyEx(heszFW.hVideo->pstRenderer, heszFW.hMap->hSprite[u16SpriteID].pstTexture, &stSrc, &stDst, 0, NULL, s8Flip))
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "_DrawEntity(): %s\n", SDL_GetError());
        return ESZ_ERROR;
    }

    return ESZ_OK;
}


static void GetEntity(tmx_object* pstTiledObject, Uint16 u16Index, Object ahObject[])
{
    ahObject[u16Index].pstTiledObject = pstTiledObject;

    if (pstTiledObject)
    {
        if (SDL_strstr(ahObject[u16Index].acType, "Entity")) Replace!
        {
            Entity* hEntity = &heszFW.hMap->hEntity[heszFW.hMap->u16EntityCount];

            if (SDL_strstr(pstTiledObject->name, "Player")) Replace!
            {
                heszFW.hMap->u16PlayerEntityID = heszFW.hMap->u16EntityCount;
                eszLockCamera();
                SetCameraTarget(heszFW.hMap->u16PlayerEntityID);
                MoveCameraToTarget();
            }

            hEntity->dPosX     = (double)ahObject[u16Index].u32PosX;
            hEntity->dPosY     = (double)ahObject[u16Index].u32PosY;
            hEntity->u16Width  = ahObject[u16Index].u16Width;
            hEntity->u16Height = ahObject[u16Index].u16Height;

            heszFW.dProperty       = 8.f; // Set default acceleration.
            GetObjectPropertyByName(ahObject + u16Index, "Acceleration");
            hEntity->dAcceleration = heszFW.dProperty;

            heszFW.bProperty = SDL_FALSE; // Set default orientation.
            GetObjectPropertyByName(ahObject + u16Index, "IsLeftOriented");
            if (heszFW.bProperty)
            {
                hEntity->eDirection = LEFT;
            }
            else
            {
                hEntity->eDirection = RIGHT;
            }

            heszFW.dProperty      = 4.5f; // Set default max. velocity along the x-axis.
            GetObjectPropertyByName(ahObject + u16Index, "MaxVelocityX");
            hEntity->dMaxVelocityX = heszFW.dProperty;

            heszFW.dProperty      = 12.f; // Set default animation speed.
            GetObjectPropertyByName(ahObject + u16Index, "AnimSpeed");
            hEntity->dMaxVelocityX = heszFW.dProperty;

            heszFW.hMap->u16EntityCount += 1;
        }
    }

    if (pstTiledObject && pstTiledObject->next)
    {
        u16Index++;
        GetEntity(pstTiledObject->next, u16Index, &(*ahObject));
    }
}

static void GetObject(tmx_object* pstTiledObject, Uint16 u16Index, Object ahObject[])
{
    ahObject[u16Index].pstTiledObject = pstTiledObject;

    if (pstTiledObject)
    {
        ahObject[u16Index].u16Id     = pstTiledObject->id;
        ahObject[u16Index].u32PosX   = pstTiledObject->x;
        ahObject[u16Index].u32PosY   = pstTiledObject->y;
        ahObject[u16Index].u16Width  = pstTiledObject->width;
        ahObject[u16Index].u16Height = pstTiledObject->height;

        SDL_strlcpy(ahObject[u16Index].acName, pstTiledObject->name, OBJECT_NAME_LEN - 1);
        SDL_strlcpy(ahObject[u16Index].acType, pstTiledObject->type, OBJECT_TYPE_LEN - 1);

        ahObject[u16Index].hBB.dBottom =
            (double)ahObject[u16Index].u32PosY + (double)ahObject[u16Index].u16Height / 2.f;
        ahObject[u16Index].hBB.dLeft =
            (double)ahObject[u16Index].u32PosX - (double)ahObject[u16Index].u16Width  / 2.f;
        ahObject[u16Index].hBB.dRight =
            (double)ahObject[u16Index].u32PosX + (double)ahObject[u16Index].u16Width  / 2.f;
        ahObject[u16Index].hBB.dTop =
            (double)ahObject[u16Index].u32PosY - (double)ahObject[u16Index].u16Height / 2.f;

        if (ahObject[u16Index].hBB.dLeft <= 0)
        {
            ahObject[u16Index].hBB.dLeft = 0;
        }

        if (ahObject[u16Index].hBB.dTop <= 0)
        {
            ahObject[u16Index].hBB.dTop = 0;
        }

        *************************************************************
         * Count special objects of type 'Entity' to allocate memory *
         *************************************************************

        if (SDL_strstr(ahObject[u16Index].acType, "Entity")) Replace !!
        {
            heszFW.hMap->u16EntityCount += 1;
        }
    }

    if (pstTiledObject && pstTiledObject->next)
    {
        u16Index++;
        GetObject(pstTiledObject->next, u16Index, &(*ahObject));
    }
}

static void GetObjects(Object ahObject[], ObjectType eType)
{
    tmx_layer*  pstLayer = heszFW.hMap->pstTiledMap->ly_head;
    tmx_object* pstTiledObject;

    while (pstLayer)
    {
        if (L_OBJGR == pstLayer->type)
        {
            pstTiledObject = pstLayer->content.objgr->head;
            switch (eType)
            {
                case GENERIC_OBJ:
                    GetObject(pstTiledObject, 0, &(*ahObject));
                    break;
                case ENTITY_OBJ:
                    * Reset entity count; this is required because we
                     * use it as a index in GetEntity().
                     *
                    heszFW.hMap->u16EntityCount = 0;
                    GetEntity(pstTiledObject, 0, &(*ahObject));
                    break;
            }
        }
        pstLayer = pstLayer->next;
    }
}

static void* GetMapPropertyByName(char* szName)
{
    tmx_property* pProperty = heszFW.hMap->pstTiledMap->properties;
    tmx_property_foreach(pProperty, GetProperty, (void*)szName);

    return szName;
}


static SDL_bool IsEntityMoving(const Entity* hEntity)
{
    if (IsFlagSet(IS_MOVING, hEntity->u16Flags))
    {
        return SDL_TRUE;
    }
    else
    {
        return SDL_FALSE;
    }
}

static SDL_bool IsEntityRising(const Entity* hEntity)
{
    if (0 > hEntity->dVelocityY)
    {
        return SDL_TRUE;
    }
    else
    {
        return SDL_FALSE;
    }
}

static void SetEntityPosition(const double dPosX, const double dPosY, Entity* hEntity)
{
    hEntity->dPosX = dPosX;
    hEntity->dPosY = dPosY;
}

static void UpdateEntity(Entity* hEntity)
{
    double dPosX = hEntity->dPosX;
    double dPosY = hEntity->dPosY;

    // Apply gravitation.
    if (0 != heszFW.hMap->dGravitation)
    {
        if (IsEntityRising(hEntity))
        {
            SetFlag(IS_IN_MID_AIR, &hEntity->u16Flags);
        }

        if (IsFlagSet(IS_IN_MID_AIR, hEntity->u16Flags))
        {
            double dG            = heszFW.hMap->dGravitation * heszFW.hMap->u8MeterInPixel;
            double dDistanceY    = dG * DELTA_TIME * DELTA_TIME;
            hEntity->dVelocityY += dDistanceY;
            dPosY               += hEntity->dVelocityY;
        }
        else
        {
            hEntity->bIsJumping = 0;
            // Correct position along the y-axis.
            hEntity->dVelocityY = 0.f;
            dPosY               = (16.f * Round(dPosY / 16.f));
        }
    }

    // Calculate horizontal velocity.
    if (IsFlagSet(IS_MOVING, hEntity->u16Flags))
    {
        double dAccel        = hEntity->dAcceleration * (double)heszFW.hMap->u8MeterInPixel;
        double dDistanceX    = dAccel * DELTA_TIME * DELTA_TIME;
        hEntity->dVelocityX += dDistanceX;
    }
    else
    {
        hEntity->dVelocityX -= hEntity->dAcceleration * DELTA_TIME;
    }

    // Set horizontal velocity limits.
    if (hEntity->dVelocityX >= hEntity->dMaxVelocityX)
    {
        hEntity->dVelocityX = hEntity->dMaxVelocityX;
    }
    if (0 > hEntity->dVelocityX)
    {
        hEntity->dVelocityX = 0;
    }

    // Set horizontal position.
    if (0 < hEntity->dVelocityX)
    {
        if (RIGHT == hEntity->eDirection)
        {
            dPosX += hEntity->dVelocityX;
        }
        else
        {
            dPosX -= hEntity->dVelocityX;
        }
    }

    // Update position.
    SetEntityPosition(dPosX, dPosY, hEntity);

    // Update axis-aligned bounding box.
    hEntity->hBB.dBottom = dPosY + (double)(hEntity->u16Height / 2.f);
    hEntity->hBB.dLeft   = dPosX - (double)(hEntity->u16Width / 2.f);
    hEntity->hBB.dRight  = dPosX + (double)(hEntity->u16Width / 2.f);
    hEntity->hBB.dTop    = dPosY - (double)(hEntity->u16Height / 2.f);

    if (hEntity->hBB.dLeft <= 0)
    {
        hEntity->hBB.dLeft = 0;
    }

    if (hEntity->hBB.dTop <= 0)
    {
        hEntity->hBB.dTop = 0;
    }

    // Update animation frame.
    if (IsFlagSet(IS_ANIMATED, hEntity->u16Flags))
    {
        hEntity->dAnimDelay += heszFW.hVideo->dDeltaTime;

        if (hEntity->u8AnimFrame < hEntity->u8AnimStart)
        {
            hEntity->u8AnimFrame = hEntity->u8AnimStart;
        }

        if (hEntity->dAnimDelay > (1.f / hEntity->dAnimSpeed - heszFW.hVideo->dDeltaTime))
        {
            hEntity->u8AnimFrame++;
            hEntity->dAnimDelay = 0.f;
        }
        // Loop animation.
        if (hEntity->u8AnimFrame >= hEntity->u8AnimEnd)
        {
            hEntity->u8AnimFrame = hEntity->u8AnimStart;
        }
    }
    else
    {
        hEntity->u8AnimFrame = hEntity->u8AnimStart;
    }
}
*/

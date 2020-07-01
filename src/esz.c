// SPDX-License-Identifier: Beerware
/**
 * @file    esz.c
 * @brief   eszFW game engine
 * @details A cross-platform game engine written in C99
 * @todo    - Update logging
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

static void       CountObjects(tmx_object* tmx_object, Uint16** object_count);
static esz_Status DrawMap(const Uint16 texture_index, const SDL_bool render_animated_tiles, const SDL_bool render_bg_color, esz_Core *core);
static Uint16     GetCameraTarget(esz_Core* core);
static esz_Status InitAnimatedTiles(esz_Core* core);
static esz_Status InitObjects(esz_Core* core);
static SDL_bool   IsCameraLocked(esz_Core* core);
static void       LoadMapPropertyByName(const char* property_name, esz_Core* core);
static esz_Status LoadSprite(const char* sprite_sheet_image, esz_Sprite* sprite, esz_Core* core);
static void       MoveCameraToTarget(esz_Core* core);
static void       PollEvents(esz_Core* core);
static Uint16     RemoveGidFlipBits(Uint16 gid);
static esz_Status Render(esz_Core *core);
static double     RoundNumber(double number);
static SDL_bool   SetCameraBoundariesToMapSize(esz_Core* core);
static void       SetCameraTarget(const Uint16 target_entity_id, esz_Core* core);
static void       StoreProperty(tmx_property* property, void* core);
static void       Update(esz_Core* core);
static void       UpdateCamera(esz_Core* core);

/*
static void      ConnectHorizontalMapEndsForEntity(Entity* hEntity);
static void      ConnectMapEndsForEntity(Entity* hEntity);
static void      ConnectVerticalMapEndsForEntity(Entity* hEntity);
static eszStatus DrawEntity(const Entity* hEntity);
static void      GetEntity(tmx_object* pstTiledObject, Uint16 u16Index, Object ahObject[]);
static SDL_bool  IsEntityMoving(const Entity* hEntity);
static SDL_bool  IsEntityRising(const Entity* hEntity);
static void      RenderScene(void);
static void      SetEntityPosition(const double dPosX, const double dPosY, Entity* hEntity);
static void      UpdateEntities(void);
static void      UpdateEntity(Entity* hEntity);
*/

/********************
 * Public functions *
 ********************/

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
 * @brief Exit engine core
 * @param core
 *        Engine core
 */
void esz_ExitCore(esz_Core* core)
{
    esz_UnloadMap(core);

    core->is_running = SDL_FALSE;
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

/**
 * @brief Initialise engine core
 * @param config
 *        Initial configuration
 * @param core
 *        Engine core
 */
esz_Status esz_InitCore(esz_Config* config, esz_Core** core)
{
    *core = SDL_calloc(1, sizeof(struct esz_Core_t));

    if (! *core)
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "esz_InitCore(): error allocating memory.\n");
        return ESZ_ERROR_CRITICAL;
    }

    (*core)->window_width          = config->window_width;
    (*core)->window_height         = config->window_height;
    (*core)->logical_window_width  = config->logical_window_width;
    (*core)->logical_window_height = config->logical_window_height;

    if (config->enable_fullscreen)
    {
        (*core)->window_flags = (*core)->window_flags | SDL_WINDOW_FULLSCREEN_DESKTOP;
    }

    return ESZ_OK;
}

/**
 * @brief Load map
 * @param map_file_name
 *        file name of the map to load
 * @param core
 *        Engine core
 */
void esz_LoadMap(const char* map_file_name, esz_Core* core)
{
    // This does nothing if no map is currently loaded.
    esz_UnloadMap(core);

    /***********************
     * Load Tiled map file *
     ***********************/

    core->map.tmx_map = tmx_load(map_file_name);
    if (! core->map.tmx_map)
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "esz_LoadMap(): %s\n", tmx_strerr());
        return;
    }
    else
    {
        core->map.is_loaded = SDL_TRUE;
        SDL_Log("Load map file: %s.\n", map_file_name);
    }

    /**********************
     * Initialise objects *
     **********************/

    if (ESZ_OK != InitObjects(core))
    {
        esz_UnloadMap(core);
        return;
    }

    /***********************
     * Initialise entities *
     ***********************/

    // tbd.

    /***********************
     * Set base attributes *
     ***********************/

    core->map.height = core->map.tmx_map->height * core->map.tmx_map->tile_height;
    core->map.width  = core->map.tmx_map->width  * core->map.tmx_map->tile_width;

    /**************************
     * Set default properties *
     **************************/

    core->map.gravitation          = 9.80665f;
    core->map.meter_in_pixel       = 42;
    core->map.tile_animation_speed = 3.f;

    /************************
     * Parse map properties *
     ************************/

    LoadMapPropertyByName("gravitation", core);
    if (core->decimal_property)
    {
        core->map.gravitation = core->decimal_property;
    }

    LoadMapPropertyByName("meter_in_pixel", core);
    if (core->integer_property)
    {
        core->map.meter_in_pixel = core->integer_property;
    }

    SDL_Log(
        "Set gravitational constant to %f (g*%dpx/s^2).\n",
        core->map.gravitation, core->map.meter_in_pixel);

    LoadMapPropertyByName("tile_animation_speed", core);
    if (core->decimal_property)
    {
        core->map.tile_animation_speed = core->decimal_property;
    }

    LoadMapPropertyByName("tileset_image", core);
    if (core->string_property)
    {
        SDL_strlcpy(core->map.resource_path, map_file_name, ESZ_MAX_PATH_LEN - 1);

        cwk_path_get_dirname(core->map.resource_path, &core->map.resource_path_length);

        SDL_snprintf(
            core->map.resource_path, (int)(core->map.resource_path_length + 1), "%.*s", (int)(core->map.resource_path_length + 1), map_file_name);

        SDL_strlcpy(core->map.tileset_image, core->map.resource_path, core->map.resource_path_length + 1);
        SDL_strlcat(core->map.tileset_image, core->string_property,   ESZ_MAX_PATH_LEN - core->map.resource_path_length - 1);

        core->map.tileset_texture = IMG_LoadTexture(core->renderer, core->map.tileset_image);

        if (! core->map.tileset_texture)
        {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "esz_LoadMap(): %s\n", IMG_GetError());
            esz_UnloadMap(core);
            return;
        }

        SDL_Log("Loading tileset image: %s.\n", core->map.tileset_image);
    }
    else
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "esz_LoadMap(): property 'tileset_image' isn't specified.\n");
        esz_UnloadMap(core);
        return;
    }

    // Todo: move to InitSprites()
    LoadMapPropertyByName("sprite_sheet_count", core);
    if (core->integer_property)
    {
        core->map.sprite_sheet_count = core->integer_property;
    }

    core->map.sprite = SDL_calloc(core->map.sprite_sheet_count, sizeof(struct esz_Sprite_t));

    if (! core->map.sprite)
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "esz_LoadMap(): error allocating memory.\n");
        esz_UnloadMap(core);
    }

    if (0 < core->map.sprite_sheet_count)
    {
        char property_name[17]                    = { 0 };
        char sprite_sheet_image[ESZ_MAX_PATH_LEN] = { 0 };

        for (Uint16 index = 0; index < core->map.sprite_sheet_count; index += 1)
        {
            SDL_snprintf(property_name, 17, "sprite_sheet_%u", index);

            LoadMapPropertyByName(property_name, core);

            SDL_snprintf(sprite_sheet_image, ESZ_MAX_PATH_LEN, "%s%s", core->map.resource_path, core->string_property);

            if (ESZ_OK != LoadSprite(sprite_sheet_image, core->map.sprite + index, core))
            {
                esz_UnloadMap(core);
                return;
            }
        }
    }

    if (ESZ_OK != InitAnimatedTiles(core))
    {
        esz_UnloadMap(core);
        return;
    }

    if (core->event.map_loaded_cb)
    {
        core->event.map_loaded_cb(core);
    }

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
void esz_RegisterEventCallback(const esz_EventType event_type, void (*callback)(void* core), esz_Core* core)
{
    switch (event_type)
    {
        case EVENT_CORE_STARTED:
            core->event.core_started_cb  = callback;
            break;
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
        case EVENT_MULTIGESTURE:
            core->event.multi_gesture_cb = callback;
            break;
    }
}

/**
 * @brief  Set zoom level
 * @param  factor
 *         Zoom-level factor
 * @param  core
 *         Engine core
 * @return Status code
 * @retval ESZ_OK
 *         OK
 * @retval ESZ_ERROR_WARNING
 *         Zoom-level could not be changed
 */
esz_Status esz_SetZoomLevel(const double factor, esz_Core* core)
{
    core->zoom_level            = factor;
    core->logical_window_width  = core->window_width  / factor;
    core->logical_window_height = core->window_height / factor;

    if (0 != SDL_RenderSetLogicalSize(core->renderer, core->logical_window_width, core->logical_window_height))
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
 * @brief  Start engine core and create window
 * @param  window_title
 *         Window title
 * @param  core
 *         Engine core
 * @return Status code
 * @retval ESZ_OK: OK
 * @retval ESZ_ERROR: Error
 */
esz_Status esz_StartCore(const char* window_title, esz_Core* core)
{
    esz_Status      status = ESZ_OK;
    SDL_DisplayMode display_mode;

    /********************
     * Initialise Video *
     ********************/

    if (0 > SDL_Init(SDL_INIT_VIDEO))
    {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "esz_StartCore(): %s\n", SDL_GetError());
        status = ESZ_ERROR_CRITICAL;
        goto quit;
    }

    if (IMG_INIT_PNG != IMG_Init(IMG_INIT_PNG))
    {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "esz_StartCore(): %s\n", IMG_GetError());
        status = ESZ_ERROR_CRITICAL;
        goto quit;
    }

    if (0 != SDL_GetCurrentDisplayMode(0, &display_mode))
    {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "esz_StartCore(): %s\n", SDL_GetError());
        status = ESZ_ERROR_CRITICAL;
        goto quit;
    }

#ifdef __ANDROID__
    core->window_flags  = 0;
    core->window_width  = display_mode.w;
    core->window_height = display_mode.h;
#endif
    core->refresh_rate  = display_mode.refresh_rate;

    core->window = SDL_CreateWindow(
        window_title,
        SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED,
        core->window_width,
        core->window_height,
        core->window_flags);

    if (! core->window)
    {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "esz_StartCore(): %s\n", SDL_GetError());
        status = ESZ_ERROR_CRITICAL;
        goto quit;
    }

    if (0 > SDL_ShowCursor(SDL_DISABLE))
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "esz_StartCore(): %s\n", SDL_GetError());
        status = ESZ_ERROR_WARNING;
    }

    SDL_GetWindowSize(core->window, &core->window_width, &core->window_height);

    core->zoom_level         = (double)core->window_height / (double)core->logical_window_height;
    core->initial_zoom_level = core->zoom_level;

    core->renderer = SDL_CreateRenderer(
        core->window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_TARGETTEXTURE | SDL_RENDERER_PRESENTVSYNC);

    if (! core->renderer)
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "esz_StartCore(): %s\n", SDL_GetError());
        status = ESZ_ERROR_CRITICAL;
        goto quit;
    }

    SDL_Log(
        "Setting up window at resolution %dx%d @ %d Hz.\n",
        core->window_width,
        core->window_height,
        core->refresh_rate);

    esz_SetZoomLevel(core->zoom_level, core);
    SDL_Log("Set initial zoom-level to factor %f.\n", core->zoom_level);

    /**************************
     * Begin main application *
     **************************/

    core->is_running = SDL_TRUE;

    if (core->event.core_started_cb)
    {
        core->event.core_started_cb(core);
    }

#ifdef __EMSCRIPTEN__
    // tbd.
#else
    while (core->is_running)
    {
        Update(core);
        Render(core);
    }
#endif

quit:
    IMG_Quit();

    if (core)
    {
        if (core->renderer)
        {
            SDL_DestroyRenderer(core->renderer);
        }

        if (core->window)
        {
            SDL_DestroyWindow(core->window);
        }

        SDL_free(core);
        SDL_Log("Quitting.\n");
    }

    SDL_Quit();
    return status;
}

/**
 * @brief Toggle between fullscreen and windowed mode
 */
esz_Status esz_ToggleFullscreen(esz_Core* core)
{
    esz_Status status = ESZ_OK;

    core->window_flags = SDL_GetWindowFlags(core->window);

    if (core->window_flags & SDL_WINDOW_FULLSCREEN_DESKTOP)
    {
        SDL_SetWindowPosition(core->window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
        if (0 > SDL_SetWindowFullscreen(core->window, 0))
        {
            status = ESZ_ERROR_WARNING;
        }
        SDL_Log("Set window to windowed mode.\n");
    }
    else
    {
        if (0 > SDL_SetWindowFullscreen(core->window, SDL_WINDOW_FULLSCREEN_DESKTOP))
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
void esz_UnloadMap(esz_Core* core)
{
    if (! core->map.is_loaded)
    {
        return;
    }
    else
    {
        core->map.is_loaded = SDL_FALSE;
    }

    if (core->map.animated_tile)
    {
        SDL_free(core->map.animated_tile);
    }

    if (core->map.animated_tile_texture)
    {
        SDL_DestroyTexture(core->map.animated_tile_texture);
    }

    if (core->map.sprite)
    {
        for (Uint16 index = 0; index < core->map.sprite_sheet_count; index += 1)
        {
            if (core->map.sprite[index].texture)
            {
                SDL_DestroyTexture(core->map.sprite[index].texture);
            }
        }

        SDL_free(core->map.sprite);
    }

    if (core->map.tileset_texture)
    {
        SDL_DestroyTexture(core->map.tileset_texture);
    }

    if (core->map.object)
    {
        SDL_free(core->map.object);
    }

    if (core->map.tmx_map)
    {
        tmx_map_free(core->map.tmx_map);
        SDL_Log("Unload map.\n");
    }
}

/**
 * @brief Disable camera lock
 */
void esz_UnlockCamera(esz_Core* core)
{
    SDL_Log("Disable camera lock.\n");
    core->camera.flags &= ~(1UL << CAMERA_IS_LOCKED);
}

/*********************
 * Private Functions *
 *********************/

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

static esz_Status DrawMap(const Uint16 texture_index, const SDL_bool render_animated_tiles, const SDL_bool render_bg_color, esz_Core *core)
{
    static double tile_animation_delay = 0.f;

    //tmx_layer* layer = core->map.tmx_map->ly_head;

    /************************************
     * Update and render animated tiles *
     ************************************/

    tile_animation_delay += core->time_between_frames;

    if (0 < core->map.animated_tile_count &&
        tile_animation_delay > (1.f / core->map.tile_animation_speed - core->time_between_frames) &&
        render_animated_tiles)
    {
        /*  Remark: if render_animated_tiles is set to true, the
         *  animated tiles of *all* layers are rendered in this pass.
         */

        if (! core->map.animated_tile_texture)
        {
            core->map.animated_tile_texture = SDL_CreateTexture(
                core->renderer,
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

        if (0 != SDL_SetRenderTarget(core->renderer, core->map.animated_tile_texture))
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

            SDL_RenderCopy(core->renderer, core->map.tileset_texture, &src, &dst);

            core->map.animated_tile[index].current_frame += 1;

            if (core->map.animated_tile[index].current_frame >= core->map.animated_tile[index].animation_length)
            {
                core->map.animated_tile[index].current_frame = 0;
            }

            next_tile_id = core->map.tmx_map->tiles[gid]->animation[core->map.animated_tile[index].current_frame].tile_id;

            core->map.animated_tile[index].id = next_tile_id;
        }

        if (tile_animation_delay > (1.f / core->map.tile_animation_speed))
        {
            tile_animation_delay = 0.f;
        }

        // Switch back to default render target.
        if (0 != SDL_SetRenderTarget(core->renderer, NULL))
        {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "DrawMap(): %s\n", SDL_GetError());
            return ESZ_ERROR_CRITICAL;
        }

        if (0 != SDL_SetTextureBlendMode(core->map.animated_tile_texture, SDL_BLENDMODE_BLEND))
        {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "DrawMap(): %s\n", SDL_GetError());
            return ESZ_ERROR_CRITICAL;
        }
    }

    /***********************************************
     * Texture has already been rendered. Draw it: *
     ***********************************************/

    /*if (core->map.pstMap->pstTexture[u16Index])
    {
        double   dRenderPosX = pstMap->dPosX - dCameraPosX;
        double   dRenderPosY = pstMap->dPosY - dCameraPosY;
        SDL_Rect stDst       = { dRenderPosX,
                           dRenderPosY,
                           pstMap->pstTmxMap->width * pstMap->pstTmxMap->tile_width,
                           pstMap->pstTmxMap->height * pstMap->pstTmxMap->tile_height };

        if (-1 ==
            SDL_RenderCopyEx(
                pstRenderer, pstMap->pstTexture[u16Index], NULL, &stDst, 0, NULL, SDL_FLIP_NONE))
        {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "%s\n", SDL_GetError());
            return -1;
        }

        if (bRenderAnimTiles)
        {
            if (pstMap->pstAnimTexture)
            {
                if (-1 ==
                    SDL_RenderCopyEx(
                        pstRenderer, pstMap->pstAnimTexture, NULL, &stDst, 0, NULL, SDL_FLIP_NONE))
                {
                    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "%s\n", SDL_GetError());
                    return -1;
                }
            }
        }

        return 0;
    }*/

    /******************************************
     * Texture does not yet exist. Render it: *
     ******************************************/

    /*
    pstMap->pstTexture[u16Index] = SDL_CreateTexture(
        pstRenderer,
        SDL_PIXELFORMAT_ARGB8888,
        SDL_TEXTUREACCESS_TARGET,
        pstMap->pstTmxMap->width * pstMap->pstTmxMap->tile_width,
        pstMap->pstTmxMap->height * pstMap->pstTmxMap->tile_height);

    if (!pstMap->pstTexture[u16Index])
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "%s\n", SDL_GetError());
        return -1;
    }

    if (0 != SDL_SetRenderTarget(pstRenderer, pstMap->pstTexture[u16Index]))
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "%s\n", SDL_GetError());
        return -1;
    }

    if (bRenderBgColour)
    {
        SDL_SetRenderDrawColor(
            pstRenderer,
            (pstMap->pstTmxMap->backgroundcolor >> 16) & 0xFF,
            (pstMap->pstTmxMap->backgroundcolor >> 8) & 0xFF,
            (pstMap->pstTmxMap->backgroundcolor) & 0xFF,
            255);
    }

    while (pstLayer)
    {
        SDL_bool     bRenderLayer = 1;
        Uint16       u16Gid;
        SDL_Rect     stDst;
        SDL_Rect     stSrc;
        tmx_tileset* pstTS;

        if (L_LAYER == pstLayer->type)
        {
            if (pacLayerName)
            {
                if (!SDL_strstr(pstLayer->name, pacLayerName))
                {
                    bRenderLayer = 0;
                }
            }
            if (pstLayer->visible && bRenderLayer)
            {
                for (Uint16 u16IndexH = 0; u16IndexH < pstMap->pstTmxMap->height; u16IndexH++)
                {
                    for (Uint32 u16IndexW = 0; u16IndexW < pstMap->pstTmxMap->width; u16IndexW++)
                    {
                        u16Gid = _ClearGidFlags(
                            pstLayer->content
                                .gids[(u16IndexH * pstMap->pstTmxMap->width) + u16IndexW]);
                        if (pstMap->pstTmxMap->tiles[u16Gid])
                        {
                            pstTS   = pstMap->pstTmxMap->tiles[1]->tileset;
                            stSrc.x = pstMap->pstTmxMap->tiles[u16Gid]->ul_x;
                            stSrc.y = pstMap->pstTmxMap->tiles[u16Gid]->ul_y;
                            stSrc.w = stDst.w = pstTS->tile_width;
                            stSrc.h = stDst.h = pstTS->tile_height;
                            stDst.x           = u16IndexW * pstTS->tile_width;
                            stDst.y           = u16IndexH * pstTS->tile_height;
                            SDL_RenderCopy(pstRenderer, pstMap->pstTileset, &stSrc, &stDst);

                            if (bRenderAnimTiles && pstMap->pstTmxMap->tiles[u16Gid]->animation)
                            {
                                Uint8  u8AnimLen;
                                Uint16 u16TileId;
                                u8AnimLen = pstMap->pstTmxMap->tiles[u16Gid]->animation_len;
                                u16TileId = pstMap->pstTmxMap->tiles[u16Gid]->animation[0].tile_id;
                                pstMap->acAnimTile[pstMap->u16AnimTileSize].u16Gid    = u16Gid;
                                pstMap->acAnimTile[pstMap->u16AnimTileSize].u16TileId = u16TileId;
                                pstMap->acAnimTile[pstMap->u16AnimTileSize].s16DstX   = stDst.x;
                                pstMap->acAnimTile[pstMap->u16AnimTileSize].s16DstY   = stDst.y;
                                pstMap->acAnimTile[pstMap->u16AnimTileSize].u8FrameCount = 0;
                                pstMap->acAnimTile[pstMap->u16AnimTileSize].u8AnimLen = u8AnimLen;
                                pstMap->u16AnimTileSize++;

                                // Prevent buffer overflow.
                                if (pstMap->u16AnimTileSize >= ANIM_TILE_MAX)
                                {
                                    pstMap->u16AnimTileSize = ANIM_TILE_MAX;
                                }
                            }
                        }
                    }
                }
                SDL_Log("Render TMX map layer: %s\n", pstLayer->name);
            }
        }
        pstLayer = pstLayer->next;
    }
    // Switch back to default render target.
    if (0 != SDL_SetRenderTarget(pstRenderer, NULL))
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "%s\n", SDL_GetError());
        return -1;
    }

    if (0 != SDL_SetTextureBlendMode(pstMap->pstTexture[u16Index], SDL_BLENDMODE_BLEND))
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "%s\n", SDL_GetError());
        return -1;
    }

    return 0;
*/

    return ESZ_OK;
}

static Uint16 GetCameraTarget(esz_Core* core)
{
    return core->camera.target_entity_id;
}

static esz_Status InitAnimatedTiles(esz_Core* core)
{
    tmx_layer* layer = core->map.tmx_map->ly_head;

    core->map.animated_tile_count = 0;

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
                                core->map.animated_tile_count += 1;
                            }
                        }
                    }
                }
            }
        }
        layer = layer->next;
    }

    if (0 == core->map.animated_tile_count)
    {
        SDL_Log("No animated tiles found.\n");
        return ESZ_OK;
    }

    /**************************************
     * Allocate memory for animated tiles *
     **************************************/

    if (core->map.animated_tile_count)
    {
        core->map.animated_tile = SDL_calloc(core->map.animated_tile_count, sizeof(struct esz_AnimatedTile_t));

        if (! core->map.animated_tile)
        {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "InitAnimatedTiles(): error allocating memory.\n");
            return ESZ_ERROR_CRITICAL;
        }
    }

    SDL_Log("Initialise %u animated tile(s).\n", core->map.animated_tile_count);
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

static void LoadMapPropertyByName(const char* property_name, esz_Core* core)
{
    core->boolean_property = SDL_FALSE;
    core->decimal_property = 0.f;
    core->integer_property = 0;
    core->string_property  = NULL;

    SDL_strlcpy(core->search_pattern, property_name, ESZ_MAX_PATTERN_LEN);

    tmx_property* property = core->map.tmx_map->properties;
    tmx_property_foreach(property, StoreProperty, (void*)core);
}

static esz_Status LoadSprite(const char* sprite_sheet_image, esz_Sprite* sprite, esz_Core* core)
{
    sprite->texture = IMG_LoadTexture(core->renderer, sprite_sheet_image);

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

static void PollEvents(esz_Core* core)
{
    while (0 != SDL_PollEvent(&core->event.event_handle))
    {
        switch (core->event.event_handle.type)
        {
            case SDL_QUIT:
                esz_ExitCore(core);
                return;
            case SDL_KEYDOWN:
                if (core->event.key_down_cb)
                {
                    core->event.key_down_cb((void*)core);
                }
                break;
            case SDL_KEYUP:
                if (core->event.key_up_cb)
                {
                    core->event.key_up_cb((void*)core);
                }
                break;
            case SDL_FINGERDOWN:
                if (core->event.finger_down_cb)
                {
                    core->event.finger_down_cb((void*)core);
                }
                break;
            case SDL_FINGERUP:
                if (core->event.finger_up_cb)
                {
                    core->event.finger_up_cb((void*)core);
                }
                break;
            case SDL_FINGERMOTION:
                if (core->event.finger_motion_cb)
                {
                    core->event.finger_motion_cb((void*)core);
                }
                break;
            case SDL_MULTIGESTURE:
                if (core->event.multi_gesture_cb)
                {
                    core->event.multi_gesture_cb((void*)core);
                }
                break;
        }
    }
}

static Uint16 RemoveGidFlipBits(Uint16 gid)
{
    return gid & TMX_FLIP_BITS_REMOVAL;
}

static esz_Status Render(esz_Core *core)
{
    static Uint32 time_a = 0;
    static Uint32 time_b = 0;

    double delta_time;

    // tbd.

    time_b = time_a;
    time_a = SDL_GetTicks();

    if (time_a > time_b)
    {
        time_a = time_b;
    }

    delta_time = (double)(time_b - time_a) / 1000.f;

    core->time_between_frames =  1000.f / core->refresh_rate - delta_time;
    core->time_between_frames /= 1000.f;

    SDL_RenderPresent(core->renderer);
    SDL_RenderClear(core->renderer);

    return ESZ_OK;
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

static SDL_bool SetCameraBoundariesToMapSize(esz_Core* core)
{
    SDL_bool camera_reached_boundaries = SDL_FALSE;

    core->camera.max_pos_x = core->map.width  - core->logical_window_width;
    core->camera.max_pos_y = core->map.height - core->logical_window_height;

    if (0 >= core->camera.pos_x)
    {
        core->camera.pos_x       = 0;
        camera_reached_boundaries = 1;
    }

    if (0 <= core->camera.pos_y)
    {
        core->camera.pos_y = 0;
    }

    if (core->camera.pos_x > core->camera.max_pos_x)
    {
        core->camera.pos_x       = core->camera.max_pos_x;
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
    if (0 == SDL_strncmp(property->name, core_ptr->search_pattern, SDL_strlen((char*)core_ptr->search_pattern)))
    {
        switch (property->type)
        {
            case PT_COLOR:
            case PT_NONE:
                // tbd.
                break;
            case PT_BOOL:
                SDL_LogVerbose(SDL_LOG_CATEGORY_APPLICATION, "Loading boolean property '%s': %u\n", core_ptr->search_pattern, property->value.boolean);
                core_ptr->boolean_property  = property->value.boolean;
                break;
            case PT_FILE:
                SDL_LogVerbose(SDL_LOG_CATEGORY_APPLICATION, "Loading string property '%s': %s\n", core_ptr->search_pattern, property->value.file);
                core_ptr->string_property   = property->value.file;
                break;
            case PT_FLOAT:
                SDL_LogVerbose(SDL_LOG_CATEGORY_APPLICATION, "Loading floating point property '%s': %f\n", core_ptr->search_pattern, property->value.decimal);
                core_ptr->decimal_property  = property->value.decimal;
                break;
            case PT_INT:
                SDL_LogVerbose(SDL_LOG_CATEGORY_APPLICATION, "Loading integer property '%s': %d\n", core_ptr->search_pattern, property->value.integer);
                core_ptr->integer_property  = property->value.integer;
                break;
            case PT_STRING:
                SDL_LogVerbose(SDL_LOG_CATEGORY_APPLICATION, "Loading string property '%s': %s\n", core_ptr->search_pattern, property->value.string);
                core_ptr->string_property   = property->value.string;
                break;
        }
    }
}

static void Update(esz_Core* core)
{
    PollEvents(core);

    if (core->is_paused)
    {
        return;
    }

    // Todo: update everything else.
}

static void UpdateCamera(esz_Core* core)
{
    MoveCameraToTarget(core);

    if (SetCameraBoundariesToMapSize(core))
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

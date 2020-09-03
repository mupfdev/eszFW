// SPDX-License-Identifier: Beerware
/**
 * @file      main.c
 * @author    Michael Fitzmayer
 * @copyright "THE BEER-WARE LICENCE" (Revision 42)
 */

#define SDL_MAIN_HANDLED

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <SDL.h>
#include <esz.h>

static void key_down_callback(esz_window_t* window, esz_core_t* core);
static void key_up_callback(esz_window_t* window, esz_core_t* core);
static void walk_left(esz_core_t* core);
static void walk_right(esz_core_t* core);

#ifdef USE_LIBTMX
    #define MAP_FILE "res/maps/city.tmx"
#else // (cute_tiled.h)
    #define MAP_FILE "res/maps/city.json"
#endif

int main()
{
    const uint8_t*      keystate = esz_get_keyboard_state();
    esz_status          status;
    esz_window_t*       window   = NULL;
    esz_window_config_t config   = { 640, 360, 384, 216, false, false };
    esz_core_t*         core     = NULL;

    status = esz_create_window("Tau Ceti", &config, &window);
    if (ESZ_OK != status)
    {
        goto quit;
    }

    SDL_LogSetAllPriority(SDL_LOG_PRIORITY_VERBOSE);

    status = esz_init_core(&core);
    if (ESZ_OK != status)
    {
        goto quit;
    }

    esz_load_map(MAP_FILE, window, core);
    esz_register_event_callback(EVENT_KEYDOWN, &key_down_callback, core);
    esz_register_event_callback(EVENT_KEYUP, &key_up_callback, core);

    while (esz_is_core_active(core))
    {
        double camera_pos_x = 0;
        double camera_pos_y = 0;

        esz_update_core(window, core);

        if (keystate[SDL_SCANCODE_TAB])
        {
            esz_unlock_camera(core);
        }
        else
        {
            esz_lock_camera(core);
        }

        if (keystate[SDL_SCANCODE_UP])
        {
            camera_pos_y -= 0.3f;
        }
        if (keystate[SDL_SCANCODE_DOWN])
        {
            camera_pos_y += 0.3f;
        }
        if (keystate[SDL_SCANCODE_LEFT])
        {
            camera_pos_x -= 0.3f;
        }
        if (keystate[SDL_SCANCODE_RIGHT])
        {
            camera_pos_x += 0.3f;
        }
        esz_set_camera_position(camera_pos_x, camera_pos_y, true, window, core);

        if (esz_is_player_moving(core))
        {
            esz_set_player_animation(2, core);
        }
        else
        {
            esz_set_player_animation(1, core);
        }

        status = esz_show_scene(window, core);
        if (ESZ_ERROR_CRITICAL == status)
        {
            break;
        }
    }

quit:
    if (esz_is_map_loaded(core))
    {
        esz_unload_map(window, core);
    }
    if (core)
    {
        esz_destroy_core(core);
    }
    if (window)
    {
        esz_destroy_window(window);
    }

    if (ESZ_OK != status)
    {
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

static void key_down_callback(esz_window_t* window, esz_core_t* core)
{
    switch (esz_get_keycode(core))
    {
        case SDLK_ESCAPE:
        case SDLK_q:
            esz_deactivate_core(core);
            break;
        case SDLK_SPACE:
            esz_set_player_state(STATE_IN_MID_AIR, core);
            break;
        case SDLK_LEFT:
            walk_left(core);
            break;
        case SDLK_RIGHT:
            walk_right(core);
            break;
        case SDLK_F4:
            esz_set_next_player_animation(core);
            break;
        case SDLK_F5:
        {
            if (esz_is_map_loaded(core))
            {
                esz_unload_map(window, core);
            }
            else
            {
                esz_load_map(MAP_FILE, window, core);
            }
            break;
        }
        case SDLK_F6:
        {
            esz_load_map(MAP_FILE, window, core);
            break;
        }
        case SDLK_F11:
            esz_toggle_fullscreen(window);
            break;
        case SDLK_1:
            esz_hide_render_layer(ESZ_MAP_BG, core);
        case SDLK_2:
            esz_hide_render_layer(ESZ_MAP_FG, core);
            break;
    }
}

static void key_up_callback(esz_window_t* window, esz_core_t* core)
{
    switch (esz_get_keycode(core))
    {
        case SDLK_LEFT:
        case SDLK_RIGHT:
            esz_clear_player_state(STATE_MOVING, core);
        case SDLK_1:
            esz_show_render_layer(ESZ_MAP_BG, core);
            break;
        case SDLK_2:
            esz_show_render_layer(ESZ_MAP_FG, core);
            break;
    }
}

static void walk_left(esz_core_t* core)
{
    if (esz_is_camera_locked(core))
    {
        esz_clear_player_state(STATE_GOING_RIGHT, core);
        esz_clear_player_state(STATE_LOOKING_RIGHT, core);
        esz_set_player_state(STATE_GOING_LEFT, core);
        esz_set_player_state(STATE_LOOKING_LEFT, core);
        esz_set_player_state(STATE_MOVING, core);
    }
    else
    {
        esz_clear_player_state(STATE_MOVING, core);
    }
}

static void walk_right(esz_core_t* core)
{
    if (esz_is_camera_locked(core))
    {
        esz_clear_player_state(STATE_GOING_LEFT, core);
        esz_clear_player_state(STATE_LOOKING_LEFT, core);
        esz_set_player_state(STATE_GOING_RIGHT, core);
        esz_set_player_state(STATE_LOOKING_RIGHT, core);
        esz_set_player_state(STATE_MOVING, core);
    }
    else
    {
        esz_clear_player_state(STATE_MOVING, core);
    }
}


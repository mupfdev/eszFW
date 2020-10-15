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

    status = esz_init_core(&core);
    if (ESZ_OK != status)
    {
        goto quit;
    }

    esz_load_map(MAP_FILE, window, core);
    esz_register_event_callback(EVENT_KEYDOWN, &key_down_callback, core);

    while (esz_is_core_active(core))
    {
        esz_update_core(window, core);

        status = esz_show_scene(window, core);
        if (ESZ_ERROR_CRITICAL == status)
        {
            break;
        }
    }

quit:
    esz_unload_map(window, core);
    esz_destroy_core(core);
    esz_destroy_window(window);

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
    }
}

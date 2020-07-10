#define SDL_MAIN_HANDLED

#include <stdlib.h>
#include <SDL.h>
#include <esz.h>

static void key_down_callback(esz_window* window, esz_core* core);

int main()
{
    esz_status        status;
    esz_window*       window = NULL;
    esz_window_config config = { 640, 360, 384, 216, SDL_FALSE, SDL_FALSE };
    esz_core*         core   = NULL;

    status = esz_create_window("eszFW", &config, &window);
    if (ESZ_OK != status)
    {
        goto quit;
    }

    status = esz_init_core(&core);
    if (ESZ_OK != status)
    {
        goto quit;
    }

    esz_load_map("res/maps/example.tmx", window, core);

    esz_register_event_callback(EVENT_KEYDOWN, &key_down_callback, core);

    while (esz_is_core_active(core))
    {
        esz_update_core(window, core);

        status = esz_draw_frame(window, core);
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

static void key_down_callback(esz_window* window, esz_core* core)
{
    switch (esz_get_keycode(core))
    {
        case SDLK_f:
            esz_toggle_fullscreen(window);
            break;
        case SDLK_q:
            esz_deactivate_core(core);
            break;
    }
}

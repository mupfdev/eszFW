// SPDX-License-Identifier: MIT
/**
 * @file  esz_init.h
 * @brief eszFW initialisation and loading routines
 */

#ifndef ESZ_INIT_H
#define ESZ_INIT_H

#include <stdint.h>
#include <SDL.h>

#include "esz_types.h"

esz_status init_animated_tiles(esz_core_t* core);
esz_status init_background(esz_window_t* window, esz_core_t* core);
esz_status init_entities(esz_core_t* core);
esz_status init_sprites(esz_window_t* window, esz_core_t* core);
esz_status load_texture_from_file(const char* file_name, SDL_Texture** texture, esz_window_t* window);
esz_status load_texture_from_memory(const unsigned char* buffer, const int length, SDL_Texture** texture, esz_window_t* window);

#endif // ESZ_INIT_H

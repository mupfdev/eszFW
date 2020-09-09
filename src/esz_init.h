// SPDX-License-Identifier: MIT
/**
 * @file  esz_init.h
 * @brief eszFW initialisation functions
 */

#ifndef ESZ_INIT_H
#define ESZ_INIT_H

#include <stdint.h>
#include <SDL.h>

#include "esz_types.h"

esz_status load_animated_tiles(esz_core_t* core);
esz_status load_background(esz_window_t* window, esz_core_t* core);
esz_status load_entities(esz_core_t* core);
esz_status load_map_path(const char* map_file_name, esz_core_t* core);
esz_status load_sprites(esz_window_t* window, esz_core_t* core);
esz_status load_tile_properties(esz_core_t* core);
esz_status load_tileset(esz_window_t* window, esz_core_t*);
esz_status load_texture_from_file(const char* file_name, SDL_Texture** texture, esz_window_t* window);
esz_status load_texture_from_memory(const unsigned char* buffer, const int length, SDL_Texture** texture, esz_window_t* window);

#endif // ESZ_INIT_H

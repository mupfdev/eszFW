// SPDX-License-Identifier: MIT
/**
 * @file    esz_render.h
 * @brief   eszFW rendering and scene drawing
 */

#ifndef ESZ_RENDER_H
#define ESZ_RENDER_H

#include <stdint.h>

#include "esz_types.h"

esz_status create_and_set_render_target(SDL_Texture** target, esz_window_t* window);
esz_status draw_scene(esz_window_t* window, esz_core_t* core);
esz_status render_actors(int32_t level, esz_window_t* window, esz_core_t* core);
esz_status render_background(esz_window_t* window, esz_core_t* core);
esz_status render_background_layer(int32_t index, esz_window_t* window, esz_core_t* core);
esz_status render_map(int32_t level, esz_window_t* window, esz_core_t* core);
esz_status render_scene(esz_window_t* window, esz_core_t* core);

#endif // ESZ_RENDER_H

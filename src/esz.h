// SPDX-License-Identifier: MIT
/**
 * @file    esz.h
 * @brief   eszFW main include header
 * @example minimal_application.c
 */
#ifndef ESZ_H
#define ESZ_H

#include <stdbool.h>
#include <stdint.h>
#include "esz_types.h"

/**
 * @brief  Check if two axis-aligned bounding boxes intersect
 * @param  bb_a Box A
 * @param  bb_b Box B
 * @return Boolean condition
 * @retval true The boxes intersect
 * @retval false The boxes do not intersect
 */
bool esz_bounding_boxes_do_intersect(const esz_aabb_t bb_a, const esz_aabb_t bb_b);

/**
 * @brief   Create window and rendering context
 * @details It tries to use the opengl rendering driver. If the driver
 *          is not found, the system's default driver is used instead.
 * @param   window_title The window title
 * @param   config Initial window configuration
 * @param   window Pointer to window handle
 * @return  Status code
 * @retval  ESZ_OK OK
 * @retval  ESZ_ERROR_CRITICAL
 *          Critical error; the application should be terminated
 */
esz_status esz_create_window(const char* window_title, esz_window_config_t* config, esz_window_t** window);

/**
 * @brief   Deactivate engine core
 * @details Can be used to signal the application that the program
 *          should be terminated.
 * @param   core Engine core
 */
void esz_deactivate_core(esz_core_t* core);

/**
 * @brief     Destroy engine core
 * @attention Before calling this function, make sure that no map is
 *            loaded for this core!
 * @param     core Engine core
 */
void esz_destroy_core(esz_core_t* core);

/**
 * @brief   Destroy window and rendering context
 * @details This function should be called when exiting the application.
 * @param   window Window handle
 */
void esz_destroy_window(esz_window_t* window);

/**
 * @brief  Get boolean map property
 * @param  name_hash Hash of the property name.  @see esz_hash()
 * @param  core Engine core
 * @return Boolean value, or 0.0 if not property does not exist
 */
bool esz_get_boolean_map_property(const uint64_t name_hash, esz_core_t* core);

/**
 * @brief  Get decimal map property
 * @param  name_hash Hash of the property name.  @see esz_hash()
 * @param  core Engine core
 * @return Boolean value, or false if property does not exist
 */
double esz_get_decimal_map_property(const uint64_t name_hash, esz_core_t* core);

/**
 * @brief  Get integer map property
 * @param  name_hash Hash of the property name.  @see esz_hash()
 * @param  core Engine core
 * @return Integer value, or 0 if property does not exist
 */
int32_t esz_get_integer_map_property(const uint64_t name_hash, esz_core_t* core);

/**
 * @brief  Get string or file type map property
 * @param  name_hash Hash of the property name.  @see esz_hash()
 * @param  core Engine core
 * @return The string, or NULL if property does not exist
 */
const char* esz_get_string_map_property(const uint64_t name_hash, esz_core_t* core);

/**
 * @brief  Get the current state of the keyboard
 * @return Returns a pointer to am array of key states. Indexes into
 *         this array are obtained by using SDL_Scancode values: See
 *         https://wiki.libsdl.org/SDL_Scancode
 */
const uint8_t* esz_get_keyboard_state(void);

/**
 * @brief   Generate hash
 * @details djb2 by Dan Bernstein.  See
 *          http://www.cse.yorku.ca/~oz/hash.html for details
 * @param   name The name from which a hash is to be generated
 * @return  Hash value
 */
uint64_t esz_hash(const unsigned char* name);

/**
 * @brief  Get the current keycode
 * @param  core Engine core

 * @return SDL_Keycode value.  See https://wiki.libsdl.org/SDL_Keycode
 *         for details
 */
int32_t esz_get_keycode(esz_core_t* core);

/**
 * @brief  Get the time since the last frame in seconds
 * @param  window Window handle
 * @return Time since last frame in seconds
 */
double esz_get_time_since_last_frame(esz_window_t* window);

/**
 * @brief  Initialise engine core
 * @param  core Engine core
 * @return Status code
 * @retval ESZ_OK OK
 * @retval ESZ_ERROR_CRITICAL
 *         Critical error; the application should be terminated
 */
esz_status esz_init_core(esz_core_t** core);

/**
 * @brief  Check if the camera is currently locked
 * @param  core Engine core
 * @return Boolean condition
 */
bool esz_is_camera_locked(esz_core_t* core);

/**
 * @brief  Check if engine core is currently active
 * @param  core Engine core
 * @return Boolean condition
 * @retval true Engine core is active
 * @retval false Engine core is not active
 */
bool esz_is_core_active(esz_core_t* core);

/**
 * @brief  Check if a map is currently loaded
 * @param  core Engine core
 * @return Boolean condition
 * @retval true Map is loaded
 * @retval false Map is not loaded
 */
bool esz_is_map_loaded(esz_core_t* core);

/**
 * @brief  Check if the active player entity is currently moving
 * @param  core Engine core
 * @return Boolean condition
 */
bool esz_is_player_moving(esz_core_t* core);

/**
 * @brief     Load map file
 * @attention Before calling this function, make sure that the engine
 *            core has been initialised!
 * @param     map_file_name Path and file name to the map file
 * @param     window Window handle
 * @param     core Engine core
 * @return    Status code
 * @retval    ESZ_OK OK
 * @retval    ESZ_WARNING Map could not be loaded
 */
esz_status esz_load_map(const char* map_file_name, esz_window_t* window, esz_core_t* core);

/**
 * @brief   Lock camera for engine core
 * @details If the camera is locked, it automatically follows the main
 *          player entity.
 * @param   core Engine core
 */
void esz_lock_camera(esz_core_t* core);

/**
 * @brief Register callback function which is called when the event
 *        occurs
 * @param event_type Event type
 * @param event_callback Callback function
 * @param core Engine core
 */
void esz_register_event_callback(const esz_event_type event_type, esz_event_callback event_callback, esz_core_t* core);

/**
 * @brief  Set active player entity
 * @param  id Entity ID
 * @param  core Engine core
*/
void esz_set_active_player_entity(int32_t id, esz_core_t* core);

/**
 * @brief  Set the position of the camera
 * @remark The camera has to be unlocked.
 * @param  pos_x Position along the x-axis
 * @param  pos_y Position along the y-axis
 * @param  pos_is_relative Use relative instead of absolute position
 * @param  window Window handle
 * @param  core Engine core
 */
void esz_set_camera_position(const double pos_x, const double pos_y, bool pos_is_relative, esz_window_t* window, esz_core_t* core);

/**
 * @brief  Set the position of the camera
 * @remark The camera has to be locked.
 * @param  id Entity ID
 * @param  core Engine core
 */
void esz_set_camera_target(const int32_t id, esz_core_t* core);

/**
 * @brief   Select and set the next animation of active player entity
 * @details If the last animation is skipped, it selects the first one
 *          again.
 * @remark  This function is mainly intended for debugging purposes.
 * @param   core Engine core
 */
void esz_set_next_player_animation(esz_core_t* core);

/**
 * @brief Set animation of active player entity
 * @param id Animation ID
 * @param core Engine core
 */
void esz_set_player_animation(int32_t id, esz_core_t* core);

/**
 * @brief  Set the window's zoom level
 * @param  factor Zoom factor
 * @param  window Window handle
 * @return Status code
 * @retval ESZ_OK OK
 * @retval ESZ_WARNING
 *         The zoom-level could not be set
 */
esz_status esz_set_zoom_level(const double factor, esz_window_t* window);

/**
 * @brief  Render and draw the current scene
 * @param  window Window handle
 * @param  core Engine core
 * @return Status code
 * @retval ESZ_OK OK
 * @retval ESZ_ERROR_CRITICAL
 *         Critical error; the application should be terminated
 */
esz_status esz_show_scene(esz_window_t* window, esz_core_t* core);

/**
 * @brief  Toggle between fullscreen and windowed mode
 * @param  window Window handle
 * @return Status code
 * @retval ESZ_OK OK
 * @retval ESZ_WARNING
 *         Fullscreen could not be toggled
 */
esz_status esz_toggle_fullscreen(esz_window_t* window);

/**
 * @brief  Unload current map
 * @remark It's always safe to call this function; if no map is
 *         currently loaded, the function does nothing.
 * @param  window Window handle
 * @param  core Engine core
 */
void esz_unload_map(esz_window_t* window, esz_core_t* core);

/**
 * @brief   Unlock camera for engine core
 * @details If the camera is unlocked, it can be moved freely around the map.
 * @param   core Engine core
 */
void esz_unlock_camera(esz_core_t* core);

/**
 * @brief   Update engine core
 * @details This function should be called cyclically in the main loop
 *          of the application.
 * @param   window Window handle
 * @param   core Engine core
 */
void esz_update_core(esz_window_t* window, esz_core_t* core);

#endif // ESZ_H

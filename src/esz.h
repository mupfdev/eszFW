// SPDX-License-Identifier: MIT
/**
 * @file    esz.h
 * @brief   eszFW main include header
 * @example minimal_application.c
 */
#ifndef ESZ_H
#define ESZ_H

#include <SDL.h>
#include <tmx.h>

typedef struct esz_window esz_window_t;
typedef struct esz_core   esz_core_t;

/**
 * @def   ESZ_MAX_PATH_LEN
 * @brief Maximum length of resource path
 */
#ifndef ESZ_MAX_PATH_LEN
#define ESZ_MAX_PATH_LEN 64
#endif

/**
 * @def   ESZ_MAX_PATTERN_LEN
 * @brief Maximum length of search pattern
 */
#ifndef ESZ_MAX_PATTERN_LEN
#define ESZ_MAX_PATTERN_LEN 32
#endif

/**
 * @brief     Event callback function type
 * @attention An event callback function always expects a window handle
 *            as first and a core handle as second parameter!
 */
typedef void (*esz_event_callback)(esz_window_t* window, esz_core_t* core);

/**
 * @brief An enumeration of alignments.
 */
typedef enum
{
    ESZ_BOT = 0,
    ESZ_TOP

} esz_alignment;

/**
 * @brief An enumeration of background types.
 */
typedef enum
{
    ESZ_BG = 0,
    ESZ_FG,
    ESZ_LAYER_MAX

} esz_layer_type;

/**
 * @brief An enumeration of directional constants.
 */
typedef enum
{
    ESZ_LEFT = 0,
    ESZ_RIGHT

} esz_direction;

/**
 * @brief An enumeration of event types
 */
typedef enum
{
    EVENT_FINGERDOWN = 0,
    EVENT_FINGERMOTION,
    EVENT_FINGERUP,
    EVENT_KEYDOWN,
    EVENT_KEYUP,
    EVENT_MAP_LOADED,
    EVENT_MAP_UNLOADED,
    EVENT_MULTIGESTURE

} esz_event_type;

/**
 * @brief An enumeration of status codes.
 */
typedef enum
{
    ESZ_OK             =  0,
    ESZ_ERROR_CRITICAL = -1,
    ESZ_ERROR_WARNING  = -2

} esz_status;

/**
 * @brief A structure that contains a axis-aligned bounding box.
 */
typedef struct esz_aabb_t
{
    double bottom;
    double left;
    double right;
    double top;

} esz_aabb;

/**
 * @brief A structure that contains an animated tile.
 */
typedef struct esz_animated_tile
{
    Sint32 dst_x;
    Sint32 dst_y;
    Uint32 animation_length;
    Uint32 current_frame;
    Uint32 gid;
    Uint32 id;

} esz_animated_tile_t;

/**
 * @brief A structure that contains a background layer.
 */
typedef struct esz_background_layer
{
    double        pos_x;
    double        pos_y;
    double        velocity;
    SDL_Texture*  render_target;
    Sint32        width;
    Sint32        height;

} esz_background_layer_t;

/**
 * @brief A structure that contains a parallax-scrolling background.
 */
typedef struct esz_background
{
    esz_alignment           alignment;
    esz_direction           direction;
    esz_background_layer_t* layer;
    SDL_Texture*            render_target;
    double                  velocity;
    double                  layer_shift;
    Sint32                  layer_count;
    char                    reserverd[4];

} esz_background_t;

/**
 * @brief A structure that contains a camera.
 */
typedef struct esz_camera
{
    double   pos_x;
    double   pos_y;
    Sint32   max_pos_x;
    Sint32   max_pos_y;
    Uint16   flags;
    Uint16   target_entity_id;
    SDL_bool is_at_horizontal_boundary;
    SDL_bool is_locked;
    char     reserverd[4];

} esz_camera_t;

/**
 * @brief A structure that contains the initial window configuration.
 */
typedef struct esz_window_config
{
    const Sint32   width;
    const Sint32   height;
    const Sint32   logical_width;
    const Sint32   logical_height;
    const SDL_bool enable_fullscreen;
    const SDL_bool enable_vsync;

} esz_window_config_t;

/**
 * @brief A structure that contains the event handler.
 */
typedef struct esz_event
{
    SDL_Event handle;

    void (*finger_down_cb)(esz_window_t*   window, esz_core_t* core);
    void (*finger_motion_cb)(esz_window_t* window, esz_core_t* core);
    void (*finger_up_cb)(esz_window_t*     window, esz_core_t* core);
    void (*key_down_cb)(esz_window_t*      window, esz_core_t* core);
    void (*key_up_cb)(esz_window_t*        window, esz_core_t* core);
    void (*map_loaded_cb)(esz_window_t*    window, esz_core_t* core);
    void (*map_unloaded_cb)(esz_window_t*  window, esz_core_t* core);
    void (*multi_gesture_cb)(esz_window_t* window, esz_core_t* core);

} esz_event_t;

/**
 * @brief A structure that contains a map object
 */
typedef struct esz_object
{
    esz_aabb    bounding_box;
    tmx_object* tmx_object;
    Uint32      pos_x;
    Uint32      pos_y;
    Uint32      height;
    Uint32      id;
    Uint32      width;
    char        reserverd[4];

} esz_object_t;

/**
 * @brief A structure that contains a sprite.
 */
typedef struct esz_sprite
{
    SDL_Texture* render_target;
    Uint32       id;
    char         reserverd[4];

} esz_sprite_t;

/**
 * @brief A structure that contains a game map.
 */
typedef struct esz_map
{
    char                  resource_path[ESZ_MAX_PATH_LEN];
    char                  search_pattern[ESZ_MAX_PATTERN_LEN];
    char                  tileset_image[ESZ_MAX_PATH_LEN];
    double                decimal_property;
    double                gravitation;
    double                pos_x;
    double                pos_y;
    double                time_since_last_anim_frame;
    SDL_Texture*          animated_tile_texture;
    SDL_Texture*          map_layer[ESZ_LAYER_MAX];
    SDL_Texture*          render_target[ESZ_LAYER_MAX];
    SDL_Texture*          tileset_texture;
    char*                 string_property;
    esz_animated_tile_t*  animated_tile;
    struct esz_background background;
    esz_object_t*         object;
    esz_sprite_t*         sprite;
    tmx_map*              tmx_map;
    Sint32                animated_tile_fps;
    Sint32                integer_property;
    Sint32                meter_in_pixel;
    Sint32                sprite_sheet_count;
    Uint32                height;
    Uint32                width;
    size_t                resource_path_length;
    Uint32                animated_tile_count;
    Uint32                object_count;
    SDL_bool              boolean_property;
    SDL_bool              is_loaded;

} esz_map_t;

/**
 * @brief A structure that contains an engine core.
 */
typedef struct esz_core
{
    struct esz_camera camera;
    struct esz_event  event;
    struct esz_map    map;
    SDL_bool          is_active;
    SDL_bool          is_paused;

} esz_core_t;

/**
 * @brief A structure that contains a window and the rendering context.
 */
typedef struct esz_window
{
    double        initial_zoom_level;
    double        time_since_last_frame;
    double        zoom_level;
    SDL_Renderer* renderer;
    SDL_Texture*  esz_logo;
    SDL_Window*   window;
    Sint32        height;
    Sint32        logical_height;
    Sint32        logical_width;
    Sint32        pos_x;
    Sint32        pos_y;
    Sint32        refresh_rate;
    Sint32        width;
    Uint32        flags;
    Uint32        time_a;
    Uint32        time_b;
    SDL_bool      is_fullscreen;
    SDL_bool      vsync_enabled;

} esz_window_t;

/**
 * @brief  Check if two axis-aligned bounding boxes intersect
 * @param  bb_a Box A
 * @param  bb_b Box B
 * @return Boolean condition
 * @retval SDL_TRUE The boxes intersect
 * @retval SDL_FALSE The boxes do not intersect
 */
SDL_bool esz_bounding_boxes_do_intersect(const esz_aabb bb_a, const esz_aabb bb_b);

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
 * @brief  Draw/render the current frame
 * @param  window Window handle
 * @param  core Engine core
 * @return Status code
 * @retval ESZ_OK OK
 * @retval ESZ_ERROR_CRITICAL
 *         Critical error; the application should be terminated
 */
esz_status esz_draw_frame(esz_window_t* window, esz_core_t* core);

/**
 * @brief  Get the current state of the keyboard
 * @return Returns a pointer to am array of key states. Indexes into
 *         this array are obtained by using SDL_Scancode values: See
 *         https://wiki.libsdl.org/SDL_Scancode
 */
const Uint8* esz_get_keyboard_state(void);

/**
 * @brief  Get the current keycode
 * @param  core Engine core
 * @return SDL_Keycode value. See https://wiki.libsdl.org/SDL_Keycode
 *         for details
 */
Sint32 esz_get_keycode(esz_core_t* core);

/**
 * @brief  Get the time since the last frame in seconds
 * @param  window Window handle
 * @return Time since last frame in seconds
 */
double esz_get_time_since_last_frame(esz_window_t* window);

/**
 * @brief  Initialise engine core
 * @param  core Pointer to engine core handle
 * @return Status code
 * @retval ESZ_OK OK
 * @retval ESZ_ERROR_CRITICAL
 *         Critical error; the application should be terminated
 */
esz_status esz_init_core(esz_core_t** core);

/**
 * @brief  Check if engine core is currently active
 * @param  core Engine core
 * @return Boolean condition
 * @retval SDL_TRUE Engine core is active
 * @retval SDL_FALSE Engine core is not active
 */
SDL_bool esz_is_core_active(esz_core_t* core);

/**
 * @brief  Check if a map is currently loaded
 * @param  core Engine core handle
 * @return Boolean condition
 * @retval SDL_TRUE Map is loaded
 * @retval SDL_FALSE Map is not loaded
 */
SDL_bool esz_is_map_loaded(esz_core_t* core);

/**
 * @brief     Load map file
 * @attention Before calling this function, make sure that the engine
 *            core has been initialised!
 * @param     map_file_name Path and file name to the tmx map file
 * @param     window Window handle
 * @param     core Engine core handle
 */
void esz_load_map(const char* map_file_name, esz_window_t* window, esz_core_t* core);

/**
 * @brief   Lock camera for engine core
 * @details If the camera is locked, it automatically follows the main
 *          player entity.
 * @param   core Engine core handle
 */
void esz_lock_camera(esz_core_t* core);

/**
 * @brief Register callback function which is called when the event
 *        occurs
 * @param event_type Event type
 * @param event_callback Callback function
 * @param core Engine core handle
 */
void esz_register_event_callback(const esz_event_type event_type, esz_event_callback event_callback, esz_core_t* core);

/**
 * @brief  Set the position of the camera
 * @remark The camera has to be unlocked first!
 * @param  pos_x Position along the x-axis
 * @param  pos_y Position along the y-axis
 * @param  pos_is_relative Use relative instead of absolute position
 * @param  window Window handle
 * @param  core Engine core handle
 */
void esz_set_camera_position(const double pos_x, const double pos_y, SDL_bool pos_is_relative, esz_window_t* window, esz_core_t* core);

/**
 * @brief  Set the window's zoom level
 * @param  factor Zoom factor
 * @param  window Window handle
 * @return Status code
 * @retval ESZ_OK OK
 * @retval ESZ_ERROR_WARNING
 *         The zoom-level could not be set
 */
esz_status esz_set_zoom_level(const double factor, esz_window_t* window);

/**
 * @brief  Toggle between fullscreen and windowed mode
 * @param  window Window handle
 * @return Status code
 * @retval ESZ_OK OK
 * @retval ESZ_ERROR_WARNING
 *         Fullscreen could not be toggled
 */
esz_status esz_toggle_fullscreen(esz_window_t* window);

/**
 * @brief  Unload current map
 * @remark It's always safe to call this function; if no map is
 *         currently loaded, the function does nothing.
 * @param  window Window handle
 * @param  core Engine core handle
 */

void esz_unload_map(esz_window_t* window, esz_core_t* core);

/**
 * @brief   Unlock camera for engine core
 * @details If the camera is unlocked, it can be moved freely around the map.
 * @param   core Engine core handle
 */
void esz_unlock_camera(esz_core_t* core);

/**
 * @brief   Update engine core
 * @details This function should be called cyclically in the main loop
 *          of the application.
 * @param   window Window handle
 * @param   core Engine core handle
 */
void esz_update_core(esz_window_t* window, esz_core_t* core);

#endif // ESZ_H

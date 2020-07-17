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
#include <SDL.h>

#ifdef USE_LIBTMX
#include <tmx.h>
#else
#include <cute_tiled.h>
#endif

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
    int32_t  dst_x;
    int32_t  dst_y;
    uint32_t animation_length;
    uint32_t current_frame;
    uint32_t gid;
    uint32_t id;

} esz_animated_tile_t;

/**
 * @brief A structure that contains a background layer.
 */
typedef struct esz_background_layer
{
    double       pos_x;
    double       pos_y;
    double       velocity;
    SDL_Texture* render_target;
    int32_t      width;
    int32_t      height;

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
    int32_t                 layer_count;
    char                    reserverd[4];

} esz_background_t;

/**
 * @brief A structure that contains a camera.
 */
typedef struct esz_camera
{
    double   pos_x;
    double   pos_y;
    int32_t  max_pos_x;
    int32_t  max_pos_y;
    uint16_t target_entity_id;
    bool     is_at_horizontal_boundary;
    bool     is_locked;
    char     reserverd[4];

} esz_camera_t;

/**
 * @brief A structure that contains the initial window configuration.
 */
typedef struct esz_window_config
{
    const int32_t width;
    const int32_t height;
    const int32_t logical_width;
    const int32_t logical_height;
    const bool    enable_fullscreen;
    const bool    enable_vsync;

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
    esz_aabb             bounding_box;

    #ifdef USE_LIBTMX
    tmx_object*          handle;
    #else
    cute_tiled_object_t* handle;
    #endif

    uint32_t             pos_x;
    uint32_t             pos_y;
    uint32_t             height;
    uint32_t             id;
    uint32_t             width;
    char                 reserverd[4];

} esz_object_t;

/**
 * @brief A structure that contains a sprite.
 */
typedef struct esz_sprite
{
    SDL_Texture* render_target;
    uint32_t     id;
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

    #ifndef USE_LIBTMX
    unsigned long long    hash_id_objectgroup;
    unsigned long long    hash_id_tilelayer;
    #endif

    SDL_Texture*          animated_tile_texture;
    SDL_Texture*          map_layer[ESZ_LAYER_MAX];
    SDL_Texture*          render_target[ESZ_LAYER_MAX];
    SDL_Texture*          tileset_texture;
    const char*           string_property;
    esz_animated_tile_t*  animated_tile;
    struct esz_background background;
    esz_object_t*         object;
    esz_sprite_t*         sprite;

    #ifdef USE_LIBTMX
    tmx_map*              handle;
    #else
    cute_tiled_map_t*     handle;
    #endif

    int32_t               animated_tile_index;
    int32_t               animated_tile_fps;
    int32_t               integer_property;
    int32_t               meter_in_pixel;
    int32_t               animated_tile_count;
    int32_t               object_count;
    int32_t               sprite_sheet_count;
    int32_t               height;
    int32_t               width;
    size_t                resource_path_length;
    bool                  boolean_property;
    bool                  is_loaded;

} esz_map_t;

/**
 * @brief A structure that contains an engine core.
 */
typedef struct esz_core
{
    struct esz_camera camera;
    struct esz_event  event;
    struct esz_map    map;
    bool              is_active;
    bool              is_paused;

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
    int32_t       height;
    int32_t       logical_height;
    int32_t       logical_width;
    int32_t       pos_x;
    int32_t       pos_y;
    int32_t       refresh_rate;
    int32_t       width;
    uint32_t      flags;
    uint32_t      time_a;
    uint32_t      time_b;
    bool          is_fullscreen;
    bool          vsync_enabled;

} esz_window_t;

/**
 * @brief  Check if two axis-aligned bounding boxes intersect
 * @param  bb_a Box A
 * @param  bb_b Box B
 * @return Boolean condition
 * @retval true The boxes intersect
 * @retval false The boxes do not intersect
 */
bool esz_bounding_boxes_do_intersect(const esz_aabb bb_a, const esz_aabb bb_b);

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
const uint8_t* esz_get_keyboard_state(void);

/**
 * @brief  Get the current keycode
 * @param  core Engine core
 * @return SDL_Keycode value. See https://wiki.libsdl.org/SDL_Keycode
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
 * @retval true Engine core is active
 * @retval false Engine core is not active
 */
bool esz_is_core_active(esz_core_t* core);

/**
 * @brief  Check if a map is currently loaded
 * @param  core Engine core handle
 * @return Boolean condition
 * @retval true Map is loaded
 * @retval false Map is not loaded
 */
bool esz_is_map_loaded(esz_core_t* core);

/**
 * @brief     Load map file
 * @attention Before calling this function, make sure that the engine
 *            core has been initialised!
 * @param     map_file_name Path and file name to the map file
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
void esz_set_camera_position(const double pos_x, const double pos_y, bool pos_is_relative, esz_window_t* window, esz_core_t* core);

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

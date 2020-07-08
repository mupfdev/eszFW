// SPDX-License-Identifier: Beerware
/**
 * @file    esz.h
 * @brief   eszFW main include header
 * @example minimal_application.c
 */
#ifndef ESZ_H
#define ESZ_H

#include <SDL.h>
#include <tmx.h>

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
typedef void (*esz_event_callback)(void* window, void* core);

/**
 * @brief An enumeration of alignments.
 */
typedef enum
{
    ESZ_BOTTOM = 0,
    ESZ_TOP

} esz_alignment;

/**
 * @brief An enumeration of camera flags.
 */
typedef enum
{
    CAMERA_IS_LOCKED = 0

} esz_camera_flag;

/**
 * @brief An enumeration of background types.
 */
typedef enum
{
    ESZ_BACKGROUND = 0,
    ESZ_FOREGROUND,
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
typedef struct esz_animated_tile_t
{
    Sint32 dst_x;
    Sint32 dst_y;
    Uint16 gid;
    Uint16 id;
    Uint8  current_frame;
    Uint8  animation_length;

} esz_animated_tile;

/**
 * @brief A structure that contains a background layer.
 */
typedef struct esz_background_layer_t
{
    double        pos_x;
    double        pos_y;
    double        velocity;
    SDL_Texture*  texture;
    Sint32        width;
    Sint32        height;

} esz_background_layer;

/**
 * @brief A structure that contains a parallax-scrolling background.
 */
typedef struct esz_background_t
{
    esz_alignment         alignment;
    esz_direction         direction;
    esz_background_layer* layer;
    Uint8                 layer_count;

} esz_background;

/**
 * @brief A structure that contains a camera.
 */
typedef struct esz_camera_t
{
    double pos_x;
    double pos_y;
    Sint32 max_pos_x;
    Sint32 max_pos_y;
    Uint16 flags;
    Uint16 target_entity_id;

} esz_camera;

/**
 * @brief A structure that contains the initial window configuration.
 */
typedef struct esz_window_config_t
{
    const Sint32   width;
    const Sint32   height;
    const Sint32   logical_width;
    const Sint32   logical_height;
    const SDL_bool enable_fullscreen;
    const SDL_bool enable_vsync;

} esz_window_config;

/**
 * @brief A structure that contains the event handler.
 */
typedef struct esz_event_t
{
    SDL_Event handle;

    void (*finger_down_cb)(void* window, void* core);
    void (*finger_motion_cb)(void* window, void* core);
    void (*finger_up_cb)(void* window, void* core);
    void (*key_down_cb)(void* window, void* core);
    void (*key_up_cb)(void* window, void* core);
    void (*map_loaded_cb)(void* window, void* core);
    void (*map_unloaded_cb)(void* window, void* core);
    void (*multi_gesture_cb)(void* window, void* core);

} esz_event;

/**
 * @brief A structure that contains a map object
 */
typedef struct esz_object_t
{
    esz_aabb    bounding_box;
    tmx_object* tmx_object;
    Uint32      pos_x;
    Uint32      pos_y;
    Uint16      height;
    Uint16      id;
    Uint16      width;

} esz_object;

/**
 * @brief A structure that contains a sprite.
 */
typedef struct esz_sprite_t
{
    SDL_Texture* texture;
    Uint16       id;

} esz_sprite;

/**
 * @brief A structure that contains a game map.
 */
typedef struct esz_map_t
{
    char               resource_path[ESZ_MAX_PATH_LEN];
    char               search_pattern[ESZ_MAX_PATTERN_LEN];
    char               tileset_image[ESZ_MAX_PATH_LEN];
    double             decimal_property;
    double             gravitation;
    double             pos_x;
    double             pos_y;
    double             time_since_last_anim_frame;
    SDL_Texture*       animated_tile_texture;
    SDL_Texture*       map_layer[ESZ_LAYER_MAX];
    SDL_Texture*       texture;
    SDL_Texture*       tileset_texture;
    char*              string_property;
    esz_animated_tile* animated_tile;
    esz_background     background;
    esz_object*        object;
    esz_sprite*        sprite;
    tmx_map*           tmx_map;
    Sint32             animated_tile_fps;
    Sint32             background_layer_count;
    Sint32             integer_property;
    Sint32             meter_in_pixel;
    Sint32             sprite_sheet_count;
    Uint32             height;
    Uint32             width;
    size_t             resource_path_length;
    Uint16             animated_tile_count;
    Uint16             object_count;
    SDL_bool           boolean_property;
    SDL_bool           is_loaded;

} esz_map;

/**
 * @brief A structure that contains an engine core.
 */
typedef struct esz_core_t
{
    esz_camera camera;
    esz_event  event;
    esz_map    map;
    SDL_bool   is_active;
    SDL_bool   is_paused;

} esz_core;

/**
 * @brief A structure that contains a window and the rendering context.
 */
typedef struct esz_window_t
{
    double        initial_zoom_level;
    double        time_since_last_frame;
    double        zoom_level;
    SDL_Renderer* renderer;
    SDL_Window*   window;
    Sint32        height;
    Sint32        logical_height;
    Sint32        logical_width;
    Sint32        pos_x;
    Sint32        pos_y;
    Sint32        refresh_rate;
    Sint32        width;
    Uint32        flags;
    SDL_bool      is_fullscreen;
    SDL_bool      vsync_enabled;

} esz_window;

SDL_bool   esz_bounding_boxes_do_intersect(const esz_aabb bb_a, const esz_aabb bb_b);
esz_status esz_create_window(const char* window_title, esz_window_config* config, esz_window** window);
void       esz_deactivate_core(esz_core* core);
void       esz_destroy_core(esz_core* core);
void       esz_destroy_window(esz_window* window);
void       esz_draw_frame(Uint32* time_a, Uint32* time_b, esz_window*, esz_core* core);
Uint32     esz_get_keycode(esz_core* core);
double     esz_get_time_since_last_frame(esz_window* core);
esz_status esz_init_core(esz_core** core);
SDL_bool   esz_is_core_active(esz_core* core);
void       esz_load_map(const char* map_file_name, esz_window* window, esz_core* core);
void       esz_lock_camera(esz_core* core);
void       esz_register_event_callback(const esz_event_type event_type, esz_event_callback event_callback, esz_core* core);
esz_status esz_set_zoom_level(const double factor, esz_window* window);
esz_status esz_toggle_fullscreen(esz_window* window);
void       esz_unlock_camera(esz_core* core);
void       esz_unload_map(esz_window* window, esz_core* core);
void       esz_update_core(esz_window* window, esz_core* core);

#endif // ESZ_H

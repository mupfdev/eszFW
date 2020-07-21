// SPDX-License-Identifier: MIT
/**
 * @file  esz_types.h
 * @brief eszFW types
 */

#ifndef ESZ_TYPES_H
#define ESZ_TYPES_H

#include <SDL.h>

#ifdef USE_LIBTMX
typedef struct _tmx_map tmx_map;
typedef struct _tmx_obj tmx_object;
typedef struct _tmx_prop tmx_property;

typedef tmx_map      esz_map_handle_t;
typedef tmx_object   esz_object_handle_t;
typedef tmx_property esz_property_handle_t;

#include <tmx.h>

#elif  USE_CUTE_TILED
struct cute_tiled_map_t;
struct cute_tiled_object_t;
struct cute_tiled_property_t;

typedef struct cute_tiled_map_t      esz_map_handle_t;
typedef struct cute_tiled_object_t   esz_object_handle_t;
typedef struct cute_tiled_property_t esz_property_handle_t;

#include <cute_tiled.h>

#endif

typedef struct esz_window esz_window_t;
typedef struct esz_core   esz_core_t;

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
 * @brief An enumeration of directional constants.
 */
typedef enum
{
    ESZ_LEFT = 0,
    ESZ_RIGHT

} esz_direction;

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
    ESZ_WARNING        = -2

} esz_status;

/**
 * @brief A structure that contains a axis-aligned bounding box.
 */
typedef struct esz_aabb
{
    double bottom;
    double left;
    double right;
    double top;

} esz_aabb_t;

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
    esz_aabb_t           bounding_box;
    esz_object_handle_t* handle;
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
    double                gravitation;
    double                pos_x;
    double                pos_y;
    double                time_since_last_anim_frame;

    #ifdef USE_LIBTMX
    unsigned long         hash_query;
    #elif  USE_CUTE_TILED
    unsigned long long    hash_id_objectgroup;
    unsigned long long    hash_id_tilelayer;
    #endif

    char*                 path;
    SDL_Texture*          animated_tile_texture;
    SDL_Texture*          map_layer[ESZ_LAYER_MAX];
    SDL_Texture*          render_target[ESZ_LAYER_MAX];
    SDL_Texture*          tileset_texture;
    esz_animated_tile_t*  animated_tile;
    struct esz_background background;
    esz_object_t*         object;
    esz_sprite_t*         sprite;
    esz_map_handle_t*     handle;
    int32_t               animated_tile_index;
    int32_t               animated_tile_fps;
    int32_t               meter_in_pixel;
    int32_t               animated_tile_count;
    int32_t               object_count;
    int32_t               sprite_sheet_count;
    int32_t               height;
    int32_t               width;
    size_t                path_length;
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
    double            decimal_property;
    const char*       string_property;
    int32_t           integer_property;
    bool              boolean_property;
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

#endif // ESZ_TYPES_H

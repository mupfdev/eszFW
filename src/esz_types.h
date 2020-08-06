// SPDX-License-Identifier: MIT
/**
 * @file  esz_types.h
 * @brief eszFW types
 */

#ifndef ESZ_TYPES_H
#define ESZ_TYPES_H

#include <SDL.h>

#ifdef USE_LIBTMX
typedef struct _tmx_layer tmx_layer;
typedef struct _tmx_map   tmx_map;
typedef struct _tmx_obj   tmx_object;
typedef struct _tmx_prop  tmx_property;
typedef struct _tmx_tile  tmx_tile;
typedef struct _tmx_ts    tmx_tileset;

typedef tmx_layer    esz_tiled_layer_t;
typedef tmx_map      esz_tiled_map_t;
typedef tmx_object   esz_tiled_object_t;
typedef tmx_property esz_tiled_property_t;
typedef tmx_tile     esz_tiled_tile_t;
typedef tmx_tileset  esz_tiled_tileset_t;

#include <tmx.h>

#else // (cute_tiled.h)
struct cute_tiled_layer_t;
struct cute_tiled_map_t;
struct cute_tiled_object_t;
struct cute_tiled_property_t;
struct cute_tiled_tile_descriptor_t;
struct cute_tiled_tileset_t;

typedef struct cute_tiled_layer_t           esz_tiled_layer_t;
typedef struct cute_tiled_map_t             esz_tiled_map_t;
typedef struct cute_tiled_object_t          esz_tiled_object_t;
typedef struct cute_tiled_property_t        esz_tiled_property_t;
typedef struct cute_tiled_tile_descriptor_t esz_tiled_tile_t;
typedef struct cute_tiled_tileset_t         esz_tiled_tileset_t;

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
 * @brief An enumeration of entity actions
 */
typedef enum
{
    ACTION_JUMP = 0

} esz_action;

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
 * @brief An enumeration of entity layer levels.
 */
typedef enum
{
    ESZ_ENTITY_LAYER_BG = 0,
    ESZ_ENTITY_LAYER_MG,
    ESZ_ENTITY_LAYER_FG,
    ESZ_ENTITY_LAYER_LEVEL_MAX

} esz_entity_layer_level;

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
 * @brief An enumeration of map layer levels.
 */
typedef enum
{
    ESZ_MAP_LAYER_BG = 0,
    ESZ_MAP_LAYER_FG,
    ESZ_MAP_LAYER_LEVEL_MAX

} esz_map_layer_level;

/**
 * @brief An enumeration of render layer levels.
 */
typedef enum
{
    ESZ_BACKGROUND = 0,
    ESZ_ENTITY_BG,
    ESZ_MAP_BG,
    ESZ_ENTITY_MG,
    ESZ_MAP_FG,
    ESZ_ENTITY_FG,
    ESZ_RENDER_LAYER_MAX

} esz_render_layer;

/**
 * @brief An enumeration of entity states
 */
typedef enum
{
    STATE_ANIMATED,
    STATE_DUCKING,
    STATE_FLOATING,
    STATE_GRAVITATIONAL,
    STATE_GOING_DOWN,
    STATE_GOING_LEFT,
    STATE_GOING_RIGHT,
    STATE_GOING_UP,
    STATE_IN_BACKGROUND,
    STATE_IN_FOREGROUND,
    STATE_IN_MID_AIR,
    STATE_IN_MIDGROUND,
    STATE_JUMPING,
    STATE_LOOKING_LEFT,
    STATE_LOOKING_RIGHT,
    STATE_MOVING,
    STATE_RISING,
    STATE_STANDING

} esz_state;

/**
 * @brief An enumeration of status codes.
 */
typedef enum
{
    ESZ_OK             = 0,
    ESZ_ERROR_CRITICAL = -1,
    ESZ_WARNING        = -2

} esz_status;

/**
 * @brief An enumeration of Tiled layer types.
 */
typedef enum
{
    ESZ_TILE_LAYER = 0,
    ESZ_OBJECT_GROUP

} esz_tiled_layer_type;

typedef enum
{
    TILE_CLIMBABLE,
    TILE_SOLID_ABOVE,
    TILE_SOLID_BELOV,
    TILE_SOLID_LEFT,
    TILE_SOLID_RIGHT

} esz_tile_property;

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
    int32_t dst_x;
    int32_t dst_y;
    int32_t animation_length;
    int32_t current_frame;
    int32_t gid;
    int32_t id;

} esz_animated_tile_t;

/**
 * @brief A structure that contains animation settings.
 */
typedef struct esz_animation
{
    int32_t first_frame;
    int32_t fps;
    int32_t length;
    int32_t offset_y;

} esz_animation_t;

/**
 * @brief A structure that contains a background layer.
 */
typedef struct esz_background_layer
{
    double       pos_x;
    double       pos_y;
    double       velocity;
    SDL_Texture* texture;
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
    double                  velocity;
    double                  layer_shift;
    int32_t                 layer_count;
    bool                    velocity_is_constant;

} esz_background_t;

/**
 * @brief A structure that contains a camera.
 */
typedef struct esz_camera
{
    double  pos_x;
    double  pos_y;
    int32_t max_pos_x;
    int32_t max_pos_y;
    int32_t target_entity_id;
    bool    is_at_horizontal_boundary;
    bool    is_locked;

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
    void (*finger_up_cb)(esz_window_t* window, esz_core_t* core);
    void (*key_down_cb)(esz_window_t* window, esz_core_t* core);
    void (*key_up_cb)(esz_window_t* window, esz_core_t* core);
    void (*map_loaded_cb)(esz_window_t* window, esz_core_t* core);
    void (*map_unloaded_cb)(esz_window_t*  window, esz_core_t* core);
    void (*multi_gesture_cb)(esz_window_t* window, esz_core_t* core);

} esz_event_t;

/**
 * @brief A structure that contains entity information.
 */
typedef struct esz_entity
{
    double           acceleration;
    double           jumping_power;
    double           max_velocity_x;
    double           spawn_pos_x;
    double           spawn_pos_y;
    double           time_since_last_anim_frame;
    double           velocity_x;
    double           velocity_y;
    esz_animation_t* animation;
    int32_t          animation_count;
    int32_t          current_animation;
    int32_t          current_frame;
    int32_t          sprite_sheet_id;
    uint32_t         action;
    uint32_t         state;
    bool             connect_horizontal_map_ends;
    bool             connect_vertical_map_ends;

} esz_entity_t;

/**
 * @brief A structure that contains a map object
 */
typedef struct esz_object
{
    struct esz_aabb     bounding_box;
    double              pos_x;
    double              pos_y;
    esz_entity_t*       entity;
    esz_tiled_object_t* handle;
    int32_t             height;
    int32_t             id;
    int32_t             index;
    int32_t             width;

} esz_object_t;

/**
 * @brief A structure that contains a sprite.
 */
typedef struct esz_sprite
{
    SDL_Texture* texture;
    int32_t      id;

} esz_sprite_t;

/**
 * @brief A structure that contains a game map.
 */
typedef struct esz_map
{
    double                decimal_property;
    double                gravitation;
    double                pos_x;
    double                pos_y;
    double                time_since_last_anim_frame;

    #ifdef USE_LIBTMX
    uint64_t              hash_query;
    #else
    long long unsigned    hash_id_objectgroup;
    long long unsigned    hash_id_tilelayer;
    #endif

    size_t                path_length;
    const char*           string_property;
    char*                 path;
    SDL_Texture*          animated_tile_texture;
    SDL_Texture*          layer_texture[ESZ_MAP_LAYER_LEVEL_MAX];
    SDL_Texture*          render_target[ESZ_RENDER_LAYER_MAX];
    SDL_Texture*          tileset_texture;
    esz_animated_tile_t*  animated_tile;
    struct esz_background background;
    esz_object_t*         object;
    esz_sprite_t*         sprite;
    esz_tiled_map_t*      handle;
    int32_t*              tile_properties;
    int32_t               active_player_entity_id;
    int32_t               animated_tile_fps;
    int32_t               animated_tile_index;
    int32_t               height;
    int32_t               integer_property;
    int32_t               meter_in_pixel;
    int32_t               object_count;
    int32_t               sprite_sheet_count;
    int32_t               width;
    bool                  boolean_property;

} esz_map_t;

/**
 * @brief A structure that contains an engine core.
 */
typedef struct esz_core
{
    struct esz_camera camera;
    struct esz_event  event;
    esz_map_t*        map;
    bool              is_active;
    bool              is_map_loaded;
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
    uint32_t      flags;
    uint32_t      time_a;
    uint32_t      time_b;
    int32_t       height;
    int32_t       logical_height;
    int32_t       logical_width;
    int32_t       pos_x;
    int32_t       pos_y;
    int32_t       refresh_rate;
    int32_t       width;
    bool          is_fullscreen;
    bool          vsync_enabled;

} esz_window_t;

#endif // ESZ_TYPES_H

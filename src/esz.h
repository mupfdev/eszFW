// SPDX-License-Identifier: Beerware
/**
 * @file  esz.h
 * @brief eszFW include header
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

#ifndef ESZ_MAX_PATTERN_LEN
#define ESZ_MAX_PATTERN_LEN 32
#endif

/**
 * @brief Alignment constants
 */
typedef enum
{
    ESZ_BOTTOM = 0,
    ESZ_TOP

} esz_Alignment;

/**
 * @brief Camera flags
 */
typedef enum
{
    CAMERA_IS_LOCKED = 0

} esz_CameraFlag;

/**
 * @brief Layer types
 */
typedef enum
{
    ESZ_BACKGROUND = 0,
    ESZ_FOREGROUND,
    ESZ_LAYER_MAX

} esz_LayerType;

/**
 * @brief Orientational constants
 */
typedef enum
{
    ESZ_LEFT = 0,
    ESZ_RIGHT

} esz_Orientation;

/**
 * @brief Event types
 */
typedef enum
{
    EVENT_CORE_STARTED = 0,
    EVENT_FINGERDOWN,
    EVENT_FINGERMOTION,
    EVENT_FINGERUP,
    EVENT_KEYDOWN,
    EVENT_KEYUP,
    EVENT_MAP_LOADED,
    EVENT_MAP_UNLOADED,
    EVENT_MULTIGESTURE

} esz_EventType;

/**
 * @brief Status codes
 */
typedef enum
{
    ESZ_OK             =  0,
    ESZ_ERROR_CRITICAL = -1,
    ESZ_ERROR_WARNING  = -2

} esz_Status;

/**
 * @struct esz_AABB
 * @brief  Axis-aligned bounding box handle
 */
typedef struct esz_AABB_t
{
    double bottom;
    double left;
    double right;
    double top;

} esz_AABB;

/**
 * @struct esz_AnimatedTile
 * @brief  Animated tile handle
 */
typedef struct esz_AnimatedTile_t
{
    Sint32 dst_x;
    Sint32 dst_y;
    Uint16 gid;
    Uint16 id;
    Uint8  current_frame;
    Uint8  animation_length;

} esz_AnimatedTile;

/**
 * @struct esz_Camera
 * @brief  Camera handle
 */
typedef struct esz_Camera_t
{
    double pos_x;
    double pos_y;
    Sint32 max_pos_x;
    Sint32 max_pos_y;
    Uint16 flags;
    Uint16 target_entity_id;

} esz_Camera;

/**
 * @brief Initial engine configuration
 */
typedef struct esz_Config_t
{
    const Sint32   window_width;
    const Sint32   window_height;
    const Sint32   logical_window_width;
    const Sint32   logical_window_height;
    const SDL_bool enable_fullscreen;

} esz_Config;

/**
 * @struct esz_Event
 * @brief  Event handle
 */
typedef struct esz_Event_t
{
    SDL_Event event_handle;

    void (*core_started_cb)(void* core);
    void (*finger_down_cb)(void* core);
    void (*finger_motion_cb)(void* core);
    void (*finger_up_cb)(void* core);
    void (*key_down_cb)(void* core);
    void (*key_up_cb)(void* core);
    void (*map_loaded_cb)(void* core);
    void (*map_unloaded_cb)(void* core);
    void (*multi_gesture_cb)(void* core);

} esz_Event;

/**
 * @typedef esz_Object
 * @brief   Object handle
 */
typedef struct esz_Object_t
{
    esz_AABB    bounding_box;
    tmx_object* tmx_object;
    Uint32      pos_x;
    Uint32      pos_y;
    Uint16      id;
    Uint16      width;
    Uint16      height;

} esz_Object;

/**
 * @struct esz_Sprite
 * @brief  Sprite handle
 */
typedef struct esz_Sprite_t
{
    SDL_Texture* texture;
    Uint16       id;

} esz_Sprite;

/**
 * @struct esz_Map
 * @brief  Map handle
 */
typedef struct esz_Map_t
{
    char              resource_path[ESZ_MAX_PATH_LEN];
    char              tileset_image[ESZ_MAX_PATH_LEN];
    tmx_map*          tmx_map;
    SDL_Texture*      map_layer[ESZ_LAYER_MAX];
    SDL_Texture*      animated_tile_texture;
    SDL_Texture*      tileset_texture;
    esz_AnimatedTile* animated_tile;
    esz_Object*       object;
    esz_Sprite*       sprite;
    double            gravitation;
    double            animation_delay;
    double            pos_x;
    double            pos_y;
    size_t            resource_path_length;
    Uint16            animated_tile_count;
    Uint16            object_count;
    Uint16            width;
    Uint16            height;
    Uint8             animated_tile_fps;
    Uint8             meter_in_pixel;
    Uint8             sprite_sheet_count;
    SDL_bool          is_loaded;

} esz_Map;

/**
 * @struct esz_Core
 * @brief  eszFW engine core data
 */
typedef struct esz_Core_t
{
    double        zoom_level;
    double        initial_zoom_level;
    double        decimal_property;
    double        time_between_frames;
    char          search_pattern[ESZ_MAX_PATTERN_LEN];
    char*         string_property;
    SDL_Renderer* renderer;
    SDL_Window*   window;
    Uint32        window_flags;
    Sint32        window_width;
    Sint32        window_height;
    Sint32        window_pos_x;
    Sint32        window_pos_y;
    Sint32        refresh_rate;
    Sint32        logical_window_width;
    Sint32        logical_window_height;
    Sint32        integer_property;
    esz_Camera    camera;
    esz_Event     event;
    esz_Map       map;
    SDL_bool      is_fullscreen;
    SDL_bool      is_paused;
    SDL_bool      is_running;
    SDL_bool      boolean_property;

} esz_Core;

SDL_bool   esz_BoundingBoxesDoIntersect(const esz_AABB bb_a, const esz_AABB bb_b);
Uint32     esz_GetKeycode(esz_Core* core);
void       esz_ExitCore(esz_Core* core);
esz_Status esz_InitCore(esz_Config* config, esz_Core** core);
void       esz_LoadMap(const char* map_file_name, esz_Core* core);
void       esz_LockCamera(esz_Core* core);
void       esz_RegisterEventCallback(const esz_EventType event_type, void (*callback)(void* core), esz_Core* core);
esz_Status esz_SetZoomLevel(const double factor, esz_Core* core);
esz_Status esz_StartCore(const char* window_title, esz_Core* core);
esz_Status esz_ToggleFullscreen(esz_Core* core);
void       esz_UnlockCamera(esz_Core* core);
void       esz_UnloadMap(esz_Core* core);

#endif // ESZ_H

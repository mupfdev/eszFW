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
    EVENT_FINGERDOWN = 0,
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
 * @brief Initial window configuration
 */
typedef struct esz_WindowConfig_t
{
    const Sint32   width;
    const Sint32   height;
    const Sint32   logical_width;
    const Sint32   logical_height;
    const SDL_bool enable_fullscreen;

} esz_WindowConfig;

/**
 * @struct esz_Event
 * @brief  Event handle
 */
typedef struct esz_Event_t
{
    SDL_Event event_handle;

    void (*finger_down_cb)(void* window, void* core);
    void (*finger_motion_cb)(void* window, void* core);
    void (*finger_up_cb)(void* window, void* core);
    void (*key_down_cb)(void* window, void* core);
    void (*key_up_cb)(void* window, void* core);
    void (*map_loaded_cb)(void* window, void* core);
    void (*map_unloaded_cb)(void* window, void* core);
    void (*multi_gesture_cb)(void* window, void* core);

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
    Uint16      height;
    Uint16      id;
    Uint16      width;

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
    char              search_pattern[ESZ_MAX_PATTERN_LEN];
    char              tileset_image[ESZ_MAX_PATH_LEN];
    double            decimal_property;
    double            gravitation;
    double            pos_x;
    double            pos_y;
    double            time_since_last_anim_frame;
    SDL_Texture*      animated_tile_texture;
    SDL_Texture*      map_layer[ESZ_LAYER_MAX];
    //SDL_Texture*      texture;
    SDL_Texture*      tileset_texture;
    char*             string_property;
    esz_AnimatedTile* animated_tile;
    esz_Object*       object;
    esz_Sprite*       sprite;
    tmx_map*          tmx_map;
    Sint32            integer_property;
    size_t            resource_path_length;
    Uint16            animated_tile_count;
    Uint16            height;
    Uint16            object_count;
    Uint16            width;
    Uint8             animated_tile_fps;
    Uint8             meter_in_pixel;
    Uint8             sprite_sheet_count;
    SDL_bool          boolean_property;
    SDL_bool          is_loaded;

} esz_Map;

/**
 * @struct esz_Core
 * @brief  eszFW engine core data
 */
typedef struct esz_Core_t
{
    esz_Camera camera;
    esz_Event  event;
    esz_Map    map;
    SDL_bool   is_active;
    SDL_bool   is_paused;

} esz_Core;

/**
 * @struct esz_Window
 * @brief  Window and rendering context handle
 */
typedef struct esz_Window_t
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

} esz_Window;

SDL_bool   esz_BoundingBoxesDoIntersect(const esz_AABB bb_a, const esz_AABB bb_b);
esz_Status esz_CreateWindow(const char* window_title, esz_WindowConfig* config, esz_Window** window);
void       esz_DeactivateCore(esz_Core* core);
void       esz_DestroyCore(esz_Core* core);
void       esz_DestroyWindow(esz_Window* window);
void       esz_DrawFrame(Uint32* time_a, Uint32* time_b, esz_Window*, esz_Core* core);
Uint32     esz_GetKeycode(esz_Core* core);
double     esz_GetTimeSinceLastFrame(esz_Window* core);
esz_Status esz_InitCore(esz_Core** core);
SDL_bool   esz_IsCoreActive(esz_Core* core);
void       esz_LoadMap(const char* map_file_name, esz_Window* window, esz_Core* core);
void       esz_LockCamera(esz_Core* core);
void       esz_RegisterEventCallback(const esz_EventType event_type, void (*callback)(void* window, void* core), esz_Core* core);
esz_Status esz_SetZoomLevel(const double factor, esz_Window* window);
esz_Status esz_ToggleFullscreen(esz_Window* window);
void       esz_UnlockCamera(esz_Core* core);
void       esz_UnloadMap(esz_Window* window, esz_Core* core);
void       esz_UpdateCore(esz_Window* window, esz_Core* core);

#endif // ESZ_H

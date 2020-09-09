// SPDX-License-Identifier: MIT
/**
 * @file    esz_compat.c
 * @brief   eszFW compatibility layer
 */

#include <picolog.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "esz_compat.h"
#include "esz_hash.h"
#include "esz_macros.h"
#include "esz_types.h"

DISABLE_WARNING_PUSH
DISABLE_WARNING_PADDING
DISABLE_WARNING_SPECTRE_MITIGATION

#include <stb_sprintf.h>

#ifdef USE_LIBTMX
    #include <tmx.h>
#else // (cute_tiled.h)
    #define CUTE_TILED_IMPLEMENTATION
    #include <cute_tiled.h>
#endif

DISABLE_WARNING_SYMBOL_NOT_DEFINED

#include <SDL.h>
#include <cwalk.h>

DISABLE_WARNING_POP

#ifdef USE_LIBTMX
static void tmxlib_store_property(esz_tiled_property_t* property, void* core);
#endif

int32_t get_first_gid(esz_tiled_map_t* tiled_map)
{
    #ifdef USE_LIBTMX
    return (int32_t)tiled_map->ts_head->firstgid;

    #else // (cute_tiled.h)
    return tiled_map->tilesets->firstgid;

    #endif
}

esz_tiled_layer_t* get_head_layer(esz_tiled_map_t* tiled_map)
{
    #ifdef USE_LIBTMX
    return tiled_map->ly_head;

    #else // (cute_tiled.h)
    return tiled_map->layers;

    #endif
}

esz_tiled_object_t* get_head_object(esz_tiled_layer_t* tiled_layer, esz_core_t* core)
{
    if (is_tiled_layer_of_type(ESZ_OBJECT_GROUP, tiled_layer, core))
    {
        #ifdef USE_LIBTMX
        return tiled_layer->content.objgr->head;

        #else // (cute_tiled.h)
        return tiled_layer->objects;

        #endif
    }

    return NULL;
}

esz_tiled_tileset_t* get_head_tileset(esz_tiled_map_t* tiled_map)
{
    #ifdef USE_LIBTMX
    int32_t first_gid = get_first_gid(tiled_map);
    return tiled_map->tiles[first_gid]->tileset;

    #else // (cute_tiled.h)
    return tiled_map->tilesets;

    #endif
}

int32_t* get_layer_content(esz_tiled_layer_t* tiled_layer)
{
    #ifdef USE_LIBTMX
    return (int32_t*)tiled_layer->content.gids;

    #else // (cute_tiled.h)
    return tiled_layer->data;

    #endif
}

const char* get_layer_name(esz_tiled_layer_t* tiled_layer)
{
    #ifdef USE_LIBTMX
    return tiled_layer->name;

    #else // (cute_tiled.h)
    return tiled_layer->name.ptr;

    #endif
}

int32_t get_layer_property_count(esz_tiled_layer_t* tiled_layer)
{
    #ifdef USE_LIBTMX
    (void)tiled_layer;
    return 0;

    #else // (cute_tiled.h)
    return tiled_layer->property_count;

    #endif
}

int32_t get_local_id(int32_t gid, esz_tiled_map_t* tiled_map)
{
    #ifdef USE_LIBTMX
    (void)tiled_map;
    return gid;

    #else // (cute_tiled.h)
    int32_t local_id = gid - get_first_gid(tiled_map);

    return local_id >= 0 ? local_id : 0;

    #endif
}

int32_t get_map_property_count(esz_tiled_map_t* tiled_map)
{
    #ifdef USE_LIBTMX
    (void)tiled_map;
    return 0;

    #else // (cute_tiled.h)
    return tiled_map->property_count;

    #endif
}

int32_t get_next_animated_tile_id(int32_t gid, int32_t current_frame, esz_tiled_map_t* tiled_map)
{
    #ifdef USE_LIBTMX
    return (int32_t)tiled_map->tiles[gid]->animation[current_frame].tile_id;

    #else // (cute_tiled.h)
    esz_tiled_tileset_t*          tileset = get_head_tileset(tiled_map);
    cute_tiled_tile_descriptor_t* tile    = tileset->tiles;
    while (tile)
    {
        if (tile->tile_index == gid)
        {
            return tile->animation[current_frame].tileid;
        }
        tile = tile->next;
    }

    #endif

    return 0;
}

const char* get_object_name(esz_tiled_object_t* tiled_object)
{
    #ifdef USE_LIBTMX
    return tiled_object->name;

    #else // (cute_tiled.h)
    return tiled_object->name.ptr;

    #endif
}

int32_t get_object_property_count(esz_tiled_object_t* tiled_object)
{
    #ifdef USE_LIBTMX
    (void)tiled_object;
    return 0;

    #else // (cute_tiled.h)
    return tiled_object->property_count;

    #endif
}

const char* get_object_type_name(esz_tiled_object_t* tiled_object)
{
    #ifdef USE_LIBTMX
    return tiled_object->type;

    #else // (cute_tiled.h)
    return tiled_object->type.ptr;

    #endif
}

int32_t get_tile_height(esz_tiled_map_t* tiled_map)
{
    #ifdef USE_LIBTMX
    int32_t first_gid = get_first_gid(tiled_map);
    return (int32_t)tiled_map->tiles[first_gid]->tileset->tile_height;

    #else // (cute_tiled.h)
    return tiled_map->tilesets->tileheight;

    #endif
}

void get_tile_position(int32_t gid, int32_t* pos_x, int32_t* pos_y, esz_tiled_map_t* tiled_map)
{
    #ifdef USE_LIBTMX
    *pos_x = (int32_t)tiled_map->tiles[gid]->ul_x;
    *pos_y = (int32_t)tiled_map->tiles[gid]->ul_y;

    #else // (cute_tiled.h)
    esz_tiled_tileset_t* tileset  = tiled_map->tilesets;
    int32_t              local_id = get_local_id(gid, tiled_map);

    *pos_x = (local_id % tileset->columns) * get_tile_width(tiled_map);
    *pos_y = (local_id / tileset->columns) * get_tile_height(tiled_map);

    #endif
}

int32_t get_tile_property_count(esz_tiled_tile_t* tiled_tile)
{
    #ifdef USE_LIBTMX
    (void)tiled_tile;
    return 0;

    #else // (cute_tiled.h)
    return tiled_tile->property_count;

    #endif
}

int32_t get_tile_width(esz_tiled_map_t* tiled_map)
{
    #ifdef USE_LIBTMX
    int32_t first_gid = get_first_gid(tiled_map);
    return (int32_t)tiled_map->tiles[first_gid]->tileset->tile_width;

    #else // (cute_tiled.h)
    return tiled_map->tilesets->tilewidth;

    #endif
}

void set_tileset_path(char* path_name, int32_t path_length, esz_core_t* core)
{
    #ifdef USE_LIBTMX
    int32_t first_gid      = get_first_gid(core->map->handle);
    char    ts_path[64]    = { 0 };
    size_t  ts_path_length = 0;

    cwk_path_get_dirname(core->map->handle->ts_head->source, &ts_path_length);

    if (63 <= ts_path_length)
    {
        ts_path_length = 63;
    }

    /* The tileset image source is stored relatively to the tileset
     * file but because we only know the location of the tileset
     * file relatively to the map file, we need to adjust the path
     * accordingly.  It's a hack, but it works.
     */

    SDL_strlcpy(ts_path, core->map->handle->ts_head->source, ts_path_length + 1);
    stbsp_snprintf(path_name, (int32_t)path_length, "%s%s%s",
        core->map->path,
        ts_path,
        core->map->handle->tiles[first_gid]->tileset->image->source);

    #else // (cute_tiled.h)
    stbsp_snprintf(path_name, (int32_t)path_length, "%s%s",
        core->map->path,
        core->map->handle->tilesets->image.ptr);

    #endif
}

int32_t get_tileset_path_length(esz_core_t* core)
{
    int32_t path_length = 0;

    #ifdef USE_LIBTMX
    int32_t first_gid      = get_first_gid(core->map->handle);
    size_t  ts_path_length = 0;

    cwk_path_get_dirname(core->map->handle->ts_head->source, &ts_path_length);

    path_length += (int32_t)strnlen(core->map->path, 64);
    path_length += strnlen(core->map->handle->tiles[first_gid]->tileset->image->source, 64);
    path_length += (int32_t)ts_path_length + 1;

    #else // (cute_tiled.h)
    if (! core->map->handle->tilesets)
    {
        plog_error("%s: no embedded tileset found.", __func__);
        return 0;
    }

    path_length += (int32_t)strnlen(core->map->path, 64);
    path_length += (int32_t)strnlen(core->map->handle->tilesets->image.ptr, 64);
    path_length += 1;

    #endif

    return path_length;
}

bool is_gid_valid(int32_t gid, esz_tiled_map_t* tiled_map)
{
    #ifdef USE_LIBTMX
    if (tiled_map->tiles[gid])
    {
        return true;
    }
    #else // (cute_tiled.h)
    (void)tiled_map;

    if (gid)
    {
        return true;
    }

    #endif
    return false;
}

bool is_tile_animated(int32_t gid, int32_t* animation_length, int32_t* id, esz_tiled_map_t* tiled_map)
{
    int32_t local_id = get_local_id(gid, tiled_map);

    #ifdef USE_LIBTMX
    if (tiled_map->tiles[local_id])
    {
        if (tiled_map->tiles[local_id]->animation)
        {
            if (animation_length)
            {
                *animation_length = (int32_t)tiled_map->tiles[local_id]->animation_len;
            }
            if (id)
            {
                *id = (int32_t)tiled_map->tiles[local_id]->animation[0].tile_id;
            }
            return true;
        }
    }

    #else // (cute_tiled.h)
    esz_tiled_tileset_t* tileset = tiled_map->tilesets;
    esz_tiled_tile_t*    tile    = tileset->tiles;

    while (tile)
    {
        if (tile->tile_index == local_id)
        {
            if (tile->animation)
            {
                if (animation_length)
                {
                    *animation_length = tile->frame_count;
                }
                if (id)
                {
                    *id = tile->animation->tileid;
                }
                return true;
            }
        }
        tile = tile->next;
    }

    #endif

    return false;
}

bool is_tiled_layer_of_type(const esz_tiled_layer_type tiled_type, esz_tiled_layer_t* tiled_layer, esz_core_t* core)
{
    switch (tiled_type)
    {
        case ESZ_TILE_LAYER:
            #ifdef USE_LIBTMX
            (void)core;

            if (L_LAYER == tiled_layer->type)
            {
                return true;
            }

            #else // (cute_tiled.h)
            if (core->map->hash_id_tilelayer == tiled_layer->type.hash_id)
            {
                return true;
            }

            #endif
            break;
        case ESZ_OBJECT_GROUP:
            #ifdef USE_LIBTMX
            if (L_OBJGR == tiled_layer->type)
            {
                return true;
            }

            #else // (cute_tiled.h)
            if (core->map->hash_id_objectgroup == tiled_layer->type.hash_id)
            {
                return true;
            }

            #endif
            break;
    }

    return false;
}

void load_property(const uint64_t name_hash, esz_tiled_property_t* properties, int32_t property_count, esz_core_t* core)
{
    #ifdef USE_LIBTMX
    (void)property_count;
    core->map->hash_query = name_hash;
    tmx_property_foreach(properties, tmxlib_store_property, (void*)core);

    #else // (cute_tiled.h)
    int index = 0;

    for (index = 0; index < property_count; index += 1)
    {
        if (name_hash == generate_hash((const unsigned char*)properties[index].name.ptr))
        {
            break;
        }
    }

    if (properties[index].name.ptr)
    {
        switch (properties[index].type)
        {
            case CUTE_TILED_PROPERTY_COLOR:
            case CUTE_TILED_PROPERTY_FILE:
            case CUTE_TILED_PROPERTY_NONE:
                break;
            case CUTE_TILED_PROPERTY_INT:
                plog_debug("Loading integer property '%s': %d", properties[index].name.ptr, properties[index].data.integer);

                core->map->integer_property = properties[index].data.integer;
                break;
            case CUTE_TILED_PROPERTY_BOOL:
                plog_debug("Loading boolean property '%s': %u", properties[index].name.ptr, properties[index].data.boolean);

                core->map->boolean_property = (bool)properties[index].data.boolean;
                break;
            case CUTE_TILED_PROPERTY_FLOAT:
                plog_debug("Loading decimal property '%s': %f", properties[index].name.ptr, (double)properties[index].data.floating);

                core->map->decimal_property = (double)properties[index].data.floating;
                break;
            case CUTE_TILED_PROPERTY_STRING:
                plog_debug("Loading string property '%s': %s", properties[index].name.ptr, properties[index].data.string.ptr);

                core->map->string_property  = properties[index].data.string.ptr;
                break;
        }
    }
    #endif
}

esz_status load_tiled_map(const char* map_file_name, esz_core_t* core)
{
    FILE* fp = fopen(map_file_name, "r");

    if (fp)
    {
        fclose(fp);
    }
    else
    {
        plog_error("%s: %s not found.", __func__, map_file_name);
        return ESZ_WARNING;
    }

    #ifdef USE_LIBTMX
    core->map->handle = (esz_tiled_map_t*)tmx_load(map_file_name);
    if (! core->map->handle)
    {
        plog_error("%s: %s.", __func__, tmx_strerr());
        return ESZ_WARNING;
    }

    #else // (cute_tiled.h)
    esz_tiled_layer_t* layer;

    core->map->handle = (esz_tiled_map_t*)cute_tiled_load_map_from_file(map_file_name, NULL);
    if (! core->map->handle)
    {
        plog_error("%s: %s.", __func__, cute_tiled_error_reason);
        return ESZ_WARNING;
    }

    layer = get_head_layer(core->map->handle);
    while (layer)
    {
        if (H_tilelayer == generate_hash((const unsigned char*)layer->type.ptr) && !core->map->hash_id_tilelayer)
        {
            core->map->hash_id_tilelayer = layer->type.hash_id;
            plog_info("Set hash ID for tile layer: %llu", core->map->hash_id_tilelayer);
        }
        else if (H_objectgroup == generate_hash((const unsigned char*)layer->type.ptr) && !core->map->hash_id_objectgroup)
        {
            core->map->hash_id_objectgroup = layer->type.hash_id;
            plog_info("Set hash ID for object group: %llu", core->map->hash_id_objectgroup);
        }
        layer = layer->next;
    }

    #endif

    return ESZ_OK;
}

int32_t remove_gid_flip_bits(int32_t gid)
{
    #ifdef USE_LIBTMX
    return gid & TMX_FLIP_BITS_REMOVAL;

    #else // (cute_tiled.h)
    return cute_tiled_unset_flags(gid);

    #endif
}

bool tile_has_properties(int32_t gid, esz_tiled_tile_t** tile, esz_tiled_map_t* tiled_map)
{
    int32_t local_id;

    #ifdef USE_LIBTMX
    return true;

    #else // (cute_tiled.h)
    local_id = gid - get_first_gid(tiled_map);

    while ((*tile))
    {
        if ((*tile)->tile_index == local_id)
        {
            if (0 < (*tile)->property_count)
            {
                return true;
            }
        }
        (*tile) = (*tile)->next;
    }

    #endif

    return false;
}

void unload_tiled_map(esz_core_t* core)
{
    #ifdef USE_LIBTMX
    if (core->map->handle)
    {
        tmx_map_free(core->map->handle);
    }

    #else // (cute_tiled.h)
    core->map->hash_id_objectgroup = 0;
    core->map->hash_id_tilelayer   = 0;

    if (core->map->handle)
    {
        cute_tiled_free_map(core->map->handle);
    }
    #endif
}

#ifdef USE_LIBTMX
static void tmxlib_store_property(esz_tiled_property_t* property, void* core)
{
    esz_core_t* core_ptr = core;

    if (core_ptr->map->hash_query == generate_hash((const unsigned char*)property->name))
    {
        switch (property->type)
        {
            case PT_COLOR:
            case PT_NONE:
                break;
            case PT_BOOL:
                plog_debug("Loading boolean property '%s': %u", property->name, property->value.boolean);

                core_ptr->map->boolean_property = (bool)property->value.boolean;
                break;
            case PT_FILE:
                plog_debug("Loading string property '%s': %s", property->name, property->value.file);

                core_ptr->map->string_property  = property->value.file;
                break;
            case PT_FLOAT:
                plog_debug("Loading decimal property '%s': %f", property->name, (double)property->value.decimal);

                core_ptr->map->decimal_property = (double)property->value.decimal;
                break;
            case PT_INT:
                plog_debug("Loading integer property '%s': %d", property->name, property->value.integer);

                core_ptr->map->integer_property = property->value.integer;
                break;
            case PT_STRING:
                plog_debug("Loading string property '%s': %s", property->name, property->value.string);

                core_ptr->map->string_property  = property->value.string;
                break;
        }
    }
}
#endif

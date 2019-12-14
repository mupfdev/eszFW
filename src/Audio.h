// SPDX-License-Identifier: Beerware
/**
 * @file    Audio.h
 * @brief   Audio handler include header
 * @ingroup Audio
 */
#pragma once

#include <SDL.h>
#include <SDL_mixer.h>

/**
 * @typedef Audio
 * @brief   Audio mixer handle type
 * @struct  Audio_t
 * @brief   Audio mixer handle data
 */
typedef struct Audio_t
{
    Sint32 s32SamplingFrequency;  ///< Sampling frequency
    Uint16 u16AudioFormat;        ///< Audio format
    Sint16 s16Channels;           ///< Channels
    Sint16 s16ChunkSize;          ///< Chunk size

} Audio;

/**
 * @typedef Music
 * @brief   Music handle type
 * @struct  Music_t
 * @brief   Music handle data
 */
typedef struct Music_t
{
    Mix_Music* pstMusic;  ///< SDL2 music handle
    Sint8      s8Loops;   ///< Loop count

} Music;

void  Audio_Free(Audio* pstAudio);
void  Audio_FreeMusic(Music* pstMusic);
Sint8 Audio_Init(Audio** pstAudio);
Sint8 Audio_InitMusic(const char* pacFileName, const Sint8 s8Loops, Music** pstMusic);
Sint8 Audio_PlayMusic(const Uint16 u16FadeInMs, const Music* pstMusic);

/**
 * @file Audio.h
 * @ingroup Audio
 */

#pragma once
#include <SDL.h>
#include <SDL_mixer.h>

typedef struct Audio_t {
    Sint32 s32SamplingFrequency;
    Uint16 u16AudioFormat;
    Sint16 s16Channels;
    Sint16 s16ChunkSize;
} Audio;

typedef struct Music_t
{
    Mix_Music *pstMusic;
    Sint8      s8Loops;
} Music;

void FreeAudio(Audio *pstAudio);
void FreeMusic(Music *pstMusic);
Sint8 InitAudio(Audio **pstAudio);
Sint8 InitMusic(const char *pacFileName, const Sint8 s8Loops, Music **pstMusic);
Sint8 PlayMusic(const Uint16 u16FadeInMs, const Music *pstMusic);

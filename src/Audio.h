/**
 * @file Audio.h
 * @ingroup Audio
 */

#ifndef _AUDIO_H_
#define _AUDIO_H_

#include <SDL_mixer.h>

typedef struct Audio_t {
    int      sSamplingFrequency;
    uint16_t u16AudioFormat;
    int      sChannels;
    int      sChunkSize;
} Audio;

typedef struct Music_t
{
    Mix_Music *pstMusic;
    int8_t     s8Loops;
} Music;

void FreeAudio(Audio *pstAudio);
void FreeMusic(Music *pstMusic);
int  InitAudio(Audio **pstAudio);
int  InitMusic(const char *pacFileName, const int8_t s8Loops, Music **pstMusic);
int  PlayMusic(const uint16_t u16FadeInMs, Music **pstMusic);

#endif // _AUDIO_H_

/**
 * @file Audio.c
 * @ingroup Audio
 * @defgroup Audio
 * @author Michael Fitzmayer
 * @copyright "THE BEER-WARE LICENCE" (Revision 42)
 */

#ifdef __ANDROID__
#include <SDL.h>
#include <SDL_mixer.h>
#else
#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>
#endif
#include <stdint.h>
#include "Audio.h"

void FreeAudio(Audio **pstAudio)
{
    Mix_CloseAudio();
    while(Mix_Init(0)) Mix_Quit();
    free(*pstAudio);
}

void FreeMusic(Music **pstMusic)
{
    Mix_FreeMusic((*pstMusic)->pstMusic);
    free(*pstMusic);
    SDL_Log("Unload music track.\n");
}

int InitAudio(Audio **pstAudio)
{
    *pstAudio = NULL;
    *pstAudio = malloc(sizeof(struct Audio_t));
    if (! *pstAudio)
    {
        SDL_LogError(
            SDL_LOG_CATEGORY_APPLICATION,
            "InitAudio(): error allocating memory.\n");
        return -1;
    }

    if (-1 == SDL_Init(SDL_INIT_AUDIO))
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "%s\n", SDL_GetError());
        return -1;
    }

    (*pstAudio)->sSamplingFrequency = 44100;
    (*pstAudio)->u16AudioFormat     = MIX_DEFAULT_FORMAT;
    (*pstAudio)->sChannels          = 2;
    (*pstAudio)->sChunkSize         = 4096;

    if (-1 == Mix_OpenAudio(
            (*pstAudio)->sSamplingFrequency,
            (*pstAudio)->u16AudioFormat,
            (*pstAudio)->sChannels,
            (*pstAudio)->sChunkSize))
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "%s\n", Mix_GetError());
        return -1;
    }

    Mix_AllocateChannels(16);

    SDL_Log("Initialise audio mixer.\n");
    return 0;
}

int InitMusic(const char *pacFileName, const int8_t s8Loops, Music **pstMusic)
{
    *pstMusic = NULL;
    *pstMusic = malloc(sizeof(struct Music_t));
    if (! pstMusic)
    {
        SDL_LogError(
            SDL_LOG_CATEGORY_APPLICATION,
            "InitMusic(): error allocating memory.\n");
        return -1;
    }

    (*pstMusic)->pstMusic = Mix_LoadMUS(pacFileName);
    (*pstMusic)->s8Loops  = s8Loops;

    if (! (*pstMusic)->pstMusic)
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "%s\n", Mix_GetError());
        return -1;
    }

    SDL_Log("Load music file: %s.\n", pacFileName);
    return 0;
}

int PlayMusic(const uint16_t u16FadeInMs, Music **pstMusic)
{
    if (0 != u16FadeInMs)
    {
        if (-1 == Mix_FadeInMusic((*pstMusic)->pstMusic, (*pstMusic)->s8Loops, u16FadeInMs))
        {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "%s\n", Mix_GetError());
            return -1;
        }

        SDL_Log("Fade in music (%d ms).\n", u16FadeInMs);
    }
    else
    {
        if (-1 == Mix_PlayMusic((*pstMusic)->pstMusic, (*pstMusic)->s8Loops))
        {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "%s\n", Mix_GetError());
            return -1;
        }

        SDL_Log("Play music.\n");
    }

    return 0;
}

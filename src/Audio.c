/**
 * @file      Audio.c
 * @brief     Audio handler source
 * @ingroup   Audio
 * @defgroup  Audio Audio/Music/SFX handler
 * @author    Michael Fitzmayer
 * @copyright "THE BEER-WARE LICENCE" (Revision 42)
 */

#include <SDL.h>
#include <SDL_mixer.h>
#include "Audio.h"

/**
 * @brief   Free audio mixer
 * @details Frees up allocated memory and de-initialises audio mixer
 * @param   pstAudio
 *          Pointer to audio mixer handle
 */
void Audio_Free(Audio* pstAudio)
{
    Mix_CloseAudio();
    while (Mix_Init(0))
    {
        Mix_Quit();
    }

    SDL_free(pstAudio);
}

/**
 * @brief   Free/Unload music file
 * @details Frees up allocated memory and unloads music file
 * @param   pstMusic
 *          Pointer to loaded music file handle
 */
void Audio_FreeMusic(Music* pstMusic)
{
    if (pstMusic)
    {
        if (pstMusic->pstMusic)
        {
            Mix_FreeMusic(pstMusic->pstMusic);
        }

        SDL_free(pstMusic);
        SDL_Log("Unload music track.\n");
    }
}

/**
 * @brief   Initialise audio mixer
 * @details Initialises audio mixer required to play any sound
 *          whatsoever
 * @param   pstAudio
 *          Pointer to audio mixer handle
 * @return  Error code
 * @retval   0: OK
 * @brief   -1: Error
 */
Sint8 Audio_Init(Audio** pstAudio)
{
    *pstAudio = SDL_calloc(sizeof(struct Audio_t), sizeof(Sint8));
    if (!*pstAudio)
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "InitAudio(): error allocating memory.\n");
        return -1;
    }

    if (-1 == SDL_Init(SDL_INIT_AUDIO))
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "%s\n", SDL_GetError());
        return -1;
    }

    (*pstAudio)->s32SamplingFrequency = 44100;
    (*pstAudio)->u16AudioFormat       = MIX_DEFAULT_FORMAT;
    (*pstAudio)->s16Channels          = 2;
    (*pstAudio)->s16ChunkSize         = 4096;

    if (-1 == Mix_OpenAudio(
            (*pstAudio)->s32SamplingFrequency,
            (*pstAudio)->u16AudioFormat,
            (*pstAudio)->s16Channels,
            (*pstAudio)->s16ChunkSize))
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "%s\n", Mix_GetError());
        return -1;
    }

    Mix_AllocateChannels(16);

    SDL_Log("Initialise audio mixer.\n");
    return 0;
}

/**
 * @brief   Initialise music file
 * @details Loads music file into memory
 * @param   pacFileName
 *          Path to music file
 * @param   s8Loops
 *          Loops to play, -1 to loop endlessly
 * @param   pstMusic
 *          Pointer to music file handle
 * @return  Error code
 * @retval   0: OK
 * @retval  -1: Error
 */
Sint8 Audio_InitMusic(const char* pacFileName, const Sint8 s8Loops, Music** pstMusic)
{
    *pstMusic = SDL_calloc(sizeof(struct Music_t), sizeof(Sint8));
    if (!*pstMusic)
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "InitMusic(): error allocating memory.\n");
        return -1;
    }

    (*pstMusic)->pstMusic = Mix_LoadMUS(pacFileName);
    (*pstMusic)->s8Loops  = s8Loops;

    if (!(*pstMusic)->pstMusic)
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "%s\n", Mix_GetError());
        return -1;
    }

    SDL_Log("Load music file: %s.\n", pacFileName);
    return 0;
}

/**
 * @brief   Play music file
 * @details Plays previously loaded music file
 * @param   u16FadeInMs
 *          Fade-in time in milliseconds
 * @param   pstMusic
 *          Pointer to music file handle
 * @return  Error code
 * @retval   0: OK
 * @retval  -1: Error
 */
Sint8 Audio_PlayMusic(const Uint16 u16FadeInMs, const Music* pstMusic)
{
    if (0 != u16FadeInMs)
    {
        if (-1 == Mix_FadeInMusic(pstMusic->pstMusic, pstMusic->s8Loops, u16FadeInMs))
        {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "%s\n", Mix_GetError());
            return -1;
        }

        SDL_Log("Fade in music (%d ms).\n", u16FadeInMs);
    }
    else
    {
        if (-1 == Mix_PlayMusic(pstMusic->pstMusic, pstMusic->s8Loops))
        {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "%s\n", Mix_GetError());
            return -1;
        }

        SDL_Log("Play music.\n");
    }

    return 0;
}

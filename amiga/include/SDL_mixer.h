/* SDL_mixer.h: the part of SDL_mixer 1.2's interface F-1 Spirit uses, for
 * GFOpenGPU's SDL 1.2 (sdl12-compat), which has no SDL_mixer of its own.
 * amiga/mixer.c mixes the channels and the music into SDL's audio: WAV
 * through SDL_LoadWAV, Ogg Vorbis through stb_vorbis (effects decoded on
 * load, music streamed). Chunks are kept in the output format (signed
 * 16-bit stereo), as SDL_mixer's are: the car engine's effect reads them.
 * Copyright (C) 2026 AmiCradle Team. GPL 2 or later, like F-1 Spirit. */
#ifndef AMIGA_SDL_MIXER_H
#define AMIGA_SDL_MIXER_H

#include "SDL.h"

#ifdef __cplusplus
extern "C" {
#endif

#define SDL_MIXER_MAJOR_VERSION 1
#define SDL_MIXER_MINOR_VERSION 2
#define SDL_MIXER_PATCHLEVEL    12
#define MIX_VERSION(X) { (X)->major = SDL_MIXER_MAJOR_VERSION; \
	(X)->minor = SDL_MIXER_MINOR_VERSION; (X)->patch = SDL_MIXER_PATCHLEVEL; }

#define MIX_CHANNELS          8
#define MIX_DEFAULT_FREQUENCY 22050
#define MIX_DEFAULT_FORMAT    AUDIO_S16SYS
#define MIX_DEFAULT_CHANNELS  2
#define MIX_MAX_VOLUME        128

typedef struct Mix_Chunk {
	int allocated;
	Uint8 *abuf;
	Uint32 alen;
	Uint8 volume;
} Mix_Chunk;

typedef struct _Mix_Music Mix_Music;

typedef void (*Mix_EffectFunc_t)(int chan, void *stream, int len, void *udata);
typedef void (*Mix_EffectDone_t)(int chan, void *udata);

const SDL_version *Mix_Linked_Version(void);
int Mix_OpenAudio(int frequency, Uint16 format, int channels, int chunksize);
void Mix_CloseAudio(void);
int Mix_QuerySpec(int *frequency, Uint16 *format, int *channels);
int Mix_AllocateChannels(int numchans);
int Mix_ReserveChannels(int num);

Mix_Chunk *Mix_LoadWAV(const char *file);
void Mix_FreeChunk(Mix_Chunk *chunk);

int Mix_PlayChannelTimed(int channel, Mix_Chunk *chunk, int loops, int ticks);
#define Mix_PlayChannel(channel, chunk, loops) Mix_PlayChannelTimed(channel, chunk, loops, -1)
int Mix_FadeInChannel(int channel, Mix_Chunk *chunk, int loops, int ms);
int Mix_FadeOutChannel(int channel, int ms);
int Mix_HaltChannel(int channel);
int Mix_Volume(int channel, int volume);
void Mix_Pause(int channel);
void Mix_Resume(int channel);
int Mix_Playing(int channel);

int Mix_RegisterEffect(int chan, Mix_EffectFunc_t f, Mix_EffectDone_t d, void *arg);
int Mix_UnregisterAllEffects(int channel);

Mix_Music *Mix_LoadMUS(const char *file);
void Mix_FreeMusic(Mix_Music *music);
int Mix_PlayMusic(Mix_Music *music, int loops);
int Mix_HaltMusic(void);
void Mix_PauseMusic(void);
void Mix_ResumeMusic(void);
int Mix_VolumeMusic(int volume);
int Mix_PlayingMusic(void);
void Mix_HookMusic(void (*mix_func)(void *udata, Uint8 *stream, int len), void *arg);

#define Mix_GetError SDL_GetError
#define Mix_SetError SDL_SetError

#ifdef __cplusplus
}
#endif

#endif

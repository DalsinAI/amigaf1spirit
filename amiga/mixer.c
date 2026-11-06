/* mixer.c: SDL_mixer 1.2's channels and music, as F-1 Spirit uses them,
 * on SDL 1.2's audio (see amiga/include/SDL_mixer.h).
 *
 * The output is signed 16-bit stereo at 22050 Hz (half the files' 44100,
 * which halves the 68k's mixing; Mix_QuerySpec tells the game). Sound
 * effects are decoded whole on load into that format; the music is one
 * Ogg Vorbis stream, decoded by stb_vorbis in the audio callback, every
 * other sample pair kept. Each channel's part of a callback is put in a
 * buffer of its own, run through its effects (the car engine's pitch
 * effect writes all of it), then added in at the channel's volume.
 *
 * Copyright (C) 2026 AmiCradle Team. GPL 2 or later, like F-1 Spirit. */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "SDL.h"
#include "SDL_mixer.h"

#define STB_VORBIS_HEADER_ONLY
#include "stb_vorbis.c"

#define OUT_RATE 22050
#define MAX_CHANNELS 32
#define MAX_EFFECTS 4

typedef struct {
	Mix_Chunk *chunk;
	Uint32 pos;          /* bytes into chunk->abuf */
	int loops;           /* -1 forever */
	int volume;          /* 0..128 */
	int paused;
	Sint32 expire;       /* SDL_GetTicks() when a timed play ends, 0 none */
	int fade_dir;        /* 1 in, -1 out, 0 none */
	Uint32 fade_start, fade_len;
	int neffects;
	Mix_EffectFunc_t effect[MAX_EFFECTS];
	Mix_EffectDone_t done[MAX_EFFECTS];
	void *effect_arg[MAX_EFFECTS];
} Channel;

struct _Mix_Music {
	stb_vorbis *v;
	int channels, rate;
};

static int audio_open = 0;
static Channel channel[MAX_CHANNELS];
static int num_channels = MIX_CHANNELS;
static int reserved = 0;
static Sint16 *chanbuf = NULL;
static int chanbuf_len = 0;

static Mix_Music *music = NULL;
static int music_loops = 0;
static int music_playing = 0;
static int music_paused = 0;
static int music_volume = MIX_MAX_VOLUME;
static void (*music_hook)(void *, Uint8 *, int) = NULL;
static void *music_hook_arg = NULL;
static short musicbuf[4096];

static const SDL_version linked = { SDL_MIXER_MAJOR_VERSION, SDL_MIXER_MINOR_VERSION,
	SDL_MIXER_PATCHLEVEL };

const SDL_version *Mix_Linked_Version(void)
{
	return &linked;
}

static Sint16 clamp16(Sint32 v)
{
	return (Sint16)(v > 32767 ? 32767 : (v < -32768 ? -32768 : v));
}

static int fade_volume(Channel *c, Uint32 now)
{
	Uint32 t;
	int vol = c->volume;
	if (!c->fade_dir || !c->fade_len)
		return vol;
	t = now - c->fade_start;
	if (t >= c->fade_len) {
		if (c->fade_dir < 0) {
			c->chunk = NULL;
			return 0;
		}
		c->fade_dir = 0;
		return vol;
	}
	if (c->fade_dir > 0)
		return (int)((Uint32)vol * t / c->fade_len);
	return (int)((Uint32)vol * (c->fade_len - t) / c->fade_len);
}

/* Up to len bytes of the channel's chunk, looping, into chanbuf. */
static int fill_channel(Channel *c, int len)
{
	int got = 0;
	Uint8 *out = (Uint8 *)chanbuf;
	while (got < len && c->chunk) {
		Uint32 left = c->chunk->alen - c->pos;
		Uint32 n = (Uint32)(len - got) < left ? (Uint32)(len - got) : left;
		memcpy(out + got, c->chunk->abuf + c->pos, n);
		got += n;
		c->pos += n;
		if (c->pos >= c->chunk->alen) {
			c->pos = 0;
			if (c->loops == 0) {
				c->chunk = NULL;
				break;
			}
			if (c->loops > 0)
				c->loops--;
			if (c->chunk->alen == 0) {
				c->chunk = NULL;
				break;
			}
		}
	}
	if (got < len)
		memset(out + got, 0, len - got);
	return got;
}

static void mix_music(Sint16 *out, int frames)
{
	int i;
	while (frames > 0 && music && music_playing && !music_paused) {
		int step = (music->rate >= 2 * OUT_RATE - 100) ? 2 : 1;
		int want = frames * step;
		int got;
		if (want > (int)(sizeof(musicbuf) / sizeof(short) / 2))
			want = sizeof(musicbuf) / sizeof(short) / 2;
		got = stb_vorbis_get_samples_short_interleaved(music->v, 2, musicbuf, want * 2);
		if (got <= 0) {
			if (music_loops == 0) {
				music_playing = 0;
				break;
			}
			if (music_loops > 0)
				music_loops--;
			stb_vorbis_seek_start(music->v);
			continue;
		}
		for (i = 0; i + step <= got && frames > 0; i += step, frames--, out += 2) {
			Sint32 l = musicbuf[i * 2], r = musicbuf[i * 2 + 1];
			if (step == 2) {
				l = (l + musicbuf[i * 2 + 2]) >> 1;
				r = (r + musicbuf[i * 2 + 3]) >> 1;
			}
			out[0] = clamp16(out[0] + ((l * music_volume) >> 7));
			out[1] = clamp16(out[1] + ((r * music_volume) >> 7));
		}
	}
}

static void callback(void *udata, Uint8 *stream, int len)
{
	Sint16 *out = (Sint16 *)stream;
	int frames = len / 4;
	Uint32 now = SDL_GetTicks();
	int i, j, e;
	(void)udata;

	memset(stream, 0, len);

	if (music_hook)
		music_hook(music_hook_arg, stream, len);
	else
		mix_music(out, frames);

	if (len > chanbuf_len)
		return;

	for (i = 0; i < num_channels; i++) {
		Channel *c = &channel[i];
		int vol;
		if (!c->chunk || c->paused)
			continue;
		if (c->expire && (Sint32)(now - c->expire) >= 0) {
			c->chunk = NULL;
			continue;
		}
		vol = fade_volume(c, now);
		if (!c->chunk)
			continue;
		fill_channel(c, len);
		for (e = 0; e < c->neffects; e++)
			c->effect[e](i, chanbuf, len, c->effect_arg[e]);
		for (j = 0; j < frames * 2; j++)
			out[j] = clamp16(out[j] + ((chanbuf[j] * vol) >> 7));
	}
}

int Mix_OpenAudio(int frequency, Uint16 format, int channels, int chunksize)
{
	SDL_AudioSpec want, have;
	(void)frequency; (void)format; (void)channels;
	if (audio_open) {
		audio_open++;
		return 0;
	}
	memset(&want, 0, sizeof(want));
	want.freq = OUT_RATE;
	want.format = AUDIO_S16SYS;
	want.channels = 2;
	want.samples = chunksize > 0 ? chunksize : 1024;
	want.callback = callback;
	if (SDL_OpenAudio(&want, &have) < 0)
		return -1;
	if (have.format != AUDIO_S16SYS || have.channels != 2 || have.freq != OUT_RATE) {
		SDL_CloseAudio();
		SDL_SetError("audio opened in a format the mixer can't use");
		return -1;
	}
	chanbuf_len = have.samples * 4 * 2;
	chanbuf = (Sint16 *)malloc(chanbuf_len);
	if (!chanbuf) {
		SDL_CloseAudio();
		return -1;
	}
	memset(channel, 0, sizeof(channel));
	for (num_channels = 0; num_channels < MIX_CHANNELS; num_channels++)
		channel[num_channels].volume = MIX_MAX_VOLUME;
	audio_open = 1;
	SDL_PauseAudio(0);
	return 0;
}

void Mix_CloseAudio(void)
{
	int i, e;
	if (!audio_open)
		return;
	if (--audio_open > 0)
		return;
	SDL_CloseAudio();
	for (i = 0; i < MAX_CHANNELS; i++)
		for (e = 0; e < channel[i].neffects; e++)
			if (channel[i].done[e])
				channel[i].done[e](i, channel[i].effect_arg[e]);
	memset(channel, 0, sizeof(channel));
	free(chanbuf);
	chanbuf = NULL;
	music_playing = 0;
}

int Mix_QuerySpec(int *frequency, Uint16 *format, int *channels)
{
	if (!audio_open)
		return 0;
	if (frequency) *frequency = OUT_RATE;
	if (format) *format = AUDIO_S16SYS;
	if (channels) *channels = 2;
	return audio_open;
}

int Mix_AllocateChannels(int numchans)
{
	int i;
	if (numchans < 0)
		return num_channels;
	if (numchans > MAX_CHANNELS)
		numchans = MAX_CHANNELS;
	SDL_LockAudio();
	for (i = numchans; i < num_channels; i++)
		channel[i].chunk = NULL;
	for (i = num_channels; i < numchans; i++) {
		memset(&channel[i], 0, sizeof(Channel));
		channel[i].volume = MIX_MAX_VOLUME;
	}
	num_channels = numchans;
	SDL_UnlockAudio();
	return num_channels;
}

int Mix_ReserveChannels(int num)
{
	reserved = num > num_channels ? num_channels : num;
	return reserved;
}

/* Signed 16-bit interleaved samples to the output format (S16 stereo at
 * OUT_RATE), averaging when going down. */
static Mix_Chunk *make_chunk(const Sint16 *pcm, int frames, int chans, int rate)
{
	Mix_Chunk *c;
	int outf, o;
	if (frames <= 0 || chans < 1 || rate <= 0)
		return NULL;
	outf = (int)((long long)frames * OUT_RATE / rate);
	if (outf < 1)
		outf = 1;
	c = (Mix_Chunk *)malloc(sizeof(Mix_Chunk));
	if (!c)
		return NULL;
	c->abuf = (Uint8 *)malloc(outf * 4);
	if (!c->abuf) {
		free(c);
		return NULL;
	}
	c->allocated = 1;
	c->alen = outf * 4;
	c->volume = MIX_MAX_VOLUME;
	for (o = 0; o < outf; o++) {
		int a = (int)((long long)o * rate / OUT_RATE);
		int b = (int)((long long)(o + 1) * rate / OUT_RATE);
		Sint32 l = 0, r = 0;
		int n, i;
		if (b <= a) b = a + 1;
		if (b > frames) b = frames;
		if (a >= frames) a = frames - 1;
		n = b - a;
		for (i = a; i < b; i++) {
			l += pcm[i * chans];
			r += pcm[i * chans + (chans > 1 ? 1 : 0)];
		}
		((Sint16 *)c->abuf)[o * 2] = (Sint16)(l / n);
		((Sint16 *)c->abuf)[o * 2 + 1] = (Sint16)(r / n);
	}
	return c;
}

static int has_ext(const char *file, const char *ext)
{
	size_t n = strlen(file), e = strlen(ext);
	return n > e && SDL_strcasecmp(file + n - e, ext) == 0;
}

Mix_Chunk *Mix_LoadWAV(const char *file)
{
	Mix_Chunk *c = NULL;
	if (has_ext(file, ".ogg")) {
		int chans = 0, rate = 0;
		short *pcm = NULL;
		int frames = stb_vorbis_decode_filename(file, &chans, &rate, &pcm);
		if (frames > 0 && pcm)
			c = make_chunk(pcm, frames, chans, rate);
		free(pcm);
	} else {
		SDL_AudioSpec spec;
		Uint8 *buf = NULL;
		Uint32 len = 0;
		if (SDL_LoadWAV(file, &spec, &buf, &len)) {
			SDL_AudioCVT cvt;
			/* first to signed 16-bit in the machine's order, same rate */
			if (SDL_BuildAudioCVT(&cvt, spec.format, spec.channels, spec.freq,
			                      AUDIO_S16SYS, spec.channels, spec.freq) >= 0) {
				cvt.len = len;
				cvt.buf = (Uint8 *)malloc(len * (cvt.len_mult > 0 ? cvt.len_mult : 1));
				if (cvt.buf) {
					memcpy(cvt.buf, buf, len);
					if (cvt.needed == 0 || SDL_ConvertAudio(&cvt) == 0) {
						int bytes = cvt.needed ? cvt.len_cvt : (int)len;
						c = make_chunk((Sint16 *)cvt.buf, bytes / 2 / spec.channels,
						               spec.channels, spec.freq);
					}
					free(cvt.buf);
				}
			}
			SDL_FreeWAV(buf);
		}
	}
	if (!c)
		SDL_SetError("Mix_LoadWAV: couldn't load %s", file);
	return c;
}

void Mix_FreeChunk(Mix_Chunk *chunk)
{
	int i;
	if (!chunk)
		return;
	SDL_LockAudio();
	for (i = 0; i < MAX_CHANNELS; i++)
		if (channel[i].chunk == chunk)
			channel[i].chunk = NULL;
	SDL_UnlockAudio();
	if (chunk->allocated)
		free(chunk->abuf);
	free(chunk);
}

static int start_channel(int which, Mix_Chunk *chunk, int loops, int ticks,
                         int fade_dir, int ms)
{
	int i;
	if (!audio_open || !chunk)
		return -1;
	SDL_LockAudio();
	if (which < 0) {
		for (i = reserved; i < num_channels; i++)
			if (!channel[i].chunk)
				break;
		which = i < num_channels ? i : -1;
	}
	if (which >= 0 && which < num_channels) {
		Channel *c = &channel[which];
		c->chunk = chunk;
		c->pos = 0;
		c->loops = loops;
		c->paused = 0;
		c->expire = ticks > 0 ? (Sint32)(SDL_GetTicks() + ticks) : 0;
		c->fade_dir = fade_dir;
		c->fade_start = SDL_GetTicks();
		c->fade_len = ms > 0 ? (Uint32)ms : 0;
	} else
		which = -1;
	SDL_UnlockAudio();
	if (which < 0)
		SDL_SetError("No free channels available");
	return which;
}

int Mix_PlayChannelTimed(int which, Mix_Chunk *chunk, int loops, int ticks)
{
	return start_channel(which, chunk, loops, ticks, 0, 0);
}

int Mix_FadeInChannel(int which, Mix_Chunk *chunk, int loops, int ms)
{
	return start_channel(which, chunk, loops, -1, 1, ms);
}

int Mix_FadeOutChannel(int which, int ms)
{
	int i, n = 0;
	SDL_LockAudio();
	for (i = 0; i < num_channels; i++)
		if ((which < 0 || which == i) && channel[i].chunk) {
			channel[i].fade_dir = -1;
			channel[i].fade_start = SDL_GetTicks();
			channel[i].fade_len = ms > 0 ? (Uint32)ms : 1;
			n++;
		}
	SDL_UnlockAudio();
	return n;
}

int Mix_HaltChannel(int which)
{
	int i;
	SDL_LockAudio();
	for (i = 0; i < num_channels; i++)
		if (which < 0 || which == i)
			channel[i].chunk = NULL;
	SDL_UnlockAudio();
	return 0;
}

int Mix_Volume(int which, int volume)
{
	int i, prev = MIX_MAX_VOLUME;
	if (volume > MIX_MAX_VOLUME)
		volume = MIX_MAX_VOLUME;
	SDL_LockAudio();
	if (which >= 0 && which < num_channels)
		prev = channel[which].volume;
	if (volume >= 0)
		for (i = 0; i < num_channels; i++)
			if (which < 0 || which == i)
				channel[i].volume = volume;
	SDL_UnlockAudio();
	return prev;
}

void Mix_Pause(int which)
{
	int i;
	SDL_LockAudio();
	for (i = 0; i < num_channels; i++)
		if (which < 0 || which == i)
			channel[i].paused = 1;
	SDL_UnlockAudio();
}

void Mix_Resume(int which)
{
	int i;
	SDL_LockAudio();
	for (i = 0; i < num_channels; i++)
		if (which < 0 || which == i)
			channel[i].paused = 0;
	SDL_UnlockAudio();
}

int Mix_Playing(int which)
{
	int i, n = 0;
	for (i = 0; i < num_channels; i++)
		if ((which < 0 || which == i) && channel[i].chunk)
			n++;
	return n;
}

int Mix_RegisterEffect(int chan, Mix_EffectFunc_t f, Mix_EffectDone_t d, void *arg)
{
	Channel *c;
	if (chan < 0 || chan >= num_channels || !f) {
		SDL_SetError("Invalid channel number");
		return 0;
	}
	c = &channel[chan];
	SDL_LockAudio();
	if (c->neffects >= MAX_EFFECTS) {
		SDL_UnlockAudio();
		SDL_SetError("Too many effects");
		return 0;
	}
	c->effect[c->neffects] = f;
	c->done[c->neffects] = d;
	c->effect_arg[c->neffects] = arg;
	c->neffects++;
	SDL_UnlockAudio();
	return 1;
}

int Mix_UnregisterAllEffects(int chan)
{
	Channel *c;
	int e;
	if (chan < 0 || chan >= num_channels)
		return 0;
	c = &channel[chan];
	SDL_LockAudio();
	for (e = 0; e < c->neffects; e++)
		if (c->done[e])
			c->done[e](chan, c->effect_arg[e]);
	c->neffects = 0;
	SDL_UnlockAudio();
	return 1;
}

Mix_Music *Mix_LoadMUS(const char *file)
{
	int err = 0;
	stb_vorbis_info info;
	Mix_Music *m;
	stb_vorbis *v = stb_vorbis_open_filename(file, &err, NULL);
	if (!v) {
		SDL_SetError("Mix_LoadMUS: couldn't open %s", file);
		return NULL;
	}
	m = (Mix_Music *)malloc(sizeof(Mix_Music));
	if (!m) {
		stb_vorbis_close(v);
		return NULL;
	}
	info = stb_vorbis_get_info(v);
	m->v = v;
	m->channels = info.channels;
	m->rate = (int)info.sample_rate;
	return m;
}

void Mix_FreeMusic(Mix_Music *m)
{
	if (!m)
		return;
	SDL_LockAudio();
	if (music == m) {
		music = NULL;
		music_playing = 0;
	}
	SDL_UnlockAudio();
	stb_vorbis_close(m->v);
	free(m);
}

int Mix_PlayMusic(Mix_Music *m, int loops)
{
	if (!audio_open || !m)
		return -1;
	SDL_LockAudio();
	stb_vorbis_seek_start(m->v);
	music = m;
	/* SDL_mixer: loops is the number of plays, -1 forever; 0 also once */
	music_loops = loops < 0 ? -1 : (loops > 0 ? loops - 1 : 0);
	music_playing = 1;
	music_paused = 0;
	SDL_UnlockAudio();
	return 0;
}

int Mix_HaltMusic(void)
{
	SDL_LockAudio();
	music_playing = 0;
	SDL_UnlockAudio();
	return 0;
}

void Mix_PauseMusic(void)
{
	music_paused = 1;
}

void Mix_ResumeMusic(void)
{
	music_paused = 0;
}

int Mix_VolumeMusic(int volume)
{
	int prev = music_volume;
	if (volume >= 0)
		music_volume = volume > MIX_MAX_VOLUME ? MIX_MAX_VOLUME : volume;
	return prev;
}

int Mix_PlayingMusic(void)
{
	return music_playing && music;
}

void Mix_HookMusic(void (*mix_func)(void *, Uint8 *, int), void *arg)
{
	SDL_LockAudio();
	music_hook = mix_func;
	music_hook_arg = arg;
	SDL_UnlockAudio();
}

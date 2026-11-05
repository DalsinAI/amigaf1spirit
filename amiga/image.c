/* image.c: IMG_Load on stb_image (see amiga/include/SDL_image.h).
 *
 * Every picture comes back as a 32-bit surface whose bytes are R, G, B, A
 * in memory, whatever the file held (SDL_image would give palette and
 * 24-bit surfaces for some): the game blits each one into such a surface
 * before it makes a texture of it, and reads pixels at ROFFSET..AOFFSET.
 * A picture without alpha gets 255 everywhere, as such a blit gives.
 *
 * Copyright (C) 2026 AmiCradle Team. GPL 2 or later, like F-1 Spirit. */

#include <stdlib.h>
#include <string.h>

#include "SDL.h"
#include "SDL_image.h"

#define STBI_ONLY_PNG
#define STBI_ONLY_JPEG
#define STBI_NO_SIMD
#define STBI_NO_THREAD_LOCALS
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

SDL_Surface *IMG_Load(const char *file)
{
	int w, h, n, y;
	unsigned char *data;
	SDL_Surface *s;
	Uint32 rmask, gmask, bmask, amask;

	data = stbi_load(file, &w, &h, &n, 4);
	if (!data) {
		SDL_SetError("IMG_Load: %s: %s", file, stbi_failure_reason());
		return NULL;
	}
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
	rmask = 0xff000000; gmask = 0x00ff0000; bmask = 0x0000ff00; amask = 0x000000ff;
#else
	rmask = 0x000000ff; gmask = 0x0000ff00; bmask = 0x00ff0000; amask = 0xff000000;
#endif
	s = SDL_CreateRGBSurface(SDL_SWSURFACE, w, h, 32, rmask, gmask, bmask, amask);
	if (s) {
		if (SDL_MUSTLOCK(s))
			SDL_LockSurface(s);
		for (y = 0; y < h; y++)
			memcpy((Uint8 *)s->pixels + y * s->pitch, data + (size_t)y * w * 4, (size_t)w * 4);
		if (SDL_MUSTLOCK(s))
			SDL_UnlockSurface(s);
	}
	stbi_image_free(data);
	return s;
}

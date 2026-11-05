/* SDL_image.h: IMG_Load, all F-1 Spirit asks of SDL_image 1.2, for
 * GFOpenGPU's SDL 1.2 (sdl12-compat), which has no SDL_image of its own.
 * amiga/image.c reads PNG and JPEG with stb_image into a 32-bit surface
 * with the bytes in R, G, B, A order (the game's own RMASK..AMASK).
 * Copyright (C) 2026 AmiCradle Team. GPL 2 or later, like F-1 Spirit. */
#ifndef AMIGA_SDL_IMAGE_H
#define AMIGA_SDL_IMAGE_H

#include "SDL.h"

#ifdef __cplusplus
extern "C" {
#endif

SDL_Surface *IMG_Load(const char *file);
#define IMG_GetError SDL_GetError

#ifdef __cplusplus
}
#endif

#endif

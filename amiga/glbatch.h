/* F-1 Spirit for AmigaOS: drawing many small things with few GL calls.
 * Copyright (c) 2026 AmiCradle Team. GPL, version 2 or later.
 *
 * Upstream draws each sprite, road strip, tyre mark, rain drop and splash
 * with its own glPushMatrix/glTranslatef/glRotatef/glBegin/.../glEnd/
 * glPopMatrix, glBindTexture and glEnable/glDisable. On a 68k each GL call
 * costs (GL.module runs Mesa's state tracker on the 68k, even when the
 * host's graphics chip does the drawing): a minute into a rainy race a
 * frame was some 20,000 glBegin/glEnd pairs and took over a second. Here
 * the corners are worked out on the CPU and collected in arrays, and one
 * glDrawArrays draws each run that shares a texture, in the order they
 * came, so the picture is the same. Small sprite pictures share a few
 * big textures (the atlas below), so the runs are long.
 *
 * The coordinates are those of the modelview matrix current when the batch
 * is flushed: a caller that changes the matrix, or draws anything another
 * way, calls glb_flush() first (glb_end() inside glb_begin()). */
#ifndef F1S_GLBATCH_H
#define F1S_GLBATCH_H

#include <GL/gl.h>

/* Sprites (GLTile) are batched while this is set (CTrack::draw sets it). */
extern bool glb_active;

/* A triangle's or line's points: x, y, z each. Colours 0..1. */
void glb_tri(GLuint tex, const float *xyz, const float *st, float r, float g, float b, float a);
void glb_quad(GLuint tex, const float *xyz, const float *st, float r, float g, float b, float a);
void glb_line(float x1, float y1, float z1, float x2, float y2, float z2, float r, float g, float b, float a);
void glb_flush(void);

/* Room for n untextured (tex 0) or textured points of mode m, written by the
 * caller: x, y, z at the pointer returned, r, g, b, a (0..1) at *c; texture
 * coordinates are not set. 0 when there is no room. */
GLfloat *glb_reserve(GLenum m, GLuint tex, int n, GLfloat **c);

/* Around a run of batched drawing with nothing else drawn in between:
 * the arrays are set up once, and texturing is turned on and off only
 * when it changes. Without them each flush sets up and puts back. */
void glb_begin(void);
void glb_end(void);

/* Something else bound a texture or turned texturing on or off. */
void glb_forget(void);

/* The texture atlas: small sprite textures share a few big textures, so
 * that runs of different sprites go in one glDrawArrays. A tile's picture
 * (w x h pixels, RGBA bytes, pitch bytes a row) is copied in once;
 * glb_atlas_add gives its page, or -1 (too big, or no room), and its
 * texture coordinates u0, v0, u1, v1. gen is the game's reload_textures:
 * when it changes, the GL context was made again and the pages are
 * forgotten. A page is deleted when the last tile on it is released and
 * it is no longer being filled. */
int glb_atlas_add(int gen, const void *pixels, int pitch, int w, int h, float *uv);
GLuint glb_atlas_texture(int page);
void glb_atlas_release(int gen, int page);

/* Counts for a timing build: glDrawArrays calls and points sent. */
extern unsigned long glb_draws, glb_points;

/* The four side planes of the view as the matrices stand now (object
 * coordinates of the current modelview matrix), to skip what can't be
 * seen. */
struct CullPlanes {
	float p[4][4];
};

void cull_setup(CullPlanes *c);

/* Could a sphere of radius r round (x, y, z) reach the screen? */
static inline bool cull_visible(const CullPlanes *c, float x, float y, float z, float r)
{
	int i;

	for (i = 0; i < 4; i++) {
		if (c->p[i][0] * x + c->p[i][1] * y + c->p[i][2] * z + c->p[i][3] < -r)
			return false;
	}

	return true;
}

#endif

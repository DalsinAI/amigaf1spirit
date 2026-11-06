/* F-1 Spirit for AmigaOS: drawing many small things with few GL calls
 * (see glbatch.h).
 * Copyright (c) 2026 AmiCradle Team. GPL, version 2 or later. */
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "glbatch.h"

bool glb_active = false;
unsigned long glb_draws = 0, glb_points = 0;

static GLenum mode = GL_TRIANGLES;
static GLuint texture = 0;
static int count = 0, room = 0;
static GLfloat *pos = 0, *st = 0;
static GLubyte *col = 0;
static GLubyte last_col[4] = { 255, 255, 255, 255 };

static bool grow(int n)
{
	int want;
	GLfloat *p, *t;
	GLubyte *c;

	if (count + n <= room)
		return true;

	want = room ? room * 2 : 4096;

	while (want < count + n)
		want *= 2;

	p = (GLfloat *)realloc(pos, want * 3 * sizeof(GLfloat));
	if (p) pos = p;
	t = (GLfloat *)realloc(st, want * 2 * sizeof(GLfloat));
	if (t) st = t;
	c = (GLubyte *)realloc(col, want * 4);
	if (c) col = c;

	if (!p || !t || !c)
		return false;

	room = want;
	return true;
}

static inline GLubyte byte_of(float f)
{
	if (f <= 0) return 0;
	if (f >= 1) return 255;
	return (GLubyte)(f * 255.0F + 0.5F);
}

/* Room for n more points of this kind; what was collected of another kind
 * is drawn first. */
static bool start(GLenum m, GLuint tex, int n)
{
	if (count > 0 && (m != mode || tex != texture))
		glb_flush();

	mode = m;
	texture = tex;

	if (!grow(n)) {
		glb_flush();
		return grow(n);
	}

	return true;
}

static inline void put(const float *xyz, const float *t, const GLubyte *c)
{
	GLfloat *p = pos + count * 3;
	GLubyte *q = col + count * 4;

	p[0] = xyz[0];
	p[1] = xyz[1];
	p[2] = xyz[2];

	if (t) {
		st[count * 2] = t[0];
		st[count * 2 + 1] = t[1];
	}

	q[0] = c[0];
	q[1] = c[1];
	q[2] = c[2];
	q[3] = c[3];
	count++;
}

void glb_tri(GLuint tex, const float *xyz, const float *t, float r, float g, float b, float a)
{
	GLubyte c[4] = { byte_of(r), byte_of(g), byte_of(b), byte_of(a) };
	int i;

	if (!start(GL_TRIANGLES, tex, 3))
		return;

	for (i = 0; i < 3; i++)
		put(xyz + i * 3, t ? t + i * 2 : 0, c);
}

/* Points 0, 1, 2, 3 round the quad, as glBegin(GL_QUADS) takes them. */
void glb_quad(GLuint tex, const float *xyz, const float *t, float r, float g, float b, float a)
{
	static const int order[6] = { 0, 1, 2, 0, 2, 3 };
	GLubyte c[4] = { byte_of(r), byte_of(g), byte_of(b), byte_of(a) };
	int i;

	if (!start(GL_TRIANGLES, tex, 6))
		return;

	for (i = 0; i < 6; i++)
		put(xyz + order[i] * 3, t ? t + order[i] * 2 : 0, c);
}

void glb_line(float x1, float y1, float z1, float x2, float y2, float z2, float r, float g, float b, float a)
{
	GLubyte c[4] = { byte_of(r), byte_of(g), byte_of(b), byte_of(a) };
	float p1[3] = { x1, y1, z1 }, p2[3] = { x2, y2, z2 };

	if (!start(GL_LINES, 0, 2))
		return;

	put(p1, 0, c);
	put(p2, 0, c);
}

/* What this file has set in GL during a batch: set once, not at every
 * flush (each GL call costs on the 68k). */
static bool in_batch = false;
static int tex_on = 0;   /* -1: not known */
static const GLvoid *set_pos = 0, *set_st = 0, *set_col = 0;
static GLuint bound = 0;

void glb_begin(void)
{
	if (in_batch)
		return;

	in_batch = true;
	tex_on = 0;
	bound = 0;
	set_pos = set_st = set_col = 0;
	glDisable(GL_TEXTURE_2D);
	glNormal3f(0.0F, 0.0F, 1.0F);     /* as upstream gives every quad */
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_COLOR_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
}

void glb_forget(void)
{
	glb_flush();
	bound = 0;
	tex_on = -1;
}

void glb_end(void)
{
	glb_flush();

	if (!in_batch)
		return;

	in_batch = false;
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glDisableClientState(GL_COLOR_ARRAY);
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisable(GL_TEXTURE_2D);

	/* The current colour after a colour array is undefined: leave the last
	 * one, as the glColor4f calls drawn one by one would have. */
	glColor4ub(last_col[0], last_col[1], last_col[2], last_col[3]);
}

void glb_flush(void)
{
	bool own = !in_batch;

	if (count == 0)
		return;

	if (own)
		glb_begin();

	if (texture) {
		if (tex_on != 1) {
			glEnable(GL_TEXTURE_2D);
			tex_on = 1;
		}

		if (texture != bound) {
			glBindTexture(GL_TEXTURE_2D, texture);
			bound = texture;
		}
	} else if (tex_on != 0) {
		glDisable(GL_TEXTURE_2D);
		tex_on = 0;
	}

	if (set_pos != pos) {
		glVertexPointer(3, GL_FLOAT, 0, pos);
		set_pos = pos;
	}

	if (set_st != st) {
		glTexCoordPointer(2, GL_FLOAT, 0, st);
		set_st = st;
	}

	if (set_col != col) {
		glColorPointer(4, GL_UNSIGNED_BYTE, 0, col);
		set_col = col;
	}

	glDrawArrays(mode, 0, count);

	memcpy(last_col, col + (count - 1) * 4, 4);
	glb_draws++;
	glb_points += count;
	count = 0;

	if (own)
		glb_end();
}

#define ATLAS_SIZE 512
#define ATLAS_PAGES 64

struct AtlasPage {
	GLuint tex;
	int refs;
	int x, y, shelf_h;	/* where the next picture goes */
};

static AtlasPage pages[ATLAS_PAGES];
static int filling = -1, atlas_gen = -1;

static int new_page(void)
{
	int i;

	for (i = 0; i < ATLAS_PAGES; i++) {
		if (pages[i].tex == 0)
			break;
	}

	if (i == ATLAS_PAGES)
		return -1;

	glGenTextures(1, &pages[i].tex);
	glBindTexture(GL_TEXTURE_2D, pages[i].tex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	{
		/* clear: the gaps between pictures are transparent, as the
		 * padding of the game's own power-of-two textures is */
		void *zero = calloc(ATLAS_SIZE * ATLAS_SIZE, 4);

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, ATLAS_SIZE, ATLAS_SIZE, 0, GL_RGBA, GL_UNSIGNED_BYTE, zero);
		::free(zero);
	}
	pages[i].refs = 0;
	pages[i].x = pages[i].y = pages[i].shelf_h = 0;
	return i;
}

/* Room for w x h (and a pixel's gap) on page p, by shelves. */
static bool page_room(AtlasPage *pg, int w, int h, int *ox, int *oy)
{
	if (pg->x + w > ATLAS_SIZE) {
		pg->y += pg->shelf_h + 1;
		pg->x = 0;
		pg->shelf_h = 0;
	}

	if (pg->y + h > ATLAS_SIZE)
		return false;

	*ox = pg->x;
	*oy = pg->y;
	pg->x += w + 1;

	if (h > pg->shelf_h)
		pg->shelf_h = h;

	return true;
}

int glb_atlas_add(int gen, const void *pixels, int pitch, int w, int h, float *uv)
{
	int x, y;
	GLint row, align;

	if (w <= 0 || h <= 0 || w > ATLAS_SIZE / 2 || h > ATLAS_SIZE / 2 || (pitch & 3) != 0)
		return -1;

	if (gen != atlas_gen) {
		/* a new GL context: the old pages went with the old one */
		memset(pages, 0, sizeof(pages));
		filling = -1;
		atlas_gen = gen;
	}

	glb_forget();		/* this binds a texture */

	if (filling < 0 || !page_room(&pages[filling], w, h, &x, &y)) {
		int old = filling;

		filling = new_page();

		if (old >= 0 && pages[old].refs == 0) {
			glDeleteTextures(1, &pages[old].tex);
			pages[old].tex = 0;
		}

		if (filling < 0 || !page_room(&pages[filling], w, h, &x, &y))
			return -1;
	}

	glBindTexture(GL_TEXTURE_2D, pages[filling].tex);
	glGetIntegerv(GL_UNPACK_ROW_LENGTH, &row);
	glGetIntegerv(GL_UNPACK_ALIGNMENT, &align);
	glPixelStorei(GL_UNPACK_ROW_LENGTH, pitch / 4);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
	glTexSubImage2D(GL_TEXTURE_2D, 0, x, y, w, h, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
	glPixelStorei(GL_UNPACK_ROW_LENGTH, row);
	glPixelStorei(GL_UNPACK_ALIGNMENT, align);

	uv[0] = float(x) / ATLAS_SIZE;
	uv[1] = float(y) / ATLAS_SIZE;
	uv[2] = float(x + w) / ATLAS_SIZE;
	uv[3] = float(y + h) / ATLAS_SIZE;
	pages[filling].refs++;
	return filling;
}

GLuint glb_atlas_texture(int page)
{
	return page >= 0 && page < ATLAS_PAGES ? pages[page].tex : 0;
}

void glb_atlas_release(int gen, int page)
{
	if (gen != atlas_gen || page < 0 || page >= ATLAS_PAGES || pages[page].tex == 0)
		return;

	if (--pages[page].refs <= 0 && page != filling) {
		glb_forget();
		glDeleteTextures(1, &pages[page].tex);
		pages[page].tex = 0;
		pages[page].refs = 0;
	}
}

void cull_setup(CullPlanes *c)
{
	GLfloat mv[16], pr[16], m[16];
	int i, j, k;

	glGetFloatv(GL_MODELVIEW_MATRIX, mv);
	glGetFloatv(GL_PROJECTION_MATRIX, pr);

	/* m = projection * modelview (column-major, as GL keeps them) */
	for (j = 0; j < 4; j++) {
		for (i = 0; i < 4; i++) {
			float s = 0;

			for (k = 0; k < 4; k++)
				s += pr[k * 4 + i] * mv[j * 4 + k];

			m[j * 4 + i] = s;
		}
	}

	/* left, right, bottom, top: row 3 plus or minus row 0 or 1 */
	for (i = 0; i < 4; i++) {
		int row = i / 2;
		float sign = (i & 1) ? -1.0F : 1.0F;
		float len;

		for (j = 0; j < 4; j++)
			c->p[i][j] = m[j * 4 + 3] + sign * m[j * 4 + row];

		len = sqrtf(c->p[i][0] * c->p[i][0] + c->p[i][1] * c->p[i][1] + c->p[i][2] * c->p[i][2]);

		if (len > 0) {
			for (j = 0; j < 4; j++)
				c->p[i][j] /= len;
		}
	}
}

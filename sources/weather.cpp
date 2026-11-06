#ifdef _WIN32
#include "windows.h"
#include "glut.h"
#else
#include <sys/time.h>
#endif

#include "stdio.h"
#include "math.h"
#include "stdlib.h"
#include "string.h"
#include <time.h>

#include "GL/gl.h"
#include "GL/glu.h"
#include "SDL.h"
#include "SDL_image.h"
#include "SDL_mixer.h"
#include "SDL_net.h"

#include "F1Spirit.h"
#include "sound.h"
#include "List.h"
#include "2DCMC.h"
#include "auxiliar.h"
#include "GLTile.h"
#include "PlacedGLTile.h"
#include "RotatedGLTile.h"
#include "keyboardstate.h"
#include "CPlayer.h"
#include "CCar.h"
#include "2DCMC.h"
#include "RoadPiece.h"
#include "track.h"
#include "RacingCCar.h"
#include "PlayerCCar.h"
#include "EnemyCCar.h"
#include "GameParameters.h"
#include "ReplayInfo.h"
#include "F1SpiritGame.h"
#include "F1SpiritApp.h"
#include "F1Spirit-auxiliar.h"
#include "randomc.h"

#include "debug.h"
#ifdef __amigaos__
#include "glbatch.h"
#endif

#ifdef KITSCHY_DEBUG_MEMORY
#include "debug_memorymanager.h"
#endif


#define SEMAPHORE_TIMMER 64


extern int N_SFX_CHANNELS, SCREEN_X, SCREEN_Y;
extern bool sound;

extern TRanrotBGenerator *rg;


void F1SpiritGame::draw_nightmode_nostencil(PlayerCCar *v)
{
	glEnable( GL_DEPTH_TEST );
	glClear(GL_DEPTH_BUFFER_BIT);

	/* Draw the car lights: */
	{
		PlayerCCar *pc;
		List<PlayerCCar> pc_l;
		EnemyCCar *ec;
		List<EnemyCCar> ec_l;

		glPushMatrix();
		glScalef(v->c_z, v->c_z, v->c_z);

		glColor4f(0, 0, 0, 0.0);

		pc_l.Instance(player_cars);
		pc_l.Rewind();

		while (pc_l.Iterate(pc)) {
			if (pc->car->state != 2) {
				glPushMatrix();
				glTranslatef(pc->car->get_x() - v->c_x, pc->car->get_y() - v->c_y, 0);
				glRotatef(pc->car->get_a() - 90, 0, 0, 1);

				glBegin(GL_TRIANGLE_FAN);
				glVertex3f(8, 8, -10);
				glVertex3f(8, -8, -10);

				glVertex3f(80, -52, -10);
				glVertex3f(95, -50, -10);
				glVertex3f(108, -40, -10);
				glVertex3f(110, -25, -10);
				glVertex3f(108, -10, -10);
				glVertex3f(100, 0, -10);
				glVertex3f(108, 10, -10);
				glVertex3f(110, 25, -10);
				glVertex3f(108, 40, -10);
				glVertex3f(95, 50, -10);
				glVertex3f(80, 52, -10);
				glEnd();

				glPopMatrix();
			} // if
		} 

		ec_l.Instance(enemy_cars);

		ec_l.Rewind();

		while (ec_l.Iterate(ec)) {
			if (pc->car->state != 2) {
				glPushMatrix();
				glTranslatef(ec->car->get_x() - v->c_x, ec->car->get_y() - v->c_y, 0);
				glRotatef(ec->car->get_a() - 90, 0, 0, 1);

				glBegin(GL_TRIANGLE_FAN);
				glVertex3f(8, 8, -10);
				glVertex3f(8, -8, -10);

				glVertex3f(40, -26, -10);
				glVertex3f(50, -25, -10);
				glVertex3f(54, -20, -10);
				glVertex3f(55, -12.5f, -10);
				glVertex3f(54, -5, -10);
				glVertex3f(50, 0, -10);
				glVertex3f(54, 5, -10);
				glVertex3f(55, 12.5F, -10);
				glVertex3f(54, 20, -10);
				glVertex3f(50, 25, -10);
				glVertex3f(40, 26, -10);
				glEnd();

				glPopMatrix();
			} // if
		} 

		glPopMatrix();

	}

	/* Darken the non-illuminated region: */
	{
		glColor4f(0, 0, 0, 0.75F);
		glNormal3f(0.0, 0.0, 1.0);

		glBegin(GL_QUADS);
		glVertex3f( -1000, -1000, -5);
		glVertex3f( -1000, 1000, -5);
		glVertex3f(1000, 1000, -5);
		glVertex3f(1000, -1000, -5);
		glEnd();
	}

	glDisable( GL_DEPTH_TEST );
} 


void F1SpiritGame::draw_nightmode_stencil(PlayerCCar *v)
{
	glPushAttrib( GL_ALL_ATTRIB_BITS );
	/* Draw the car lights in the stencil buffer: */
	{
		int i;
		PlayerCCar *pc;
		List<PlayerCCar> pc_l;
		EnemyCCar *ec;
		List<EnemyCCar> ec_l;

		glClearStencil(0);
		glClear( GL_STENCIL_BUFFER_BIT );
		glEnable( GL_STENCIL_TEST );
		glStencilFunc( GL_ALWAYS, 1, 0xFFFF );
		glStencilOp( GL_KEEP, GL_KEEP, GL_INCR );

		glPushMatrix();
		glScalef(v->c_z, v->c_z, v->c_z);

		glColor4f(0, 0, 0, 0.0);

		pc_l.Instance(player_cars);
		pc_l.Rewind();

		while (pc_l.Iterate(pc)) {
			if (pc->car->state != 2) {
				glPushMatrix();
				glTranslatef(pc->car->get_x() - v->c_x, pc->car->get_y() - v->c_y, 0);
				glRotatef(pc->car->get_a() - 90, 0, 0, 1);

				for (i = 0;i < 16;i++) {
					float f = i * 0.0625f;
					float MAGNIFYING = 48.0f;
					glBegin(GL_TRIANGLE_FAN);
					glVertex3f(10, 8 + f*4, -4);
					glVertex3f(10, -8 + f*4, -4);

					glVertex3f(80 + f*MAGNIFYING*0.75, -52 - f*MAGNIFYING*0.5f, -5);
					glVertex3f(95 + f*MAGNIFYING, -50 - f*MAGNIFYING*0.5f, -5);
					glVertex3f(105 + f*MAGNIFYING, -40 - f*MAGNIFYING*0.375f, -5);
					glVertex3f(110 + f*MAGNIFYING, -25 - f*MAGNIFYING*0.25f, -5);
					glVertex3f(108 + f*MAGNIFYING, -10 - f*MAGNIFYING*0.125f, -5);
					glVertex3f(100 + f*MAGNIFYING, 0, -5);
					glVertex3f(108 + f*MAGNIFYING, 10 + f*MAGNIFYING*0.125f, -5);
					glVertex3f(110 + f*MAGNIFYING, 25 + f*MAGNIFYING*0.25f, -5);
					glVertex3f(105 + f*MAGNIFYING, 40 + f*MAGNIFYING*0.375f, -5);
					glVertex3f(95 + f*MAGNIFYING, 50 + f*MAGNIFYING*0.5f, -5);
					glVertex3f(80 + f*MAGNIFYING*0.75, 52 + f*MAGNIFYING*0.5, -5);
					glEnd();
				} // for

				glPopMatrix();
			} // if

		} 

		ec_l.Instance(enemy_cars);

		ec_l.Rewind();

		while (ec_l.Iterate(ec)) {
			if (pc->car->state != 2) {
				glPushMatrix();
				glTranslatef(ec->car->get_x() - v->c_x, ec->car->get_y() - v->c_y, 0);
				glRotatef(ec->car->get_a() - 90, 0, 0, 1);

				for (i = 0;i < 16;i++) {
					float f = i * 0.0625f;
					float MAGNIFYING = 24.0f;
					glBegin(GL_TRIANGLE_FAN);
					glVertex3f(10, 8 + f*4, -4);
					glVertex3f(10, -8 + f*4, -4);

					glVertex3f(40 + f*MAGNIFYING*0.75, -26 - f*MAGNIFYING*0.5f, -5);
					glVertex3f(48 + f*MAGNIFYING, -25 - f*MAGNIFYING*0.5f, -5);
					glVertex3f(53 + f*MAGNIFYING, -20 - f*MAGNIFYING*0.375f, -5);
					glVertex3f(55 + f*MAGNIFYING, -13 - f*MAGNIFYING*0.25f, -5);
					glVertex3f(54 + f*MAGNIFYING, -5 - f*MAGNIFYING*0.125f, -5);
					glVertex3f(50 + f*MAGNIFYING, 0, -5);
					glVertex3f(54 + f*MAGNIFYING, 5 + f*MAGNIFYING*0.125f, -5);
					glVertex3f(55 + f*MAGNIFYING, 13 + f*MAGNIFYING*0.25f, -5);
					glVertex3f(53 + f*MAGNIFYING, 20 + f*MAGNIFYING*0.375f, -5);
					glVertex3f(48 + f*MAGNIFYING, 25 + f*MAGNIFYING*0.5f, -5);
					glVertex3f(40 + f*MAGNIFYING*0.75, 26 + f*MAGNIFYING*0.5, -5);
					glEnd();
				} // if

				glPopMatrix();
			} // if
		} 

		glPopMatrix();

	}

	/* Darken the non-illuminated region: */
	{
//		float darkness[]={0.5F,0.5F,0.25F};
//		float darkness[]={0.75f, 0.7f,0.6f,0.5f,0.2f, 0.15f,0.1f,0.05f,0.0f};
//		float darkness[]={0.4f, 0.35f,0.3f,0.25f,0.2f, 0.15f,0.1f,0.05f,0.0f};
//		float darkness[]={0.8f, 0.7f,0.6f,0.5f,0.4f, 0.3f,0.2f,0.1f,0.0f};
		int i;
		glNormal3f(0.0, 0.0, 1.0);

		for (i = 32;i >= 0;i--) {
			glColor4f(0, 0, 0, (32 - i)*0.025f);
			glStencilFunc( GL_EQUAL, i, 0xFF );
			glStencilOp( GL_KEEP, GL_KEEP, GL_KEEP );
			glBegin(GL_QUADS);
			glVertex3f(-1000, -1000, -4);
			glVertex3f(-1000, 1000, -4);
			glVertex3f(1000, 1000, -4);
			glVertex3f(1000, -1000, -4);
			glEnd();
		} // if
	}

	glPopAttrib();
} 


void F1SpiritGame::draw_rain(PlayerCCar *v, float rain_strength)
{
	float v1 = 1.5f, v2 = 3.0f;
	float f;
	RainDrop *rd;

	glPushMatrix();
	glScalef(v->c_z, v->c_z, v->c_z);

	glNormal3f(0.0, 0.0, 1.0);

#ifdef __amigaos__
	/* AmiCradle: the drops on the screen only (they fall over a square
	 * 1400 pixels across round the car), in two glDrawArrays calls rather
	 * than a glBegin/glEnd each (amiga/glbatch.h): heavy rain starts 800
	 * drops a cycle, some 20,000 drops and splashes at once. The splashes
	 * on the ground go first and the falling drops over them, rather than
	 * mixed in the list's order. The points are written straight into the
	 * batch's arrays. */
	{
		CullPlanes cull;
		const RainDrops *rd = &m_rain;
		const float *X = rd->x, *Y = rd->y;
		const short *Z = rd->z;
		float cx = v->c_x, cy = v->c_y;
		float ox = v->c_old_x, oy = v->c_old_y;
		int i, n = rd->n;
		float sb[4], db[4], db2[4];
		bool sbox, dbox;

		cull_setup(&cull);
		/* boxes round what can be seen at ground level (the splashes) and
		 * from 200 up to it (the drops): four comparisons a drop rather
		 * than the four planes; what is in the box but off the screen is
		 * drawn and clipped, as it would be without the test */
		sbox = cull_box(&cull, 0, 4, sb);
		dbox = cull_box(&cull, 0, 64, db) && cull_box(&cull, -200, 64, db2);

		if (dbox) {
			if (db2[0] < db[0]) db[0] = db2[0];
			if (db2[1] < db[1]) db[1] = db2[1];
			if (db2[2] > db[2]) db[2] = db2[2];
			if (db2[3] > db[3]) db[3] = db2[3];
		}

		/* to the track's coordinates, as the drops are kept */
		sb[0] += cx; sb[2] += cx; sb[1] += cy; sb[3] += cy;
		db[0] += cx; db[2] += cx; db[1] += cy; db[3] += cy;

		glb_begin();

		/* the splashes, each a six-point fan as four triangles */
		for (i = 0;i < n;i++) {
			int z = Z[i];
			float px, py, a;
			GLfloat *p, *c;
			int j;

			if (z < 0 || z >= 400)
				continue;

			if (sbox && (X[i] < sb[0] || X[i] > sb[2] || Y[i] < sb[1] || Y[i] > sb[3]))
				continue;

			px = X[i] - cx;
			py = Y[i] - cy;

			if (!sbox && !cull_visible(&cull, px, py, 0, 4))
				continue;

			p = glb_reserve(GL_TRIANGLES, 0, 12, &c);

			if (!p)
				break;

			f = z / 500.0f;
			v1 = 0.5f * (f + 0.5f);
			v2 = 1.5f * (f + 0.5f);
			a = (1.0f - f) * 0.5f;

			{
				float fx[6] = { px, px + v2, px + v2, px, px - v2, px - v2 };
				float fy[6] = { py - v2, py - v1, py + v1, py + v2, py + v1, py - v1 };

				for (j = 1;j < 5;j++, p += 9) {
					p[0] = fx[0];     p[1] = fy[0];     p[2] = 0;
					p[3] = fx[j];     p[4] = fy[j];     p[5] = 0;
					p[6] = fx[j + 1]; p[7] = fy[j + 1]; p[8] = 0;
				}
			}

			if (a > 1) a = 1;
			if (a < 0) a = 0;

			for (j = 0;j < 12;j++, c += 4) {
				c[0] = 0.05f;
				c[1] = 0.05f;
				c[2] = 0.1f;
				c[3] = a;
			}
		}

		glb_flush();

		/* the falling drops, a line each */
		for (i = 0;i < n;i++) {
			int z = Z[i];
			float px, py;
			GLfloat *p, *c;
			int j;

			if (z >= 0)
				continue;

			if (dbox && (X[i] < db[0] || X[i] > db[2] || Y[i] < db[1] || Y[i] > db[3]))
				continue;

			px = X[i] - cx;
			py = Y[i] - cy;

			if (!dbox && !cull_visible(&cull, px, py, float(z), 64))
				continue;

			p = glb_reserve(GL_LINES, 0, 2, &c);

			if (!p)
				break;

			p[0] = px;
			p[1] = py;
			p[2] = float(z);
			p[3] = X[i] - 2 * 2 - ox;	/* rd->x - rd->inc_x*2 - v->c_old_x */
			p[4] = Y[i] - 2 * 2 - oy;
			p[5] = float(z - 40);

			for (j = 0;j < 2;j++, c += 4) {
				c[0] = 0.75f;
				c[1] = 0.75f;
				c[2] = 1.0f;
				c[3] = 0.5f;
			}
		}

		glb_flush();
		glb_end();
	}
#else
	m_rain_drops.Rewind();

	while (m_rain_drops.Iterate(rd)) {
		if (rd->z < 0) {
			// Draws a rain drop:
			glColor4f(0.75f, 0.75f, 1.0f, 0.5f);
			glBegin(GL_LINES);
			glVertex3f(rd->x - v->c_x, rd->y - v->c_y, rd->z);
			glVertex3f(rd->x - rd->inc_x*2 - v->c_old_x, rd->y - rd->inc_y*2 - v->c_old_y, rd->z - rd->inc_z*2);
			glEnd();
		} else {
			// Rain splash
			f = rd->z / 500.0f;

			v1 = 0.5f * (f + 0.5f);
			v2 = 1.5f * (f + 0.5f);

			f = (1.0f - f) * 0.5f;
			glColor4f(0.05f, 0.05f, 0.1f, f);

			glBegin(GL_TRIANGLE_FAN);

			glVertex3f(rd->x - v->c_x, rd->y - v->c_y - v2, 0);
			glVertex3f(rd->x - v->c_x + v2, rd->y - v->c_y - v1, 0);
			glVertex3f(rd->x - v->c_x + v2, rd->y - v->c_y + v1, 0);
			glVertex3f(rd->x - v->c_x, rd->y - v->c_y + v2, 0);
			glVertex3f(rd->x - v->c_x - v2, rd->y - v->c_y + v1, 0);
			glVertex3f(rd->x - v->c_x - v2, rd->y - v->c_y - v1, 0);

			glEnd();
		} // if
	} // while
#endif

	glPopMatrix();

	glNormal3f(0.0, 0.0, 1.0);

	glColor4f(0, 0, 0, rain_strength*0.25f);

	glBegin(GL_QUADS);

	glVertex3f(-1000, -1000, -4);

	glVertex3f(-1000, 1000, -4);

	glVertex3f(1000, 1000, -4);

	glVertex3f(1000, -1000, -4);

	glEnd();


} 


// quick random for rain:
#define RAIN_RANDOM_SIZE	99999
#define RAIN_DROPS_PER_CYCLE	800
#define RAIN_DROPS_SPAN	700

bool quick_rain_random_generated = false;
float quick_random_a[RAIN_RANDOM_SIZE];
float quick_random_b[RAIN_RANDOM_SIZE];
int quick_rain_random_index = 0;

RainDrop *to_delete_buffer[RAIN_DROPS_PER_CYCLE*10];
int to_delete_index;



#ifdef __amigaos__
bool RainDrops::grow(int want)
{
	float *nx, *ny;
	short *nz;
	int *ne;
	int r = room ? room : 1024;

	if (want <= room)
		return true;

	while (r < want)
		r *= 2;

	nx = new float[r];
	ny = new float[r];
	nz = new short[r];
	ne = new int[r];

	if (n > 0) {
		memcpy(nx, x, n * sizeof(float));
		memcpy(ny, y, n * sizeof(float));
		memcpy(nz, z, n * sizeof(short));
	}

	delete []x;
	delete []y;
	delete []z;
	delete []ended;
	x = nx;
	y = ny;
	z = nz;
	ended = ne;
	room = r;
	return true;
}


/* AmiCradle: upstream's cycle_rain (below) on the arrays of RainDrops: the
 * same drops, in the same order, given the same random numbers, at a
 * fraction of the cost (each drop was an object on a linked list). Drops
 * that end are reused for the new ones last first, as upstream does; the
 * rest are dropped from the arrays in the next cycle's pass. */
void F1SpiritGame::cycle_rain(PlayerCCar *v, float rain_strength)
{
	int i, r, w, nended;
	RainDrops *rd = &m_rain;
	float *X, *Y;
	short *Z;
	PlayerCCar *pc;
	List<PlayerCCar> pc_l;

	if (!quick_rain_random_generated) {
		for (i = 0;i < RAIN_RANDOM_SIZE;i++) {
			quick_random_a[i] = rg->IRandom(0, RAIN_DROPS_SPAN * 2);
			quick_random_b[i] = rg->IRandom(0, 20);
		} // for

		quick_rain_random_generated = true;
	} // if

	X = rd->x;
	Y = rd->y;
	Z = rd->z;
	nended = 0;

	for (r = w = 0;r < rd->n;r++) {
		float x, y;
		int z = Z[r];

		if (z >= 400)
			continue;	/* ended last cycle and not reused */

		x = X[r];
		y = Y[r];
		z += 20;

		if (z < 0) {
			x += 2;
			y += 2;
		} // if

		if (z == 0) {
			z = int(quick_random_b[quick_rain_random_index++] * 20);

			if (quick_rain_random_index >= RAIN_RANDOM_SIZE) quick_rain_random_index = 0;
		} // if

		X[w] = x;
		Y[w] = y;
		Z[w] = z;

		if (z >= 400)
			rd->ended[nended++] = w;

		w++;
	} // for

	rd->n = w;

	pc_l.Instance(player_cars);
	pc_l.Rewind();

	while (pc_l.Iterate(pc)) {
		for (i = 0;i < RAIN_DROPS_PER_CYCLE*rain_strength;i++) {
			int s;

			if (nended > 0) {
				// Reuse a drop that ended this cycle:
				s = rd->ended[--nended];
			} else {
				if (!rd->grow(rd->n + 1))
					return;

				s = rd->n++;
			} // if

			rd->x[s] = pc->car->get_x() + quick_random_a[quick_rain_random_index++] - RAIN_DROPS_SPAN;

			if (quick_rain_random_index >= RAIN_RANDOM_SIZE) quick_rain_random_index = 0;

			rd->y[s] = pc->car->get_y() + quick_random_a[quick_rain_random_index++] - RAIN_DROPS_SPAN;

			if (quick_rain_random_index >= RAIN_RANDOM_SIZE) quick_rain_random_index = 0;

			rd->z[s] = -200;
		} // for
	} // while

	/* the drops still in ended[] keep z >= 400: draw_rain skips them and
	 * the next cycle's pass leaves them out */
}
#else
void F1SpiritGame::cycle_rain(PlayerCCar *v, float rain_strength)
{
	int i;
	RainDrop *rd;
	PlayerCCar *pc;
	List<PlayerCCar> pc_l;

	if (!quick_rain_random_generated) {
		for (i = 0;i < RAIN_RANDOM_SIZE;i++) {
			quick_random_a[i] = rg->IRandom(0, RAIN_DROPS_SPAN * 2);
			quick_random_b[i] = rg->IRandom(0, 20);
		} // for

		quick_rain_random_generated = true;
	} // if

	to_delete_index = 0;

	m_rain_drops.Rewind();

	while (m_rain_drops.Iterate(rd)) {
		rd->z += rd->inc_z;

		if (rd->z < 0) {
			rd->x += rd->inc_x;
			rd->y += rd->inc_y;
		} // if

		if (rd->z == 0) {
			rd->z = quick_random_b[quick_rain_random_index++] * 20;

			if (quick_rain_random_index >= RAIN_RANDOM_SIZE) quick_rain_random_index = 0;
		} // if

		if (rd->z >= 400) {
			to_delete_buffer[to_delete_index++] = rd;
		} // if
	} // while

	pc_l.Instance(player_cars);

	pc_l.Rewind();

	while (pc_l.Iterate(pc)) {
		for (i = 0;i < RAIN_DROPS_PER_CYCLE*rain_strength;i++) {
			if (to_delete_index > 0) {
				// Reuse a previous drop if available:
				rd = to_delete_buffer[--to_delete_index];
			} else {
				rd = new RainDrop();
				m_rain_drops.Add(rd);
			} // if

			rd->x = pc->car->get_x() + quick_random_a[quick_rain_random_index++] - RAIN_DROPS_SPAN;

			if (quick_rain_random_index >= RAIN_RANDOM_SIZE) quick_rain_random_index = 0;

			rd->y = pc->car->get_y() + quick_random_a[quick_rain_random_index++] - RAIN_DROPS_SPAN;

			if (quick_rain_random_index >= RAIN_RANDOM_SIZE) quick_rain_random_index = 0;

			rd->z = -200;

			rd->inc_x = 2;

			rd->inc_y = 2;

			rd->inc_z = 20;
		} // for
	} // while


	while (to_delete_index > 0) {
		rd = to_delete_buffer[--to_delete_index];
		m_rain_drops.DeleteElement(rd);
		delete rd;
	} // while
} 
#endif

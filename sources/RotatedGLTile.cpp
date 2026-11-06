#ifdef _WIN32
#include "windows.h"
#endif

#include "GL/gl.h"
#include "GL/glu.h"
#include "SDL.h"
#include "SDL_image.h"

#include "auxiliar.h"
#include "2DCMC.h"
#include "GLTile.h"
#include "RotatedGLTile.h"
#include "SDL_glutaux.h"

#ifdef KITSCHY_DEBUG_MEMORY
#include "debug_memorymanager.h"
#endif

CRotatedGLTile::CRotatedGLTile(float ax, float ay, float az, float aa1, float aa2, GLTile *t)
{
	x = ax;
	y = ay;
	z = az;
	angle1 = aa1;
	angle2 = aa2;
	tile = t;
	different_color = false;


} 


CRotatedGLTile::~CRotatedGLTile()
{
	tile = 0;
} 


void CRotatedGLTile::draw(void)
{
	if (angle2 != 0) {
		glPushMatrix();
		glTranslatef(x, y, z);

		if (angle1 != 0)
			glRotatef(angle1, 0, 0, 1);

		glRotatef(angle2, 0, 1, 0);

		if (different_color)
			tile->draw(r, g, b, a);
		else
			tile->draw();

		glPopMatrix();
	} else {
		if (different_color)
			tile->draw(r, g, b, a, x, y, z, 0, 1);
		else
			tile->draw(x, y, z, 0, 1);
	} 
} 


void CRotatedGLTile::draw(float dx, float dy, float dz, float pangle, float zoom)
{
	if (tile != 0) {
#ifdef __amigaos__
		/* As below: the turn about z only when angle1 is not 0. Batched
		 * only in white: the race's lighting (ambient 1) makes a white tile
		 * white whichever way its normal turns, but not a coloured one. */
		if (angle2 != 0 && !different_color &&
		        tile->batch(1, 1, 1, 1, x + dx, y + dy, z + dz, angle1 != 0 ? angle1 + pangle : 0, angle2, zoom))
			return;

#endif
		if (angle2 != 0) {
			glPushMatrix();
			glTranslatef(x + dx, y + dy, z + dz);

			if (angle1 != 0)
				glRotatef(angle1 + pangle, 0, 0, 1);

			glRotatef(angle2, 0, 1, 0);

			if (different_color)
				tile->draw(r, g, b, a, 0, 0, 0, 0, zoom);
			else
				tile->draw(0, 0, 0, 0, zoom);

			glPopMatrix();
		} else {
			if (different_color)
				tile->draw(r, g, b, a, x + dx, y + dy, z + dz, angle1 + pangle, zoom);
			else
				tile->draw(x + dx, y + dy, z + dz, angle1 + pangle, zoom);
		} 
	} 
} 






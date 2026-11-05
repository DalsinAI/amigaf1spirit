/* glu.c: gluPerspective and gluLookAt, as the GLU specification gives
 * them (see amiga/include/GL/glu.h).
 * Copyright (C) 2026 AmiCradle Team. GPL 2 or later, like F-1 Spirit. */
#include <math.h>
#include <GL/gl.h>
#include "GL/glu.h"

void gluPerspective(GLdouble fovy, GLdouble aspect, GLdouble zNear, GLdouble zFar)
{
	GLdouble ymax = zNear * tan(fovy * M_PI / 360.0);
	GLdouble xmax = ymax * aspect;
	glFrustum(-xmax, xmax, -ymax, ymax, zNear, zFar);
}

static void normalize(GLdouble v[3])
{
	GLdouble l = sqrt(v[0] * v[0] + v[1] * v[1] + v[2] * v[2]);
	if (l > 0.0) {
		v[0] /= l; v[1] /= l; v[2] /= l;
	}
}

static void cross(const GLdouble a[3], const GLdouble b[3], GLdouble r[3])
{
	r[0] = a[1] * b[2] - a[2] * b[1];
	r[1] = a[2] * b[0] - a[0] * b[2];
	r[2] = a[0] * b[1] - a[1] * b[0];
}

void gluLookAt(GLdouble eyex, GLdouble eyey, GLdouble eyez,
               GLdouble centerx, GLdouble centery, GLdouble centerz,
               GLdouble upx, GLdouble upy, GLdouble upz)
{
	GLdouble f[3], up[3], s[3], u[3], m[16];
	f[0] = centerx - eyex; f[1] = centery - eyey; f[2] = centerz - eyez;
	up[0] = upx; up[1] = upy; up[2] = upz;
	normalize(f);
	cross(f, up, s);
	normalize(s);
	cross(s, f, u);
	m[0] = s[0]; m[4] = s[1]; m[8] = s[2];  m[12] = 0.0;
	m[1] = u[0]; m[5] = u[1]; m[9] = u[2];  m[13] = 0.0;
	m[2] = -f[0]; m[6] = -f[1]; m[10] = -f[2]; m[14] = 0.0;
	m[3] = 0.0; m[7] = 0.0; m[11] = 0.0; m[15] = 1.0;
	glMultMatrixd(m);
	glTranslated(-eyex, -eyey, -eyez);
}

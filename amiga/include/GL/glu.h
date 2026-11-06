/* GL/glu.h: the kit has no GLU. F-1 Spirit calls two of its functions,
 * gluPerspective and gluLookAt (sources/F1SpiritApp.cpp), which
 * amiga/glu.c makes from glFrustum and glMultMatrixd as GLU does.
 * Copyright (C) 2026 AmiCradle Team. GPL 2 or later, like F-1 Spirit. */
#ifndef AMIGA_GLU_H
#define AMIGA_GLU_H
#include <GL/gl.h>
#ifdef __cplusplus
extern "C" {
#endif
void gluPerspective(GLdouble fovy, GLdouble aspect, GLdouble zNear, GLdouble zFar);
void gluLookAt(GLdouble eyex, GLdouble eyey, GLdouble eyez,
               GLdouble centerx, GLdouble centery, GLdouble centerz,
               GLdouble upx, GLdouble upy, GLdouble upz);
#ifdef __cplusplus
}
#endif
#endif

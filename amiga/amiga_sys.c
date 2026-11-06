/* amiga_sys.c: F-1 Spirit's AmigaOS odds and ends.
 *
 * The game reads its data by paths relative to the current drawer
 * ("graphics/...", "tracks/..."), as it does on the PC when started in its
 * folder. From the Workbench that is the program's drawer already; from a
 * Shell it may not be. So before main() the program's drawer (PROGDIR:)
 * becomes the current drawer (amiga_init(), the first thing main() does),
 * and at exit the old one is put back.
 *
 * GFOpenGPU's SDL2.module can leave empty "SDL" screens behind after a
 * full-screen window; at exit (after SDL_Quit) every new screen with no
 * window and SDL's title is closed, or sent behind the Workbench if
 * Intuition refuses (as in the Abbaye and Seven Kingdoms ports).
 *
 * A Shell gives 4 KB of stack; the game's C++ wants more (libGL.a gives
 * its GL calls a stack of their own). $VER for Version.
 *
 * Copyright (C) 2026 AmiCradle Team. GPL 2 or later, like F-1 Spirit. */

#include <exec/types.h>
#include <dos/dos.h>
#include <intuition/intuition.h>
#include <intuition/intuitionbase.h>
#include <intuition/screens.h>
#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/intuition.h>
#include <stdlib.h>
#include <string.h>

extern struct IntuitionBase *IntuitionBase;

unsigned long __stack = 512 * 1024;
static const char version[] __attribute__((used)) = "$VER: F1Spirit 0.rc9.1615 Amiga (05.11.2026)";

static BPTR progdir_lock = 0;
static BPTR old_dir = 0;
static int dir_changed = 0;

#define MAX_KNOWN 32
static struct Screen *known[MAX_KNOWN];
static int known_count = -1;

static int was_known(struct Screen *s)
{
	int i;
	for (i = 0; i < known_count; i++)
		if (known[i] == s)
			return 1;
	return 0;
}

static void amiga_cleanup(void)
{
	struct Screen *left[MAX_KNOWN];
	int n = 0, i;
	struct Screen *s;
	ULONG lock;

	if (known_count >= 0) {
		lock = LockIBase(0);
		for (s = IntuitionBase->FirstScreen; s && n < MAX_KNOWN; s = s->NextScreen)
			if (!was_known(s) && !s->FirstWindow && (s->Flags & SCREENTYPE) != WBENCHSCREEN
			    && s->Title && strcmp((const char *)s->Title, "SDL") == 0)
				left[n++] = s;
		UnlockIBase(lock);
		for (i = 0; i < n; i++)
			if (!CloseScreen(left[i]))
				ScreenToBack(left[i]);
		if (n)
			WBenchToFront();
	}

	if (dir_changed) {
		CurrentDir(old_dir);
		dir_changed = 0;
	}
	if (progdir_lock) {
		UnLock(progdir_lock);
		progdir_lock = 0;
	}
}

void amiga_init(void)
{
	struct Screen *s;
	ULONG lock;

	progdir_lock = Lock((CONST_STRPTR)"PROGDIR:", ACCESS_READ);
	if (progdir_lock) {
		old_dir = CurrentDir(progdir_lock);
		dir_changed = 1;
	}

	lock = LockIBase(0);
	known_count = 0;
	for (s = IntuitionBase->FirstScreen; s && known_count < MAX_KNOWN; s = s->NextScreen)
		known[known_count++] = s;
	UnlockIBase(lock);

	/* registered before SDL's own exit work, so it runs after it */
	atexit(amiga_cleanup);
}

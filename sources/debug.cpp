#ifdef _WIN32
#include "windows.h"
#endif

#include <stdio.h>
#include <stdarg.h>

#include "filehandling.h"

#ifdef KITSCHY_DEBUG_MEMORY
#include "debug_memorymanager.h"
#endif

FILE *fp = 0;

void output_debug_message(const char *fmt, ...)
{
	char text[256];
	va_list ap;

	if (fmt == 0)
		return ;

#if defined(__amigaos__) && !defined(F1SPIRIT_AMIGA_DEBUG)
	/* AmigaOS: no log unless built with -DF1SPIRIT_AMIGA_DEBUG. It is
	   flushed to disk at every message, even from the audio callback (the
	   car engine's effect, many times a second), which slowed the game to
	   a crawl in the lab. */
	return ;
#endif

	va_start(ap, fmt);

	vsnprintf(text, sizeof(text), fmt, ap);

	va_end(ap);

	if (fp == 0)
		fp = f1open("f1spirit.dbg", "w", USERDATA);

	if (fp == 0)
		return ;

	fputs(text, fp);

	fflush(fp);
} /* glprintf */




void close_debug_messages(void)
{
	if (fp != 0)
		fclose(fp);
	fp = 0;
} /* close_debug_messages */

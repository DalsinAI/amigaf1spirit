/* F1Shttp_amiga.cpp: upstream's F1Shttp (sources/F1Shttp.cpp) fetched the
 * on-line hi-score pages with libcurl from braingames.getput.com, which
 * is gone. This port has no libcurl and no network: an F1Shttp never gets
 * a page (get_web() stays 0), so the on-line hi-score menus wait until
 * the player leaves them, as they would with the servers down.
 * Copyright (C) 2026 AmiCradle Team. GPL 2 or later, like F-1 Spirit. */

#include <stdlib.h>
#include <string.h>

#include "curl/curl.h"
#include "F1Shttp.h"

F1Shttp::F1Shttp(char *p_url)
{
	curl_handle = 0;
	thread = 0;
	finished = false;
	web = 0;
	size = 0;
	current_server = 0;
	url = new char[strlen(p_url) + 1];
	strcpy(url, p_url);
}

F1Shttp::~F1Shttp()
{
	delete []url;
	url = 0;
}

char *F1Shttp::get_web(void)
{
	return 0;
}

/* SDL_net.h: SDL_net 1.2's interface as F-1 Spirit uses it, with no
 * network behind it. GFOpenGPU's kit has SDL2_net only for SDL 2 programs;
 * this port runs on SDL 1.2 (sdl12-compat), so network play is left out:
 * SDLNet_Init() fails, the game then sets network = false and offers no
 * network games. Every other call fails too, harmlessly, should one be
 * made. The byte-order helpers are real (SDL_net's own definitions).
 * Copyright (C) 2026 AmiCradle Team. GPL 2 or later, like F-1 Spirit. */
#ifndef AMIGA_SDL_NET_H
#define AMIGA_SDL_NET_H

#include "SDL.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	Uint32 host;
	Uint16 port;
} IPaddress;

#define INADDR_ANY       0x00000000
#define INADDR_NONE      0xFFFFFFFF
#define INADDR_BROADCAST 0xFFFFFFFF

typedef struct _TCPsocket *TCPsocket;
typedef struct _UDPsocket *UDPsocket;
typedef struct _SDLNet_SocketSet *SDLNet_SocketSet;
typedef struct {
	int ready;
} *SDLNet_GenericSocket;

typedef struct {
	int channel;
	Uint8 *data;
	int len;
	int maxlen;
	int status;
	IPaddress address;
} UDPpacket;

static __inline__ int SDLNet_Init(void) { SDL_SetError("no network in this port"); return -1; }
static __inline__ void SDLNet_Quit(void) {}
static __inline__ const char *SDLNet_GetError(void) { return SDL_GetError(); }
static __inline__ int SDLNet_ResolveHost(IPaddress *a, const char *h, Uint16 p)
	{ (void)a; (void)h; (void)p; return -1; }

static __inline__ TCPsocket SDLNet_TCP_Open(IPaddress *ip) { (void)ip; return 0; }
static __inline__ TCPsocket SDLNet_TCP_Accept(TCPsocket s) { (void)s; return 0; }
static __inline__ IPaddress *SDLNet_TCP_GetPeerAddress(TCPsocket s) { (void)s; return 0; }
static __inline__ int SDLNet_TCP_Send(TCPsocket s, const void *d, int l)
	{ (void)s; (void)d; (void)l; return 0; }
static __inline__ int SDLNet_TCP_Recv(TCPsocket s, void *d, int m)
	{ (void)s; (void)d; (void)m; return -1; }
static __inline__ void SDLNet_TCP_Close(TCPsocket s) { (void)s; }

static __inline__ UDPsocket SDLNet_UDP_Open(Uint16 p) { (void)p; return 0; }
static __inline__ void SDLNet_UDP_Close(UDPsocket s) { (void)s; }
static __inline__ int SDLNet_UDP_Send(UDPsocket s, int c, UDPpacket *p)
	{ (void)s; (void)c; (void)p; return 0; }
static __inline__ int SDLNet_UDP_Recv(UDPsocket s, UDPpacket *p)
	{ (void)s; (void)p; return 0; }

static __inline__ UDPpacket *SDLNet_AllocPacket(int size)
{
	UDPpacket *p = (UDPpacket *)SDL_malloc(sizeof(UDPpacket));
	if (!p) return 0;
	p->data = (Uint8 *)SDL_malloc(size > 0 ? size : 1);
	if (!p->data) { SDL_free(p); return 0; }
	p->channel = -1; p->len = 0; p->maxlen = size; p->status = 0;
	p->address.host = 0; p->address.port = 0;
	return p;
}
static __inline__ void SDLNet_FreePacket(UDPpacket *p) { if (p) { SDL_free(p->data); SDL_free(p); } }

static __inline__ SDLNet_SocketSet SDLNet_AllocSocketSet(int n) { (void)n; return 0; }
static __inline__ void SDLNet_FreeSocketSet(SDLNet_SocketSet s) { (void)s; }
static __inline__ int SDLNet_AddSocket(SDLNet_SocketSet s, void *sock) { (void)s; (void)sock; return -1; }
static __inline__ int SDLNet_DelSocket(SDLNet_SocketSet s, void *sock) { (void)s; (void)sock; return -1; }
#define SDLNet_TCP_AddSocket(set, sock) SDLNet_AddSocket(set, (void *)(sock))
#define SDLNet_UDP_AddSocket(set, sock) SDLNet_AddSocket(set, (void *)(sock))
#define SDLNet_TCP_DelSocket(set, sock) SDLNet_DelSocket(set, (void *)(sock))
#define SDLNet_UDP_DelSocket(set, sock) SDLNet_DelSocket(set, (void *)(sock))
static __inline__ int SDLNet_CheckSockets(SDLNet_SocketSet s, Uint32 t) { (void)s; (void)t; return 0; }
#define SDLNet_SocketReady(sock) 0

/* Network byte order (big-endian), as SDL_net's own. */
static __inline__ void SDLNet_Write16(Uint16 v, void *area)
	{ Uint8 *a = (Uint8 *)area; a[0] = (Uint8)(v >> 8); a[1] = (Uint8)v; }
static __inline__ void SDLNet_Write32(Uint32 v, void *area)
	{ Uint8 *a = (Uint8 *)area; a[0] = (Uint8)(v >> 24); a[1] = (Uint8)(v >> 16);
	  a[2] = (Uint8)(v >> 8); a[3] = (Uint8)v; }
static __inline__ Uint16 SDLNet_Read16(const void *area)
	{ const Uint8 *a = (const Uint8 *)area; return (Uint16)((a[0] << 8) | a[1]); }
static __inline__ Uint32 SDLNet_Read32(const void *area)
	{ const Uint8 *a = (const Uint8 *)area;
	  return ((Uint32)a[0] << 24) | ((Uint32)a[1] << 16) | ((Uint32)a[2] << 8) | a[3]; }

#ifdef __cplusplus
}
#endif

#endif

F-1 Spirit for AmigaOS 3.x
==========================

A port of F-1 Spirit 0.rc9 build 1615 (Brain Games' remake of Konami's
1987 MSX racing game, by Santi "Popolon" Ontanon and the Brain Games
team, from f1spirit.src_0.rc9-1615.tgz, SHA-256
fc1778dcec68ccd31b9da893abee67e64a7931a34c75a63f8563355b810acbd7) for
AmigaOS 3.2 on a 68040 or 68060 with an FPU and an RTG screen, on
GFOpenGPU's SDL 1.2 (sdl12-compat on SDL 2) and OpenGL.

Status: test build. It starts, the menus work and a race starts and
runs in the AmiCradle lab, but the race runs far below full speed (see
"Known limits"), so it is not playable yet.


What you need
-------------

- A 68040 or 68060 with an FPU (not an LC040 or EC040), AmigaOS 3.2, an
  RTG card (or an AmiCradle instance).
- GFOpenGPU with LIBS:GFOpenGPU/SDL2.module and GL.module.
- About 34 MB of disk, and Fast RAM to spare: the sound effects are
  decoded into memory when a race loads.

Everything the game needs is in the drawer: the program, its graphics,
sounds, music, tracks and demo replays, all under the game's GPL (see
Licences.txt).


Installing
----------

Unpack the archive and double-click Install-F1Spirit beside the F1Spirit
drawer. It checks the CPU and FPU, asks for the parent drawer (default
Work:Games) and copies the drawer there; on an update it leaves the
drawer's User drawer (your settings, players, hi-scores and replays)
alone. Copying the F1Spirit drawer by hand works as well.


How to play
-----------

Double-click F1Spirit in its drawer, or from a Shell:

    cd Work:Games/F1Spirit
    F1Spirit

It opens a 640x480 window on the Workbench. Space on the title screen
gives the main menu (New Game, Continue Game, Practice). The game's own
keys (ReadMe-f1spirit has the whole manual):

- Cursor left/right steer, cursor up/down shift gears.
- Space accelerates, M brakes.
- Escape pauses a race (Continue, Quit, Restart, the volumes and the
  players' keys) and goes back one menu elsewhere.
- Right Amiga+Q quits at once from anywhere, as does the window's close
  gadget. (F12, upstream's quit key, is not on an Amiga keyboard.)

The settings (f1spirit.cfg), the players, the hi-scores and the replays
are kept in the drawer's User drawer.


Building
--------

The os32-gcc16-open6 stove (GFOpenAmigaGCC 16.2.0-open6 with the
GFOpenGPU 0.8 kit):

    make -f amiga/Makefile.amigaos -j4 dist STOVE=/path/to/os32-gcc16-open6

STOVE defaults to ~/AmiCradle/stoves/os32-gcc16-open6. Output:
build/amigaos/f1spirit; 'dist' makes dist/F1Spirit/ (F1Spirit and its
data, ReadMe, ReadMe-f1spirit, COPYING.txt, Licences.txt, with
GlowIcons), dist/F1Spirit.info and dist/Install-F1Spirit with its icon.
The icons are made from amiga/icons/icons.json and the installer from
amiga/installer.json with AmiCradle's scripts/port_icons.py and
port_installer.py.


What was changed
----------------

F-1 Spirit is an SDL 1.2 and OpenGL game. GFOpenGPU has SDL 1.2 as
sdl12-compat with OpenGL, but no SDL_image, SDL_mixer or SDL_net for
1.2, no GLU and no libcurl, so the port brings what the game uses of
them:

- amiga/image.c: IMG_Load on stb_image 2.30 (PNG and JPEG) into 32-bit
  surfaces in the byte order the game's own masks expect, on both byte
  orders.
- amiga/mixer.c: the SDL_mixer 1.2 calls the game makes, on SDL audio:
  channels with volume, loops, fades and pause, effects (the car
  engine's pitch effect writes into a channel), and one Ogg music
  stream (stb_vorbis 1.22). Effects are decoded on load into 16-bit
  stereo at 22050 Hz.
- amiga/glu.c: gluPerspective and gluLookAt from glFrustum and
  glMultMatrixd, as GLU does.
- amiga/include/SDL_net.h and amiga/F1Shttp_amiga.cpp: no network. Net
  play turns itself off (SDLNet_Init fails); the on-line hi-score pages
  are never fetched (their servers are gone anyway).
- amiga/amiga_sys.c: PROGDIR: becomes the current drawer at start (the
  game reads its data by relative paths) and the old one is put back at
  exit; empty SDL screens left behind are closed at exit; 512 KB of
  stack and a $VER string.
- sources/filehandling.cpp: player data in PROGDIR:User/ rather than
  HOME.
- sources/main.cpp: Right Amiga+Q quits, read from the key event's own
  modifiers (the events can be drained a frame late, when the Amiga key
  is already up).
- sources/keyboardstate.cpp/h: a key pressed and let go between two
  game cycles still counts as held for one cycle (the game reads the
  key state once a cycle, so a quick tap in a slow frame was lost).
- sources/debug.cpp: no debug log unless built with
  -DF1SPIRIT_AMIGA_DEBUG (upstream writes and flushes a log line at
  every message, even from the audio callback, which stalled the game).
- amiga/Makefile.amigaos links SDL 1.2's OpenGL as sdl-config --gl
  does (-Wl,-u,_SDL12Amiga_gl).

Big-endian: the images are read through stb_image into bytes in R, G,
B, A order, the order the game's masks give on a big-endian machine;
the tracks, players and replays are text or byte files.


Known limits
------------

- Speed: everything is drawn with OpenGL, every frame. In the lab the
  title and menus draw at a few frames a second and answer the keys,
  but in a race a frame takes up to a second or more, and the race
  clock moved about one second in two to three minutes. The race is
  not playable at this speed. Where the time goes (GL.module's drawing
  of the many textured quads, or the game itself) is still to be
  measured.
- No network play and no on-line hi-scores (see above).
- The lab is muted, so the sound and music have not been heard.


Licence
-------

F-1 Spirit is under the GNU GPL (debian/copyright names no version, so
any version may be used; COPYING.txt is version 2). The port's files
are by the AmiCradle Team under the GPL, version 2 or later. The data
and the libraries built into the program are covered in Licences.txt.


Testing
-------

Tested in an AmiCradle lab (a copy of Instance-32: AmigaOS 3.2, 68040,
RTG; muted), 5 November 2026, with the build from a clean checkout:

- The title screen draws in its window; Space gives the main menu.
- Practice, then 1 Player, the Stock track (its preview and track
  information), the car selection (V8 2900): the race loaded, the grid
  with 54 cars, the HUD, the minimap and the rain drew, and the start
  lights went to green; the lap clock ran, but very slowly (see Known
  limits).
- Escape paused the race and showed the pause menu; Space continued.
- The window's close gadget quit to the Workbench with no screen left
  behind. Right Amiga+Q quit from the title screen, and
  User/f1spirit.cfg was written.

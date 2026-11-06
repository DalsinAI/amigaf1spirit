F-1 Spirit for AmigaOS 3.x
==========================

A port of F-1 Spirit 0.rc9 build 1615 (Brain Games' remake of Konami's
1987 MSX racing game, by Santi "Popolon" Ontanon and the Brain Games
team, from f1spirit.src_0.rc9-1615.tgz, SHA-256
fc1778dcec68ccd31b9da893abee67e64a7931a34c75a63f8563355b810acbd7) for
AmigaOS 3.2 on a 68040 or 68060 with an FPU and an RTG screen, on
GFOpenGPU's SDL 1.2 (sdl12-compat on SDL 2) and OpenGL.

Status: test build. It starts, the menus work, and a race runs and can
be driven in the AmiCradle lab, at full speed on a quiet host and about
60 per cent of it on a busy one (see "Speed").


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
- amiga/glbatch.cpp/h, sources/track.cpp, RoadPiece.cpp, GLTile.cpp/h,
  RotatedGLTile.cpp, weather.cpp, F1SpiritGame.cpp: the race is drawn
  with a few dozen glDrawArrays a frame rather than some 20,000
  glBegin/glEnd pairs, and what is off the screen is not drawn (see
  "Speed").

Big-endian: the images are read through stb_image into bytes in R, G,
B, A order, the order the game's masks give on a big-endian machine;
the tracks, players and replays are text or byte files.


Speed
-----

The game runs 50 cycles a second and draws when it can; when drawing
falls behind it runs up to ten cycles between two frames, and past
200 ms behind it lets the clock go. So a slow frame slows the race
itself, not only the picture.

Measured in the lab (a copy of Instance-32, 68040 on the AmiCradle
runtime, GL.module on the PC's graphics chip through virgl), 6 November
2026, on the Stock track in the rain, with a timing build that counted
the GL calls and timed the game cycle, the drawing and
SDL_GL_SwapBuffers apart:

| | before | after |
| --- | --- | --- |
| glBegin/glEnd a frame | 8,000 to 20,000 | 4 |
| glBindTexture a frame | 3,960 | about 32 |
| glEnable/glDisable a frame | 6,030 | 29 |
| glDrawArrays a frame | 0 | about 31 (22,600 points) |
| drawing a frame | 930 to 1,460 ms | 27 to 60 ms (most 5 s windows) |
| SDL_GL_SwapBuffers | 3 to 15 ms | 4 to 12 ms |
| game cycles in 5 s (250 is full speed) | 3 to 7 | 74 to 260 (mostly 150 to 240) |

Before, a frame drew some 20,000 quads one glBegin/glEnd at a time
(a minute into a race: tyre marks build up, and heavy rain is some
20,000 drops), each with its own glPushMatrix, glTranslatef,
glRotatef, glBindTexture, glColor4f and glNormal3f. Each GL call goes
through libGL.a into Mesa on the 68k, and each sprite with its own
matrix is a draw of its own in Mesa: about 50 to 80 microseconds a
quad. The game logic took 3 to 9 ms a cycle and the swap a few ms; the
drawing took the rest (so the earlier sound-off build was as slow).

Now:

- track.cpp: tyre marks, scenery, cars and clouds are tested against
  the view's four side planes (glbatch.h's CullPlanes, from the
  projection and modelview matrices as they are when the track is
  drawn) and only what can reach the screen is drawn. Upstream drew
  every car, cloud and tyre mark on the track.
- amiga/glbatch.cpp: sprites (GLTile, RotatedGLTile), road strips,
  tyre marks, rain, the HUD and the minimap are worked out on the CPU
  (the sprite's own translate, turn and scale) and collected into
  vertex arrays; one glDrawArrays draws each run with one texture, in
  the order upstream drew them. Small sprite pictures (up to 256x256)
  are copied, the second time they are drawn, into 512x512 atlas
  textures, so most sprites share a texture and a run. A road piece
  sends its road, its two sides and its lines as four runs. Coloured
  sprites turned about y are drawn as before: their lighting depends
  on the turn. The colours go as floats: GL.module on virgl turns
  byte colours into floats on the 68k at every draw, which halved
  glDrawArrays' time when it went (5.7 to 6.2 ms a frame down to 2.6
  to 3.7 ms in the lab's quieter windows).
- weather.cpp: rain drops off the screen are skipped; the splashes
  and the falling drops are two draws.

The picture is the same as before as far as the lab's screenshots
show (cars, crowd, trees, signs, tyre marks, rain, HUD, minimap).

With the drawing at 30 to 60 ms the game cycle is now the larger part:
with all 54 cars racing, 10 to 30 ms a cycle in the lab, mostly
CRoadPiece::get_path_position (the cars finding their place on the
road, about 10 per cent of the emulated CPU's time) and
F1SpiritGame::cycle_rain (moving some 20,000 rain drops, about 9 per
cent). The lab's host was heavily loaded during these runs (load
average 90 to 135 on 12 threads, against 25 for the "before" run), and
the emulation thread got only 35 to 55 per cent of a host CPU, against
75 to 83 per cent before; the race ran at 30 to 100 per cent of full
speed, mostly 60 to 95. On a quieter host it should come close to full
speed; that is not measured yet.

What is left in the drawing is mostly the port's own work on the CPU
(placing 22,600 points a frame, most of them rain) and GL.module's
handling of the arrays (2.6 ms a frame and up).

The second round went after the game cycle and the sound:

- RoadPiece.cpp: a car's nearest path point is found without trying
  every point (the points are at most a known step apart, so those
  that can't be nearer are skipped), starting from the one found last;
  the distances are worked out as upstream does, so the same point is
  found. Checked in the lab: a build that ran upstream's loop beside it
  found the same point in 3.9 million calls, and the three title
  demos (replays) gave the same car positions, bit for bit, for 4,680
  cycles with and without the change.
- weather.cpp: the rain drops are kept in arrays rather than a list of
  objects, the heights as whole numbers, and are written straight into
  the batch; the screen is tested as a box at the drops' heights. The
  same drops in the same order (checked against upstream's list in 4,373
  cycles of heavy rain), and the rain looks the same.
- amiga/mixer.c: SDL 2's audio is opened at 22050 Hz directly. Through
  SDL 1.2's SDL_OpenAudio (sdl12-compat) the device was opened at 44100
  Hz and all the sound resampled on the 68k: 6 to 23 per cent of the
  emulated CPU's time in a race, the more the busier the host. Falls back to SDL 1.2's call if SDL 2 won't give 22050.
- auxiliar.cpp, ReplayInfo.cpp, F1SpiritGame.cpp: replays (the title
  demos among them) are read in the PC's byte order, as they were
  made; read in the 68k's order the demo cars' positions were not
  numbers and the title screen hung once a demo started. The players'
  and hi-score files stay as earlier Amiga builds wrote them.
- main.cpp: no millisecond's sleep when the next cycle is due already.

Measured with the timing build in a race in the rain (Practice, Stock,
V8 2900, accelerator held, 100 s of race), before (main) and after
(this round) back to back, with the host's load average (12 threads):

| run | load | frames a second | cycles in 5 s (250 full) | emulated instructions a cycle |
| --- | --- | --- | --- | --- |
| before | 33 to 49 | 3.3 | 111 | 11.2 M |
| after | 42 to 50 | 7.0 | 157 | 8.2 M |
| before | 11 to 16 | 37.6 | 249 | 14.3 M |
| after | 10 to 18 | 44.5 | 250 | 13.4 M |
| before | 15 to 38 | 11.3 | 157 | 18.1 M |
| after | 20 to 23 | 33.4 | 249 | 11.0 M |

On a busy host the race now runs about 40 per cent faster than before
(the game clock is the measure: a cycle is 20 ms of race); on a quiet
host both reach full speed and the gain shows as frames (44.5 a
second against 37.6). The third pair's loads differ, the after run's the lower.
In the runtime's profile the nearest-point loop (8 per cent),
the rain (5 to 11 per cent) and SDL 2's resampler (9 to 23 per
cent) are gone; what is left is spread thin, nothing over about 4 per
cent (the rain's drawing, the cars' collisions, the music's
Vorbis decoding).

-O3 and -flto were tried and are not used: both reached full speed on
the quiet host, as -O2 does (39 and 40 frames a second against 33 to 45
for -O2, inside the runs' spread), and -O3 makes the program 7 per cent
larger.


Known limits
------------

- Speed: see "Speed". On a quiet host the race runs at full speed; on
  a busy one (load average 40 to 50 on 12 threads) it still falls to
  about 60 per cent, the emulation thread getting a third of a host
  CPU.
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

6 November 2026, the same lab, with the speed changes: Practice, the
Stock track, a race in the rain; the cars, crowd, trees, signs, tyre
marks, rain, HUD and minimap drew as before, the lap clock ran at about three quarters of real time on the busy
host, and the car was driven (Space held: it reached 218 km/h; the
cursor keys steered it; Escape paused the race). The track preview in the menus was drawn
in 30 to 40 ms rather than 290 to 390 ms. Right Amiga+Q quit from the
race.

6 November 2026, the same lab, the build from a clean checkout of the
second round (host load average 25 to 55): the title screen,
Practice, the Stock track, a race at full speed (the lap clock about
kept time with the wall clock), the car driven (Space held to
115 km/h, the cursor keys steering), Escape paused the race, Space
continued it, and Right Amiga+Q quit to the Workbench, with
User/f1spirit.cfg written.

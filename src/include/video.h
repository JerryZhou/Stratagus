//       _________ __                 __
//      /   _____//  |_____________ _/  |______     ____  __ __  ______
//      \_____  \\   __\_  __ \__  \\   __\__  \   / ___\|  |  \/  ___/
//      /        \|  |  |  | \// __ \|  |  / __ \_/ /_/  >  |  /\___ |
//     /_______  /|__|  |__|  (____  /__| (____  /\___  /|____//____  >
//             \/                  \/          \//_____/            \/
//  ______________________                           ______________________
//                        T H E   W A R   B E G I N S
//         Stratagus - A free fantasy real time strategy game engine
//
/**@name video.h - The video headerfile. */
//
//      (c) Copyright 1999-2005 by Lutz Sammer, Nehal Mistry, and Jimmy Salmon
//
//      This program is free software; you can redistribute it and/or modify
//      it under the terms of the GNU General Public License as published by
//      the Free Software Foundation; only version 2 of the License.
//
//      This program is distributed in the hope that it will be useful,
//      but WITHOUT ANY WARRANTY; without even the implied warranty of
//      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//      GNU General Public License for more details.
//
//      You should have received a copy of the GNU General Public License
//      along with this program; if not, write to the Free Software
//      Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
//      02111-1307, USA.
//
//      $Id$

#ifndef __VIDEO_H__
#define __VIDEO_H__

//@{

#include "SDL.h"

#ifdef USE_OPENGL
#define DrawIcon WinDrawIcon
#include "SDL_opengl.h"
#undef DrawIcon
#endif

class Graphic {
public:
	// Draw
	void DrawSub(int gx, int gy, int w, int h, int x, int y) const;
	void DrawSubClip(int gx, int gy, int w, int h, int x, int y) const;
	void DrawSubTrans(int gx, int gy, int w, int h, int x, int y,
		unsigned char alpha) const;
	void DrawSubClipTrans(int gx, int gy, int w, int h, int x, int y,
		unsigned char alpha) const;

	// Draw frame
	void DrawFrame(unsigned frame, int x, int y) const;
#ifdef USE_OPENGL
	void DoDrawFrameClip(GLuint *textures, unsigned frame, int x, int y) const;
#endif
	void DrawFrameClip(unsigned frame, int x, int y) const;
	void DrawFrameTrans(unsigned frame, int x, int y, int alpha) const;
	void DrawFrameClipTrans(unsigned frame, int x, int y, int alpha) const;
	void DrawPlayerColorFrameClip(int player, unsigned frame, int x, int y) const;

	// Draw frame flipped horizontally
	void DrawFrameX(unsigned frame, int x, int y) const;
#ifdef USE_OPENGL
	void DoDrawFrameClipX(GLuint* textures, unsigned frame, int x, int y) const;
#endif
	void DrawFrameClipX(unsigned frame, int x, int y) const;
	void DrawFrameTransX(unsigned frame, int x, int y, int alpha) const;
	void DrawFrameClipTransX(unsigned frame, int x, int y, int alpha) const;
	void DrawPlayerColorFrameClipX(int player, unsigned frame, int x, int y) const;


	void Load();
	void Flip();
	void Resize(int w, int h);
	int TransparentPixel(int x, int y);
	void MakeShadow();

	inline bool Loaded() { return Surface != NULL; }


	char *File;                /// Filename
	char *HashFile;            /// Filename used in hash
	SDL_Surface *Surface;      /// Surface
#ifndef USE_OPENGL
	SDL_Surface *SurfaceFlip;  /// Flipped surface
#endif
	int Width;                 /// Width of a frame
	int Height;                /// Height of a frame
	int NumFrames;             /// Number of frames
	int GraphicWidth;          /// Original graphic width
	int GraphicHeight;         /// Original graphic height
	int Refs;                  /// Uses of this graphic
#ifdef USE_OPENGL
	GLfloat TextureWidth;      /// Width of the texture
	GLfloat TextureHeight;     /// Height of the texture
	GLuint *Textures;          /// Texture names
	GLuint *PlayerColorTextures[PlayerMax];/// Textures with player colors
	int NumTextures;
#endif
};

#ifdef USE_MNG
#include <libmng.h>

class Mng {
public:
	Mng();
	~Mng();
	int Load(const char *name);
	void Reset();
	void Draw(int x, int y);

	char *name;
	FILE *fd;
	mng_handle handle;
	SDL_Surface *surface;
	unsigned char *buffer;
	unsigned long ticks;
	int iteration;
#ifdef USE_OPENGL
	GLfloat texture_width;   /// Width of the texture
	GLfloat texture_height;  /// Height of the texture
	GLuint texture_name;     /// Texture name
#endif
};
#endif

typedef struct _unit_colors_ {
	SDL_Color *Colors;
} UnitColors;

/**
**  Event call back.
**
**  This is placed in the video part, because it depends on the video
**  hardware driver.
*/
typedef struct _event_callback_ {

		/// Callback for mouse button press
	void (*ButtonPressed)(unsigned buttons);
		/// Callback for mouse button release
	void (*ButtonReleased)(unsigned buttons);
		/// Callback for mouse move
	void (*MouseMoved)(int x, int y);
		/// Callback for mouse exit of game window
	void (*MouseExit)(void);

		/// Callback for key press
	void (*KeyPressed)(unsigned keycode, unsigned keychar);
		/// Callback for key release
	void (*KeyReleased)(unsigned keycode, unsigned keychar);
		/// Callback for key repeated
	void (*KeyRepeated)(unsigned keycode, unsigned keychar);

		/// Callback for network event
	void (*NetworkEvent)(void);

} EventCallback;

class CVideo
{
public:
	CVideo() : Width(640), Height(480), Depth(0), FullScreen(false) {}

	void LockScreen();
	void UnlockScreen();

	int Width;
	int Height;
	int Depth;
	bool FullScreen;
};

extern CVideo Video;

	/**
	**  Video synchronization speed. Synchronization time in percent.
	**  If =0, video framerate is not synchronized. 100 is exact
	**  CYCLES_PER_SECOND (30). Game will try to redraw screen within
	**  intervals of VideoSyncSpeed, not more, not less.
	**  @see CYCLES_PER_SECOND
	*/
extern int VideoSyncSpeed;

extern int SkipFrames;

	/// Fullscreen or windowed set from commandline.
extern char VideoForceFullScreen;

	/// Next frame ticks
extern unsigned long NextFrameTicks;

	/// Counts frames
extern unsigned long FrameCounter;

	/// Counts quantity of slow frames
extern int SlowFrameCounter;

	/// Initialize Pixels[] for all players.
	/// (bring Players[] in sync with Pixels[])
extern void SetPlayersPalette(void);

	/**
	**  Architecture-dependant videomemory. Set by InitVideoXXX.
	**  FIXME: need a new function to set it, see #ifdef SDL code
	**  @see InitVideo @see InitVideoSdl
	**  @see VMemType
	*/
extern SDL_Surface *TheScreen;

#ifdef USE_OPENGL
	/// Max texture size supported on the video card
extern int GLMaxTextureSize;
#endif

#if SDL_BYTEORDER == SDL_BIG_ENDIAN
#define RMASK 0xff000000
#define GMASK 0x00ff0000
#define BMASK 0x0000ff00
#define AMASK 0x000000ff
#else
#define RMASK 0x000000ff
#define GMASK 0x0000ff00
#define BMASK 0x00ff0000
#define AMASK 0xff000000
#endif

	/// initialize the video part
extern void InitVideo(void);

	/// Check if a resolution is valid
extern int VideoValidResolution(int w, int h);

	/// Load graphic from PNG file
extern int LoadGraphicPNG(Graphic *g);

#ifdef USE_OPENGL
	/// Make an OpenGL texture
extern void MakeTexture(Graphic *graphic);
	/// Make an OpenGL texture of the player color pixels only.
extern void MakePlayerColorTexture(Graphic *graphic, int player);
#endif

#ifdef USE_OPENGL
	/// Reload OpenGL graphics
extern void ReloadGraphics(void);
#endif

	/// Initializes video synchronization.
extern void SetVideoSync(void);

	/// Clear video screen
extern void VideoClearScreen(void);

	/// Make graphic
extern Graphic *NewGraphic(const char *file, int w, int h);

	/// Make graphic
extern Graphic *ForceNewGraphic(const char *file, int w, int h);

	/// Free Graphic taking into account the number of uses.
extern void FreeGraphic(Graphic *g);

	/// Init line draw
extern void InitLineDraw(void);

	/// Simply invalidates whole window or screen.
extern void Invalidate(void);

	/// Invalidates selected area on window or screen. Use for accurate
	/// redrawing. in so
extern void InvalidateArea(int x, int y, int w, int h);

	/// Set clipping for nearly all vector primitives. Functions which support
	/// clipping will be marked Clip. Set the system-wide clipping rectangle.
extern void SetClipping(int left, int top, int right, int bottom);

	/// Realize video memory.
extern void RealizeVideoMemory(void);

	/// Save a screenshot to a PNG file
extern void SaveScreenshotPNG(const char* name);

	/// Process all system events. Returns if the time for a frame is over
extern void WaitEventsOneFrame(const EventCallback *callbacks);

	/// Toggle full screen mode
extern void ToggleFullScreen(void);

	/// Push current clipping.
extern void PushClipping(void);

	/// Pop current clipping.
extern void PopClipping(void);

	/// Returns the ticks in ms since start
extern unsigned long GetTicks(void);

	/// Toggle mouse grab mode
extern void ToggleGrabMouse(int mode);

extern EventCallback *Callbacks;    /// Current callbacks
extern EventCallback GameCallbacks; /// Game callbacks
extern EventCallback MenuCallbacks; /// Menu callbacks

extern Uint32 ColorBlack;
extern Uint32 ColorDarkGreen;
extern Uint32 ColorBlue;
extern Uint32 ColorOrange;
extern Uint32 ColorWhite;
extern Uint32 ColorGray;
extern Uint32 ColorRed;
extern Uint32 ColorGreen;
extern Uint32 ColorYellow;

#ifndef USE_OPENGL
#define VideoMapRGB(f, r, g, b) SDL_MapRGB((f), (r), (g), (b))
#define VideoMapRGBA(f, r, g, b, a) SDL_MapRGBA((f), (r), (g), (b), (a))
#define VideoGetRGB(c, f, r, g, b) SDL_GetRGB((c), (f), (r), (g), (b))
#define VideoGetRGBA(c, f, r, g, b, a) SDL_GetRGBA((c), (f), (r), (g), (b), (a))
#else
#define VideoMapRGB(f, r, g, b) VideoMapRGBA((f), (r), (g), (b), 0xff)
#define VideoMapRGBA(f, r, g, b, a) ((r) | ((g) << 8) | ((b) << 16) | ((a) << 24))
#define VideoGetRGB(c, r, g, b) { \
	*(r) = ((c) >> 0) & 0xff; \
	*(g) = ((c) >> 8) & 0xff; \
	*(b) = ((c) >> 16) & 0xff; }
#define VideoGetRGBA(c, r, g, b, a) { \
	*(r) = ((c) >> 0) & 0xff; \
	*(g) = ((c) >> 8) & 0xff; \
	*(b) = ((c) >> 16) & 0xff; \
	*(a) = ((c) >> 24) & 0xff; }
#endif

#ifdef USE_OPENGL
void DrawTexture(const Graphic *g, GLuint *textures, int sx, int sy,
	int ex, int ey, int x, int y, int flip);
#endif

#ifndef USE_OPENGL
	/// Draw pixel unclipped.
extern void (*VideoDrawPixel)(Uint32 color, int x, int y);

	/// Draw translucent pixel unclipped.
extern void (*VideoDrawTransPixel)(Uint32 color, int x, int y,
	unsigned char alpha);
#else
	/// Draw pixel unclipped.
extern void VideoDrawPixel(Uint32 color, int x, int y);

	/// Draw translucent pixel unclipped.
extern void VideoDrawTransPixel(Uint32 color, int x, int y,
	unsigned char alpha);
#endif

	/// Draw pixel clipped to current clip setting.
extern void VideoDrawPixelClip(Uint32 color, int x, int y);

	/// Draw translucent pixel clipped to current clip setting.
extern void VideoDrawTransPixelClip(Uint32 color, int x, int y,
	unsigned char alpha);

	/// Draw vertical line unclipped.
extern void VideoDrawVLine(Uint32 color, int x, int y,
	int height);

	/// Draw translucent vertical line unclipped.
extern void VideoDrawTransVLine(Uint32 color, int x, int y,
	int height, unsigned char alpha);

	/// Draw vertical line clipped to current clip setting
extern void VideoDrawVLineClip(Uint32 color, int x, int y,
	int height);

	/// Draw translucent vertical line clipped to current clip setting
extern void VideoDrawTransVLineClip(Uint32 color, int x, int y,
	int height, unsigned char alpha);

	/// Draw horizontal line unclipped.
extern void VideoDrawHLine(Uint32 color, int x, int y,
	int width);

	/// Draw translucent horizontal line unclipped.
extern void VideoDrawTransHLine(Uint32 color, int x, int y,
	int width, unsigned char alpha);

	/// Draw horizontal line clipped to current clip setting
extern void VideoDrawHLineClip(Uint32 color, int x, int y,
	int width);

	/// Draw translucent horizontal line clipped to current clip setting
extern void VideoDrawTransHLineClip(Uint32 color, int x, int y,
	int width, unsigned char alpha);

	/// Draw line unclipped.
extern void VideoDrawLine(Uint32 color, int sx, int sy, int dx, int dy);

	/// Draw translucent line unclipped.
extern void VideoDrawTransLine(Uint32 color, int sx, int sy, int dx, int dy,
	unsigned char alpha);

	/// Draw line clipped to current clip setting
extern void VideoDrawLineClip(Uint32 color, int sx, int sy, int dx, int dy);

	/// Draw translucent line clipped to current clip setting
extern void VideoDrawTransLineClip(Uint32 color, int sx, int sy,
	int dx, int dy, unsigned char alpha);

	/// Draw rectangle.
extern void VideoDrawRectangle(Uint32 color, int x, int y,
	int w, int h);

	/// Draw translucent rectangle.
extern void VideoDrawTransRectangle(Uint32 color, int x, int y,
	int w, int h, unsigned char alpha);

	/// Draw rectangle clipped.
extern void VideoDrawRectangleClip(Uint32 color, int x, int y,
	int w, int h);

	/// Draw translucent rectangle clipped.
extern void VideoDrawTransRectangleClip(Uint32 color, int x, int y,
	int w, int h, unsigned char alpha);

	/// Draw 8bit raw graphic data clipped, using given pixel pallette
extern void VideoDrawRawClip(SDL_Surface *surface, int x, int y, int w, int h);

	/// Draw circle.
extern void VideoDrawCircle(Uint32 color, int x, int y, int r);

	/// Draw translucent circle.
extern void VideoDrawTransCircle(Uint32 color, int x, int y, int r,
	unsigned char alpha);

	/// Draw circle clipped.
extern void VideoDrawCircleClip(Uint32 color, int x, int y, int r);

	/// Draw translucent circle clipped.
extern void VideoDrawTransCircleClip(Uint32 color, int x, int y, int r,
	unsigned char alpha);

	/// Fill rectangle.
extern void VideoFillRectangle(Uint32 color, int x, int y,
	int w, int h);

	/// Fill translucent rectangle.
extern void VideoFillTransRectangle(Uint32 color, int x, int y,
	int w, int h, unsigned char alpha);

	/// Fill rectangle clipped.
extern void VideoFillRectangleClip(Uint32 color, int x, int y,
	int w, int h);

	/// Fill translucent rectangle clipped.
extern void VideoFillTransRectangleClip(Uint32 color, int x, int y,
	int w, int h, unsigned char alpha);

	/// Fill circle.
extern void VideoFillCircle(Uint32 color, int x, int y, int radius);

	/// Fill translucent circle.
extern void VideoFillTransCircle(Uint32 color, int x, int y, int radius,
	unsigned char alpha);

	/// Fill circle clipped.
extern void VideoFillCircleClip(Uint32 color, int x, int y, int radius);

	/// Fill translucent circle clipped.
extern void VideoFillTransCircleClip(Uint32 color, int x, int y, int radius,
	unsigned char alpha);

//@}

#endif // !__VIDEO_H__

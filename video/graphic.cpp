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
/**@name graphic.c - The general graphic functions. */
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

//@{

/*----------------------------------------------------------------------------
--  Includes
----------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "stratagus.h"
#include "video.h"
#include "map.h"
#include "player.h"
#include "intern_video.h"
#include "util.h"
#include "iocompat.h"
#include "iolib.h"

/*----------------------------------------------------------------------------
--  Variables
----------------------------------------------------------------------------*/

PaletteLink* PaletteList;        /// List of all used palettes.

static hashtable(Graphic*, 4099) GraphicHash;/// lookup table for graphic data


/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/

/**
**  Video draw part of graphic.
**
**  @param g   Pointer to object
**  @param gx  X offset into object
**  @param gy  Y offset into object
**  @param w   width to display
**  @param h   height to display
**  @param x   X screen position
**  @param y   Y screen position
*/
#ifndef USE_OPENGL
void VideoDrawSub(const Graphic* g, int gx, int gy,
	int w, int h, int x, int y)
{
	SDL_Rect srect;
	SDL_Rect drect;

	srect.x = gx;
	srect.y = gy;
	srect.w = w;
	srect.h = h;

	drect.x = x;
	drect.y = y;

	SDL_BlitSurface(g->Surface, &srect, TheScreen, &drect);
}
#else
void VideoDrawSub(const Graphic* g, int gx, int gy,
	int w, int h, int x, int y)
{
	int sx;
	int ex;
	int sy;
	int ey;
	GLfloat stx;
	GLfloat etx;
	GLfloat sty;
	GLfloat ety;

	sx = x;
	ex = sx + w;
	sy = y;
	ey = y + h;

	stx = (GLfloat)gx / g->Width * g->TextureWidth;
	etx = (GLfloat)(gx + w) / g->Width * g->TextureWidth;
	sty = (GLfloat)gy / g->Height * g->TextureHeight;
	ety = (GLfloat)(gy + h) / g->Height * g->TextureHeight;

	glBindTexture(GL_TEXTURE_2D, g->Textures[0]);
	glBegin(GL_QUADS);
	glTexCoord2f(stx, sty);
	glVertex2i(sx, sy);
	glTexCoord2f(stx, ety);
	glVertex2i(sx, ey);
	glTexCoord2f(etx, ety);
	glVertex2i(ex, ey);
	glTexCoord2f(etx, sty);
	glVertex2i(ex, sy);
	glEnd();
}
#endif

/**
**  Video draw part of graphic clipped.
**
**  @param g   Pointer to object
**  @param gx  X offset into object
**  @param gy  Y offset into object
**  @param w   width to display
**  @param h   height to display
**  @param x   X screen position
**  @param y   Y screen position
*/
void VideoDrawSubClip(const Graphic* g, int gx, int gy,
	int w, int h, int x, int y)
{
	int oldx;
	int oldy;

	oldx = x;
	oldy = y;
	CLIP_RECTANGLE(x, y, w, h);
	VideoDrawSub(g, gx + x - oldx, gy + y - oldy, w, h, x, y);
}

/**
**  Video draw part of graphic with alpha.
**
**  @param g      Pointer to object
**  @param gx     X offset into object
**  @param gy     Y offset into object
**  @param w      width to display
**  @param h      height to display
**  @param x      X screen position
**  @param y      Y screen position
**  @param alpha  Alpha
*/
#ifndef USE_OPENGL
void VideoDrawSubTrans(const Graphic* g, int gx, int gy,
	int w, int h, int x, int y, unsigned char alpha)
{
	int oldalpha;

	oldalpha = g->Surface->format->alpha;
	SDL_SetAlpha(g->Surface, SDL_SRCALPHA, alpha);
	VideoDrawSub(g, gx, gy, w, h, x, y);
	SDL_SetAlpha(g->Surface, SDL_SRCALPHA, oldalpha);
}
#else
void VideoDrawSubTrans(const Graphic* g, int gx, int gy,
	int w, int h, int x, int y, unsigned char alpha)
{
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glColor4ub(255, 255, 255, alpha);
	VideoDrawSub(g, gx, gy, w, h, x, y);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
}
#endif

/**
**  Video draw part of graphic with alpha and clipped.
**
**  @param g      Pointer to object
**  @param gx     X offset into object
**  @param gy     Y offset into object
**  @param w      width to display
**  @param h      height to display
**  @param x      X screen position
**  @param y      Y screen position
**  @param alpha  Alpha
*/
void VideoDrawSubClipTrans(const Graphic* g, int gx, int gy,
	int w, int h, int x, int y, unsigned char alpha)
{
	int oldx;
	int oldy;

	oldx = x;
	oldy = y;
	CLIP_RECTANGLE(x, y, w, h);
	VideoDrawSubTrans(g, gx + x - oldx, gy + y - oldy, w, h, x, y, alpha);
}

/*----------------------------------------------------------------------------
--  Global functions
----------------------------------------------------------------------------*/

/**
**  Make a new graphic object.
**
**  @param file  Filename
**  @param w     Width of a frame (optional)
**  @param h     Height of a frame (optional)
**
**  @return      New graphic object (malloced).
*/
Graphic* NewGraphic(const char* file, int w, int h)
{
	Graphic* g;
	Graphic** ptr;

	ptr = (Graphic**)hash_find(GraphicHash, file);
	if (!ptr || !*ptr) {
		g = calloc(1, sizeof(Graphic));
		if (!g) {
			fprintf(stderr, "Out of memory\n");
			ExitFatal(-1);
		}
		g->File = strdup(file);
		g->Width = w;
		g->Height = h;
		g->NumFrames = 1;
		g->Refs = 1;
		*(Graphic**)hash_add(GraphicHash, file) = g;
	} else {
		g = *ptr;
		++g->Refs;
		Assert((w == 0 || g->Width == w) && (g->Height == h || h == 0));
	}

	return g;
}

/**
**  Load a graphic
**
**  @param g  Graphic object to load
*/
void LoadGraphic(Graphic* g)
{
	if (g->Surface) {
		return;
	}

	// TODO: More formats?
	if (LoadGraphicPNG(g) == -1) {
		fprintf(stderr, "Can't load the graphic `%s'\n", g->File);
		ExitFatal(-1);
	}
	if (g->Surface->format->BytesPerPixel == 1) {
		VideoPaletteListAdd(g->Surface);
	}

	if (!g->Width) {
		g->Width = g->GraphicWidth;
	}
	if (!g->Height) {
		g->Height = g->GraphicHeight;
	}

	Assert(g->Width <= g->GraphicWidth && g->Height <= g->GraphicHeight);

	if ((g->GraphicWidth / g->Width) * g->Width != g->GraphicWidth ||
			(g->GraphicHeight / g->Height) * g->Height != g->GraphicHeight) {
		fprintf(stderr, "Invalid graphic (width, height) %s\n", g->File);
		fprintf(stderr, "Expected: (%d,%d)  Found: (%d,%d)\n",
			g->Width, g->Height, g->GraphicWidth, g->GraphicHeight);
		ExitFatal(1);
	}

	g->NumFrames = g->GraphicWidth / g->Width * g->GraphicHeight / g->Height;

#ifdef USE_OPENGL
	MakeTexture(g);
#endif
}

/**
**  Free a graphic
**
**  @param g  Pointer to the graphic
*/
void FreeGraphic(Graphic* g)
{
#ifdef USE_OPENGL
	int i;
#endif
	void* pixels;

	if (!g) {
		return;
	}

	Assert(g->Refs);
	
	--g->Refs;
	if (!g->Refs) {
		// No more uses of this graphic
#ifdef USE_OPENGL
		if (g->Textures) {
			glDeleteTextures(g->NumFrames, g->Textures);
			free(g->Textures);
		}
		for (i = 0; i < PlayerMax; ++i) {
			if (g->PlayerColorTextures[i]) {
				glDeleteTextures(g->NumFrames, g->PlayerColorTextures[i]);
			}
		}
#endif

		if (g->Surface) {
			if (g->Surface->format->BytesPerPixel == 1) {
				VideoPaletteListRemove(g->Surface);
			}
			if (g->Surface->flags & SDL_PREALLOC) {
				pixels = g->Surface->pixels;
			} else {
				pixels = NULL;
			}
			SDL_FreeSurface(g->Surface);
			free(pixels);
		}
#ifndef USE_OPENGL
		if (g->SurfaceFlip) {
			if (g->SurfaceFlip->format->BytesPerPixel == 1) {
				VideoPaletteListRemove(g->SurfaceFlip);
			}
			if (g->SurfaceFlip->flags & SDL_PREALLOC) {
				pixels = g->SurfaceFlip->pixels;
			} else {
				pixels = NULL;
			}
			SDL_FreeSurface(g->SurfaceFlip);
			free(pixels);
		}
#endif
		Assert(g->File);

		hash_del(GraphicHash, g->File);
		free(g->File);
		free(g);
	}
}

/**
**  Flip graphic and store in graphic->SurfaceFlip
**
**  @param g  Pointer to object
*/
void FlipGraphic(Graphic* g)
{
#ifdef USE_OPENGL
	return;
#else
	int i;
	int j;
	SDL_Surface* s;

	if (g->SurfaceFlip) {
		return;
	}

	s = g->SurfaceFlip = SDL_ConvertSurface(g->Surface,
		g->Surface->format, SDL_SWSURFACE);
	if (g->Surface->flags & SDL_SRCCOLORKEY) {
		SDL_SetColorKey(g->SurfaceFlip, SDL_SRCCOLORKEY | SDL_RLEACCEL,
			g->Surface->format->colorkey);
	}
	if (g->SurfaceFlip->format->BytesPerPixel == 1) {
		VideoPaletteListAdd(g->SurfaceFlip);
	}

	SDL_LockSurface(g->Surface);
	SDL_LockSurface(s);
	switch (s->format->BytesPerPixel) {
		case 1:
			for (i = 0; i < s->h; ++i) {
				for (j = 0; j < s->w; ++j) {
					((char*)s->pixels)[j + i * s->pitch] =
						((char*)g->Surface->pixels)[s->w - j - 1 + i * g->Surface->pitch];
				}
			}
			break;
		case 3:
			for (i = 0; i < s->h; ++i) {
				for (j = 0; j < s->w; ++j) {
					memcpy(&((char*)s->pixels)[j + i * s->pitch],
						&((char*)g->Surface->pixels)[(s->w - j - 1) * 3 + i * g->Surface->pitch], 3);
				}
			}
			break;
		case 4:
			for (i = 0; i < s->h; ++i) {
				for (j = 0; j < s->w; ++j) {
					*(Uint32*)&((char*)s->pixels)[j * 4 + i * s->pitch] =
						*(Uint32*)&((char*)g->Surface->pixels)[(s->w - j - 1) * 4 + i * g->Surface->pitch];
				}
			}
			break;
	}
	SDL_UnlockSurface(g->Surface);
	SDL_UnlockSurface(s);
#endif
}

/**
**  Make an OpenGL texture or textures out of a graphic object.
**
**  @param g       The graphic object.
**  @param width   Graphic width.
**  @param height  Graphic height.
*/
#ifdef USE_OPENGL
static void MakeTextures(Graphic* g, GLuint* textures, UnitColors* colors)
{
	int i;
	int j;
	int h;
	int w;
	int x;
	unsigned char* tex;
	unsigned char* tp;
	const unsigned char* sp;
	int fl;
	Uint32 ckey;
	int useckey;
	int bpp;
	int size;
	unsigned char alpha;
	Uint32 b;
	Uint32 c;
	Uint32 pc;
	SDL_PixelFormat* f;

	glGetIntegerv(GL_MAX_TEXTURE_SIZE, &size);
	if (g->Width > size || g->Height > size) {
		DebugPrint("Image too large (%d,%d), max size: %d\n" _C_
			g->Width _C_ g->Height _C_ size);
		return;
	}

	fl = g->GraphicWidth / g->Width;
	useckey = g->Surface->flags & SDL_SRCCOLORKEY;
	f = g->Surface->format;
	bpp = f->BytesPerPixel;
	ckey = f->colorkey;

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	for (w = 1; w < g->Width; w <<= 1) {
	}
	for (h = 1; h < g->Height; h <<= 1) {
	}
	g->TextureWidth = (float)g->Width / w;
	g->TextureHeight = (float)g->Height / h;
	tex = (unsigned char*)malloc(w * h * 4);
	if (g->Surface->flags & SDL_SRCALPHA) {
		alpha = f->alpha;
	} else {
		alpha = 0xff;
	}

	SDL_LockSurface(g->Surface);
	for (x = 0; x < g->NumFrames; ++x) {
		glBindTexture(GL_TEXTURE_2D, textures[x]);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		for (i = 0; i < g->Height; ++i) {
			sp = (const unsigned char*)g->Surface->pixels + (x % fl) * g->Width * bpp +
				((x / fl) * g->Height + i) * g->Surface->pitch;
			tp = tex + i * w * 4;
			for (j = 0; j < g->Width; ++j) {
				int z;
				SDL_Color p;

				if (bpp == 1) {
					if (useckey && *sp == ckey) {
						tp[3] = 0;
					} else {
						p = f->palette->colors[*sp];
						tp[0] = p.r;
						tp[1] = p.g;
						tp[2] = p.b;
						tp[3] = alpha;
					}
					if (colors) {
						for (z = 0; z < 4; ++z) {
							if (*sp == 208 + z) {
								p = colors->Colors[z];
								tp[0] = p.r;
								tp[1] = p.g;
								tp[2] = p.b;
								tp[3] = 0xff;
								break;
							}
						}
					}
					++sp;
				} else {
					if (bpp == 4) {
						c = *(Uint32*)sp;
					} else {
						c = (sp[f->Rshift >> 3] << f->Rshift) |
							(sp[f->Gshift >> 3] << f->Gshift) |
							(sp[f->Bshift >> 3] << f->Bshift);
						c |= ((alpha | (alpha << 8) | (alpha << 16) | (alpha << 24)) ^
							(f->Rmask | f->Gmask | f->Bmask));
					}
					*(Uint32*)tp = c;
					if (colors) {
						b = (c & f->Bmask) >> f->Bshift;
						if (b && ((c & f->Rmask) >> f->Rshift) == 0 &&
								((c & f->Gmask) >> f->Gshift) == b) {
							pc = ((colors->Colors[0].r * b / 255) << f->Rshift) |
								((colors->Colors[0].g * b / 255) << f->Gshift) |
								((colors->Colors[0].b * b / 255) << f->Bshift);
							if (bpp == 4) {
								pc |= (c & f->Amask);
							} else {
								pc |= (0xFFFFFFFF ^ (f->Rmask | f->Gmask | f->Bmask));
							}
							*(Uint32*)tp = pc;
						}
					}
					sp += bpp;
				}
				tp += 4;
			}
		}
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, tex);
#ifdef DEBUG
		if ((i = glGetError())) {
			DebugPrint("glTexImage2D(%x)\n" _C_ i);
		}
#endif
	}
	SDL_UnlockSurface(g->Surface);
	free(tex);
}

/**
**  Make an OpenGL texture or textures out of a graphic object.
**
**  @param g  The graphic object.
*/
void MakeTexture(Graphic* g)
{
	if (g->Textures) {
		return;
	}

	g->Textures = (GLuint*)malloc(g->NumFrames * sizeof(GLuint));
	glGenTextures(g->NumFrames, g->Textures);
	MakeTextures(g, g->Textures, NULL);
}

/**
**  Make an OpenGL texture with the player colors.
**
**  @param g       The graphic to texture with player colors.
**  @param player  Player number to make textures for.
*/
void MakePlayerColorTexture(Graphic* g, int player)
{
	g->PlayerColorTextures[player] = malloc(g->NumFrames * sizeof(GLuint));
	glGenTextures(g->NumFrames, g->PlayerColorTextures[player]);
	MakeTextures(g, g->PlayerColorTextures[player], &Players[player].UnitColors);
}
#endif

/**
**  Resize a graphic
**
**  @param g  Graphic object.
**  @param w  New width of graphic.
**  @param h  New height of graphic.
**
**  @todo FIXME: Higher quality resizing.
**        FIXME: Works only with 8bit indexed graphic objects.
*/
void ResizeGraphic(Graphic* g, int w, int h)
{
	int i;
	int j;
	unsigned char* data;
	int x;
	SDL_Color pal[256];
	Uint32 ckey;
	int useckey;

	// FIXME: Support more formats
	if (g->Surface->format->BytesPerPixel != 1) {
		return;
	}

	if (g->GraphicWidth == w && g->GraphicHeight == h) {
		return;
	}

	SDL_LockSurface(g->Surface);

	data = (unsigned char*)malloc(w * h);
	x = 0;

	for (i = 0; i < h; ++i) {
		for (j = 0; j < w; ++j) {
			data[x] = ((unsigned char*)g->Surface->pixels)[
				(i * g->Height / h) * g->Surface->pitch + j * g->Width / w];
			++x;
		}
	}

	SDL_UnlockSurface(g->Surface);
	memcpy(pal, g->Surface->format->palette->colors, sizeof(SDL_Color) * 256);
	if (g->Surface->format->BytesPerPixel == 1) {
		VideoPaletteListRemove(g->Surface);
	}
	useckey = g->Surface->flags & SDL_SRCCOLORKEY;
	ckey = g->Surface->format->colorkey;
	SDL_FreeSurface(g->Surface);

	g->Surface = SDL_CreateRGBSurfaceFrom(data, w, h, 8, w, 0, 0, 0, 0);
	if (g->Surface->format->BytesPerPixel == 1) {
		VideoPaletteListAdd(g->Surface);
	}
	SDL_SetPalette(g->Surface, SDL_LOGPAL | SDL_PHYSPAL, pal, 0, 256);
	if (useckey) {
		SDL_SetColorKey(g->Surface, SDL_SRCCOLORKEY | SDL_RLEACCEL, ckey);
	}

	g->Width = g->GraphicWidth = w;
	g->Height = g->GraphicHeight = h;

#ifdef USE_OPENGL
	glDeleteTextures(g->NumFrames, g->Textures);
	free(g->Textures);
	g->Textures = NULL;
	MakeTexture(g);
#endif
}

/**
**  Check if a pixel is transparent
**
**  @param g  Graphic to check
**  @param x  X coordinate
**  @param y  Y coordinate
**
**  @return   True if the pixel is transparent, False otherwise
*/
int GraphicTransparentPixel(Graphic* g, int x, int y)
{
	unsigned char* p;
	int bpp;
	int ret;

	bpp = g->Surface->format->BytesPerPixel;
	if ((bpp == 1 && !(g->Surface->flags & SDL_SRCCOLORKEY)) || bpp == 3) {
		return 0;
	}

	ret = 0;
	SDL_LockSurface(g->Surface);
	p = (unsigned char*)g->Surface->pixels + y * g->Surface->pitch + x * bpp;
	if (bpp == 1) {
		if (*p == g->Surface->format->colorkey) {
			ret = 1;
		}
	} else {
		if (p[g->Surface->format->Ashift >> 3] == 255) {
			ret = 1;
		}
	}
	SDL_UnlockSurface(g->Surface);

	return ret;
}

//@}

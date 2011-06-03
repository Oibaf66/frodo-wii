/*
 *  Display.cpp - C64 graphics display, emulator window handling
 *
 *  Frodo (C) 1994-1997,2002-2005 Christian Bauer
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "sysdeps.h"
#if defined(GEKKO)
# include <ogc/system.h>
#endif

#include "Display.h"
#include "Version.h"
#include "main.h"
#include "Prefs.h"
#include "C64.h"
#include "CIA.h"
#include "utils.hh"

#include "gui/gui.hh"
#include "gui/status_bar.hh"
#include "gui/virtual_keyboard.hh"

// LED states
enum {
	LED_OFF,		// LED off
	LED_ON,			// LED on (green)
	LED_ERROR_ON,	// LED blinking (red), currently on
	LED_ERROR_OFF	// LED blinking, currently off
};


#define USE_PEPTO_COLORS 1

#ifdef USE_PEPTO_COLORS

// C64 color palette
// Values based on measurements by Philip "Pepto" Timmermann <pepto@pepto.de>
// (see http://www.pepto.de/projects/colorvic/)
const uint8 palette_red[16] = {
	0x00, 0xff, 0x86, 0x4c, 0x88, 0x35, 0x20, 0xcf, 0x88, 0x40, 0xcb, 0x34, 0x68, 0x8b, 0x68, 0xa1
};

const uint8 palette_green[16] = {
	0x00, 0xff, 0x19, 0xc1, 0x17, 0xac, 0x07, 0xf2, 0x3e, 0x2a, 0x55, 0x34, 0x68, 0xff, 0x4a, 0xa1
};

const uint8 palette_blue[16] = {
	0x00, 0xff, 0x01, 0xe3, 0xbd, 0x0a, 0xc0, 0x2d, 0x00, 0x00, 0x37, 0x34, 0x68, 0x59, 0xff, 0xa1
};

#else

// C64 color palette (traditional Frodo colors)
const uint8 palette_red[16] = {
	0x00, 0xff, 0x99, 0x00, 0xcc, 0x44, 0x11, 0xff, 0xaa, 0x66, 0xff, 0x40, 0x80, 0x66, 0x77, 0xc0
};

const uint8 palette_green[16] = {
	0x00, 0xff, 0x00, 0xff, 0x00, 0xcc, 0x00, 0xdd, 0x55, 0x33, 0x66, 0x40, 0x80, 0xff, 0x77, 0xc0
};

const uint8 palette_blue[16] = {
	0x00, 0xff, 0x00, 0xcc, 0xcc, 0x44, 0x99, 0x00, 0x00, 0x00, 0x66, 0x40, 0x80, 0x66, 0xff, 0xc0
};

#endif


/*
 *  Update drive LED display (deferred until Update())
 */

void C64Display::UpdateLEDs(int l0, int l1, int l2, int l3)
{
	led_state[0] = l0;
	led_state[1] = l1;
	led_state[2] = l2;
	led_state[3] = l3;
}


// Display surface
static Uint8 screen[DISPLAY_X * DISPLAY_Y];
//static Uint16 *screen_16;
//static Uint32 *screen_32;
static int screen_bits_per_pixel;

static SDL_Surface *sdl_screen;
SDL_Surface *real_screen = NULL;

// Keyboard
static bool num_locked = false;

// SDL joysticks
static SDL_Joystick *joy[2] = {NULL, NULL};

static Uint16 palette_16[PALETTE_SIZE];
static Uint32 palette_32[PALETTE_SIZE];
SDL_Color sdl_palette[PALETTE_SIZE];

/*
  C64 keyboard matrix:

    Bit 7   6   5   4   3   2   1   0
  0    CUD  F5  F3  F1  F7 CLR RET DEL
  1    SHL  E   S   Z   4   A   W   3
  2     X   T   F   C   6   D   R   5
  3     V   U   H   B   8   G   Y   7
  4     N   O   K   M   0   J   I   9
  5     ,   @   :   .   -   L   P   +
  6     /   ^   =  SHR HOM  ;   *   �
  7    R/S  Q   C= SPC  2  CTL  <-  1
*/

#define MATRIX(a,b) (((a) << 3) | (b))


/*
 *  Open window
 */

int init_graphics(void)
{
	Uint32 rmask, gmask, bmask, amask;
        const SDL_VideoInfo *info = SDL_GetVideoInfo();
        Uint32 flags = SDL_DOUBLEBUF;

	/* SDL interprets each pixel as a 32-bit number, so our masks must depend
           on the endianness (byte order) of the machine */
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
	rmask = 0xff000000;
	gmask = 0x00ff0000;
	bmask = 0x0000ff00;
	amask = 0x000000ff;
#else
     	rmask = 0x000000ff;
	gmask = 0x0000ff00;
	bmask = 0x00ff0000;
	amask = 0xff000000;
#endif

	// Open window
	SDL_ShowCursor(SDL_DISABLE);

	SDL_FreeSurface(sdl_screen);
	//sdl_screen = SDL_CreateRGBSurface(SDL_SWSURFACE, DISPLAY_X, DISPLAY_Y + 17, 8, rmask, gmask, bmask, amask);
	screen_bits_per_pixel = info->vfmt->BitsPerPixel;
	sdl_screen = SDL_CreateRGBSurface(SDL_SWSURFACE, DISPLAY_X, DISPLAY_Y, screen_bits_per_pixel, rmask, gmask, bmask, amask);
	
	if (!sdl_screen)
	{
		fprintf(stderr, "Cannot allocate surface to draw on: %s\n",
				SDL_GetError());
		exit(1);
	}
	
	#ifndef GEKKO
	if (ThePrefs.DisplayType == DISPTYPE_SCREEN)
		flags |= SDL_FULLSCREEN;
	#endif

	screen_bits_per_pixel = info->vfmt->BitsPerPixel;
	SDL_FreeSurface(real_screen);
	real_screen = SDL_SetVideoMode(FULL_DISPLAY_X, FULL_DISPLAY_Y, screen_bits_per_pixel,
			flags);
	if (!real_screen)
	{
		fprintf(stderr, "\n\nCannot initialize video: %s\n", SDL_GetError());
		exit(1);
	}
	//this part of code seems useless
	/*
	free(screen_16);
	free(screen_32);

	switch (screen_bits_per_pixel)
	{
	case 8:
		 
		Default, no need to do anything further 
		break;
	case 16:
		Allocate a 16 bit screen
		screen_16 = (Uint16*)calloc(real_screen->pitch * FULL_DISPLAY_Y, sizeof(Uint16) );
		break;
	case 24:
	case 32:
		screen_32 = (Uint32*)calloc(real_screen->pitch * FULL_DISPLAY_Y, sizeof(Uint32) );
		break;
	default:
		printf("What is this???\n");
		break;
	}
	*/
	
	return 1;
}


/*
 *  Display constructor
 */

C64Display::C64Display(C64 *the_c64) : TheC64(the_c64)
{
	quit_requested = false;
	speedometer_string[0] = 0;
	networktraffic_string[0] = 0;
	this->text_message_send = NULL;

	// Open window
	SDL_WM_SetCaption(VERSION_STRING, "Frodo");
	// LEDs off
	for (int i=0; i<4; i++)
		led_state[i] = old_led_state[i] = LED_OFF;
}


/*
 *  Display destructor
 */

C64Display::~C64Display()
{
	SDL_Quit();
}


/*
 *  Prefs may have changed
 */

void C64Display::NewPrefs(Prefs *prefs)
{
}


/*
 *  Redraw bitmap
 */
void C64Display::Update_32(uint8 *src_pixels)
{
	const Uint16 src_pitch = DISPLAY_X;
	const int x_border = (DISPLAY_X - FULL_DISPLAY_X / 2) / 2;
	const int y_border = (DISPLAY_Y - FULL_DISPLAY_Y / 2) / 2;
	Uint32 *dst_pixels = (Uint32*)real_screen->pixels;
	const int dst_pitch = real_screen->pitch / sizeof(Uint32);

	/* Center, double size */
	for (int y = y_border; y < (FULL_DISPLAY_Y/2) + y_border; y++)
	{
		for (int x = x_border; x < (FULL_DISPLAY_X / 2 + x_border); x++)
		{
			int src_off = y * src_pitch + x;
			int dst_off = ((y * 2 - y_border * 2) * dst_pitch + (x * 2 - x_border * 2));
			Uint32 v = palette_32[src_pixels[src_off]];

			dst_pixels[ dst_off ] = v;
			dst_pixels[ dst_off + 1 ] = v;
			dst_pixels[ dst_off + dst_pitch ] = v;
			dst_pixels[ dst_off + dst_pitch + 1] = v;
		}
	}
}

void C64Display::Update_16(uint8 *src_pixels)
{
	const Uint16 src_pitch = DISPLAY_X;

	#ifdef GEKKO
	
	if (ThePrefs.DisplayType == DISPTYPE_WINDOW)
	{	
		SDL_Rect srcrect = {0, 0, DISPLAY_X, DISPLAY_Y};
		SDL_Rect dstrect = {0, 8, FULL_DISPLAY_X, FULL_DISPLAY_Y-16};
		Uint16 *dst_pixels = (Uint16*)sdl_screen->pixels ;
		const Uint16 src_pitch = DISPLAY_X;
		const Uint16 dst_pitch = sdl_screen->pitch / sizeof(Uint16);

		/* Draw 1-1 */
		for (int y = 0; y < DISPLAY_Y; y++)
		{
			for (int x = 0; x < DISPLAY_X; x++)
			{
				int src_off = y * src_pitch + x;
				int dst_off = y * dst_pitch + x;
				Uint16 v = palette_16[src_pixels[src_off]];			
				dst_pixels[ dst_off ] = v;
			}
		}

	/* Stretch */
	SDL_SoftStretch(sdl_screen, &srcrect, real_screen, &dstrect);	
	}
	
	else
	
	#endif
	{

	const int x_border = (DISPLAY_X - FULL_DISPLAY_X / 2) / 2;
	const int y_border = (DISPLAY_Y - FULL_DISPLAY_Y / 2) / 2;
	Uint16 *dst_pixels = (Uint16*)real_screen->pixels;
	const Uint16 dst_pitch = real_screen->pitch / sizeof(Uint16);

	/* Center, double size */
	for (int y = y_border; y < (FULL_DISPLAY_Y/2) + y_border; y++)
	{
		for (int x = x_border; x < (FULL_DISPLAY_X / 2 + x_border); x++)
		{
			int src_off = y * src_pitch + x;
			int dst_off = ((y * 2 - y_border * 2) * dst_pitch + (x * 2 - x_border * 2));
			Uint16 v = palette_16[src_pixels[src_off]];

			dst_pixels[ dst_off ] = v;
			dst_pixels[ dst_off + 1 ] = v;
			dst_pixels[ dst_off + dst_pitch ] = v;
			dst_pixels[ dst_off + dst_pitch + 1] = v;
		}
	}
	}

}

void C64Display::Update_8(uint8 *src_pixels)
{
	const Uint16 src_pitch = DISPLAY_X;
	const int x_border = (DISPLAY_X - FULL_DISPLAY_X / 2) / 2;
	const int y_border = (DISPLAY_Y - FULL_DISPLAY_Y / 2) / 2;
	Uint8 *dst_pixels = (Uint8*)real_screen->pixels;
	const Uint16 dst_pitch = real_screen->pitch;

	/* Center, double size */
	for (int y = y_border; y < (FULL_DISPLAY_Y/2) + y_border; y++)
	{
		for (int x = x_border; x < (FULL_DISPLAY_X / 2 + x_border); x++)
		{
			int src_off = y * src_pitch + x;
			int dst_off = (y * 2 - y_border * 2) * dst_pitch + (x * 2 - x_border * 2);
			Uint8 v = src_pixels[src_off];

			dst_pixels[ dst_off ] = v;
			dst_pixels[ dst_off + 1 ] = v;
			dst_pixels[ dst_off + dst_pitch ] = v;
			dst_pixels[ dst_off + dst_pitch + 1] = v;
		}
	}
}

void C64Display::Update_stretched(uint8 *src_pixels)
{
	SDL_Rect srcrect = {0, 0, DISPLAY_X, DISPLAY_Y};
	SDL_Rect dstrect = {0, 0, FULL_DISPLAY_X, FULL_DISPLAY_Y};
	Uint8 *dst_pixels = (Uint8*)sdl_screen->pixels;
	const Uint16 src_pitch = DISPLAY_X;

	/* Draw 1-1 */
	for (int y = 0; y < DISPLAY_Y; y++)
	{
		for (int x = 0; x < DISPLAY_X; x++)
		{
			int src_off = y * src_pitch + x;
			int dst_off = src_off;
			Uint8 v = src_pixels[src_off];

			dst_pixels[ dst_off ] = v;
		}
	}

	/* Stretch */
	SDL_SoftStretch(sdl_screen, &srcrect, real_screen, &dstrect);
}

void C64Display::Update(uint8 *src_pixels)
{
	if (0)
		this->Update_stretched(src_pixels);
	else
	{
		switch (screen_bits_per_pixel)
		{
		case 8:
			this->Update_8(src_pixels); break;
		case 16:
			this->Update_16(src_pixels); break;
		case 24:
		case 32:
		default:
			this->Update_32((Uint8*)src_pixels); break;
		}
	}
	Gui::gui->draw(real_screen);

	SDL_Flip(real_screen);
}

void C64Display::Update()
{
	this->Update((Uint8*)screen);
}

SDL_Surface *C64Display::SurfaceFromC64Display()
{
	Uint32 rmask,gmask,bmask,amask;
	SDL_Surface *out;

#if SDL_BYTEORDER == SDL_BIG_ENDIAN
    rmask = 0xff000000;
    gmask = 0x00ff0000;
    bmask = 0x0000ff00;
    amask = 0x000000ff;
#else
    rmask = 0x000000ff;
    gmask = 0x0000ff00;
    bmask = 0x00ff0000;
    amask = 0xff000000;
#endif

	out = SDL_CreateRGBSurface(SDL_SWSURFACE, DISPLAY_X / 2, DISPLAY_Y / 2, 8,
			rmask, gmask, bmask, amask);
	if (!out)
		return NULL;

	Uint8 *dst_pixels = (Uint8*)out->pixels;
	const Uint16 src_pitch = DISPLAY_X;

	/* Draw 1-1 */
	for (int y = 0; y < DISPLAY_Y / 2; y++)
	{
		for (int x = 0; x < DISPLAY_X / 2; x++)
		{
			int src_off = (y * 2) * src_pitch + (x * 2);
			int dst_off = y * out->pitch + x;
			Uint8 v = screen[src_off];

			dst_pixels[ dst_off ] = v;
		}
	}
	SDL_SetColors(out, sdl_palette, 0, PALETTE_SIZE);

	return out;
}

/*
 *  Draw string into surface using the C64 ROM font
 */

/*
 *  Draw speedometer
 */

void C64Display::Speedometer(int speed)
{
	static int delay = 0;

	if (delay >= 20) {
		delay = 0;
		sprintf(speedometer_string, "%d%%", speed);
	} else
		delay++;
}

void C64Display::NetworkTrafficMeter(float kb_per_s, bool is_throttled)
{
	snprintf(this->networktraffic_string, sizeof(this->networktraffic_string),
			"%6.2f KB/S%s", kb_per_s, is_throttled ? " THROTTLED" : "");
}

/*
 *  Return pointer to bitmap data
 */

uint8 *C64Display::BitmapBase(void)
{
	return screen;
}


/*
 *  Return number of bytes per row
 */

int C64Display::BitmapXMod(void)
{
	return DISPLAY_X;
}

void C64Display::FakeKeyPress(int kc, uint8 *CIA_key_matrix,
		uint8 *CIA_rev_matrix)
{
	// Clear matrices
        for (int i = 0; i < 8; i ++)
        {
                CIA_key_matrix[i] = 0xFF;
                CIA_rev_matrix[i] = 0xFF;
        }
        if (kc != -1)
        	this->UpdateKeyMatrix(kc, false, CIA_key_matrix, CIA_rev_matrix,
        			NULL);
}

void C64Display::UpdateKeyMatrix(int c64_key, bool key_up,
		uint8 *key_matrix, uint8 *rev_matrix, uint8 *joystick)
{
	bool shifted = c64_key & 0x80;
	int c64_byte;
	int c64_bit;

	c64_key &= ~0x80;
	c64_byte = (c64_key >> 3) & 7;
	c64_bit = c64_key & 7;

	// Handle joystick emulation
	if (joystick && (c64_key & 0x40)) {
		c64_key &= 0x1f;
		if (key_up)
			*joystick |= c64_key;
		else
			*joystick &= ~c64_key;
		return;
	}

	if (key_up) {
		if (shifted) {
			key_matrix[6] |= 0x10;
			rev_matrix[4] |= 0x40;
		}
		key_matrix[c64_byte] |= (1 << c64_bit);
		rev_matrix[c64_bit] |= (1 << c64_byte);
	} else {
		if (shifted) {
			key_matrix[6] &= 0xef;
			rev_matrix[4] &= 0xbf;
		}
		key_matrix[c64_byte] &= ~(1 << c64_bit);
		rev_matrix[c64_bit] &= ~(1 << c64_byte);
	}
}

/*
 *  Poll the keyboard
 */

void C64Display::TranslateKey(SDLKey key, bool key_up, uint8 *key_matrix,
		uint8 *rev_matrix, uint8 *joystick)
{
	static bool shift_on = false;
	int c64_key = -1;

	switch (key) {
		case SDLK_a: c64_key = MATRIX(1,2); break;
		case SDLK_b: c64_key = MATRIX(3,4); break;
		case SDLK_c: c64_key = MATRIX(2,4); break;
		case SDLK_d: c64_key = MATRIX(2,2); break;
		case SDLK_e: c64_key = MATRIX(1,6); break;
		case SDLK_f: c64_key = MATRIX(2,5); break;
		case SDLK_g: c64_key = MATRIX(3,2); break;
		case SDLK_h: c64_key = MATRIX(3,5); break;
		case SDLK_i: c64_key = MATRIX(4,1); break;
		case SDLK_j: c64_key = MATRIX(4,2); break;
		case SDLK_k: c64_key = MATRIX(4,5); break;
		case SDLK_l: c64_key = MATRIX(5,2); break;
		case SDLK_m: c64_key = MATRIX(4,4); break;
		case SDLK_n: c64_key = MATRIX(4,7); break;
		case SDLK_o: c64_key = MATRIX(4,6); break;
		case SDLK_p: c64_key = MATRIX(5,1); break;
		case SDLK_q: c64_key = MATRIX(7,6); break;
		case SDLK_r: c64_key = MATRIX(2,1); break;
		case SDLK_s: c64_key = MATRIX(1,5); break;
		case SDLK_t: c64_key = MATRIX(2,6); break;
		case SDLK_u: c64_key = MATRIX(3,6); break;
		case SDLK_v: c64_key = MATRIX(3,7); break;
		case SDLK_w: c64_key = MATRIX(1,1); break;
		case SDLK_x: c64_key = MATRIX(2,7); break;
		case SDLK_y: c64_key = MATRIX(3,1); break;
		case SDLK_z: c64_key = MATRIX(1,4); break;

		case SDLK_0: c64_key = MATRIX(4,3); break;
		case SDLK_1: c64_key = MATRIX(7,0); break;
		case SDLK_2: c64_key = MATRIX(7,3); break;
		case SDLK_3: c64_key = MATRIX(1,0); break;
		case SDLK_4: c64_key = MATRIX(1,3); break;
		case SDLK_5: c64_key = MATRIX(2,0); break;
		case SDLK_6: c64_key = MATRIX(2,3); break;
		case SDLK_7: c64_key = MATRIX(3,0); break;
		case SDLK_8: c64_key = MATRIX(3,3); break;
		case SDLK_9: c64_key = MATRIX(4,0); break;

		case SDLK_SPACE: c64_key = MATRIX(7,4); break;
		case SDLK_BACKQUOTE: c64_key = MATRIX(7,1); break;
		case SDLK_BACKSLASH: c64_key = MATRIX(6,6); break;
		case SDLK_COMMA: c64_key = MATRIX(5,7); break;
		case SDLK_PERIOD: c64_key = MATRIX(5,4); break;
		case SDLK_MINUS: c64_key = MATRIX(5,0); break;
		case SDLK_EQUALS: c64_key = MATRIX(5,3); break;
		case SDLK_LEFTBRACKET: c64_key = MATRIX(5,6); break;
		case SDLK_RIGHTBRACKET: c64_key = MATRIX(6,1); break;
		case SDLK_SEMICOLON: c64_key = MATRIX(5,5); break;
		case SDLK_QUOTE: c64_key = MATRIX(6,2); break;
		case SDLK_SLASH: c64_key = MATRIX(6,7); break;

		case SDLK_ESCAPE: c64_key = MATRIX(7,7); break;
		case SDLK_RETURN: c64_key = MATRIX(0,1); break;
		case SDLK_BACKSPACE: case SDLK_DELETE: c64_key = MATRIX(0,0); break;
		case SDLK_INSERT: c64_key = MATRIX(6,3); break;
		case SDLK_HOME: c64_key = MATRIX(6,3); break;
		case SDLK_END: c64_key = MATRIX(6,0); break;
		case SDLK_PAGEUP: c64_key = MATRIX(6,0); break;
		case SDLK_PAGEDOWN: c64_key = MATRIX(6,5); break;

		case SDLK_LCTRL: c64_key = 0x10 | 0x40;  break;
		case SDLK_TAB: c64_key = MATRIX(7,2); break;
		case SDLK_RCTRL: c64_key = MATRIX(7,5); break;
		case SDLK_LSHIFT: c64_key = MATRIX(1,7); break;
		case SDLK_RSHIFT: c64_key = MATRIX(6,4); break;
		case SDLK_LALT: case SDLK_LMETA: c64_key = MATRIX(7,5); break;
		case SDLK_RALT: case SDLK_RMETA: c64_key = MATRIX(7,5); break;
		case SDLK_UP:
		{
			if (ThePrefs.CursorKeysForJoystick)
				c64_key = 0x01 | 0x40;
			else
				c64_key = MATRIX(0,7) | 0x80;
			break;
		}
		case SDLK_DOWN:
		{
			if (ThePrefs.CursorKeysForJoystick)
				c64_key = 0x02 | 0x40;
			else
				c64_key = MATRIX(0,7);
			break;
		}
		case SDLK_LEFT:
		{
			if (ThePrefs.CursorKeysForJoystick)
				c64_key = 0x04 | 0x40;
			else
				c64_key = MATRIX(0,2) | 0x80;
			break;
		}
		case SDLK_RIGHT:
		{
			if (ThePrefs.CursorKeysForJoystick)
				c64_key = 0x08 | 0x40;
			else
				c64_key = MATRIX(0,2);
			break;
		}
		case SDLK_F1: c64_key = MATRIX(0,4); break;
		case SDLK_F2: c64_key = MATRIX(0,4) | 0x80; break;
		case SDLK_F3: c64_key = MATRIX(0,5); break;
		case SDLK_F4: c64_key = MATRIX(0,5) | 0x80; break;
		case SDLK_F5: c64_key = MATRIX(0,6); break;
		case SDLK_F6: c64_key = MATRIX(0,6) | 0x80; break;
		case SDLK_F7: c64_key = MATRIX(0,3); break;
		case SDLK_F8: c64_key = MATRIX(0,3) | 0x80; break;

		case SDLK_KP0: case SDLK_KP5: c64_key = 0x10 | 0x40; break;
		case SDLK_KP1: c64_key = 0x06 | 0x40; break;
		case SDLK_KP2: c64_key = 0x02 | 0x40; break;
		case SDLK_KP3: c64_key = 0x0a | 0x40; break;
		case SDLK_KP4: c64_key = 0x04 | 0x40; break;
		case SDLK_KP6: c64_key = 0x08 | 0x40; break;
		case SDLK_KP7: c64_key = 0x05 | 0x40; break;
		case SDLK_KP8: c64_key = 0x01 | 0x40; break;
		case SDLK_KP9: c64_key = 0x09 | 0x40; break;

		case SDLK_KP_DIVIDE: c64_key = MATRIX(6,7); break;
		case SDLK_KP_ENTER: c64_key = MATRIX(0,1); break;
		default: break;
	}

	if (c64_key < 0)
		return;
	/* Ugly handling of shift. Sorry about that */
	if (!key_up && (c64_key == MATRIX(1,7) || c64_key == MATRIX(6,4)))
		shift_on = true;
	else if (c64_key == MATRIX(1,7) || c64_key == MATRIX(6,4))
		shift_on = false;

	this->UpdateKeyMatrix(c64_key, key_up, key_matrix, rev_matrix, joystick);
}

const char *C64Display::GetTextMessage()
{
	const char *out = this->text_message_send;

	this->text_message_send = NULL;

	return out;
}

class TypeNetworkMessageListener : public KeyboardListener
{
public:
	TypeNetworkMessageListener(const char **out)
	{
		this->out = out;
	}

	virtual void stringCallback(const char *str)
	{
		*out = (const char *)xstrdup(str);
		if (strlen(str) > 0)
			Gui::gui->status_bar->queueMessage("Network message sent!");
		else
			Gui::gui->status_bar->queueMessage("Not sending empty message");
		/* Remove thyself! */
		delete this;
	}

private:
	const char **out;
};

void C64Display::TypeNetworkMessage(bool broadcast)
{
	TypeNetworkMessageListener *nl = new TypeNetworkMessageListener(&this->text_message_send);

	this->text_message_broadcast = broadcast;
	Gui::gui->status_bar->queueMessage("Type message to send to peer");
	VirtualKeyboard::kbd->registerListener(nl);
	VirtualKeyboard::kbd->activate();
}


void C64Display::PollKeyboard(uint8 *key_matrix, uint8 *rev_matrix, uint8 *joystick)
{
	SDL_Event event;
	while (SDL_PollEvent(&event)) {
		Gui::gui->pushEvent(&event);

		/* Ignore keyboard input while the menu is active */
		if (Gui::gui->is_active || Gui::gui->kbd)
			continue;

		switch (event.type) {

			// Key pressed
			case SDL_KEYDOWN:
				switch (event.key.keysym.sym) {

					case SDLK_F10:	// F10/ScrLk: Enter text (for network taunts)
					case SDLK_SCROLLOCK:
						if (TheC64->network_connection_type == CLIENT ||
								TheC64->network_connection_type == MASTER)
							this->TypeNetworkMessage();
						break;

					case SDLK_F11:	// F11: NMI (Restore)
						TheC64->NMI();
						break;

					case SDLK_F12:	// F12: Reset
						TheC64->Reset();
						break;

					case SDLK_HOME:	// Home: Pause and enter menu
						Gui::gui->activate();
						break;

					case SDLK_KP_PLUS:	// '+' on keypad: Increase SkipFrames
						ThePrefs.SkipFrames++;
						break;

					case SDLK_KP_MINUS:	// '-' on keypad: Decrease SkipFrames
						if (ThePrefs.SkipFrames > 1)
							ThePrefs.SkipFrames--;
						break;

					case SDLK_KP_MULTIPLY:	// '*' on keypad: Toggle speed limiter
						ThePrefs.LimitSpeed = !ThePrefs.LimitSpeed;
						break;

					default:
						TranslateKey(event.key.keysym.sym, false, key_matrix, rev_matrix, joystick);
						break;
				}
				break;

			// Key released
			case SDL_KEYUP:
				TranslateKey(event.key.keysym.sym, true, key_matrix, rev_matrix, joystick);
				break;

			// Quit Frodo
			case SDL_QUIT:
				quit_requested = true;
				break;
		}
	}
#if defined(GEKKO)
	if (SYS_ResetButtonDown() != 0)
		quit_requested = true;
#endif
}


/*
 *  Check if NumLock is down (for switching the joystick keyboard emulation)
 */

bool C64Display::NumLock(void)
{
	return num_locked;
}


/*
 *  Open/close joystick drivers given old and new state of
 *  joystick preferences
 */
void C64::open_joystick(int port)
{
	joy_minx[port] = joy_miny[port] = -32767;	// Reset calibration
	joy_maxx[port] = joy_maxy[port] = 32768;
	joy[port] = SDL_JoystickOpen(port);
	if (joy[port] == NULL)
		fprintf(stderr, "Couldn't open joystick %d\n", port + 1);
}

void C64::close_joystick(int port)
{
	if (joy[port]) {
		SDL_JoystickClose(joy[port]);
		joy[port] = NULL;
	}
}

/* The implementation principles are borrowed from UAE */
uint8 C64::poll_joystick_axes(int port, bool *has_event)
{
	SDL_Joystick *js = joy[port];
	unsigned int i, axes;
	uint8 out = 0xff;

	axes = SDL_JoystickNumAxes (js);
	for (i = 0; i < axes; i++) {
		int axis;

		if (ThePrefs.JoystickAxes[i] == JOY_NONE)
			continue;

		axis = SDL_JoystickGetAxis (js, i);

		/* Assume horizontal */
		int *max_axis = &this->joy_maxx[port];
		int *min_axis = &this->joy_minx[port];
		uint8 neg_val = 0xfb;
		uint8 pos_val = 0xf7;
		event_t gui_neg_val = KEY_LEFT;
		event_t gui_pos_val = KEY_RIGHT;

		if (ThePrefs.JoystickAxes[i] == JOY_VERT)
		{
			max_axis = &this->joy_maxy[port];
			min_axis = &this->joy_miny[port];
			neg_val = 0xfe;
			pos_val = 0xfd;
			gui_neg_val = KEY_UP;
			gui_pos_val = KEY_DOWN;
		}

		/* Dynamic joystick calibration */
		if (axis > *max_axis)
			*max_axis = axis;
		if (axis < *min_axis)
			*min_axis = axis;

		/* Too small as of yet */
		if (*max_axis - *min_axis < 1000)
			continue;

		if (axis < (*min_axis + (*max_axis - *min_axis)/3)) {
			out &= neg_val;
			Gui::gui->pushJoystickEvent(gui_neg_val);
			*has_event = true;
		}
		else if (axis > (*min_axis + 2*(*max_axis - *min_axis)/3)) {
			out &= pos_val;
			Gui::gui->pushJoystickEvent(gui_pos_val);
			*has_event = true;
		}
	}

	return out;
}

uint8 C64::poll_joystick_hats(int port, bool *has_event)
{
	SDL_Joystick *js = joy[port];
	unsigned int i, hats;
	uint8 out = 0xff;

	hats = SDL_JoystickNumHats(js);
	for (i = 0; i < hats; i++) {

		Uint8 v = SDL_JoystickGetHat (js, i);
		Uint8 up_mask    = 0xfe;
		Uint8 down_mask  = 0xfd;
		Uint8 left_mask  = 0xfb;
		Uint8 right_mask = 0xf7;
		event_t up_ev    = KEY_UP;
		event_t down_ev  = KEY_DOWN;
		event_t left_ev  = KEY_LEFT;
		event_t right_ev = KEY_RIGHT;

		if (ThePrefs.MenuJoystickHats[i] == HAT_ROTATED_90)
		{
			up_mask    = 0xf7;
			down_mask  = 0xfb;
			left_mask  = 0xfe;
			right_mask = 0xfd;
			up_ev    = KEY_RIGHT;
			down_ev  = KEY_LEFT;
			left_ev  = KEY_UP;
			right_ev = KEY_DOWN;
		}

		if ((v & (SDL_HAT_UP | SDL_HAT_DOWN | SDL_HAT_LEFT | SDL_HAT_RIGHT)) == 0)
			continue;
		*has_event = true;

		if (v & SDL_HAT_UP) {
			out &= up_mask;
			Gui::gui->pushJoystickEvent(up_ev);
		}
		if (v & SDL_HAT_DOWN) {
			out &= down_mask;
			Gui::gui->pushJoystickEvent(down_ev);
		}
		if (v & SDL_HAT_LEFT) {
			out &= left_mask;
			Gui::gui->pushJoystickEvent(left_ev);
		}
		if (v & SDL_HAT_RIGHT) {
			out &= right_mask;
			Gui::gui->pushJoystickEvent(right_ev);
		}
	}

	return out;
}

uint8 C64::poll_joystick_buttons(int port, uint8 *table, bool *has_event)
{
	SDL_Joystick *js = joy[port];
	uint8 out = 0xff;
	int i;

	for (i = 0; i < SDL_JoystickNumButtons (js); i++) {
		bool cur = SDL_JoystickGetButton (js, i) ? true : false;
		int kc = ThePrefs.JoystickButtons[i];
		event_t ev = (event_t)ThePrefs.MenuJoystickButtons[i];

		if (cur && ev != EVENT_NONE)
		{
			Gui::gui->pushJoystickEvent(ev);
			*has_event = true;
		}
		if (kc == JOY_NONE)
			continue;

		if (table[kc] == 0)
			table[kc] = cur ? 2 : 1;
		/* Special case for joysticks: Each button can be pressed multiple times */
		if ((kc & 0x40) && cur)
			table[kc] = 2;
	}

	return out;
}

/*
 *  Poll joystick port, return CIA mask
 */
uint8 C64::poll_joystick(int port)
{
	bool has_event = false;
	uint8 out = 0xff;
	static uint8 last_table_ports[2][0xff];
	uint8 table_ports[2][0xff];
	uint8 *last_table = last_table_ports[port];
	uint8 *table = table_ports[port];

	if (!joy[port])
		return out;

	memset(table, 0, 0xff);

	out &= this->poll_joystick_axes(port, &has_event);
	out &= this->poll_joystick_hats(port, &has_event);
	out &= this->poll_joystick_buttons(port, table, &has_event);

	if (!has_event)
		Gui::gui->pushJoystickEvent(EVENT_NONE);

	/* No joystick input when the Gui is active */
	if (Gui::gui->is_active || Gui::gui->kbd)
		return 0xff;

	/* Handle keyboard codes */
	for (int i = 0; i < 0x51; i++)
	{
		if (table[i] == 0)
			continue;
		if ( !(i & 0x40) && table[i] == last_table[i] )
			continue;

		TheDisplay->UpdateKeyMatrix(i, table[i] == 1,
				TheCIA1->KeyMatrix, TheCIA1->RevMatrix,	&out);
	}

	memcpy(last_table, table, 0xff);

	return out;
}


/*
 *  Allocate C64 colors
 */

void C64Display::InitColors(uint8 *colors)
{
	for (int i=0; i<16; i++) {
		sdl_palette[i].r = palette_red[i];
		sdl_palette[i].g = palette_green[i];
		sdl_palette[i].b = palette_blue[i];
	}
	sdl_palette[fill_gray].r = sdl_palette[fill_gray].g = sdl_palette[fill_gray].b = 0xd0;
	sdl_palette[shine_gray].r = sdl_palette[shine_gray].g = sdl_palette[shine_gray].b = 0xf0;
	sdl_palette[shadow_gray].r = sdl_palette[shadow_gray].g = sdl_palette[shadow_gray].b = 0x80;
	sdl_palette[red].r = 0xf0;
	sdl_palette[red].g = sdl_palette[red].b = 0;
	sdl_palette[green].g = 0xf0;
	sdl_palette[green].r = sdl_palette[green].b = 0;

	if (real_screen->format->BitsPerPixel == 8)
		SDL_SetColors(real_screen, sdl_palette, 0, PALETTE_SIZE);
 	for (int i = 0; i < PALETTE_SIZE; i++) {
 		int rs = real_screen->format->Rshift;
 		int gs = real_screen->format->Gshift;
 		int bs = real_screen->format->Bshift;
 		int rl = real_screen->format->Rloss;
 		int gl = real_screen->format->Gloss;
 		int bl = real_screen->format->Bloss;
 		int rm = real_screen->format->Rmask;
 		int gm = real_screen->format->Gmask;
 		int bm = real_screen->format->Bmask;
 		uint32 r = palette_red[i] & 0xff;
 		uint32 g = palette_green[i] & 0xff;
 		uint32 b = palette_blue[i] & 0xff;

		palette_16[i] = (((r >> rl) << rs) & rm) | (((g >> gl) << gs) & gm) | (((b >> bl) << bs) & bm);
		palette_32[i] = (((r >> rl) << rs) & rm) | (((g >> gl) << gs) & gm) | (((b >> bl) << bs) & bm);
	}

	for (int i=0; i<256; i++)
		colors[i] = i & 0x0f;
}


/*
 *  Show a requester (error message)
 */

long int ShowRequester(const char *a, const char *b, const char *)
{
	printf("%s: %s\n", a, b);
	return 1;
}

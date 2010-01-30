/*
 *  Display_SDL.h - C64 graphics display, emulator window handling,
 *                  SDL specific stuff
 *
 *  Frodo (C) 1994-1997,2002-2009 Christian Bauer
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

#include "C64.h"
#include "SAM.h"
#include "Version.h"

#include "CIA.h"
#include "gui/gui.hh"
#include "gui/virtual_keyboard.hh"

#include <SDL.h>
#if defined(GEKKO)
#include <wiiuse/wpad.h>
#endif

// Display surface
static Uint8 screen[DISPLAY_X * DISPLAY_Y];
static Uint16 *screen_16;
static Uint32 *screen_32;
static int screen_bits_per_pixel;

static SDL_Surface *sdl_screen;
SDL_Surface *real_screen = NULL;

// Keyboard
static bool num_locked = false;

#if defined(DO_ERROR_BLINK)
// For LED error blinking
static C64Display *c64_disp;
static struct sigaction pulse_sa;
static itimerval pulse_tv;
#endif
// SDL joysticks
static SDL_Joystick *joy[2] = {NULL, NULL};

// Colors for speedometer/drive LEDs
enum {
	black = 0,
	white = 1,
	fill_gray = 16,
	shine_gray = 17,
	shadow_gray = 18,
	red = 19,
	green = 20,
	PALETTE_SIZE = 21,
};
static Uint16 palette_16[PALETTE_SIZE];
static Uint32 palette_32[PALETTE_SIZE];

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

	sdl_screen = SDL_CreateRGBSurface(SDL_SWSURFACE, DISPLAY_X, DISPLAY_Y + 17, 8,
			rmask, gmask, bmask, amask);
	if (!sdl_screen)
	{
		fprintf(stderr, "Cannot allocate surface to draw on: %s\n",
				SDL_GetError());
		exit(1);
	}
	screen_bits_per_pixel = info->vfmt->BitsPerPixel;
	real_screen = SDL_SetVideoMode(FULL_DISPLAY_X, FULL_DISPLAY_Y, screen_bits_per_pixel,
			SDL_DOUBLEBUF);
	if (!real_screen)
	{
		fprintf(stderr, "\n\nCannot initialize video: %s\n", SDL_GetError());
		exit(1);
	}

	switch (screen_bits_per_pixel)
	{
	case 8:
		/* Default, no need to do anything further */
		break;
	case 16:
		/* Allocate a 16 bit screen */
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
	this->on_screen_message = NULL;
	this->on_screen_message_start_time = 0;
	this->on_screen_message_time = 0;
	memset(this->text_message, 0, sizeof(this->text_message));
	this->text_message_idx = 0;
	this->entering_text_message = false;
	this->text_message_send = NULL;

	// Open window
	SDL_WM_SetCaption(VERSION_STRING, "Frodo");
	// LEDs off
	for (int i=0; i<4; i++)
		led_state[i] = old_led_state[i] = LED_OFF;

#if defined(DO_ERROR_BLINK)
	// Start timer for LED error blinking
	c64_disp = this;
	pulse_sa.sa_handler = (void (*)(int))pulse_handler;
	pulse_sa.sa_flags = SA_RESTART;
	sigemptyset(&pulse_sa.sa_mask);
	sigaction(SIGALRM, &pulse_sa, NULL);
	pulse_tv.it_interval.tv_sec = 0;
	pulse_tv.it_interval.tv_usec = 400000;
	pulse_tv.it_value.tv_sec = 0;
	pulse_tv.it_value.tv_usec = 400000;
	setitimer(ITIMER_REAL, &pulse_tv, NULL);
#endif
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
	if (ThePrefs.DisplayOption != 0)
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
			this->Update_32((Uint8*)screen); break;
		}
	}
	Gui::gui->draw(real_screen);

	SDL_Flip(real_screen);
}

void C64Display::Update()
{
	this->Update((Uint8*)screen);
}

void C64Display::display_status_string(char *str, int seconds)
{
	Uint32 time_now = SDL_GetTicks();

	this->on_screen_message = str;
	this->on_screen_message_start_time = time_now;
	this->on_screen_message_time = seconds;
}

/*
 *  Draw string into surface using the C64 ROM font
 */

void C64Display::draw_string(SDL_Surface *s, int x, int y, const char *str, uint8 front_color, uint8 back_color)
{
	uint8 *pb = (uint8 *)s->pixels + s->pitch*y + x;
	char c;
	while ((c = *str++) != 0) {
		uint8 *q = TheC64->Char + c*8 + 0x800;
		uint8 *p = pb;
		for (int y=0; y<8; y++) {
			uint8 v = *q++;
			p[0] = (v & 0x80) ? front_color : back_color;
			p[1] = (v & 0x40) ? front_color : back_color;
			p[2] = (v & 0x20) ? front_color : back_color;
			p[3] = (v & 0x10) ? front_color : back_color;
			p[4] = (v & 0x08) ? front_color : back_color;
			p[5] = (v & 0x04) ? front_color : back_color;
			p[6] = (v & 0x02) ? front_color : back_color;
			p[7] = (v & 0x01) ? front_color : back_color;
			p += s->pitch;
		}
		pb += 8;
	}
}


/*
 *  LED error blink
 */

#if defined(DO_ERROR_BLINK)
void C64Display::pulse_handler(...)
{
	for (int i=0; i<4; i++)
		switch (c64_disp->led_state[i]) {
			case LED_ERROR_ON:
				c64_disp->led_state[i] = LED_ERROR_OFF;
				break;
			case LED_ERROR_OFF:
				c64_disp->led_state[i] = LED_ERROR_ON;
				break;
		}
}
#endif


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

#if defined(GEKKO)
		case SDLK_LCTRL:
#else
		case SDLK_LCTRL: c64_key = 0x10 | 0x40;  break;
#endif
		case SDLK_TAB: c64_key = MATRIX(7,2); break;
		case SDLK_RCTRL: c64_key = MATRIX(7,5); break;
		case SDLK_LSHIFT: c64_key = MATRIX(1,7); break;
		case SDLK_RSHIFT: c64_key = MATRIX(6,4); break;
		case SDLK_LALT: case SDLK_LMETA: c64_key = MATRIX(7,5); break;
		case SDLK_RALT: case SDLK_RMETA: c64_key = MATRIX(7,5); break;

#if defined(GEKKO)
                case SDLK_UP: c64_key = MATRIX(0,7)| 0x80; break;
                case SDLK_DOWN: c64_key = MATRIX(0,7); break;
 	        case SDLK_LEFT: c64_key = MATRIX(0,2) | 0x80; break;
	        case SDLK_RIGHT: c64_key = MATRIX(0,2); break;
#else
		case SDLK_UP: c64_key = 0x01 | 0x40; break;
		case SDLK_DOWN: c64_key = 0x02 | 0x40; break;
		case SDLK_LEFT: c64_key = 0x04 | 0x40; break;
		case SDLK_RIGHT: c64_key = 0x08 | 0x40; break;
#endif /* GEKKO */

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
	else if (!key_up && this->entering_text_message)
	{
		char c = Gui::gui->kbd->keycodeToChar(c64_key | (shift_on ? 0x80 : 0) );

		if ((size_t)this->text_message_idx >= sizeof(this->text_message) - 2 ||
				c == '\n')
		{
			this->text_message[this->text_message_idx] = '\0';
			this->text_message_send = this->text_message;
			this->text_message_idx = 0;
			this->entering_text_message = false;
			return;
		}
		if (c == '\b')
		{
			this->text_message_idx--;
			if (this->text_message_idx < 0)
				this->text_message_idx = 0;
			this->text_message[this->text_message_idx] = '\0';
			return;
		}

		this->text_message[this->text_message_idx] = c;
		this->text_message[this->text_message_idx + 1] = '\0';
		this->text_message_idx++;
		return;
	}

	this->UpdateKeyMatrix(c64_key, key_up, key_matrix, rev_matrix, joystick);
}

char *C64Display::GetTextMessage()
{
	char *out = this->text_message_send;
	this->text_message_send = NULL;

	return out;
}

void C64Display::PollKeyboard(uint8 *key_matrix, uint8 *rev_matrix, uint8 *joystick)
{
	SDL_Event event;
	while (SDL_PollEvent(&event)) {
		Gui::gui->pushEvent(&event);

		/* Ignore keyboard input while the menu is active */
		if (Gui::gui->is_active)
			continue;

		switch (event.type) {

			// Key pressed
			case SDL_KEYDOWN:
				switch (event.key.keysym.sym) {

					case SDLK_F10:	// F10/ScrLk: Enter text (for network taunts)
					case SDLK_SCROLLOCK:
						this->entering_text_message = !this->entering_text_message;
						if (this->entering_text_message)
							this->text_message[0] = '\0';
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
void C64::open_close_joystick(int port, int oldjoy, int newjoy)
{
#if !defined(GEKKO)
	if (oldjoy != newjoy) {
		joy_minx[port] = joy_miny[port] = 32767;	// Reset calibration
		joy_maxx[port] = joy_maxy[port] = -32768;
		if (newjoy) {
			joy[port] = SDL_JoystickOpen(newjoy - 1);
			if (joy[port] == NULL)
				fprintf(stderr, "Couldn't open joystick %d\n", port + 1);
		} else {
			if (joy[port]) {
				SDL_JoystickClose(joy[port]);
				joy[port] = NULL;
			}
		}
	}
#endif
}

void C64::open_close_joysticks(int oldjoy1, int oldjoy2, int newjoy1, int newjoy2)
{
	open_close_joystick(0, oldjoy1, newjoy1);
	open_close_joystick(1, oldjoy2, newjoy2);
}

#if defined(GEKKO)
void check_analogue_joystick(joystick_t *js,
		int *extra_keys)
{
	int held = 1;

	if (js->mag < 0.9)
		return;

	// left
	if (js->ang>=270-45 && js->ang<=270+45)
		extra_keys[WIIMOTE_LEFT] = held;

	// right
	if (js->ang>=90-45 && js->ang<=90+45)
		extra_keys[WIIMOTE_RIGHT] = held;

	// up
	if (js->ang>=360-45 || js->ang<=45)
		extra_keys[WIIMOTE_UP] = held;

	// down
	if (js->ang>=180-45 && js->ang<=180+45)
		extra_keys[WIIMOTE_DOWN] = held;

	// up/left
	if (js->ang>=315-20 && js->ang<=315+20)
		extra_keys[WIIMOTE_LEFT] = extra_keys[WIIMOTE_UP] = held;

	//up/right
	if (js->ang>=45-20 && js->ang<=45+20)
		extra_keys[WIIMOTE_RIGHT] = extra_keys[WIIMOTE_UP] = held;

	//down/right
	if (js->ang>=135-20 && js->ang<=135+20)
		extra_keys[WIIMOTE_RIGHT] = extra_keys[WIIMOTE_DOWN] = held;

	//down/left
	if (js->ang>=225-20 && js->ang<=225+20)
		extra_keys[WIIMOTE_LEFT] = extra_keys[WIIMOTE_DOWN] = held;
}
#endif

/* The implementation principles are borrowed from UAE */
uint8 C64::poll_joystick_axes(int port)
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

		if (ThePrefs.JoystickAxes[i] == JOY_VERT)
		{
			max_axis = &this->joy_maxy[port];
			min_axis = &this->joy_miny[port];
			neg_val = 0xfe;
			pos_val = 0xfd;
		}

		/* Dynamic joystick calibration */
		if (axis > *max_axis)
			*max_axis = axis;
		if (axis < *min_axis)
			*min_axis = axis;

		/* Too small as of yet */
		if (*max_axis - *min_axis < 100)
			continue;

		if (axis < (*min_axis + (*max_axis - *min_axis)/3))
			out &= neg_val;
		else if (axis > (*min_axis + 2*(*max_axis - *min_axis)/3))
			out &= pos_val;
	}

	return out;
}

uint8 C64::poll_joystick_hats(int port)
{
	SDL_Joystick *js = joy[port];
	unsigned int i, hats;
	uint8 out = 0xff;

	hats = SDL_JoystickNumHats(js);
	for (i = 0; i < hats; i++) {

		Uint8 v = SDL_JoystickGetHat (js, i);

		/* FIXME! This is the wrong way for the Wii */
		if (v & SDL_HAT_UP)
			out &= 0xfe;
		if (v & SDL_HAT_DOWN)
			out &= 0xfd;
		if (v & SDL_HAT_LEFT)
			out &= 0xfb;
		if (v & SDL_HAT_RIGHT)
			out &= 0xf7;
	}

	return out;
}

uint8 C64::poll_joystick_buttons(int port)
{
	SDL_Joystick *js = joy[port];
	uint8 out = 0xff;
	int i;

	for (i = 0; i < SDL_JoystickNumButtons (js); i++) {
		bool old = this->joy_button_pressed[i];
		bool cur = SDL_JoystickGetButton (js, i) ? true : false;
		int kc = ThePrefs.JoystickButtons[i];

		this->joy_button_pressed[i] = cur;

		if (kc == JOY_NONE)
			continue;

		if (cur != old)
			TheDisplay->UpdateKeyMatrix(kc, !cur,
					TheCIA1->KeyMatrix, TheCIA1->RevMatrix,	&out);
	}

	return out;
}

/*
 *  Poll joystick port, return CIA mask
 */
uint8 C64::poll_joystick(int port)
{
	uint8 out = 0xff;

	if (port == 0 && (joy[0] || joy[1]))
		SDL_JoystickUpdate();

	if (!joy[port])
		return out;

	out &= this->poll_joystick_axes(port);
	out &= this->poll_joystick_hats(port);
	out &= this->poll_joystick_buttons(port);

	return out;
}


/*
 *  Allocate C64 colors
 */

void C64Display::InitColors(uint8 *colors)
{
	SDL_Color palette[PALETTE_SIZE];
	for (int i=0; i<16; i++) {
		palette[i].r = palette_red[i];
		palette[i].g = palette_green[i];
		palette[i].b = palette_blue[i];
	}
	palette[fill_gray].r = palette[fill_gray].g = palette[fill_gray].b = 0xd0;
	palette[shine_gray].r = palette[shine_gray].g = palette[shine_gray].b = 0xf0;
	palette[shadow_gray].r = palette[shadow_gray].g = palette[shadow_gray].b = 0x80;
	palette[red].r = 0xf0;
	palette[red].g = palette[red].b = 0;
	palette[green].g = 0xf0;
	palette[green].r = palette[green].b = 0;

	if (real_screen->format->BitsPerPixel == 8)
		SDL_SetColors(real_screen, palette, 0, PALETTE_SIZE);
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

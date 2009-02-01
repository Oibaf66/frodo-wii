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

#include <SDL.h>
#if defined(GEKKO)
#include <wiiuse/wpad.h>
#endif

// Display surface
static Uint8 screen[DISPLAY_X * DISPLAY_Y];
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
	PALETTE_SIZE = 21
};

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
	real_screen = SDL_SetVideoMode(FULL_DISPLAY_X, FULL_DISPLAY_Y, 8,
			SDL_DOUBLEBUF);
	if (!real_screen)
	{
		fprintf(stderr, "\n\nCannot initialize video: %s\n", SDL_GetError());
		exit(1);
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

	printf("ssof2 %d:%d\n", sizeof(C64Display), sizeof(C64));
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

void C64Display::Update(uint8 *src_pixels)
{
	const Uint16 src_pitch = DISPLAY_X;

	if (ThePrefs.DisplayOption == 0) {
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
	else {
		SDL_Rect srcrect = {0, 0, DISPLAY_X, DISPLAY_Y};
		SDL_Rect dstrect = {0, 0, FULL_DISPLAY_X, FULL_DISPLAY_Y};
		Uint8 *dst_pixels = (Uint8*)sdl_screen->pixels;
		const Uint16 dst_pitch = sdl_screen->pitch;

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

	draw_string(real_screen, 0, 0, networktraffic_string, black, fill_gray);
	for (int i = 0; i < Network::n_peers; i++)
	{
		Network *peer = Network::peers[i];

		peer->DrawTransferredBlocks(real_screen);
	}
	SDL_Flip(real_screen);
}

void C64Display::Update()
{
	this->Update((Uint8*)screen);
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
	sprintf(this->networktraffic_string, "%6.2f KB/S%s",
			kb_per_s, is_throttled ? " THROTTLED" : "");
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
	int shifted = kc & 0x80;
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
	int c64_byte = (c64_key >> 3) & 7;
	int c64_bit = c64_key & 7;

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

		case SDLK_UP: c64_key = 0x01 | 0x40; break;
		case SDLK_DOWN: c64_key = 0x02 | 0x40; break;
		case SDLK_LEFT: c64_key = 0x04 | 0x40; break;
		case SDLK_RIGHT: c64_key = 0x08 | 0x40; break;

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

	this->UpdateKeyMatrix(c64_key, key_up, key_matrix, rev_matrix, joystick);
}

void C64Display::PollKeyboard(uint8 *key_matrix, uint8 *rev_matrix, uint8 *joystick)
{
	SDL_Event event;
	while (SDL_PollEvent(&event)) {
		switch (event.type) {

			// Key pressed
			case SDL_KEYDOWN:
				switch (event.key.keysym.sym) {

					case SDLK_F9:	// F9: Invoke SAM
						SAM(TheC64);
						break;

					case SDLK_F10:	// F10: Quit
						quit_requested = true;
						break;

					case SDLK_F11:	// F11: NMI (Restore)
						TheC64->NMI();
						break;

					case SDLK_F12:	// F12: Reset
						TheC64->Reset();
						break;

					case SDLK_HOME:	// Home: Pause and enter menu
						TheC64->enter_menu();
						break;

					case SDLK_SCROLLOCK:
						num_locked = true;
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
				if (event.key.keysym.sym == SDLK_SCROLLOCK)
					num_locked = false;
				else
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

/*
 *  Poll joystick port, return CIA mask
 */

uint8 C64::poll_joystick(int port)
{
	uint8 j = 0xff;

#ifdef GEKKO
	int extra_keys[N_WIIMOTE_BINDINGS];
	int controller = port;
	Uint32 held = 0;
	Uint32 held_other = 0;
	Uint32 held_classic = 0;
	Uint32 held_classic_other = 0;
        WPADData *wpad, *wpad_other;

	if (ThePrefs.JoystickSwap)
		controller = !port;

        wpad = WPAD_Data(controller);
        wpad_other = WPAD_Data(!controller);
        if (!wpad && !wpad_other)
        	return 0xff;

        held = wpad->btns_h;
        held_other = wpad_other->btns_h;

	// Check classic controller as well
	if (wpad->exp.type == WPAD_EXP_CLASSIC)
		held_classic = wpad->exp.classic.btns_held; 
	if (wpad_other->exp.type == WPAD_EXP_CLASSIC)
		held_classic_other = wpad_other->exp.classic.btns_held;

	if ( (held & WPAD_BUTTON_UP) || (held_classic & CLASSIC_CTRL_BUTTON_LEFT) )
		j &= 0xfb; // Left
	if ( (held & WPAD_BUTTON_DOWN) || (held_classic & CLASSIC_CTRL_BUTTON_RIGHT) )
		j &= 0xf7; // Right
	if ( (held & WPAD_BUTTON_RIGHT) || (held_classic & CLASSIC_CTRL_BUTTON_UP) )
		j &= 0xfe; // Up
	if ( (held & WPAD_BUTTON_LEFT) || (held_classic & CLASSIC_CTRL_BUTTON_DOWN) )
		j &= 0xfd; // Down
	if ( (held & WPAD_BUTTON_2) || (held_classic & CLASSIC_CTRL_BUTTON_A) )
		j &= 0xef; // Button
	if ( (held & WPAD_BUTTON_HOME) || (held_classic & CLASSIC_CTRL_BUTTON_HOME) )
		TheC64->enter_menu();

	extra_keys[WIIMOTE_A] = (held | held_other) & WPAD_BUTTON_A;
	extra_keys[WIIMOTE_B] = (held | held_other) & WPAD_BUTTON_B;
	extra_keys[WIIMOTE_1] = (held | held_other) & WPAD_BUTTON_1;

	/* Classic buttons (might not be connected) */
	extra_keys[CLASSIC_X] = (held_classic | held_classic_other) & CLASSIC_CTRL_BUTTON_X;
	extra_keys[CLASSIC_Y] = (held_classic | held_classic_other) & CLASSIC_CTRL_BUTTON_Y;
	extra_keys[CLASSIC_B] = (held_classic | held_classic_other) & CLASSIC_CTRL_BUTTON_B;
	extra_keys[CLASSIC_L] = (held_classic | held_classic_other) & CLASSIC_CTRL_BUTTON_FULL_L;
	extra_keys[CLASSIC_R] = (held_classic | held_classic_other) & CLASSIC_CTRL_BUTTON_FULL_R;
	extra_keys[CLASSIC_ZL] = (held_classic | held_classic_other) & CLASSIC_CTRL_BUTTON_ZL;
	extra_keys[CLASSIC_ZR] = (held_classic | held_classic_other) & CLASSIC_CTRL_BUTTON_ZR;

	extra_keys[WIIMOTE_PLUS] = ((held_classic | held_classic_other) & CLASSIC_CTRL_BUTTON_MINUS) |
		(held | held_other) & WPAD_BUTTON_PLUS;
	extra_keys[WIIMOTE_MINUS] = ((held_classic | held_classic_other) & CLASSIC_CTRL_BUTTON_PLUS) |
		(held | held_other) & WPAD_BUTTON_MINUS;

	for (int i = 0; i < N_WIIMOTE_BINDINGS; i++)
	{
		static bool is_pressed[N_WIIMOTE_BINDINGS];
		int kc = ThePrefs.JoystickKeyBinding[i];

		if ( kc >= 0)
		{
			if (extra_keys[i])
			{
				TheDisplay->UpdateKeyMatrix(kc, false,
						TheCIA1->KeyMatrix, TheCIA1->RevMatrix,
						&j);
				is_pressed[i] = true;
			}
			else if (is_pressed[i])
			{
				TheDisplay->UpdateKeyMatrix(kc, true,
						TheCIA1->KeyMatrix, TheCIA1->RevMatrix,
						&j);
				is_pressed[i] = false;
			}
		}
	}

	return j;
#else
	if (port == 0 && (joy[0] || joy[1]))
		SDL_JoystickUpdate();

	if (joy[port]) {
		int x = SDL_JoystickGetAxis(joy[port], 0), y = SDL_JoystickGetAxis(joy[port], 1);

		if (x > joy_maxx[port])
			joy_maxx[port] = x;
		if (x < joy_minx[port])
			joy_minx[port] = x;
		if (y > joy_maxy[port])
			joy_maxy[port] = y;
		if (y < joy_miny[port])
			joy_miny[port] = y;

		if (joy_maxx[port] - joy_minx[port] < 100 || joy_maxy[port] - joy_miny[port] < 100)
			return 0xff;

		if (x < (joy_minx[port] + (joy_maxx[port]-joy_minx[port])/3))
			j &= 0xfb;							// Left
		else if (x > (joy_minx[port] + 2*(joy_maxx[port]-joy_minx[port])/3))
			j &= 0xf7;							// Right

		if (y < (joy_miny[port] + (joy_maxy[port]-joy_miny[port])/3))
			j &= 0xfe;							// Up
		else if (y > (joy_miny[port] + 2*(joy_maxy[port]-joy_miny[port])/3))
			j &= 0xfd;							// Down

		if (SDL_JoystickGetButton(joy[port], 0))
			j &= 0xef;							// Button
	}

	return j;
#endif
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
	SDL_SetColors(real_screen, palette, 0, PALETTE_SIZE);


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

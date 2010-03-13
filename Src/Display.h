/*
 *  Display.h - C64 graphics display, emulator window handling
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

#ifndef _DISPLAY_H
#define _DISPLAY_H

#include <SDL.h>
extern SDL_Surface *real_screen;

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


// Display dimensions
#if defined(SMALL_DISPLAY)
const int DISPLAY_X = 0x168;
const int DISPLAY_Y = 0x110;
#else
const int DISPLAY_X = 0x180;
const int DISPLAY_Y = 0x110;
#endif

#if defined(HAVE_SDL)
const int FULL_DISPLAY_X = 640;
const int FULL_DISPLAY_Y = 480;
#endif

class C64Window;
class C64Screen;
class C64;
class Prefs;

// Class for C64 graphics display
class C64Display {
public:
	C64Display(C64 *the_c64);
	~C64Display();

	void Update(void);
	void UpdateLEDs(int l0, int l1, int l2, int l3);
	void Speedometer(int speed);
	void NetworkTrafficMeter(float kb_per_s, bool has_throttled);
	uint8 *BitmapBase(void);
	int BitmapXMod(void);

	void PollKeyboard(uint8 *key_matrix, uint8 *rev_matrix, uint8 *joystick);

	void FakeKeyPress(int kc, uint8 *CIA_key_matrix, uint8 *CIA_rev_matrix);
	void TranslateKey(SDLKey key, bool key_up, uint8 *key_matrix, uint8 *rev_matrix, uint8 *joystick);
	void UpdateKeyMatrix(int c64_key, bool key_up, uint8 *key_matrix,
			uint8 *rev_matrix, uint8 *joystick);
	void Update(uint8 *src_pixels);
	void Update_8(uint8 *src_pixels);
	void Update_16(uint8 *src_pixels);
	void Update_32(uint8 *src_pixels);
	void Update_stretched(uint8 *src_pixels);
	SDL_Surface *SurfaceFromC64Display();
	const char *GetTextMessage();
	bool NumLock(void);
	void InitColors(uint8 *colors);
	void NewPrefs(Prefs *prefs);

	void TypeNetworkMessage(bool broadcast = false);

	C64 *TheC64;

	bool quit_requested;


	/* FIXME! Should not be public */
	bool text_message_broadcast;
private:
	int led_state[4];
	int old_led_state[4];

	char speedometer_string[16];		// Speedometer text
	char networktraffic_string[80];		// Speedometer text
	const char *text_message_send;
};


// Exported functions
extern long ShowRequester(const char *str, const char *button1, const char *button2 = NULL);


#endif

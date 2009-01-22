/*********************************************************************
 *
 * Copyright (C) 2009,  Simon Kagstrom
 *
 * Filename:      VirtualKeyboard.c
 * Author:        Simon Kagstrom <simon.kagstrom@gmail.com>
 * Description:   A virtual keyboard
 *
 * $Id$
 *
 ********************************************************************/
#include <SDL.h>
#include <SDL_ttf.h>

#include "menu.h"
#include "VirtualKeyboard.h"

typedef struct
{
	const char *name;
	int kc;
	bool is_shift;
} virtkey_t;

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

#define K(name, a,b) \
	{ name, MATRIX(a,b), false }
#define S(name, a,b) \
	{ name, MATRIX(a,b), true }

#define KEY_COLS 15
#define KEY_ROWS 5

static virtkey_t keys[KEY_COLS * KEY_ROWS] = {
	K("<-",7,1),       K("1", 7,0), K("2", 7,3), K("3", 1,0), K("4", 1,3), K("5", 2,0), K("6", 2,3), K("7", 3,0), K("8", 3,3), K("9", 4,0), K("0", 4,3), K("+", 5,0), K("-", 5,3), K("£", 0,0), K("Hom", 6,3),
	K("Ctr", 7,2),     K("q", 7,6), K("w", 1,1), K("e", 1,6), K("r", 2,2), K("t", 2,6), K("y", 3,1), K("u", 3,6), K("i", 4,1), K("o", 6,6), K("p", 5,1), K("@", 5,6), K("*", 6,1), K("|^", 6,0),K("Rstr", 4,0),
	K("R/Stp", 7,6),   K(0,   0,0), K("a", 1,2), K("s", 1,5), K("d", 2,2), K("f", 2,5), K("g", 3,2), K("h", 3,5), K("j", 4,2), K("k", 4,5), K("l", 5,2), K(":", 5,5), K(";", 6,2), K("=", 6,5), K("Ret", 0,1),
	K("C=", 7,6),      S("Sh",1,7), K("z", 1,4), K("x", 2,7), K("c", 2,4), K("v", 3,7), K("b", 3,4), K("n", 4,7), K("m", 4,4), K(",", 5,7), K(".", 5,4), S("Sh",6,4), K("Dwn", 0,7),K("Rgt", 0,2),
	K(0, 0,0),         K(0, 0,0),   K(0, 0,0),   K("space", 7,4),K(0, 0,0),K(0, 0,0),   K("f1", 0,4),K("f3", 0,5),K("f5", 0,6),K("f7", 0,3),K(0, 0,0),   K(0, 0,0),   K(0, 0,0),   K(0, 0,0),   K("Del", 0,0),
};

VirtualKeyboard::VirtualKeyboard(SDL_Surface *screen, TTF_Font *font)
{
	this->screen = screen;
	this->font = font;
	this->sel_x = 0;
	this->sel_y = 0;
	this->shift_on = false;
}

void VirtualKeyboard::draw()
{
	int screen_w = 640;
	int screen_h = 480;
	int key_w = 36;
	int key_h = 36;
	int border_x = (screen_w - (key_w * KEY_COLS)) / 2;
	int border_y = (screen_h - (key_h * KEY_ROWS)) / 2;

	for (int y = 0; y < KEY_ROWS; y++ )
	{
		for (int x = 0; x < KEY_COLS; x++ )
		{
			int which = y * KEY_COLS + x;
			virtkey_t key = keys[which];
			int r = 255, g = 255, b = 255;

			/* Skip empty positions */
			if (key.name == NULL)
				continue;

			if ( (x == this->sel_x && y == this->sel_y) ||
					(this->shift_on && key.is_shift))
				b = 0;

			menu_print_font(this->screen, this->font, r, g, b,
					x * key_w + border_x, y * key_h + border_x,
					key.name);
		}
	}
}

void VirtualKeyboard::select_next(int dx, int dy)
{
	int next_x = (this->sel_x + dx) % KEY_COLS;
	int next_y = (this->sel_y + dy) % KEY_ROWS;
	virtkey_t key;

	if (next_x < 0)
		next_x = KEY_COLS + next_x;
	if (next_y < 0)
		next_y = KEY_ROWS + next_y;
	this->sel_x = next_x;
	this->sel_y = next_y;

	key = keys[ next_y * KEY_COLS + next_x ];

	/* Skip the empty spots */
	if (key.name == NULL)
		this->select_next(dx, dy);
}

void VirtualKeyboard::toggle_shift()
{
	this->shift_on = !this->shift_on;
}

bool VirtualKeyboard::get_key(int *kc, bool *shifted)
{
	bool out = false;

	while(1)
	{
		uint32_t k;

		SDL_FillRect(this->screen, 0, SDL_MapRGB(screen->format, 0x00, 0x80, 0x80));

		this->draw();
		SDL_Flip(this->screen);

		k = menu_wait_key_press();

		if (k & KEY_UP)
			this->select_next(0, -1);
		else if (k & KEY_DOWN)
			this->select_next(0, 1);
		else if (k & KEY_LEFT)
			this->select_next(-1, 0);
		else if (k & KEY_RIGHT)
			this->select_next(1, 0);
		else if (k & KEY_ESCAPE)
			break;
		else if (k & KEY_SELECT)
		{
			virtkey_t key = keys[ this->sel_y * KEY_COLS + this->sel_x ];
			out = true;

			*kc = key.kc;
			*shifted = this->shift_on;
			if (key.is_shift == true)
				this->toggle_shift();
			else
				break;
		}
	}

	SDL_FillRect(screen, 0, SDL_MapRGB(screen->format, 0, 0, 0));
	return out;
}

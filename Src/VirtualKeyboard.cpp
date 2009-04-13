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

typedef struct virtkey
{
	const char *name;
	int kc;
	bool is_shift;
	bool is_done;
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
	{ name, MATRIX(a,b), false, false }
#define S(name, a,b) \
	{ name, MATRIX(a,b), true, false }
#define N(name) \
	{ name, -1, false, false }
#define D(name) \
	{ name, -1, false, true }
#define J(name, v) \
	{ name, 0x40 | (v), false, false }

#define KEY_COLS 15
#define KEY_ROWS 8

static virtkey_t keys[KEY_COLS * KEY_ROWS] = {
	K("<-",7,1),       K("1", 7,0), K("2", 7,3), K("3", 1,0), K("4", 1,3), K("5", 2,0), K("6", 2,3), K("7", 3,0), K("8", 3,3), K("9", 4,0), K("0", 4,3), K("+", 5,0), K("-", 5,3), K("£", 6,0), K("Hom", 6,3),
	K("Cr", 7,2),      K("Q", 7,6), K("W", 1,1), K("E", 1,6), K("R", 2,1), K("T", 2,6), K("Y", 3,1), K("U", 3,6), K("I", 4,1), K("O", 4,6), K("P", 5,1), K("@", 5,6), K("*", 6,1), K("Au", 6,6),K("Rstr",4,0),
	K("R/Stp", 7,7),   K(NULL,0,0), K("A", 1,2), K("S", 1,5), K("D", 2,2), K("F", 2,5), K("G", 3,2), K("H", 3,5), K("J", 4,2), K("K", 4,5), K("L", 5,2), K(":", 5,5), K(";", 6,2), K("=", 6,5), K("Ret", 0,1),
	K("C=", 7,5),      S("Shft",1,7),K(NULL,0,0),K("Z", 1,4), K("X", 2,7), K("C", 2,4), K("V", 3,7), K("B", 3,4), K("N", 4,7), K("M", 4,4), K(",", 5,7), K(".", 5,4), K("/", 6,7), K("Dwn",0,7),K("Rgt", 0,2),
	N("None"),         K(NULL,0,0), K(NULL,0,0), K("space", 7,4),K(0, 0,0),K(NULL,0,0), K("f1",0,4), K("f3",0,5), K("f5",0,6), K("f7",0,3), K("Del",0,0),K(NULL,0,0), K(NULL,0,0), K(NULL,0,0), D("DONE"),
	K(NULL,0,0),       K(NULL,0,0), K(NULL,0,0), K(NULL,0,0), K(0, 0,0),J("Joystick up",1),K(0, 0,0),K(NULL,0,0), K(NULL,0,0), K(NULL,0,0), K(NULL, 0,0),K(NULL,0,0), K(NULL,0,0), K(NULL,0,0), K(NULL, 0,0),
	J("Joystick left",4),K(0, 0,0), K(NULL,0,0), K(NULL,0,0), K(0,0,0),J("Joystick fire",0x10),K(0,0,0),K(0,0,0), K(NULL,0,0), K(0,0,0),J("Joystick right",8),K(0, 0,0),K(0, 0,0), K(NULL,0,0), K(NULL,0,0),
	K(NULL,0,0),       K(0, 0,0),   K(NULL,0,0), K(NULL,0,0), K(0,0,0),J("Joystick down",2),K(0,0,0),K(NULL,0,0),K(NULL,0,0), K(NULL,0,0), K(NULL,0,0), K(NULL,0,0), K(NULL, 0,0),K(NULL,0,0), K(NULL, 0,0),
};

static const char *shifted_names[KEY_COLS * KEY_ROWS] = {
	NULL,              "!",         "\"",        "#",         "$",         "%",         "&",         "'",         "(",         ")",         NULL,        NULL,        NULL,        NULL,        "Clr",
	NULL,              NULL,        NULL,        NULL,        NULL,        NULL,        NULL,        NULL,        NULL,        NULL,        NULL,        NULL,        NULL,        NULL,        NULL,
	NULL,              NULL,        NULL,        NULL,        NULL,        NULL,        NULL,        NULL,        NULL,        NULL,        NULL,        "[",         "]",         NULL,        NULL,
	NULL,              NULL,        NULL,        NULL,        NULL,        NULL,        NULL,        NULL,        NULL,        NULL,         "<",         ">",        "?",         "Up",        "Lft",
	NULL,              NULL,        NULL,        NULL,        NULL,        NULL,        "f2",        "f4",        "f6",        "f8",        "Ins",       NULL,        NULL,        NULL,        NULL,
};

VirtualKeyboard::VirtualKeyboard(SDL_Surface *screen, TTF_Font *font)
{
	this->screen = screen;
	this->font = font;
	this->sel_x = 0;
	this->sel_y = 0;
	this->shift_on = false;

	memset(this->buf, 0, sizeof(this->buf));
}

void VirtualKeyboard::draw()
{
	int screen_w = this->screen->w;
	int screen_h = this->screen->h;
	int key_w = 36;
	int key_h = 36;
	int border_x = (screen_w - (key_w * KEY_COLS)) / 2;
	int border_y = (screen_h - (key_h * KEY_ROWS)) / 2;
	SDL_Rect bg_rect = {border_x, border_y,
			key_w * KEY_COLS, key_h * KEY_ROWS};

	SDL_FillRect(this->screen, &bg_rect,
			SDL_MapRGB(screen->format, 0x00, 0x80, 0x80));

	for (int y = 0; y < KEY_ROWS; y++ )
	{
		for (int x = 0; x < KEY_COLS; x++ )
		{
			int which = y * KEY_COLS + x;
			virtkey_t key = keys[which];
			int r = 255, g = 255, b = 255;
			const char *what = key.name;

			/* Skip empty positions */
			if (key.name == NULL)
				continue;
			if (this->shift_on && shifted_names[which])
				what = shifted_names[which];

			if ( key.is_done )
				r = 0;
			if ( (x == this->sel_x && y == this->sel_y) ||
					(this->shift_on && key.is_shift))
				b = 0;

			menu_print_font(this->screen, r, g, b,
					x * key_w + border_x, y * key_h + border_y,
					what);
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
	{
		if (dy != 0) /* Look left */
			this->select_next(-1, 0);
		else
			this->select_next(dx, dy);
	}
}

void VirtualKeyboard::toggle_shift()
{
	this->shift_on = !this->shift_on;
}

const char *VirtualKeyboard::keycode_to_string(int kc)
{
	bool shifted = kc & 0x80;
	int kc_raw = kc & ~0x80;
	const char *out = "Unknown";

	if (kc < 0)
		return "None";

	/* Just loop through all of them */
	for (int i = 0; i < KEY_COLS * KEY_ROWS; i++)
	{
		virtkey_t key = keys[i];

		if (key.kc == kc_raw && key.name != NULL)
		{
			out = key.name;

			if (shifted && shifted_names[i])
				out = shifted_names[i];
			break;
		}
	}

	return out;
}

int VirtualKeyboard::char_to_keycode(char c)
{
	for (int i = 0; i < KEY_COLS * KEY_ROWS; i++)
	{
		virtkey_t key = keys[i];

		if (key.name != NULL)
		{
			if (strlen(key.name) == 1)
			{
				if (key.name[0] == c)
					return key.kc;
				if (shifted_names[i] && strlen(shifted_names[i]) == 1 &&
						shifted_names[i][0] == c)
					return key.kc | 0x80;
			}

			/* OK, ugly special cases, but these are pretty important */
			if (c == ' ' && strcmp(key.name, "space") == 0)
				return key.kc;
			if (c == '\n' && strcmp(key.name, "Ret") == 0)
				return key.kc;
		}
	}

	return -1;
}

const char VirtualKeyboard::keycode_to_char(int kc)
{
	const char *s = this->keycode_to_string(kc);

	if (strcmp(s, "space") == 0)
		return ' ';
	if (strcmp(s, "Ret") == 0)
		return '\n';
	if (strcmp(s, "Del") == 0)
		return '\b';

	/* NULL is never, ever returned */
	return s[0];
}

struct virtkey *VirtualKeyboard::get_key_internal()
{
	while(1)
	{
		uint32_t k;

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
			return NULL;
		else if (k & KEY_SELECT)
		{
			virtkey_t *key = &keys[ this->sel_y * KEY_COLS + this->sel_x ];

			if (key->is_shift == true)
				this->toggle_shift();
			else
				return key;
		}
	}

	return NULL;
}

int VirtualKeyboard::get_key()
{
	virtkey_t *key;

	SDL_FillRect(this->screen, 0, SDL_MapRGB(screen->format, 0x00, 0x80, 0x80));

	key = this->get_key_internal();
	if (key == NULL)
		return -2;

	if (this->shift_on)
		return key->kc | 0x80;
	return key->kc;
}

const char *VirtualKeyboard::get_string()
{
	int cnt = 0;

	SDL_FillRect(this->screen, 0, SDL_MapRGB(screen->format, 0x00, 0x80, 0x80));
	memset(this->buf, 0, sizeof(this->buf));

	while (true)
	{
		virtkey_t *key = this->get_key_internal();
		char c;

		/* Abort */
		if (key == NULL)
			return NULL;

		/* Done */
		if (key->is_done)
			return this->buf;
		/* Skip None */
		if (key->kc == -1)
			continue;

		/* Special-case for delete */
		if (strcmp(key->name, "Del") == 0)
		{
			if (cnt < 1)
				continue;
			this->buf[cnt - 1] = ' ';
			cnt -= 2;
		}
		else
		{
			c = this->keycode_to_char( this->shift_on ? key->kc | 0x80 : key->kc );

			this->buf[cnt] = c;
		}

		cnt++;
		if (cnt >= sizeof(this->buf) - 1)
			return this->buf;

		/* SDL_Flip is done in get_key_internal() */
		SDL_FillRect(this->screen, 0, SDL_MapRGB(screen->format, 0x00, 0x80, 0x80));
		menu_print_font(this->screen, 255, 255, 0,
				40, screen->h - 50,
				this->buf);
	}

	/* Not reachable */
	return NULL;
}

VirtualKeyboard *virtual_keyboard;

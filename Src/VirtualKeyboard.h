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

struct virtkey; 

class VirtualKeyboard
{
public:
	VirtualKeyboard(SDL_Surface *screen, TTF_Font *font);
	int get_key();
	const char *get_string();
	const char *keycode_to_string(int kc);
	const char keycode_to_char(int kc);

	int char_to_keycode(char c);

private:
	struct virtkey *get_key_internal();
	void draw();
	void select_next(int dx, int dy);
	void toggle_shift();

	SDL_Surface *screen;
	TTF_Font *font;
	int sel_x;
	int sel_y;
	bool shift_on;

	char buf[255];
};

extern VirtualKeyboard *virtual_keyboard;


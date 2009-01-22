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

class VirtualKeyboard
{
public:
	VirtualKeyboard(SDL_Surface *screen, TTF_Font *font);
	bool get_key(int *kc, bool *shifted);
	const char *keycode_to_string(int kc);

private:
	void draw();
	void select_next(int dx, int dy);
	void toggle_shift();

	SDL_Surface *screen;
	TTF_Font *font;
	int sel_x;
	int sel_y;
	bool shift_on;
};

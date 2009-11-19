/*********************************************************************
 *
 * Copyright (C) 2004, 2008,  Simon Kagstrom
 *
 * Filename:      menu.h
 * Author:        Simon Kagstrom <simon.kagstrom@gmail.com>
 * Description:
 *
 * $Id$
 *
 ********************************************************************/
#ifndef __MENU_H__
#define __MENU_H__

#include <SDL.h>
#include <SDL_ttf.h>
#include <stdint.h>

enum {
	KEY_UP,
	KEY_DOWN,
	KEY_LEFT,
	KEY_RIGHT,
	KEY_SELECT,
	KEY_ESCAPE,
	KEY_PAGEDOWN,
	KEY_PAGEUP,
	KEY_HELP,
};

class Menu
{
public:
	Menu(TTF_Font *font, SDL_Color clr,	int w, int h);

	void setText(const char **messages);

	void pushEvent(SDL_Event *ev);

	void runLogic();

	void draw(SDL_Surface *where,
			int x, int y);

	~Menu();

private:
	const char *title;
	const char **pp_msgs;
	TTF_Font *font;
	SDL_Color text_color;

	int (*hover_callback)(Menu *me, int index);
	int (*selection_callback)(Menu *me, int index);

	/* Width and height */
	int w, h;

	/* Relative to this menu */
	int		   mouse_x, mouse_y;

	int        n_submenus;
	submenu_t *p_submenus;

	int        cur_sel; /* Main selection */
	int        start_entry_visible;
	int        n_entries;


};

#endif /* !__MENU_H__ */

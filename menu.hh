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

#include "font.hh"

enum {
	EVENT_NONE   = 0,
	KEY_UP       = 1,
	KEY_DOWN     = 2,
	KEY_LEFT     = 4,
	KEY_RIGHT    = 8,
	KEY_SELECT   = 16,
	KEY_ESCAPE   = 32,
	KEY_PAGEDOWN = 64,
	KEY_PAGEUP   = 128,
	KEY_HELP     = 256,
};


typedef struct
{
	int n_entries;
	int index;
	int sel;
} submenu_t;

typedef int event_t;

class Menu
{
public:
	Menu(Font *font);

	~Menu();

	void setFont(Font *font)
	{
		this->font = font;
	}

	void setTextColor(SDL_Color clr);

	void setSelectedBackground(SDL_Surface *left, SDL_Surface *middle, SDL_Surface *right,
			SDL_Surface *submenu_left, SDL_Surface *submenu_middle, SDL_Surface *submenu_right)
	{
		this->text_bg_left = left;
		this->text_bg_middle = middle;
		this->text_bg_right = right;

		this->submenu_bg_left = submenu_left;
		this->submenu_bg_middle = submenu_middle;
		this->submenu_bg_right = submenu_right;
	}

	void setText(const char **messages, int *submenu_defaults = NULL);

	void pushEvent(SDL_Event *ev);

	void runLogic();

	void draw(SDL_Surface *where,
			int x, int y, int w, int h);

protected:
	void highlightBackground(SDL_Surface *where,
			SDL_Surface *bg_left, SDL_Surface *bg_middle, SDL_Surface *bg_right,
			int x, int y, int w, int h);

	void printText(SDL_Surface *where, const char *msg, SDL_Color clr,
			int x, int y, int w, int h);

	virtual void hoverCallback(int which) = 0;

	virtual void selectCallback(int which) = 0;

	virtual void escapeCallback(int which) = 0;

	submenu_t *findSubmenu(int index);

	int getNextEntry(int dy);

	int selectOne(int which);

	int selectNext(int dx, int dy);

	int selectNext(event_t ev);

	void pushEvent(event_t ev);

	event_t popEvent();

	const char *title;
	const char **pp_msgs;
	Font *font;
	SDL_Color text_color;

	SDL_Surface *text_bg_left;
	SDL_Surface *text_bg_right;
	SDL_Surface *text_bg_middle;

	SDL_Surface *submenu_bg_left;
	SDL_Surface *submenu_bg_right;
	SDL_Surface *submenu_bg_middle;

	/* Relative to this menu */
	int	mouse_x, mouse_y;

	int        n_submenus;
	submenu_t *p_submenus;

	int        cur_sel; /* Main selection */
	int        n_entries;

	int 	   ev_head, ev_tail;
	event_t	   event_stack[8];
};

#endif /* !__MENU_H__ */

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
#include "widget.hh"

typedef struct
{
	int n_entries;
	int index;
	int sel;
} submenu_t;


class Menu : public Widget
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

	virtual void runLogic();

	void draw(SDL_Surface *where,
			int x, int y, int w, int h);

protected:
	void printText(SDL_Surface *where, const char *msg, SDL_Color clr,
			int x, int y, int w, int h);

	virtual void hoverCallback(int which) = 0;

	virtual void selectCallback(int which) = 0;

	virtual void escapeCallback(int which) = 0;

	submenu_t *findSubmenu(int index);

public:
	virtual int getNextEntry(int dy);

	virtual int selectOne(int which);

	virtual int selectNext(int dx, int dy);

	virtual int selectNext(event_t ev);

protected:

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
};

#endif /* !__MENU_H__ */

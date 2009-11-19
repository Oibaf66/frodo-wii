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
#include "sysdeps.h"
#include "Network.h"

#define KEY_UP         1
#define KEY_DOWN       2
#define KEY_LEFT       4
#define KEY_RIGHT      8
#define KEY_SELECT    16
#define KEY_ESCAPE    32
#define KEY_PAGEDOWN  64
#define KEY_PAGEUP   128
#define KEY_HELP     256

class Menu
{
public:
	Menu(int x, int y, int w, int h);

	void setTextColor(const SDL_Color clr);

	void pushEvent(SDL_Event *ev);

	void runLogic();

	void draw(SDL_Surface *where);

	~Menu();

private:
	const char *title;
	const char **pp_msgs;
	TTF_Font *p_font;
	SDL_Color text_color;

	int (*hover_callback)(Menu *me, int index);
	int (*selection_callback)(Menu *me, int index);

	/* Start and end of the menu */
	int        x1,y1;
	int        x2,y2;
	int        text_w;
	int        text_h;

	int        n_submenus;
	submenu_t *p_submenus;

	int        cur_sel; /* Main selection */
	int        start_entry_visible;
	int        n_entries;
};


void menu_print_font(SDL_Surface *screen, int r, int g, int b, int x, int y, const char *msg);
void menu_print_font64(SDL_Surface *screen, int r, int g, int b, int x, int y, const char *msg);

/* Various option selects */
int menu_select(const char *title, const char **pp_msgs, int *p_submenus);
int menu_select(const char **pp_msgs, int *p_submenus);
int menu_select_sized(char *title, const char **msgs, int *submenus,
		int x, int y, int w, int h);
const char *menu_select_file(const char *dir_path);
const char *menu_select_file_start(const char *dir_path, const char **d64_name);
int menu_select_peer(NetworkUpdatePeerInfo *peers, int n_peers);

uint32_t menu_wait_key_press(void);

extern bool msgKill(SDL_Rect *rc);
extern int msgInfo(char *text, int duration, SDL_Rect *rc);

extern bool msgYesNo(char *text, bool def,int x, int y);


void menu_init();

#endif /* !__MENU_H__ */

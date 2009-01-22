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

#if defined(__cplusplus)
extern "C" {
#endif /* __cplusplus */

#define KEY_UP         1
#define KEY_DOWN       2
#define KEY_LEFT       4
#define KEY_RIGHT      8
#define KEY_SELECT    16
#define KEY_ESCAPE    32
#define KEY_PAGEDOWN  64
#define KEY_PAGEUP   128

typedef struct
{
  int n_entries;
  int index;
  int sel;
} submenu_t;


typedef struct
{
  const char **pp_msgs;
  TTF_Font  *p_font;
  int        x1,y1;
  int        x2,y2;
  int        text_w;
  int        text_h;

  int        n_submenus;
  submenu_t *p_submenus;

  int        cur_sel; /* Main selection */
  int        start_entry_visible;
  int        n_entries;
} menu_t;

void menu_print_font(SDL_Surface *screen, TTF_Font *font, int r, int g, int b,
                       int x, int y, const char *msg);

void menu_init(menu_t *p_menu, TTF_Font *p_font, const char **pp_msgs,
	       int16_t x1, int16_t y1, int16_t x2, int16_t y2);
void menu_fini(menu_t *p_menu);

int menu_select(SDL_Surface *screen, menu_t *p_menu,
                int *p_submenus);

uint32_t menu_wait_key_press(void);

#if defined(__cplusplus)
};
#endif /* __cplusplus */

#endif /* !__MENU_H__ */

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

void menu_init(menu_t *p_menu, TTF_Font *p_font, const char **pp_msgs,
	       int16_t x1, int16_t y1, int16_t x2, int16_t y2);
void menu_fini(menu_t *p_menu);

int menu_select(SDL_Surface *screen, menu_t *p_menu,
                int *p_submenus);

#if defined(__cplusplus)
};
#endif /* __cplusplus */

#endif /* !__MENU_H__ */

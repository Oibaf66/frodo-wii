/*********************************************************************
 *
 * Copyright (C) 2004,2008,  Simon Kagstrom
 *
 * Filename:      menu.c
 * Author:        Simon Kagstrom <simon.kagstrom@gmail.com>
 * Description:   Code for menus (originally for Mophun)
 *
 * $Id$
 *
 ********************************************************************/
#include <stdio.h>
#include <stdlib.h>

#if defined(GEKKO)
# include <wiiuse/wpad.h>
#endif

#include "menu.h"

#define KEY_UP         1
#define KEY_DOWN       2
#define KEY_LEFT       4
#define KEY_RIGHT      8
#define KEY_SELECT    16
#define KEY_ESCAPE    32
#define KEY_PAGEDOWN  64
#define KEY_PAGEUP   128

#define IS_SUBMENU(p_msg) ( (p_msg)[0] == '^' )

static submenu_t *find_submenu(menu_t *p_menu, int index)
{
  int i;

  for (i=0; i<p_menu->n_submenus; i++)
    {
      if (p_menu->p_submenus[i].index == index)
	return &p_menu->p_submenus[i];
    }

  return NULL;
}

static void print_font(SDL_Surface *screen, TTF_Font *font, int r, int g, int b,
                       int x, int y, const char *msg)
{
  SDL_Surface *font_surf;
  SDL_Rect dst = {x, y,  0, 0};
  SDL_Color color = {r, g, b};
  char buf[255];
  unsigned int i;

  memset(buf, 0, sizeof(buf));
  strncpy(buf, msg, 254);

  /* Fixup multi-menu option look */
  for (i = 0; i < strlen(buf) ; i++)
  {
	  if (buf[i] == '^' || buf[i] == '|')
		  buf[i] = ' ';
  }

  font_surf = TTF_RenderText_Blended(font, buf,
                                   color);
  if (!font_surf)
    {
      fprintf(stderr, "%s\n", TTF_GetError());
      exit(1);
    }

  SDL_BlitSurface(font_surf, NULL, screen, &dst);

  SDL_FreeSurface(font_surf);
}


static void menu_draw(SDL_Surface *screen, menu_t *p_menu)
{
  int x_start = p_menu->x1 + (p_menu->x2 - p_menu->x1) / 2 - p_menu->text_w / 2;
  int y_start = p_menu->y1 + (p_menu->y2 - p_menu->y1) / 2 - p_menu->text_h / 2;
  int font_height = TTF_FontHeight(p_menu->p_font);
  int line_height = (font_height + font_height / 4);
  int entries_visible = p_menu->y2 / line_height;
  int i;

  if ( p_menu->n_entries * line_height > p_menu->y2 )
	  y_start = p_menu->y1;
  if ( p_menu->cur_sel - p_menu->start_entry_visible > entries_visible )
	  p_menu->start_entry_visible += p_menu->cur_sel - entries_visible;
  else if ( p_menu->cur_sel < p_menu->start_entry_visible )
	  p_menu->start_entry_visible = p_menu->cur_sel;

  for (i = p_menu->start_entry_visible; i < p_menu->n_entries; i++)
    {
      const char *msg = p_menu->pp_msgs[i];
      int y = (i - p_menu->start_entry_visible) * line_height;

      if (p_menu->cur_sel == i) /* Selected - color */
	print_font(screen, p_menu->p_font, 255,255,0, x_start,
                   y_start + y, msg);
      else /* Otherwise white */
	print_font(screen, p_menu->p_font, 255,255,255, x_start,
                   y_start + y, msg);
      if (IS_SUBMENU(msg))
	{
	  submenu_t *p_submenu = find_submenu(p_menu, i);
	  int n_pipe = 0;
	  int n;

	  for (n=0; msg[n] != '\0'; n++)
	    {
	      /* Underline the selected entry */
	      if (msg[n] == '|')
		{
		  int16_t n_chars;

		  for (n_chars = 1; msg[n+n_chars] && msg[n+n_chars] != '|'; n_chars++);

		  n_pipe++;
		  if (p_submenu->sel == n_pipe-1)
		    {
                      SDL_Rect r;
                      int w;
                      int h;

                      if (TTF_SizeText(p_menu->p_font, "X", &w, &h) < 0)
                      {
                	      fprintf(stderr, "%s\n", TTF_GetError());
                	      exit(1);
                      }

                      r = (SDL_Rect) { x_start + (n+1) * w-1,
                	      y_start + (i+1 - p_menu->start_entry_visible) * ((h + h/4)-1),
                	      (n_chars - 1) * w,
                	      2 };
                      SDL_FillRect(screen, &r, SDL_MapRGB(screen->format, 0, 255, 0));
		      break;
		    }
		}
	    }
	}
    }
}

static int get_next_seq_y(menu_t *p_menu, int v, int dy)
{
	if (v + dy < 0)
		return p_menu->n_entries - 1;
	if (v + dy > p_menu->n_entries - 1)
		return 0;
	return v + dy;
}

static void select_next(menu_t *p_menu, int dx, int dy)
{
  int next;

  p_menu->cur_sel = get_next_seq_y(p_menu, p_menu->cur_sel, dy);
  next = get_next_seq_y(p_menu, p_menu->cur_sel, dy + 1);

  if (p_menu->pp_msgs[p_menu->cur_sel][0] == ' ' ||
      IS_SUBMENU(p_menu->pp_msgs[p_menu->cur_sel]) )
    select_next(p_menu, dx, dy);
  /* If the next is a submenu */
  if (dx != 0 &&
      IS_SUBMENU(p_menu->pp_msgs[next]))
    {
      submenu_t *p_submenu = find_submenu(p_menu, next);

      p_submenu->sel = (p_submenu->sel + dx) < 0 ? p_submenu->n_entries - 1 :
	(p_submenu->sel + dx) % p_submenu->n_entries;
    }
}

static int is_submenu_title(menu_t *p_menu, int n)
{
  if (n+1 >= p_menu->n_entries)
    return 0;
  else
    return IS_SUBMENU(p_menu->pp_msgs[n+1]);
}


void menu_init(menu_t *p_menu, TTF_Font *p_font, const char **pp_msgs,
	       int16_t x1, int16_t y1, int16_t x2, int16_t y2)
{
  int i;
  int j;

  memset(p_menu, 0, sizeof(menu_t));

  p_menu->pp_msgs = pp_msgs;
  p_menu->p_font = p_font;
  p_menu->x1 = x1;
  p_menu->y1 = y1;
  p_menu->x2 = x2;
  p_menu->y2 = y2;

  p_menu->text_w = 0;
  p_menu->n_submenus = 0;

  for (p_menu->n_entries = 0; p_menu->pp_msgs[p_menu->n_entries]; p_menu->n_entries++)
    {
      int text_w_font;

      /* Is this a submenu? */
      if (IS_SUBMENU(p_menu->pp_msgs[p_menu->n_entries]))
	{
	  p_menu->n_submenus++;
	  continue; /* Length of submenus is unimportant */
	}

      if (TTF_SizeText(p_font, p_menu->pp_msgs[p_menu->n_entries], &text_w_font, NULL) != 0)
        {
          fprintf(stderr, "%s\n", TTF_GetError());
          exit(1);
        }
      if (text_w_font > p_menu->text_w)
	p_menu->text_w = text_w_font;
    }
  if (p_menu->text_w > p_menu->x2 - p_menu->x1)
	  p_menu->text_w = p_menu->x2 - p_menu->x1;

  if ( !(p_menu->p_submenus = (submenu_t *)malloc(sizeof(submenu_t) * p_menu->n_submenus)) )
    {
      perror("malloc failed!\n");
      exit(1);
    }

  j=0;
  for (i=0; i<p_menu->n_submenus; i++)
    {

      for (; j < p_menu->n_entries; j++)
	{
	  if (IS_SUBMENU(p_menu->pp_msgs[j]))
	    {
	      int n;

	      p_menu->p_submenus[i].index = j;
	      p_menu->p_submenus[i].sel = 0;
	      p_menu->p_submenus[i].n_entries = 0;
	      for (n=0; p_menu->pp_msgs[j][n] != '\0'; n++)
		{
		  if (p_menu->pp_msgs[j][n] == '|')
		    p_menu->p_submenus[i].n_entries++;
		}
	    }
	}
    }
  p_menu->text_h = p_menu->n_entries * (TTF_FontHeight(p_font) + TTF_FontHeight(p_font) / 4);
}

void menu_fini(menu_t *p_menu)
{
  free(p_menu->p_submenus);
}


static uint32_t wait_key_press(void)
{
	SDL_Event ev;
	uint32_t keys = 0;

	while (1)
	{
#if defined(GEKKO)
		Uint32 remote_keys;

		WPAD_ScanPads();
		remote_keys = WPAD_ButtonsDown(WPAD_CHAN_0) | WPAD_ButtonsDown(WPAD_CHAN_1);

		if (remote_keys & WPAD_BUTTON_DOWN)
			keys |= KEY_RIGHT;
		if (remote_keys & WPAD_BUTTON_UP)
			keys |= KEY_LEFT;
		if (remote_keys & WPAD_BUTTON_LEFT)
			keys |= KEY_DOWN;
		if (remote_keys & WPAD_BUTTON_RIGHT)
			keys |= KEY_UP;
		if (remote_keys & WPAD_BUTTON_PLUS)
			keys |= KEY_PAGEUP;
		if (remote_keys & WPAD_BUTTON_MINUS)
			keys |= KEY_PAGEDOWN;
		if (remote_keys & (WPAD_BUTTON_A | WPAD_BUTTON_2) )
			keys |= KEY_SELECT;
		if (remote_keys & (WPAD_BUTTON_1 | WPAD_BUTTON_HOME) )
			keys |= KEY_ESCAPE;
#endif
		if (SDL_PollEvent(&ev))
		{
			switch(ev.type)
			{
			case SDL_KEYDOWN:
				switch (ev.key.keysym.sym)
				{
				case SDLK_UP:
					keys |= KEY_UP;
					break;
				case SDLK_DOWN:
					keys |= KEY_DOWN;
					break;
				case SDLK_LEFT:
					keys |= KEY_LEFT;
					break;
				case SDLK_RIGHT:
					keys |= KEY_RIGHT;
					break;
				case SDLK_PAGEDOWN:
					keys |= KEY_PAGEDOWN;
					break;
				case SDLK_PAGEUP:
					keys |= KEY_PAGEUP;
					break;
				case SDLK_RETURN:
				case SDLK_SPACE:
					keys |= KEY_SELECT;
					break;
				case SDLK_ESCAPE:
					keys |= KEY_ESCAPE;
					break;
				default:
					break;
				}
				break;
				case SDL_QUIT:
					exit(0);
					break;
				default:
					break;
			}
			break;
		}
		if (keys != 0)
			return keys;
		SDL_Delay(100);
	}

	return keys;
}


int menu_select(SDL_Surface *screen, menu_t *p_menu,
                int *p_submenus)
{
	int ret = -1;

	for (int i = 0; i < p_menu->n_submenus; i++)
		p_menu->p_submenus[i].sel = p_submenus[i];

	while(1)
	{
		uint32_t keys;

		SDL_FillRect(screen, 0, SDL_MapRGB(screen->format, 0x00, 0x80, 0x80));

		menu_draw(screen, p_menu);
		SDL_Flip(screen);

		keys = wait_key_press();

		if (keys & KEY_UP)
			select_next(p_menu, 0, -1);
		else if (keys & KEY_DOWN)
			select_next(p_menu, 0, 1);
		else if (keys & KEY_PAGEUP)
			select_next(p_menu, 0, -6);
		else if (keys & KEY_PAGEDOWN)
			select_next(p_menu, 0, 6);
		else if (keys & KEY_LEFT)
			select_next(p_menu, -1, 0);
		else if (keys & KEY_RIGHT)
			select_next(p_menu, 1, 0);
		else if (keys & KEY_ESCAPE)
			break;
		else if (keys & KEY_SELECT)
		{
			ret = p_menu->cur_sel;
			int i;

			for (i=0; i<p_menu->n_submenus; i++)
				p_submenus[i] = p_menu->p_submenus[i].sel;
			break;
		}
	}

	SDL_FillRect(screen, 0, SDL_MapRGB(screen->format, 0, 0, 0));
	return ret;
}

#if defined(TEST_MENU)
char *main_menu[] = {
		"Insert disc",
		"Reset C64",
		"Joystick port",
		"^|1|2",
		" ",
		"Quit",
		NULL,
};

int main(int argc, char *argv[])
{
	SDL_Surface *screen;
	TTF_Font *font;
	const SDL_VideoInfo *info;
	int submenus[1] = {0};
	int selected;
	menu_t menu;

	if (SDL_Init( SDL_INIT_EVERYTHING ) < 0)
	{
	        fprintf(stderr, "Unable to init SDL: %s\n", SDL_GetError() );
	        exit(1);		
	}
	if (TTF_Init() < 0)
	{
	        fprintf(stderr, "Unable to init TTF: %s\n", TTF_GetError() );
	        exit(1);		
	}

	atexit(SDL_Quit);
	atexit(TTF_Quit);
	font = TTF_OpenFont("FreeMono.ttf", 20);
	if (!font)
	{
	        fprintf(stderr, "Unable to open font: %s\n", TTF_GetError() );
	        exit(1);		
	}

	info = SDL_GetVideoInfo();
	/* Open a 640x480 display with the optimal color depth  */
	screen = SDL_SetVideoMode(640, 480, info->vfmt->BitsPerPixel,
				      info->hw_available ? SDL_HWSURFACE : SDL_SWSURFACE);
	menu_init(&menu, font, main_menu, 640 / 3, 480 / 3, 400, 400);

	selected = menu_select(screen, &menu, ~0, submenus);
	printf("Selected: %d:%d\n",
			selected, submenus[0]);

	menu_fini(&menu);

	return 0;
}
#endif

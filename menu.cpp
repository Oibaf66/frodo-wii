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

#include "menu.hh"
#include "font.hh"
#include "utils.hh"

#define IS_SUBMENU(p_msg) ( (p_msg)[0] == '^' )

void Menu::printText(SDL_Surface *where, const char *msg, SDL_Color clr,
		int x, int y, int w, int h)
{
	char *buf;
	unsigned int i;
	int tw;

	buf = strdup(msg);
	tw = this->font->getWidth(buf);

	/* Crop text */
	if (x + tw > w)
	{
		int pixels_per_char = tw / strlen(msg);
		int last_char = w / pixels_per_char;

		/* FIXME! Handle some corner cases here (short strings etc) */
		panic_if((unsigned)last_char > strlen(msg),
				"last character (%d) is after the string length (%d)\n",
				last_char, strlen(msg));
		if (last_char > 3)
		{
			buf[last_char - 2] = '.';
			buf[last_char - 1] = '.';
			buf[last_char] = '\0';
		}
	}

	/* Fixup multi-menu option look */
	for (i = 0; i < strlen(buf) ; i++)
	{
		if (buf[i] == '^' || buf[i] == '|')
			buf[i] = ' ';
	}

	this->font->draw(where, buf, x, y, w, h);
	free(buf);
}

void Menu::highlightBackground(SDL_Surface *where,
		SDL_Surface *bg_left, SDL_Surface *bg_middle, SDL_Surface *bg_right,
		int x, int y, int w, int h)
{
	SDL_Rect dst;

	/* Can't highlight without images */
	if (!bg_left ||	!bg_middle || !bg_right)
		return;

	int font_height = this->font->getHeight("X");
	int bg_y_start = y + font_height / 2 -
			bg_left->h / 2;
	int bg_x_start = x - bg_left->w / 3;
	int bg_x_end = x + w - (2 * bg_right->w) / 3;
	int n_mid = (bg_x_end - bg_x_start) / bg_middle->w;

	/* Left */
	dst = (SDL_Rect){bg_x_start, bg_y_start, 0,0};
	SDL_BlitSurface(bg_left, NULL, where, &dst);

	/* Middle */
	for (int i = 1; i < n_mid; i++)
	{
		dst = (SDL_Rect){bg_x_start + i * bg_middle->w, bg_y_start, 0,0};
		SDL_BlitSurface(bg_middle, NULL, where, &dst);
	}
	dst = (SDL_Rect){bg_x_end - bg_middle->w, bg_y_start, 0,0};
	SDL_BlitSurface(bg_middle, NULL, where, &dst);

	/* Right */
	dst = (SDL_Rect){bg_x_end, bg_y_start, 0,0};
	SDL_BlitSurface(bg_right, NULL,	where, &dst);
}


void Menu::draw(SDL_Surface *where, int x, int y, int w, int h)
{
	int font_height = this->font->getHeight("X");
	int line_height = (font_height + font_height / 4);
	int x_start = x;
	int entries_visible = h / line_height - 2;
	int start_entry_visible = 0;

	panic_if(!this->pp_msgs, "Set the messages before drawing, thank you\n");

	if (this->cur_sel - start_entry_visible > entries_visible)
	{
		while (this->cur_sel - start_entry_visible > entries_visible)
		{
			start_entry_visible++;
			if (start_entry_visible > this->n_entries)
			{
				start_entry_visible = 0;
				break;
			}
		}
	}
	else if ( this->cur_sel < start_entry_visible )
		start_entry_visible = this->cur_sel;

	for (int i = start_entry_visible;
			i <= start_entry_visible + entries_visible; i++)
	{
		const char *msg = this->pp_msgs[i];
		int cur_y;

		if (i >= this->n_entries)
			break;

		cur_y = y + (i - start_entry_visible) * line_height;

		/* Draw the background for the selected entry */
		if (this->cur_sel == i) {
			int tw, th;

			tw = this->font->getWidth(msg);
			th = this->font->getHeight(msg);

			this->highlightBackground(where,
					this->text_bg_left, this->text_bg_middle, this->text_bg_right,
					x_start, cur_y, tw, th);
		}

		if (IS_SUBMENU(msg))
		{
			submenu_t *p_submenu = this->findSubmenu(i);
			int n_pipe = 0;
			int total_chars = 0;
			int tw, th, tw_first;
			int n_chars;
			char *p;
			int n;

			for (n = 0; msg[n] != '\0'; n++)
			{
				if (msg[n] != '|')
					continue;
				/* msg[n] == '|' */

				/* Count the number of chars until next pipe */
				for (n_chars = 1; msg[n+n_chars] && msg[n+n_chars] != '|'; n_chars++);

				total_chars += n_chars;

				n_pipe++;
				/* Found the selection yet? */
				if (p_submenu->sel == n_pipe-1)
					break;
			}

			p = (char*)xmalloc(total_chars + 1);
			strncpy(p, msg, n + 1);
			tw_first = this->font->getWidth(p);

			memset(p, 0, total_chars + 1);
			strncpy(p, msg + n, n_chars - 1);
			tw = this->font->getWidth(p);
			th = this->font->getHeight(p);

			this->highlightBackground(where,
					this->submenu_bg_left, this->submenu_bg_middle, this->submenu_bg_right,
					 x_start + tw_first, cur_y, tw, th);
			free(p);
		}

		/* And print the text on top */
		this->printText(where, msg, this->text_color,
				x_start, cur_y, w, h);
	}
}


int Menu::getNextEntry(int dy)
{
	if (this->cur_sel + dy < 0)
		return this->n_entries - 1;
	if (this->cur_sel + dy > this->n_entries - 1)
		return 0;
	return this->cur_sel + dy;
}

submenu_t *Menu::findSubmenu(int index)
{
	int i;

	for (i = 0; i < this->n_submenus; i++)
	{
		if (this->p_submenus[i].index == index)
			return &this->p_submenus[i];
	}

	return NULL;
}


void Menu::selectOne(int which)
{
	if (which >= this->n_entries)
		which = 0;
	this->cur_sel = which;

	if (this->pp_msgs[this->cur_sel][0] == ' ' ||
			IS_SUBMENU(this->pp_msgs[this->cur_sel]))
		this->selectNext(0, 1);
}

void Menu::selectNext(int dx, int dy)
{
	int next;

	this->cur_sel = this->getNextEntry(dy);
	next = this->getNextEntry(dy + 1);

	/* We want to skip this for some reason */
	if (this->pp_msgs[this->cur_sel][0] == ' ' ||
			IS_SUBMENU(this->pp_msgs[this->cur_sel]) ) {
		this->selectNext(dx, dy);
		return;
	}

	/* If the next is a submenu */
	if (dx != 0 && IS_SUBMENU(this->pp_msgs[next]))
	{
		submenu_t *p_submenu = findSubmenu(next);

		panic_if(!p_submenu, "submenu in the menu text but no actual submenu\n");
		p_submenu->sel = (p_submenu->sel + dx) < 0 ? p_submenu->n_entries - 1 :
		(p_submenu->sel + dx) % p_submenu->n_entries;
	}
}

void Menu::selectNext(event_t ev)
{
	switch (ev)
	{
	case KEY_UP:
		this->selectNext(0, -1); break;
	case KEY_DOWN:
		this->selectNext(0, 1); break;
	case KEY_LEFT:
		this->selectNext(-1, 0); break;
	case KEY_RIGHT:
		this->selectNext(1, 0); break;
	default:
		panic("selectNext(ev) called with event %d\n", ev);
	}
}

void Menu::runLogic()
{
	event_t ev;

	while ( (ev = this->popEvent()) != EVENT_NONE )
	{
		switch (ev)
		{
		case KEY_UP:
		case KEY_DOWN:
		case KEY_LEFT:
		case KEY_RIGHT:
			this->selectNext(ev);
			break;
		case KEY_SELECT:
			this->selectCallback(this->cur_sel); break;
		case KEY_ESCAPE:
			this->escapeCallback(this->cur_sel); break;
			break;
		default:
			break;
		}
	}
}

event_t Menu::popEvent()
{
	event_t out;

	if (this->ev_head == this->ev_tail)
		return EVENT_NONE;
	out = this->event_stack[this->ev_tail];
	this->ev_tail = (this->ev_tail + 1) % 8;

	return out;
}

void Menu::pushEvent(event_t ev)
{
	/* Push... */
	this->event_stack[this->ev_head] = ev;

	/* ... and update */
	this->ev_head = (this->ev_head + 1) % 8;
	if (this->ev_head == this->ev_tail)
		this->ev_tail = (this->ev_tail + 1) % 8;
}

void Menu::pushEvent(SDL_Event *ev)
{
	switch(ev->type)
	{
	case SDL_KEYDOWN:
		switch (ev->key.keysym.sym)
		{
		case SDLK_UP:
			this->pushEvent(KEY_UP);
			break;
		case SDLK_DOWN:
			this->pushEvent(KEY_DOWN);
			break;
		case SDLK_LEFT:
			this->pushEvent(KEY_LEFT);
			break;
		case SDLK_RIGHT:
			this->pushEvent(KEY_RIGHT);
			break;
		case SDLK_PAGEDOWN:
			this->pushEvent(KEY_PAGEDOWN);
			break;
		case SDLK_PAGEUP:
			this->pushEvent(KEY_PAGEUP);
			break;
		case SDLK_RETURN:
		case SDLK_SPACE:
			this->pushEvent(KEY_SELECT);
			break;
		case SDLK_HOME:
		case SDLK_ESCAPE:
			this->pushEvent(KEY_ESCAPE);
			break;
		default:
			break;
		}
		default:
			break;

	}
}

void Menu::setText(const char **messages, int *submenu_defaults)
{
	int submenu;

	/* Free the old stuff */
	this->n_submenus = 0;
	free(this->p_submenus);
	free(this->pp_msgs);

	for (this->n_entries = 0; messages[this->n_entries]; this->n_entries++)
	{
		/* Is this a submenu? */
		if (IS_SUBMENU(messages[this->n_entries]))
		{
			this->n_submenus++;
			continue; /* Length of submenus is unimportant */
		}
	}
	this->pp_msgs = (const char **)xmalloc(this->n_entries * sizeof(const char *));
	this->p_submenus = (submenu_t *)xmalloc(this->n_submenus * sizeof(submenu_t));
	for (int i = 0; i < this->n_entries; i++)
		this->pp_msgs[i] = xstrdup(messages[i]);

	submenu = 0;

	for (int i = 0; i < this->n_entries; i++)
	{
		if (IS_SUBMENU(this->pp_msgs[i]))
		{
			int n;

			this->p_submenus[submenu].index = i;
			this->p_submenus[submenu].sel = submenu_defaults ? submenu_defaults[submenu] : 0;
			this->p_submenus[submenu].n_entries = 0;
			for (n = 0; this->pp_msgs[i][n] != '\0'; n++)
			{
				if (this->pp_msgs[i][n] == '|')
					this->p_submenus[submenu].n_entries++;
			}
			submenu++;
		}
	}

	/* Move selection to the first entry */
	this->selectOne(0);
}

Menu::Menu(Font *font)
{
	this->setTextColor((SDL_Color){0xff,0xff,0xff,0});
	this->font = font;

	this->text_bg_left = NULL;
	this->text_bg_middle = NULL;
	this->text_bg_right = NULL;
	this->submenu_bg_left =  NULL;
	this->submenu_bg_middle = NULL;
	this->submenu_bg_right = NULL;

	this->pp_msgs = NULL;
	this->n_entries = 0;
	this->p_submenus = NULL;
	this->n_submenus = 0;

	this->cur_sel = 0;
	this->mouse_x = -1;
	this->mouse_y = -1;

	memset(this->event_stack, 0, sizeof(this->event_stack));
	this->ev_head = this->ev_tail = 0;
}

void Menu::setTextColor(SDL_Color clr)
{
	this->text_color = clr;
}

Menu::~Menu()
{
	free(this->pp_msgs);
	free(this->p_submenus);
}

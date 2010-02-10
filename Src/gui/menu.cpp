/*********************************************************************
*
* Copyright (C) 2004-2009,  Simon Kagstrom
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

#include <utils.hh>

#include "menu.hh"
#include "font.hh"
#include "gui.hh"

#define IS_SUBMENU(p_msg) ( (p_msg)[0] == '^' )
#define IS_EMPTY(p_msg) ( (p_msg)[0] == '#' )

void Menu::printText(SDL_Surface *where, const char *msg, SDL_Color clr,
		int x, int y, int w, int h)
{
	char *buf;
	unsigned int i;
	int tw;

	buf = xstrdup(msg);
	tw = this->font->getWidth(buf);

	/* Crop text */
	if (tw > w)
	{
		int pixels_per_char = tw / strlen(buf);
		int last_char = (w / pixels_per_char) - 1;

		if ((unsigned)last_char > strlen(buf))
			last_char = strlen(buf) - 3;

		/* FIXME! Handle some corner cases here (short strings etc) */
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
	if (IS_EMPTY(buf))
		buf[0] = ' ';

	this->font->draw(where, buf, x, y, w, h);
	free(buf);
}


void Menu::draw(SDL_Surface *where, int x, int y, int w, int h)
{
	int font_height = this->font->getHeight("X");
	int line_height = (font_height + font_height / 4);
	int x_start = x;
	int entries_visible = h / line_height - 1;
	int start_entry_visible = 0;

	/* No messages - nothing to draw */
	if (!this->pp_msgs)
		return;

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

	if (start_entry_visible + entries_visible > this->n_entries)
		entries_visible = this->n_entries - start_entry_visible;

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

			highlight_background(where, this->font,
					this->text_bg_left, this->text_bg_middle, this->text_bg_right,
					x_start, cur_y, tw, th);
		}

		if (IS_SUBMENU(msg))
		{
			submenu_t *p_submenu = this->findSubmenu(i);
			int n_pipe = 0;
			int total_chars = 0;
			int tw, th, tw_first;
			int n_chars = 0;
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

			highlight_background(where, this->font,
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


int Menu::selectOne(int which)
{
	panic_if(!this->pp_msgs,
			"Can't select a message without any messages!");

	if (which >= this->n_entries)
		which = 0;
	this->cur_sel = which;

	if (this->pp_msgs[this->cur_sel][0] == ' ' ||
			IS_SUBMENU(this->pp_msgs[this->cur_sel]))
		this->selectNext(0, 1);
	this->hoverCallback(this->cur_sel);

	return this->cur_sel;
}

int Menu::selectNext(int dx, int dy)
{
	int next;

	panic_if(!this->pp_msgs,
			"Can't select the next message without any messages!");

	this->cur_sel = this->getNextEntry(dy);
	next = this->getNextEntry(dy + 1);

	/* We want to skip this for some reason */
	if (this->pp_msgs[this->cur_sel][0] == ' ' ||
			IS_SUBMENU(this->pp_msgs[this->cur_sel]) ) {
		return this->selectNext(dx, dy);
	}

	/* If the next is a submenu */
	if (dx != 0 && IS_SUBMENU(this->pp_msgs[next]))
	{
		submenu_t *p_submenu = findSubmenu(next);

		panic_if(!p_submenu, "submenu in the menu text but no actual submenu\n");
		p_submenu->sel = (p_submenu->sel + dx) < 0 ? p_submenu->n_entries - 1 :
		(p_submenu->sel + dx) % p_submenu->n_entries;
	}

	return this->cur_sel;
}

int Menu::selectNext(event_t ev)
{
	int now = this->cur_sel;
	int next;

	switch (ev)
	{
	case KEY_UP:
		next = this->selectNext(0, -1); break;
	case KEY_DOWN:
		next = this->selectNext(0, 1); break;
	case KEY_PAGEUP:
		next = this->selectNext(0, -6); break;
	case KEY_PAGEDOWN:
		next = this->selectNext(0, 6); break;
	case KEY_LEFT:
		next = this->selectNext(-1, 0); break;
	case KEY_RIGHT:
		next = this->selectNext(1, 0); break;
	default:
		panic("selectNext(ev) called with event %d\n", ev);
	}

	if (now != next)
		this->hoverCallback(this->cur_sel);

	return this->cur_sel;
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
		case KEY_PAGEUP:
		case KEY_PAGEDOWN:
		case KEY_LEFT:
		case KEY_RIGHT:
			this->selectNext(ev);
			break;
		case KEY_SELECT:
			this->selectCallback(this->cur_sel);
			/* Might be deleted */
			return;
		case KEY_ESCAPE:
			this->escapeCallback(this->cur_sel);
			/* Might be deleted */
			return;
		default:
			break;
		}
	}
}

void Menu::setText(const char **messages, int *submenu_defaults)
{
	int submenu;
	int i;

	/* Free the old stuff */
	for (i = 0; i < this->n_entries; i++)
		free((void*)this->pp_msgs[i]);
	this->n_submenus = 0;
	free(this->p_submenus);
	free(this->pp_msgs);

	/* Empty messages are allowed */
	this->p_submenus = NULL;
	this->pp_msgs = NULL;
	this->n_entries = 0;
	if (!messages)
		return;

	for (this->n_entries = 0; messages[this->n_entries]; this->n_entries++)
	{
		/* Is this a submenu? */
		if (IS_SUBMENU(messages[this->n_entries]))
		{
			this->n_submenus++;
			continue; /* Length of submenus is unimportant */
		}
	}

	this->pp_msgs = (const char **)xmalloc((this->n_entries + 1) * sizeof(const char *));
	if (this->n_submenus)
		this->p_submenus = (submenu_t *)xmalloc(this->n_submenus * sizeof(submenu_t));
	for (i = 0; i < this->n_entries; i++)
		this->pp_msgs[i] = xstrdup(messages[i]);
	this->pp_msgs[i] = NULL;


	submenu = 0;

	for (i = 0; i < this->n_entries; i++)
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

Menu::Menu(Font *font) : Widget()
{
	this->setTextColor((SDL_Color){0xff,0xff,0xff,0});
	this->font = font;

	this->pp_msgs = NULL;
	this->n_entries = 0;
	this->p_submenus = NULL;
	this->n_submenus = 0;

	this->cur_sel = 0;
	this->mouse_x = -1;
	this->mouse_y = -1;

	this->setSelectedBackground(Gui::gui->bg_left, Gui::gui->bg_middle,
			Gui::gui->bg_right, Gui::gui->bg_submenu_left,
			Gui::gui->bg_submenu_middle, Gui::gui->bg_submenu_right);
}

void Menu::setTextColor(SDL_Color clr)
{
	this->text_color = clr;
}

Menu::~Menu()
{
	for (int i = 0; i < this->n_entries; i++)
		free((void*)this->pp_msgs[i]);
	free(this->pp_msgs);
	free(this->p_submenus);
}

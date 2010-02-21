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
#ifndef __VIRTUAL_KEYBORD_HH__
#define __VIRTUAL_KEYBORD_HH__

#include <SDL.h>

#include "widget.hh"
#include "gui_view.hh"
#include "font.hh"
#include "listener.hh"

struct virtkey; 

class KeyboardListener : public Listener
{
public:
	~KeyboardListener();

	/* Each key is a string */
	virtual void keyCallback(bool shift, const char *str)
	{
	}

	virtual void stringCallback(const char *str)
	{
	}
};

class VirtualKeyboard : public GuiView, public ListenerManager
{
public:
	VirtualKeyboard(Font *font);
	~VirtualKeyboard();

	/* Conversions */
	const char *keycodeToString(int kc);
	const char keycodeToChar(int kc);
	int charToKeycode(char c);
	int stringToKeycode(const char *str);
	struct virtkey eventToVirtkey(event_t ev);

	void activate(bool default_shifted = true);

	void setFont(Font *font)
	{
		this->font = font;
	}

	void deactivate();

	bool isActive()
	{
		return this->is_active;
	}

	virtual void updateTheme();

	void draw(SDL_Surface *where);

	void runLogic();

	void draw(SDL_Surface *where, int x, int y, int w, int h);

	void pushEvent(event_t ev);

	void pushEvent(SDL_Event *ev);

	const char *getString();

	/* Singleton object */
	static VirtualKeyboard *kbd;
private:
	void selectNext(int dx, int dy);

	void toggleShift();

	void pushKey(struct virtkey *vk);

	void done();

	Font *font;
	int sel_x;
	int sel_y;
	bool shift_on;

	bool kbd_only_input;
	bool default_shifted;

	bool is_active;
	struct virtkey *buf;
	unsigned buf_head;
	size_t buf_len;
};

#endif /* __VIRTUAL_KEYBORD_HH__ */

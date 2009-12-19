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
#include "font.hh"

struct virtkey; 

class KeyListener
{
public:
	/* Each key is a string */
	virtual void keyCallback(bool shift, const char *str) = 0;
};

class StringListener
{
public:
	virtual void stringCallback(const char *str) = 0;
};

class VirtualKeyboard : public Widget
{
public:
	VirtualKeyboard(Font *font);

	void registerListener(KeyListener *kl);
	void registerListener(StringListener *sl);
	void unregisterListener(KeyListener *kl);
	void unregisterListener(StringListener *sl);

	/* Conversions */
	const char *keycodeToString(int kc);
	const char keycodeToChar(int kc);
	int charToKeycode(char c);
	int stringToKeycode(const char *str);

	void activate();

	void setFont(Font *font)
	{
		this->font = font;
	}

	void deactivate()
	{
		this->is_active = false;
	}

	void runLogic();

	void draw(SDL_Surface *where, int x, int y, int w, int h);

	/* Singleton object */
	static VirtualKeyboard *kbd;
private:
	KeyListener *keyListeners[8];
	StringListener *stringListeners[8];

	void selectNext(int dx, int dy);
	void toggleShift();

	Font *font;
	int sel_x;
	int sel_y;
	bool shift_on;

	bool is_active;
	char buf[255];
	unsigned buf_head;
};

#endif /* __VIRTUAL_KEYBORD_HH__ */

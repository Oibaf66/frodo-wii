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
#include <SDL.h>
#include <SDL_ttf.h>

struct virtkey; 

class KeyListener
{
	void keyCallback(char c);
};

class StringListener
{
	void stringCallback(const char *str);
};

class VirtualKeyboard : public Widget
{
public:
	VirtualKeyboard(TTF_Font *font);

	void registerListener(KeyListener *kl);
	void registerListener(StringListener *sl);
	void unregisterListener(KeyListener *kl);
	void unregisterListener(StringListener *sl);

	/* Conversions */
	int getKey();
	const char *getString();
	const char *keycodeToString(int kc);
	const char keycodeToChar(int kc);
	int charToKeycode(char c);

	void pushEvent(SDL_Event *ev);

	void draw(SDL_Surface *where, int x, int y, int w, int h);

	/* Singleton object */
	static VirtualKeyboard *kbd;
private:
	KeyListener *keyListeners[8];
	StringListener *stringListeners[8];

	struct virtkey *getKeyInternal();
	void selectNext(int dx, int dy);
	void toggleShift();

	TTF_Font *font;
	int sel_x;
	int sel_y;
	bool shift_on;

	char buf[255];
};

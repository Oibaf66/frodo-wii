#ifndef WIDGET_HH
#define WIDGET_HH

#include <SDL.h>

enum {
	EVENT_NONE   = 0,
	KEY_UP       = 1,
	KEY_DOWN     = 2,
	KEY_LEFT     = 4,
	KEY_RIGHT    = 8,
	KEY_SELECT   = 16,
	KEY_ESCAPE   = 32,
	KEY_PAGEDOWN = 64,
	KEY_PAGEUP   = 128,
	KEY_HELP     = 256,
};


class Widget
{
public:
	virtual void pushEvent(SDL_Event *ev) = 0;

	virtual void runLogic() = 0;

	virtual void draw(SDL_Surface *where,
			int x, int y, int w, int h) = 0;
};

#endif

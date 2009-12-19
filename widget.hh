#ifndef WIDGET_HH
#define WIDGET_HH

#include <SDL.h>

enum key_event {
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

typedef enum key_event event_t;

class Widget
{
public:
	virtual void pushEvent(event_t ev);

	virtual void pushEvent(SDL_Event *ev);

	virtual void runLogic() = 0;

	virtual void draw(SDL_Surface *where,
			int x, int y, int w, int h) = 0;

	virtual event_t popEvent();

protected:
	int 	   ev_head, ev_tail;
	event_t	   event_stack[8];
};

#endif

#ifndef WIDGET_HH
#define WIDGET_HH

#include <SDL.h>

enum key_event {
	EVENT_NONE   = (1 << 8),
	KEY_UP       = (1 << 9),
	KEY_DOWN     = (1 << 10),
	KEY_LEFT     = (1 << 11),
	KEY_RIGHT    = (1 << 12),
	KEY_SELECT   = (1 << 13),
	KEY_ESCAPE   = (1 << 14),
	KEY_PAGEDOWN = (1 << 15),
	KEY_PAGEUP   = (1 << 16),
	KEY_HELP     = (1 << 17),
	KEY_ENTER_MENU = (1 << 18),
};

typedef enum key_event event_t;

class Widget
{
public:
	Widget();

	virtual void pushEvent(event_t ev);

	virtual void runLogic() = 0;

	virtual void draw(SDL_Surface *where,
			int x, int y, int w, int h);

	virtual event_t popEvent();

protected:
	int 	   ev_head, ev_tail;
	event_t	   event_stack[8];
};

#endif

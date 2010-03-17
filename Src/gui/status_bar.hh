#ifndef __STATUS_BAR_HH__
#define __STATUS_BAR_HH__

#include <timer.hh>

#include "menu.hh"
#include "gui.hh"

#define N_STATUS_MESSAGES 10

class StatusBar : public Menu, public TimeoutHandler
{
public:
	StatusBar();

	void queueMessage(const char *message, ...);

	virtual void draw(SDL_Surface *where);

	virtual void hoverCallback(int which) {};

	virtual void selectCallback(int which) {};

	virtual void escapeCallback(int which) {};

protected:
	virtual void timeoutCallback();

	const char *dequeueMessage();

	const char *messages[N_STATUS_MESSAGES];
	const char *cur_message;
	int head, tail;
	int x;
	int y;
};

#endif /* __DIALOGUE_BOX_HH__ */

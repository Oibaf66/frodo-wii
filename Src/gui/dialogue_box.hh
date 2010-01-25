#ifndef __DIALOGUE_BOX_HH__
#define __DIALOGUE_BOX_HH__

#include "menu.hh"
#include "listener.hh"
#include "gui_view.hh"
#include "gui.hh"

class DialogueBox;

class DialogueListener : public Listener
{
public:
	virtual void selectCallback(DialogueBox *which, int selected);

	virtual void escapeCallback(DialogueBox *which, int selected);
};

class DialogueBox : public Menu, public ListenerManager
{
public:
	DialogueBox(const char *msgs[], bool delete_on_action = true);

	virtual void selectCallback(int which);

	virtual void escapeCallback(int which);

	virtual void hoverCallback(int which);

	virtual int selectNext(event_t ev);

	virtual void draw(SDL_Surface *where);

protected:
	bool delete_on_action;
};

#endif /* __DIALOGUE_BOX_HH__ */

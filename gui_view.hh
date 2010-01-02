#ifndef __GUI_VIEW_HH__
#define __GUI_VIEW_HH__

#include <SDL.h>

#include "widget.hh"

class GuiView : public Widget
{
public:
	GuiView();

	virtual void updateTheme();

	virtual void viewPushCallback();

	virtual void viewPopCallback();

	virtual void draw(SDL_Surface *where) = 0;
};

#endif /* __GUI_VIEW_HH__ */

#include "gui.hh"
#include "menu.hh"
#include "help_box.hh"
#include "virtual_keyboard.hh"

#include <sysdeps.h>
#include <C64.h>

class NetworkRegionView;

class NetworkRegionMenu : public Menu
{
	friend class NetworkRegionView;

public:
	NetworkRegionMenu(Font *font) : Menu(font)
	{
		memset(this->messages, 0, sizeof(this->messages));
		for (int i = REGION_UNKNOWN; i < REGION_ANTARTICA; i++)
			this->messages[i] = region_to_str(i);

		this->setText(this->messages);
	}

	~NetworkRegionMenu()
	{
	}

	virtual void selectCallback(int which)
	{
		Gui::gui->np->NetworkRegion = which;
		Gui::gui->popView();
	}

	virtual void escapeCallback(int which)
	{
		Gui::gui->popView();
	}

	virtual void hoverCallback(int which)
	{
	}

private:
	void updateMessages()
	{
	}

	const char *messages[REGION_ANTARTICA + 1];

	HelpBox *help;
};


class NetworkRegionView : public GuiView
{
public:
	NetworkRegionView() : GuiView()
	{
		this->menu = new NetworkRegionMenu(Gui::gui->default_font);
	}

	~NetworkRegionView()
	{
		delete this->menu;
	}

	void runLogic()
	{
		this->menu->runLogic();
	}

	void pushEvent(event_t ev)
	{
		this->menu->pushEvent(ev);
	}

	void viewPushCallback()
	{
		this->menu->updateMessages();
	}

	void draw(SDL_Surface *where)
	{
		 SDL_Rect dst;

		 /* Blit the backgrounds */
		 dst = (SDL_Rect){20,45,300,400};
		 SDL_BlitSurface(Gui::gui->main_menu_bg, NULL, where, &dst);

		 dst = (SDL_Rect){350,13,0,0};
		 SDL_BlitSurface(Gui::gui->infobox, NULL, where, &dst);

		 this->menu->draw(where, 50, 70, 300, 400);
	}

protected:
	NetworkRegionMenu *menu;
};

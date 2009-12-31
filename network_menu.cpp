#include "gui.hh"
#include "menu.hh"
#include "help_box.hh"

class NetworkView;
class NetworkMenu : public Menu
{
	friend class NetworkView;

public:
	NetworkMenu(Font *font, HelpBox *help) : Menu(font)
	{
		this->help = help;
		this->updateMessages();
	}

	~NetworkMenu()
	{
	}

	virtual void selectCallback(int which)
	{
		printf("option entry %d selected: %s\n", which, this->pp_msgs[which]);
		switch (which)
		{
		case 0: /* Insert disc */
			break;
		case 2: /* Load/save states */
			break;
		case 4: /* Keyboard */
			break;
		case 7: /* Reset the C64 */
			break;
		case 8: /* Networking */
			break;
		case 9: /* Options */
			break;
		case 10: /* Help */
			break;
		}
	}

	virtual void hoverCallback(int which)
	{
		this->help->updateHelpMessage(which);
	}

	virtual void escapeCallback(int which)
	{
		Gui::gui->popView();
	}

private:
	void updateMessages()
	{
		memset(this->strs, 0, sizeof(this->strs));
		snprintf(this->strs[0], sizeof(this->strs[0]) - 1, "Set username (%s)",
				"Vobb"); // FIXME!
		snprintf(this->strs[1], sizeof(this->strs[1]) - 1, "Server (%s)",
				"play.c64-network.org"); // FIXME!
		snprintf(this->strs[2], sizeof(this->strs[2]) - 1, "Server port (%d)",
				46214); // FIXME!

		this->messages[0] = this->strs[0];
		this->messages[1] = this->strs[1];
		this->messages[2] = this->strs[2];

		this->messages[3] = " ";
		this->messages[4] = "Connect to the network!";
		this->messages[5] = " ";
		this->messages[6] = "Post network message";
		this->messages[7] = NULL;
		this->setText(this->messages);
	}

	char strs[3][255];
	const char *messages[8];

	HelpBox *help;
};


class NetworkView : public GuiView
{
public:
	NetworkView() : GuiView()
	{
		this->help = new HelpBox(NULL, network_menu_help);
		this->menu = new NetworkMenu(NULL, this->help);
	}

	~NetworkView()
	{
		delete this->help;
		delete this->menu;
	}

	void updateTheme()
	{
		this->menu->setFont(Gui::gui->default_font);
		this->help->setFont(Gui::gui->small_font);
		this->menu->setSelectedBackground(Gui::gui->bg_left, Gui::gui->bg_middle,
				Gui::gui->bg_right, Gui::gui->bg_submenu_left,
				Gui::gui->bg_submenu_middle, Gui::gui->bg_submenu_right);
	}

	void runLogic()
	{
		this->menu->runLogic();
	}

	void pushEvent(SDL_Event *ev)
	{
		this->menu->pushEvent(ev);
	}

	void draw(SDL_Surface *where)
	{
		 SDL_Rect dst;

		 /* Blit the backgrounds */
		 dst = (SDL_Rect){20,45,300,400};
		 SDL_BlitSurface(Gui::gui->main_menu_bg, NULL, where, &dst);

		 dst = (SDL_Rect){350,13,0,0};
		 SDL_BlitSurface(Gui::gui->infobox, NULL, where, &dst);

		 dst = (SDL_Rect){350,242,0,0};
		 SDL_BlitSurface(Gui::gui->textbox, NULL, where, &dst);

		 this->menu->draw(where, 50, 70, 300, 400);
		 this->help->draw(where, 354, 24, 264, 210);
	}

protected:
	NetworkMenu *menu;
	HelpBox *help;
};
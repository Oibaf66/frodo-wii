#include "gui.hh"
#include "menu.hh"
#include "help_box.hh"

class OptionsView;
class OptionsMenu : public Menu
{
	friend class OptionsView;

public:
	OptionsMenu(Font *font, HelpBox *help) : Menu(font)
	{
		this->help = help;
	}

	~OptionsMenu()
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
	HelpBox *help;
};


class OptionsView : public GuiView
{
public:
	OptionsView() : GuiView()
	{
		this->help = new HelpBox(NULL, options_menu_help);
		this->menu = new OptionsMenu(NULL, this->help);
		this->menu->setText(options_menu_messages);
	}

	~OptionsView()
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
	OptionsMenu *menu;
	HelpBox *help;
};

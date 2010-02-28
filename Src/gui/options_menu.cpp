#include "gui.hh"
#include "menu.hh"
#include "help_box.hh"
#include "status_bar.hh"

#include <Prefs.h>

class OptionsView;
class OptionsMenu : public Menu
{
	friend class OptionsView;

public:
	OptionsMenu(Font *font, HelpBox *help) : Menu(font)
	{
		this->help = help;
		this->setText(options_menu_messages);
	}

	~OptionsMenu()
	{
	}

	virtual void selectCallback(int which)
	{
		if (which == 10) /* Game info */
		{
			Gui::gui->status_bar->queueMessage("Resetting the C64");
			Gui::gui->exitMenu();
			TheC64->Reset();
			return;
		}
		/* Select theme */
		if (which == 12)
		{
			Gui::gui->tv->setDirectory(Gui::gui->theme_base_path);
			Gui::gui->pushView(Gui::gui->tv);
			return;
		}

		/* Doesn't matter which otherwise, it's just selection */
		this->updatePrefs();
		Gui::gui->popView();
	}

	virtual void hoverCallback(int which)
	{
		this->help->updateHelpMessage(which);
	}

	virtual void escapeCallback(int which)
	{
		this->updatePrefs();
		Gui::gui->popView();
	}

	void updatePrefs()
	{
		Gui::gui->np->JoystickSwap = !this->p_submenus[0].sel;
		Gui::gui->np->Emul1541Proc = !this->p_submenus[1].sel;
		Gui::gui->np->ShowLEDs = !this->p_submenus[2].sel;
		Gui::gui->np->DisplayOption = this->p_submenus[3].sel;

		switch (this->p_submenus[4].sel)
		{
		case 0:
			Gui::gui->np->MsPerFrame = SPEED_95; break;
		case 1:
			Gui::gui->np->MsPerFrame = SPEED_100; break;
		case 2:
			Gui::gui->np->MsPerFrame = SPEED_110; break;
		default:
			panic("Impossible submenu value: %d\n", this->p_submenus[4].sel);
		}
	}

	void updateSubmenus()
	{
		int submenu_defs[5];

		submenu_defs[0] = Gui::gui->np->JoystickSwap == true ? 0 : 1;
		submenu_defs[1] = !Gui::gui->np->Emul1541Proc;
		submenu_defs[2] = !Gui::gui->np->ShowLEDs;
		submenu_defs[3] = Gui::gui->np->DisplayOption;

		switch (Gui::gui->np->MsPerFrame)
		{
	        case SPEED_95:
	        	submenu_defs[4] = 0; break;
		case SPEED_110:
			submenu_defs[4] = 2; break;
		default:
	                /* If it has some other value... */
		        submenu_defs[4] = 1; break;
		}

		this->setText(options_menu_messages, submenu_defs);
	}

private:
	HelpBox *help;
};


class OptionsView : public GuiView
{
public:
	OptionsView() : GuiView()
	{
		this->help = new HelpBox(Gui::gui->small_font, options_menu_help);
		this->menu = new OptionsMenu(Gui::gui->default_font, this->help);
	}

	~OptionsView()
	{
		delete this->help;
		delete this->menu;
	}

	void viewPushCallback()
	{
		this->menu->updateSubmenus();
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
		 SDL_BlitSurface(Gui::gui->network_message_box, NULL, where, &dst);

		 this->menu->draw(where, 50, 70, 300, 400);
		 this->help->draw(where, 354, 24, 264, 210);
	}

protected:
	OptionsMenu *menu;
	HelpBox *help;
};

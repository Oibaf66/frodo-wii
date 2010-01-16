#include "menu.hh"
#include "dialogue_box.hh"

class KeyboardTypingListener : public KeyboardListener
{
	virtual void stringCallback(const char *str)
	{
		printf("string: %s\n", str);
		/* Remove thyself! */
		delete this;
	}

	virtual void keyCallback(bool shift, const char *str)
	{
		printf("Vobb: %d: %s\n", shift, str);
	}
};

class ExitListener : public DialogueListener
{
	void escapeCallback(DialogueBox *which, int selected)
	{
	}

	void selectCallback(DialogueBox *which, int selected)
	{
		/* Cancel? */
		if (selected != 1)
			exit(0);
	}
};

class MainView;
class MainMenu : public Menu
{
	friend class MainView;

public:
	MainMenu(Font *font, HelpBox *help) : Menu(font)
	{
		this->help = help;

		this->updatePauseState();
	}

	virtual void selectCallback(int which)
	{
		printf("entry %d selected: %s\n", which, this->pp_msgs[which]);
		switch (which)
		{
		case 0:
			TheC64->IsPaused() ? TheC64->Resume() : TheC64->Pause();
			if (TheC64->IsPaused())
				Gui::gui->status_bar->queueMessage("C64 emulation paused");
			else
				Gui::gui->status_bar->queueMessage("C64 emulation resumed");
			this->updatePauseState();
			break;
		case 2: /* Insert disc */
			if (this->p_submenus[0].sel == 0) {
				Gui::gui->dv->setDirectory("discs");
				Gui::gui->pushView(Gui::gui->dv);
			}
			break;
		case 4: /* Load/save states */
			break;
		case 6: /* Keyboard */
			switch(this->p_submenus[2].sel)
			{
			case 0:
				VirtualKeyboard::kbd->activate();
				VirtualKeyboard::kbd->registerListener(new KeyboardTypingListener());
				break;
			case 1:
				break;
			case 2:
				Gui::gui->pushView(Gui::gui->bkv);
				break;
			default:
				panic("Illegal selection\n");
			}
			break;
		case 9: /* Game info */
			Gui::gui->pushView(Gui::gui->giv);
			break;
		case 10: /* Networking */
			Gui::gui->pushView(Gui::gui->nv);
			break;
		case 11: /* Options */
			Gui::gui->pushView(Gui::gui->ov);
			break;
		case 12: /* Exit */
			DialogueBox *exit_dialogue = new DialogueBox(exit_dialogue_messages);
			exit_dialogue->registerListener(new ExitListener());
			Gui::gui->pushDialogueBox(exit_dialogue);
			break;
		}
	}

	virtual void hoverCallback(int which)
	{
		this->help->updateHelpMessage(which);
	}

	virtual void escapeCallback(int which)
	{
		Gui::gui->exitMenu();
	}

private:
	void updatePauseState()
	{
		if (TheC64->IsPaused())
			main_menu_messages[0] = "Resume";
		else
			main_menu_messages[0] = "Pause";
		this->setText(main_menu_messages);
	}

	HelpBox *help;
};


class MainView : public GuiView
{
public:
	MainView() : GuiView()
	{
		panic_if(!Gui::gui->default_font,
				"Theme does not seem correctly loaded\n");

		this->help = new HelpBox(Gui::gui->small_font, main_menu_help);
		this->menu = new MainMenu(Gui::gui->default_font, this->help);
	}

	~MainView()
	{
		delete this->help;
		delete this->menu;
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
	MainMenu *menu;
	HelpBox *help;
};

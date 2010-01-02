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
		delete this;
	}

	void selectCallback(DialogueBox *which, int selected)
	{
		if (selected != which->cancelIndex())
			exit(0);
		Gui::gui->popDialogueBox();
		delete this;
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
	}

	virtual void selectCallback(int which)
	{
		printf("entry %d selected: %s\n", which, this->pp_msgs[which]);
		switch (which)
		{
		case 0: /* Insert disc */
			if (this->p_submenus[0].sel == 0) {
				Gui::gui->dv->setDirectory("discs");
				Gui::gui->pushView(Gui::gui->dv);
			}
			break;
		case 2: /* Load/save states */
			break;
		case 4: /* Keyboard */
			Gui::gui->kv->activate();
			Gui::gui->kv->registerListener(new KeyboardTypingListener());
			break;
		case 7: /* Reset the C64 */
			printf("Resetting the C64\n");
			break;
		case 8: /* Networking */
			Gui::gui->pushView(Gui::gui->nv);
			break;
		case 9: /* Options */
			Gui::gui->pushView(Gui::gui->ov);
			break;
		case 10: /* Help */
			break;

		case 11: /* Exit */
			DialogueBox *exit_dialogue = new DialogueBox(exit_dialogue_messages, 1);
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
	HelpBox *help;
};


class MainView : public GuiView
{
public:
	MainView() : GuiView()
	{
		this->help = new HelpBox(NULL, main_menu_help);
		this->menu = new MainMenu(NULL, this->help);
		this->menu->setText(main_menu_messages);
	}

	~MainView()
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
	MainMenu *menu;
	HelpBox *help;
};

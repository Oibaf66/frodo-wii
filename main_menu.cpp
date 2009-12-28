#include "menu.hh"

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

class MainView;
class MainMenu : public Menu
{
	friend class MainView;

	class ExitDialogue : public DialogueBox
	{
	public:
		ExitDialogue(Font *font) : DialogueBox(font, exit_dialogue_messages, 1)
		{
		}

		void selectCallback(int which)
		{
			this->m_selected = this->p_submenus[0].sel;
			/* Do the exit */
			if (this->m_selected != this->m_cancel)
				exit(1);
		}
	};

public:
	MainMenu(Font *font, HelpBox *help) : Menu(font)
	{
		this->help = help;
		/* The dialogue box is only present when needed */
		this->dialogue = NULL;
	}

	~MainMenu()
	{
		if (this->dialogue)
			delete this->dialogue;
	}

	void runLogic()
	{
		if (this->dialogue)
		{
			this->dialogue->runLogic();
			if (this->dialogue->selected() >= 0)
			{
				delete this->dialogue;
				this->dialogue = NULL;
			}
			return;
		}

		Menu::runLogic();
	}

	void pushEvent(SDL_Event *ev)
	{
		if (this->dialogue)
			this->dialogue->pushEvent(ev);
		else
			Menu::pushEvent(ev);
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
			Gui::gui->pushView(Gui::gui->kv);
			Gui::gui->kv->activate();
			Gui::gui->kv->registerListener(new KeyboardTypingListener());
			break;
		case 7: /* Reset the C64 */
			printf("Resetting the C64\n");
			break;
		case 8: /* Networking */
			break;
		case 9: /* Options */
			Gui::gui->pushView(Gui::gui->ov);
			break;
		case 10: /* Help */
			break;

		case 11: /* Exit */
			this->dialogue = new ExitDialogue(this->font);
			this->dialogue->setSelectedBackground(NULL, NULL, NULL,
					this->submenu_bg_left, this->submenu_bg_middle,
					this->submenu_bg_right);
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
	DialogueBox *dialogue;
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
		 if (this->menu->dialogue) {
			 int d_x = where->w / 2 - Gui::gui->dialogue_bg->w / 2;
			 int d_y = where->h / 2 - Gui::gui->dialogue_bg->h / 2;

			 dst = (SDL_Rect){d_x, d_y,
				 Gui::gui->dialogue_bg->w, Gui::gui->dialogue_bg->h};
			 SDL_BlitSurface(Gui::gui->dialogue_bg, NULL, where, &dst);

			 this->menu->dialogue->draw(where, d_x + 10, d_y + 10,
					 Gui::gui->dialogue_bg->w - 10, Gui::gui->dialogue_bg->h - 10);
		 }
	}

protected:
	MainMenu *menu;
	HelpBox *help;
};

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
	MainMenu(Font *font, HelpBox *help, GuiView *parent) : Menu(font)
	{
		this->parent = parent;
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
		case 11:
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
		this->parent->parent->exitMenu();
	}

private:
	DialogueBox *dialogue;
	GuiView *parent;
	HelpBox *help;
};


class MainView : public GuiView
{
public:
	MainView(Gui *parent) : GuiView(parent)
	{
		this->help = new HelpBox(NULL, main_menu_help);
		this->menu = new MainMenu(NULL, this->help, this);
		this->menu->setText(main_menu_messages);
		this->bg = NULL;
		this->infobox = NULL;
		this->textbox = NULL;
		this->dialogue_bg = NULL;
	}

	void updateTheme()
	{
		this->bg = parent->main_menu_bg;
		this->infobox = parent->infobox;
		this->textbox = parent->textbox;
		this->dialogue_bg = parent->dialogue_bg;

		this->menu->setFont(this->parent->default_font);
		this->help->setFont(this->parent->small_font);
		this->menu->setSelectedBackground(this->parent->bg_left, this->parent->bg_middle,
				this->parent->bg_right, this->parent->bg_submenu_left,
				this->parent->bg_submenu_middle, this->parent->bg_submenu_right);
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
		 SDL_BlitSurface(this->bg, NULL, where, &dst);

		 dst = (SDL_Rect){350,13,0,0};
		 SDL_BlitSurface(this->infobox, NULL, where, &dst);

		 dst = (SDL_Rect){350,242,0,0};
		 SDL_BlitSurface(this->textbox, NULL, where, &dst);

		 this->menu->draw(where, 50, 70, 300, 400);
		 this->help->draw(where, 354, 24, 264, 210);
		 if (this->menu->dialogue) {
			 int d_x = where->w / 2 - this->dialogue_bg->w / 2;
			 int d_y = where->h / 2 - this->dialogue_bg->h / 2;

			 dst = (SDL_Rect){d_x, d_y, this->dialogue_bg->w, this->dialogue_bg->h};
			 SDL_BlitSurface(this->dialogue_bg, NULL, where, &dst);

			 this->menu->dialogue->draw(where, d_x + 10, d_y + 10,
					 this->dialogue_bg->w - 10, this->dialogue_bg->h - 10);
		 }
	}

protected:
	MainMenu *menu;
	HelpBox *help;
	SDL_Surface *bg;
	SDL_Surface *infobox;
	SDL_Surface *textbox;
	SDL_Surface *dialogue_bg;
};

#include "gui.hh"
#include "menu.hh"
#include "help_box.hh"
#include "virtual_keyboard.hh"

class BindKeysView;
class BindKeysMenu : public Menu, public KeyboardListener
{
	friend class BindKeysView;

public:
	BindKeysMenu(Font *font, HelpBox *help) : Menu(font)
	{
		this->help = help;
		memset(this->hm, 0, sizeof(this->hm));
		this->setText(bind_key_menu_messages);
	}

	~BindKeysMenu()
	{
		this->freeHelpMessages();
	}

	virtual void selectCallback(int which)
	{
		int *ck = NULL;

		switch(which)
		{
		case 0: /* Classic */
			switch (this->p_submenus[0].sel)
			{
			case 0: ck = &Gui::gui->np->JoystickHats[0][0]; break;
			case 1: ck = &Gui::gui->np->JoystickHats[0][1]; break;
			case 2: ck = &Gui::gui->np->JoystickHats[0][2]; break;
			case 3: ck = &Gui::gui->np->JoystickHats[0][3]; break;
			case 4: ck = &Gui::gui->np->JoystickButtons[0][0]; break;
			case 5: ck = &Gui::gui->np->JoystickButtons[0][1]; break;
			case 6: ck = &Gui::gui->np->JoystickButtons[0][2]; break;
			case 7: ck = &Gui::gui->np->JoystickButtons[0][3]; break;
			case 8: ck = &Gui::gui->np->JoystickButtons[0][4]; break;
			default:
				panic("Classic: impossible selection %d", this->p_submenus[0].sel); break;
			}
			break;
		case 2: /* Nunchuk */
			switch (this->p_submenus[1].sel)
			{
			case 0: ck = &Gui::gui->np->JoystickAxes[0][0]; break;
			case 1: ck = &Gui::gui->np->JoystickAxes[0][1]; break;
			case 2: ck = &Gui::gui->np->JoystickButtons[0][7]; break;
			case 3: ck = &Gui::gui->np->JoystickButtons[0][8]; break;
			default:
				panic("Nunchuk: impossible selection %d", this->p_submenus[1].sel); break;
			}
			break;
		case 4: /* Classic */
			switch (this->p_submenus[2].sel)
			{
			case 0: ck = &Gui::gui->np->JoystickHats[0][0]; break;
			case 1: ck = &Gui::gui->np->JoystickHats[0][1]; break;
			case 2: ck = &Gui::gui->np->JoystickHats[0][2]; break;
			case 3: ck = &Gui::gui->np->JoystickHats[0][3]; break;
			case 4: ck = &Gui::gui->np->JoystickButtons[0][9]; break;
			case 5: ck = &Gui::gui->np->JoystickButtons[0][10]; break;
			case 6: ck = &Gui::gui->np->JoystickButtons[0][11]; break;
			case 7: ck = &Gui::gui->np->JoystickButtons[0][12]; break;
			case 8: ck = &Gui::gui->np->JoystickButtons[0][13]; break;
			case 9: ck = &Gui::gui->np->JoystickButtons[0][14]; break;
			case 10: ck = &Gui::gui->np->JoystickButtons[0][15]; break;
			case 11: ck = &Gui::gui->np->JoystickButtons[0][16]; break;
			case 12: ck = &Gui::gui->np->JoystickButtons[0][17]; break;
			case 13: ck = &Gui::gui->np->JoystickButtons[0][18]; break;
			default:
				panic("Classic: impossible selection %d", this->p_submenus[2].sel); break;
			}
			break;
		case 6:
			switch (this->p_submenus[3].sel)
			{
			case 0: ck = &Gui::gui->np->JoystickAxes[0][0]; break;
			case 1: ck = &Gui::gui->np->JoystickAxes[0][1]; break;
			default:
				panic("Classic: impossible selection %d", this->p_submenus[3].sel); break;
			}
		case 8:
			switch (this->p_submenus[4].sel)
			{
			case 0: ck = &Gui::gui->np->JoystickAxes[0][2]; break;
			case 1: ck = &Gui::gui->np->JoystickAxes[0][3]; break;
			default:
				panic("Classic: impossible selection %d", this->p_submenus[4].sel); break;
			}
		default:
			panic("Impossible menu option\n");
			break;
		}
		this->cur_key = ck;

		VirtualKeyboard::kbd->activate();
		VirtualKeyboard::kbd->registerListener(this);
	}

	virtual void hoverCallback(int which)
	{
		this->help->updateHelpMessage(which);
	}

	virtual void escapeCallback(int which)
	{
		Gui::gui->popView();
	}

	virtual void keyCallback(bool shift, const char *str)
	{
		panic_if(!this->cur_key, "No key selected but keyboard active???\n");

		*this->cur_key = this->stringToKeycode(str);
		this->updateHelpMessages();
		this->help->updateHelpMessage(this->cur_sel);
		VirtualKeyboard::kbd->deactivate();
		this->cur_key = NULL;
	}

	void updateHelpMessages()
	{
		this->freeHelpMessages();

		this->hm[0] = this->addOne(this->hm[0], this->allocOne("Up: %s", stringToPtr_Wiimote("Up")));
		this->hm[0] = this->addOne(this->hm[0], this->allocOne("Down: %s", stringToPtr_Wiimote("Down")));
		this->hm[0] = this->addOne(this->hm[0], this->allocOne("Left: %s", stringToPtr_Wiimote("Left")));
		this->hm[0] = this->addOne(this->hm[0], this->allocOne("Right: %s", stringToPtr_Wiimote("Right")));
		this->hm[0] = this->addOne(this->hm[0], this->allocOne("A: %s", stringToPtr_Wiimote("A")));
		this->hm[0] = this->addOne(this->hm[0], this->allocOne("B: %s", stringToPtr_Wiimote("B")));
		this->hm[0] = this->addOne(this->hm[0], this->allocOne("+: %s", stringToPtr_Wiimote("+")));
		this->hm[0] = this->addOne(this->hm[0], this->allocOne("-: %s", stringToPtr_Wiimote("-")));

		/* Nunchuk */
		this->hm[2] = this->addOne(this->hm[2], this->allocOne("Horiz: %s", stringToPtr_Nunchuk("Horiz")));
		this->hm[2] = this->addOne(this->hm[2], this->allocOne("Vert: %s", stringToPtr_Nunchuk("Vert")));
		this->hm[2] = this->addOne(this->hm[2], this->allocOne("Z: %s", stringToPtr_Nunchuk("Z")));
		this->hm[2] = this->addOne(this->hm[2], this->allocOne("C: %s", stringToPtr_Nunchuk("C")));

		/* Classic */
		this->hm[4] = this->addOne(this->hm[4], this->allocOne("Up: %s", stringToPtr_Classic("Up")));
		this->hm[4] = this->addOne(this->hm[4], this->allocOne("Down: %s", stringToPtr_Classic("Down")));
		this->hm[4] = this->addOne(this->hm[4], this->allocOne("Left: %s", stringToPtr_Classic("Left")));
		this->hm[4] = this->addOne(this->hm[4], this->allocOne("Right: %s", stringToPtr_Classic("Right")));
		this->hm[4] = this->addOne(this->hm[4], this->allocOne("a: %s", stringToPtr_Classic("a")));
		this->hm[4] = this->addOne(this->hm[4], this->allocOne("b: %s", stringToPtr_Classic("b")));
		this->hm[4] = this->addOne(this->hm[4], this->allocOne("x: %s", stringToPtr_Classic("x")));
		this->hm[4] = this->addOne(this->hm[4], this->allocOne("y: %s", stringToPtr_Classic("y")));
		this->hm[4] = this->addOne(this->hm[4], this->allocOne("Zl: %s", stringToPtr_Classic("Zl")));
		this->hm[4] = this->addOne(this->hm[4], this->allocOne("Zr: %s", stringToPtr_Classic("Zr")));
		this->hm[4] = this->addOne(this->hm[4], this->allocOne("+: %s", stringToPtr_Classic("+")));
		this->hm[4] = this->addOne(this->hm[4], this->allocOne("-: %s", stringToPtr_Classic("-")));

		/*
		this->hm[2] = this->addOne(this->hm[2], this->allocOne("l: %s", stringToPtr_Classic("l")));
		this->hm[2] = this->addOne(this->hm[2], this->allocOne("r: %s", stringToPtr_Classic("r")));
		this->hm[2] = this->addOne(this->hm[2], this->allocOne("R-toggle: %s", stringToPtr_Classic("RA")));
		this->hm[2] = this->addOne(this->hm[2], this->allocOne("L-toggle: %s", stringToPtr_Classic("LA")));
		*/

		this->hm[6] = this->addOne(this->hm[6], this->allocOne("Horiz: %s", stringToPtr_Classic("LAH")));
		this->hm[6] = this->addOne(this->hm[6], this->allocOne("Vert: %s", stringToPtr_Classic("LAV")));

		this->hm[8] = this->addOne(this->hm[8], this->allocOne("Horiz: %s", stringToPtr_Classic("RAH")));
		this->hm[8] = this->addOne(this->hm[8], this->allocOne("Vert: %s", stringToPtr_Classic("RAV")));

		this->help->setHelpMessages(this->hm);
	}

private:
	void freeHelpMessages()
	{
		for (unsigned i = 0; i < ARRAY_SIZE(this->hm); i++)
		{
			if (this->hm[i])
			{
				for (int j = 0; this->hm[i][j]; j++)
					free((void*)this->hm[i][j]);
				free(this->hm[i]);
			}
			this->hm[i] = NULL;
		}
	}

#define EQ(b) (strcmp(str, b) == 0)
	int *stringToPtr_Classic(const char *str)
	{
		if (EQ("Up"))
			return &Gui::gui->np->JoystickHats[0][0];
		if (EQ("Down"))
			return &Gui::gui->np->JoystickHats[0][1];
		if (EQ("Left"))
			return &Gui::gui->np->JoystickHats[0][2];
		if (EQ("Right"))
			return &Gui::gui->np->JoystickHats[0][3];
		if (EQ("LAH"))
			return &Gui::gui->np->JoystickAxes[0][0];
		if (EQ("LAV"))
			return &Gui::gui->np->JoystickAxes[0][1];
		if (EQ("RAH"))
			return &Gui::gui->np->JoystickAxes[0][2];
		if (EQ("RAV"))
			return &Gui::gui->np->JoystickAxes[0][3];
		if (EQ("RA"))
			return &Gui::gui->np->JoystickAxes[0][4];
		if (EQ("LA"))
			return &Gui::gui->np->JoystickAxes[0][5];
		if (EQ("a"))
			return &Gui::gui->np->JoystickButtons[0][9];
		if (EQ("b"))
			return &Gui::gui->np->JoystickButtons[0][10];
		if (EQ("x"))
			return &Gui::gui->np->JoystickButtons[0][11];
		if (EQ("y"))
			return &Gui::gui->np->JoystickButtons[0][12];
		if (EQ("L"))
			return &Gui::gui->np->JoystickButtons[0][13];
		if (EQ("R"))
			return &Gui::gui->np->JoystickButtons[0][14];
		if (EQ("Zl"))
			return &Gui::gui->np->JoystickButtons[0][15];
		if (EQ("Zr"))
			return &Gui::gui->np->JoystickButtons[0][16];
		if (EQ("-"))
			return &Gui::gui->np->JoystickButtons[0][17];
		if (EQ("+"))
			return &Gui::gui->np->JoystickButtons[0][18];
		if (EQ("Home"))
			return &Gui::gui->np->JoystickButtons[0][19];

		/* Shound never happen! */
		panic("Illegal string %s\n", str);

		return NULL;
	}

	int *stringToPtr_Nunchuk(const char *str)
	{
		if (EQ("Horiz"))
			return &Gui::gui->np->JoystickAxes[0][0];
		if (EQ("Vert"))
			return &Gui::gui->np->JoystickAxes[0][1];
		if (EQ("Z"))
			return &Gui::gui->np->JoystickButtons[0][7];
		if (EQ("C"))
			return &Gui::gui->np->JoystickButtons[0][8];

		/* Shound never happen! */
		panic("Illegal string %s\n", str);

		return NULL;
	}

	int *stringToPtr_Wiimote(const char *str)
	{
		if (EQ("Up"))
			return &Gui::gui->np->JoystickHats[0][0];
		if (EQ("Down"))
			return &Gui::gui->np->JoystickHats[0][1];
		if (EQ("Left"))
			return &Gui::gui->np->JoystickHats[0][2];
		if (EQ("Right"))
			return &Gui::gui->np->JoystickHats[0][3];
		if (EQ("A"))
			return &Gui::gui->np->JoystickButtons[0][0];
		if (EQ("B"))
			return &Gui::gui->np->JoystickButtons[0][1];
		if (EQ("1"))
			return &Gui::gui->np->JoystickButtons[0][2];
		if (EQ("2"))
			return &Gui::gui->np->JoystickButtons[0][3];
		if (EQ("+"))
			return &Gui::gui->np->JoystickButtons[0][4];
		if (EQ("-"))
			return &Gui::gui->np->JoystickButtons[0][5];
		if (EQ("Home"))
			return &Gui::gui->np->JoystickButtons[0][6];

		/* Shound never happen! */
		panic("Illegal string %s\n", str);

		return NULL;
	}
#undef EQ

	const char **addOne(const char **dst, const char *what)
	{
		int cur;
		int n = 0;

		if (dst != NULL)
		{
			for (n = 0; dst[n]; n++)
				;
		}
		cur = n;
		n++;
		dst = (const char **)xrealloc(dst, (n+1) * sizeof(const char*));
		dst[cur] = what;
		dst[n] = NULL;

		return dst;
	}

	const char *allocOne(const char *fmt, int *what)
	{
		const char *str = this->bindingToString(*what);
		size_t len = strlen(str) + strlen(fmt) + 2;
		char *out = (char *)xmalloc(len);

		sprintf(out, fmt, str);

		return out;
	}

	int stringToKeycode(const char *str)
	{
		if (strcmp(str, "None") == 0)
			return 0;

		/* default: */
		return VirtualKeyboard::kbd->stringToKeycode(str);
	}

	const char *bindingToString(int val)
	{
		switch(val)
		{
		case JOY_NONE:
			return "None";
		case JOY_HORIZ:
			return "Horizontal";
		case JOY_VERT:
			return "Vertical";
		case JOY_FIRE:
			return "Fire";
		default:
			break;
		}

		/* default: */
		return VirtualKeyboard::kbd->keycodeToString(val);
	}

	HelpBox *help;
	int *cur_key;
	const char **hm[10];
};


class BindKeysView : public GuiView
{
public:
	BindKeysView() : GuiView()
	{
		this->help = new HelpBox(NULL, NULL);
		this->menu = new BindKeysMenu(NULL, this->help);
	}

	~BindKeysView()
	{
		delete this->help;
		delete this->menu;
	}

	void updateTheme()
	{
		this->menu->setFont(Gui::gui->small_font);
		this->help->setFont(Gui::gui->small_font);
		this->menu->setSelectedBackground(Gui::gui->bg_left, Gui::gui->bg_middle,
				Gui::gui->bg_right, Gui::gui->bg_submenu_left,
				Gui::gui->bg_submenu_middle, Gui::gui->bg_submenu_right);
	}

	void viewPushCallback()
	{
		this->menu->updateHelpMessages();
		this->help->updateHelpMessage(0);
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
		 SDL_BlitSurface(Gui::gui->disc_info, NULL, where, &dst);

		 this->menu->draw(where, 50, 70, 300, 400);
		 this->help->draw(where, 354, 24, 264, 210);
	}

protected:
	BindKeysMenu *menu;
	HelpBox *help;
};

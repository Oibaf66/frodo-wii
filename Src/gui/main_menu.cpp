#include "menu.hh"
#include "dialogue_box.hh"

extern bool usbismount; 
extern bool smbismount;
extern bool sdismount;

class KeyboardTypingListener : public KeyboardListener, TimeoutHandler
{
public:
	virtual void stringCallback(const char *str)
	{
		/* Remove thyself! */
		delete this;
	}

	virtual void keyCallback(bool shift, const char *str)
	{
		this->kc = VirtualKeyboard::kbd->stringToKeycode(str);

		if (shift)
			this->kc |= 0x80;

		TheC64->pushKeyCode(this->kc, false);
		/* Release it soon */
		TimerController::controller->arm(this, 1);
	}

	virtual void timeoutCallback()
	{
		TheC64->pushKeyCode(this->kc, true);
	}

private:
	int kc;
};

class ExitListener : public DialogueListener
{
	void escapeCallback(DialogueBox *which, int selected)
	{
		delete this;
	}

	void selectCallback(DialogueBox *which, int selected)
	{
		/* Cancel? */
		if (selected != 1)
		{
			Gui::gui->exitMenu();
			TheC64->quit();
		}

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
		this->setText(main_menu_messages);
	}

	virtual void selectCallback(int which)
	{
		switch (which)
		{
		case 0: /* Insert disc */
			if (Gui::gui->np->Port == PORT_SD && sdismount) Gui::gui->dv->setDirectory(Gui::gui->game_base_path_sd);
			else if (Gui::gui->np->Port == PORT_USB && usbismount) Gui::gui->dv->setDirectory(Gui::gui->game_base_path_usb);
			else if (Gui::gui->np->Port == PORT_SMB && smbismount) Gui::gui->dv->setDirectory(Gui::gui->game_base_path_smb);
			else Gui::gui->dv->setDirectory(Gui::gui->game_base_path); //DEFAULT
			
			Gui::gui->pushView(Gui::gui->dv);
			
			Gui::gui->dv->runStartSequence(this->p_submenus[0].sel == 0);
			break;
		case 2: /* Load/save states */
			if (this->p_submenus[1].sel == 1)
				Gui::gui->sgv->saveSnapshot();
			else
			{
				Gui::gui->sgv->setDirectory(Gui::gui->save_game_path);
				Gui::gui->pushView(Gui::gui->sgv);

				Gui::gui->sgv->setLoadSnapshot(this->p_submenus[1].sel == 0);
			}
			break;
		case 4: /* Keyboard */
			switch(this->p_submenus[2].sel)
			{
			case 0:
				VirtualKeyboard::kbd->activate(false);
				VirtualKeyboard::kbd->registerListener(new KeyboardTypingListener());
				Gui::gui->exitMenu();
				break;
			case 1:
				Gui::gui->pushView(Gui::gui->bkv);
				break;
			case 2:
				Gui::gui->np->CursorKeysForJoystick = !Gui::gui->np->CursorKeysForJoystick;
				if (Gui::gui->np->CursorKeysForJoystick)
					Gui::gui->status_bar->queueMessage("Cursor keys used as joystick");
				else
					Gui::gui->status_bar->queueMessage("Cursor keys used as keyboard cursors");
				break;
			default:
				panic("Illegal selection\n");
			}
			break;
		case 7: /* Game info */
			Gui::gui->pushView(Gui::gui->giv);
			break;	
		case 8: /* Help */
			Gui::gui->pushDialogueBox(new DialogueBox(frodo_help));
			break;
		case 9: /* Networking */
			Gui::gui->pushView(Gui::gui->nv);
			break;
		case 10: /* Options */
			Gui::gui->pushView(Gui::gui->ov);
			break;
		case 11: /* Save Prefs */
			ThePrefs = *Gui::gui->np;
			ThePrefs.Save(ThePrefs.PrefsPath);
			Gui::gui->pushDialogueBox(new DialogueBox(save_prefs_done));
			break;
		case 12: /* Reset c64 */
			Gui::gui->status_bar->queueMessage("Resetting the C64");
			Gui::gui->exitMenu();
			TheC64->Reset();
			break;
		case 13: /* Exit */
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

	void pushEvent(event_t ev)
	{
		this->menu->pushEvent(ev);
	}

	void viewPushCallback()
	{
		this->menu->selectOne(0);
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
		 this->help->draw(where, 354, 24, 264, 210);
		 Gui::gui->server_msgs->draw(where);
	}

protected:
	MainMenu *menu;
	HelpBox *help;
};

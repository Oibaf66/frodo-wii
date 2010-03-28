#include <Display.h>
#include <C64.h>

#include "gui.hh"
#include "menu.hh"
#include "game_info_box.hh"
#include "virtual_keyboard.hh"

class GameInfoView;

class MultiSelectionDialogue : public DialogueBox
{
public:
	MultiSelectionDialogue(GameInfoBox *box, const char *msgs[]) : DialogueBox(msgs, true)
	{
		this->setSelectedBackground(Gui::gui->bg_left, Gui::gui->bg_middle,
				Gui::gui->bg_right,
				Gui::gui->bg_submenu_left, Gui::gui->bg_submenu_middle,
				Gui::gui->bg_submenu_right);
		this->cur_sel = 0;
		this->box = box;
	}

	int selectNext(event_t ev)
	{
		return Menu::selectNext(ev);
	}

protected:
	GameInfoBox *box;
};

class GenreDialogue : public MultiSelectionDialogue
{
public:
	GenreDialogue(GameInfoBox *box, const char *msgs[]) : MultiSelectionDialogue(box, msgs)
	{
	}

	void selectCallback(int which)
	{
		Gui::gui->popDialogueBox();

		box->gi->genre = which + 1;
		box->updateMessages();

		delete this;
	}
};

class PlayersDialogue : public MultiSelectionDialogue
{
public:
	PlayersDialogue(GameInfoBox *box, const char *msgs[]) : MultiSelectionDialogue(box, msgs)
	{
	}

	void selectCallback(int which)
	{
		Gui::gui->popDialogueBox();

		box->gi->players = which + 1;
		box->updateMessages();

		delete this;
	}
};

class GameInfoMenu : public Menu, public KeyboardListener
{
	friend class GameInfoView;

public:
	GameInfoMenu(Font *font, GameInfoBox *box) : Menu(font)
	{
		this->box = box;
		this->setText(game_info_menu_messages);
	}

	virtual void stringCallback(const char *str)
	{
		switch (this->cur_sel)
		{
		case 2:
			this->box->gi->setName(str);
			break;
		case 3:
			this->box->gi->setAuthor(str);
			break;
		case 4:
			this->box->gi->setCreator(str);
			break;
		case 5:
			this->box->gi->setMusician(str);
			break;
		case 6:
			this->box->gi->setGraphicsArtist(str);
			break;
		case 7:
		{
			unsigned long v;
			char *endp;

			v = strtoul(str, &endp, 0);
			if (str != endp)
			{
				if (v < 1976 || v > 2040)
					Gui::gui->pushDialogueBox(new DialogueBox(game_info_bad_year_dlg));
				else
					this->box->gi->setYear(v);
			}
			else
				Gui::gui->pushDialogueBox(new DialogueBox(game_info_bad_number_dlg));
		} break;
		default:
			panic("Cur sel is %d, not possible!\n", this->cur_sel);
			break;
		}
		this->box->updateMessages();
	}

	virtual void selectCallback(int which)
	{
		switch (which)
		{
		case 0:
		{
			SDL_Surface *tmp = sdl_surface_8bit_copy(Gui::gui->screenshot);

			this->box->gi->setScreenshot(tmp);
		} break;
		case 2:
		case 3:
		case 4:
		case 5:
		case 6:
		case 7:
			VirtualKeyboard::kbd->activate();
			VirtualKeyboard::kbd->registerListener(this);
			break;
		case 8:
			Gui::gui->pushDialogueBox(new GenreDialogue(this->box, genre_dlg));
			break;
		case 9:
			Gui::gui->pushDialogueBox(new PlayersDialogue(this->box, players_dlg));
			break;
		default:
			panic("Impossible menu option\n");
			break;
		}
	}

	virtual void hoverCallback(int which)
	{
	}

	virtual void escapeCallback(int which)
	{
		/* If we haven't' saved a screenshot, save it anyway */
		if (!this->box->gi->screenshot)
		{
			SDL_Surface *p = sdl_surface_8bit_copy(Gui::gui->screenshot);

			this->box->gi->setScreenshot(p);
		}
		Gui::gui->popView();
	}

private:
	GameInfoBox *box;
};


class GameInfoView : public GuiView
{
public:
	GameInfoView() : GuiView()
	{
		this->gameInfo = new GameInfoBox(Gui::gui->default_font);;
		this->menu = new GameInfoMenu(Gui::gui->default_font, this->gameInfo);
	}

	~GameInfoView()
	{
		delete this->gameInfo;
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
		this->gameInfo->setGameInfo(Gui::gui->cur_gameInfo);
	}

	void viewPopCallback()
	{
		Gui::gui->cur_gameInfo = this->gameInfo->gi;
	}

	void draw(SDL_Surface *where)
	{
		 SDL_Rect dst;

		 /* Blit the backgrounds */
		 dst = (SDL_Rect){20,45,300,400};
		 SDL_BlitSurface(Gui::gui->main_menu_bg, NULL, where, &dst);

		 dst = (SDL_Rect){350,13,0,0};
		 SDL_BlitSurface(Gui::gui->disc_info, NULL, where, &dst);

		 this->menu->draw(where, 50, 70, 280, 375);
		 this->gameInfo->draw(where, 390, 55, 242, 447);
	}

protected:
	GameInfoMenu *menu;
	GameInfoBox *gameInfo;
};

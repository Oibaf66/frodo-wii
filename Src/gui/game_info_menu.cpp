#include <Display.h>
#include <C64.h>

#include "gui.hh"
#include "menu.hh"
#include "game_info_box.hh"
#include "virtual_keyboard.hh"

class GameInfoView;

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
			this->box->gi->setScreenshot(TheC64->TheDisplay->SurfaceFromC64Display());
			break;
		case 2:
		case 3:
			VirtualKeyboard::kbd->activate();
			VirtualKeyboard::kbd->registerListener(this);
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
			SDL_Surface *p = TheC64->TheDisplay->SurfaceFromC64Display();

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

	void pushEvent(SDL_Event *ev)
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
		 this->gameInfo->draw(where, 360, 55, 262, 447);
	}

protected:
	GameInfoMenu *menu;
	GameInfoBox *gameInfo;
};

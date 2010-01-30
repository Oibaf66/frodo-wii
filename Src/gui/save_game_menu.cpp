#include <unistd.h> /* unlink */

#include <C64.h>

#include <utils.hh>

#include "menu.hh"
#include "file_browser.hh"
#include "game_info.hh"
#include "game_info_box.hh"

static const char *save_exts[] = {".sav", ".SAV", NULL};

class SaveGameMenu;

class SaveGameView : public GuiView
{
public:
	SaveGameView();

	~SaveGameView();

	void pushEvent(SDL_Event *ev);

	void loadGameInfo(const char *what);

	void setDirectory(const char *path);

	void setLoadSnapshot(bool which);

	void saveSnapshot();

	/* Inherited */
	void runLogic();

	void draw(SDL_Surface *where);

	SaveGameMenu *menu;
	GameInfoBox *gameInfo;
};


class SaveGameMenu : public FileBrowser
{
	friend class SaveGameView;

public:
	SaveGameMenu(Font *font) :
		FileBrowser(save_exts, font)
	{
		this->loadSnapshot = true;
	}

	~SaveGameMenu()
	{
	}

	virtual void selectCallback(int which)
	{
		const char *fileName = this->pp_msgs[this->cur_sel];
		char *new_path;

		/* If we selected a directory, just take the next one */
		if (fileName[0] == '[')
		{
			this->pushDirectory(fileName);
			return;
		}
		new_path = (char *)xmalloc(strlen(this->cur_path_prefix) + 3 + strlen(fileName));

		sprintf(new_path, "%s/%s",
				this->cur_path_prefix, fileName);
		if (this->loadSnapshot)
			TheC64->LoadSnapshot(new_path);
		else
			unlink(new_path);
		free(new_path);
		Gui::gui->popView();
	}

	virtual void hoverCallback(int which)
	{
	}

	virtual void escapeCallback(int which)
	{
		Gui::gui->popView();
	}

	bool loadSnapshot;
};


SaveGameView::SaveGameView() : GuiView()
{
	this->menu = new SaveGameMenu(Gui::gui->default_font);
	this->gameInfo = new GameInfoBox(Gui::gui->default_font);
}

SaveGameView::~SaveGameView()
{
	delete this->menu;
	delete this->gameInfo;
}

void SaveGameView::loadGameInfo(const char *what)
{
	this->gameInfo->loadGameInfo(what);
}

void SaveGameView::setDirectory(const char *path)
{
	this->menu->setDirectory(path);
}

void SaveGameView::setLoadSnapshot(bool what)
{
	this->menu->loadSnapshot = what;
}

void SaveGameView::saveSnapshot()
{
	const char *name = "unknown";
	char buf[255];

	if (strlen(Gui::gui->np->DrivePath[0]) != 0)
		name = Gui::gui->np->DrivePath[0];
	snprintf(buf, sizeof(buf), "%s/%s.sav", Gui::gui->save_game_path, name);

	TheC64->SaveSnapshot(buf);
}

void SaveGameView::runLogic()
{
	this->menu->runLogic();
}

void SaveGameView::pushEvent(SDL_Event *ev)
{
	this->menu->pushEvent(ev);
}

void SaveGameView::draw(SDL_Surface *where)
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

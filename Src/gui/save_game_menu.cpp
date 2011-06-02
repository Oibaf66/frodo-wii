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

	void pushEvent(event_t ev);

	void loadGameInfo(const char *what);

	void setDirectory(const char *path);

	void setLoadSnapshot(bool which);

	void viewPushCallback();

	void saveSnapshot();

	/* Inherited */
	void runLogic();

	void draw(SDL_Surface *where);

	SaveGameMenu *menu;
	GameInfoBox *gameInfo;
};


class SaveGameMenu : public FileBrowser, TimeoutHandler
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
		char *prefs_path;

		/* If we selected a directory, just take the next one */
		if (fileName[0] == '[')
		{
			this->pushDirectory(fileName);
			return;
		}
		new_path = (char *)xmalloc(strlen(this->cur_path_prefix) + 3 + strlen(fileName));
		prefs_path = (char *)xmalloc(strlen(this->cur_path_prefix) + 8 + strlen(fileName));

		sprintf(new_path, "%s/%s", this->cur_path_prefix, fileName);
		sprintf(prefs_path, "%s.prefs", new_path);

		if (this->loadSnapshot)
		{
			//int display_type = Gui::gui->np->DisplayType;

			TheC64->Reset();
			TheC64->LoadSnapshot(new_path);

			this->updateGameInfo(fileName);
			Gui::gui->updateGameInfo(Gui::gui->sgv->gameInfo->gi);
			Gui::gui->np->Load(prefs_path);
			/* Don't change display type */
			//Gui::gui->np->DisplayType = display_type;
		} else
			unlink(new_path);
		free(prefs_path);
		free(new_path);
		Gui::gui->popView();
	}

	virtual void hoverCallback(int which)
	{
		TimerController::controller->arm(this, 350);
	}

	virtual void escapeCallback(int which)
	{
		Gui::gui->popView();
	}

	void updateGameInfo(const char *fileName)
	{
		char *cpy = xstrdup(fileName);
		char *p = strstr(cpy, ".sav");

		if (p)
			*p = '\0';
		Gui::gui->sgv->loadGameInfo(cpy);
		free(cpy);
	}

	virtual void timeoutCallback()
	{
		this->updateGameInfo(this->pp_msgs[this->cur_sel]);
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
	this->gameInfo->loadGameInfo(what, Gui::gui->save_game_path);
}

void SaveGameView::setDirectory(const char *path)
{
	this->menu->setDirectory(path);
}

void SaveGameView::setLoadSnapshot(bool what)
{
	this->menu->loadSnapshot = what;
}

void SaveGameView::viewPushCallback()
{
	this->gameInfo->setGameInfo(NULL);
}

void SaveGameView::saveSnapshot()
{
	const char *name = "unknown";
	const char *out_name;
	char *prefs_name;
	char *save;

	/*
	if (strlen(Gui::gui->np->DrivePath[0]) != 0)
		name = Gui::gui->np->DrivePath[0];
	out_name = strrchr(name, '/');
	if (!out_name)
		out_name = name;
	else
		out_name++;
	*/
	//take the filename from gameinfo instead from drivepath
	out_name = Gui::gui->cur_gameInfo->filename;
	if (!out_name)
		out_name = name;
	
	save = (char*)xmalloc( strlen(Gui::gui->save_game_path) + strlen(out_name) + 6 );
	prefs_name = (char*)xmalloc( strlen(Gui::gui->save_game_path) + strlen(out_name) + 12 );

	sprintf(save, "%s/%s.sav", Gui::gui->save_game_path, out_name);
	sprintf(prefs_name, "%s.prefs", save);

	bool was_paused = TheC64->IsPaused();
	if (!was_paused)
		TheC64->Pause();
	TheC64->SaveSnapshot(save);
	if (!was_paused)
		TheC64->Resume();

	//Take the screenshot from the one saved when the gui was activacted
	Gui::gui->cur_gameInfo->setScreenshot(sdl_surface_8bit_copy(Gui::gui->screenshot));
	Gui::gui->saveGameInfo(Gui::gui->save_game_path, out_name);
	
	ThePrefs = *Gui::gui->np; 
	ThePrefs.Save_game(prefs_name);

	Gui::gui->pushDialogueBox(new DialogueBox(save_state_done));

	free(save);
	free(prefs_name);
}

void SaveGameView::runLogic()
{
	this->menu->runLogic();
}

void SaveGameView::pushEvent(event_t ev)
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
	this->gameInfo->draw(where, 390, 55, 242, 447);
}

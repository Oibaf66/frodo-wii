#include <unistd.h> /* unlink */

#include <C64.h>
#include <utils.hh>

#include "menu.hh"
#include "file_browser.hh"
#include "game_info.hh"
#include "game_info_box.hh"

static const char *game_exts[] = {".d64", ".D64", ".t64", ".T64",
	".prg",".PRG", ".p00", ".P00", NULL};
static const char *prg_exts[] = {".prg",".PRG", ".p00", ".P00", NULL};

class DiscMenu;

class DiscView : public GuiView
{
public:
	DiscView();

	~DiscView();

	void pushEvent(event_t ev);

	void loadGameInfo(const char *what);

	void setDirectory(const char *path);

	void runStartSequence(bool what);

	void viewPushCallback();

	/* Inherited */
	void runLogic();

	void draw(SDL_Surface *where);

	DiscMenu *menu;
	GameInfoBox *gameInfo;
};

class SaveScreenshot : public TimeoutHandler
{
public:
	SaveScreenshot() : TimeoutHandler()
	{
		/* ~45 seconds from now */
		Gui::gui->timerController->arm(this, 45000);
	}

	virtual void timeoutCallback()
	{
		if (!Gui::gui->cur_gameInfo->screenshot)
		{
			Gui::gui->cur_gameInfo->setScreenshot(TheC64->TheDisplay->SurfaceFromC64Display());
			Gui::gui->updateGameInfo(new GameInfo(Gui::gui->cur_gameInfo));
		}

		delete this;
	}
};

class DiscMenu : public FileBrowser, TimeoutHandler
{
	friend class DiscView;

public:
	DiscMenu(Font *font) :
		FileBrowser(game_exts, font), TimeoutHandler()
	{
		this->runStartSequence = false;
	}

	~DiscMenu()
	{
	}

	virtual void selectCallback(int which)
	{
		const char *fileName = this->pp_msgs[this->cur_sel];

		/* If we selected a directory, just take the next one */
		if (fileName[0] == '[')
		{
			this->pushDirectory(fileName);
			return;
		}

                snprintf(Gui::gui->np->DrivePath[0], sizeof(Gui::gui->np->DrivePath[0]),
                		"%s/%s", this->cur_path_prefix, fileName);

                if (ext_matches_list(fileName, prg_exts)) {
                	char *tmp_filename;
                        FILE *src, *dst;

                        tmp_filename = (char *)xmalloc(strlen(Gui::gui->tmp_path) + 4);
                        sprintf(tmp_filename, "%s/a", Gui::gui->tmp_path);

                        /* Clean temp dir first (we only want one file) */
                        unlink(tmp_filename);

                        src = fopen(Gui::gui->np->DrivePath[0], "r");
                        if (src != NULL)
                        {
                                snprintf(Gui::gui->np->DrivePath[0], sizeof(Gui::gui->np->DrivePath[0]),
                                		"%s", Gui::gui->tmp_path);

                                /* Special handling of .prg: Copy to TMP_PATH and
                                 * load that as a dir */
                                dst = fopen(tmp_filename, "w");
                                if (dst)
                                {
                                        Uint8 buf[1024];
                                        size_t v;

                                        do {
                                                v = fread(buf, 1, sizeof(buf), src);
                                                fwrite(buf, 1, v, dst);
                                        } while (v > 0);
                                        fclose(dst);
                                }
                                fclose(src);
                        }
                        free(tmp_filename);
                }

		Gui::gui->timerController->disarm(this);
		Gui::gui->dv->loadGameInfo(fileName);

		if (Gui::gui->dv->gameInfo->gi)
			Gui::gui->updateGameInfo(Gui::gui->dv->gameInfo->gi);
		else
			Gui::gui->updateGameInfo(new GameInfo(fileName));

		Gui::gui->popView();

		if (this->runStartSequence)
		{
			/* Timeout and save the screenshot if there isn't one */
			new SaveScreenshot();

			Gui::gui->exitMenu();
			TheC64->startFakeKeySequence("\nLOAD \"*\",8,1\nRUN\n");
		}
	}

	virtual void hoverCallback(int which)
	{
		Gui::gui->timerController->arm(this, 350);
	}

	virtual void timeoutCallback()
	{
		Gui::gui->dv->loadGameInfo(this->pp_msgs[this->cur_sel]);
	}

	virtual void escapeCallback(int which)
	{
		Gui::gui->timerController->disarm(this);
		Gui::gui->popView();
	}

	bool runStartSequence;
};


DiscView::DiscView() : GuiView()
{
	this->menu = new DiscMenu(Gui::gui->default_font);
	this->gameInfo = new GameInfoBox(Gui::gui->default_font);
}

DiscView::~DiscView()
{
	delete this->menu;
	delete this->gameInfo;
}

void DiscView::viewPushCallback()
{
	this->gameInfo->setGameInfo(NULL);
}

void DiscView::loadGameInfo(const char *what)
{
	this->gameInfo->loadGameInfo(what, Gui::gui->metadata_base_path);
}

void DiscView::setDirectory(const char *path)
{
	this->menu->setDirectory(path);
}

void DiscView::runStartSequence(bool what)
{
	this->menu->runStartSequence = what;
}

void DiscView::runLogic()
{
	this->menu->runLogic();
}

void DiscView::pushEvent(event_t ev)
{
	this->menu->pushEvent(ev);
}

void DiscView::draw(SDL_Surface *where)
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

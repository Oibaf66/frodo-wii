#include "menu.hh"
#include "file_browser.hh"
#include "game_info.hh"

static const char *game_exts[] = {".d64", ".D64", ".t64", ".T64",
	".prg",".PRG", ".p00", ".P00", NULL};

class DiscMenu;
class GameInfoBox;

class DiscView : public GuiView
{
public:
	DiscView();

	~DiscView();

	void pushEvent(SDL_Event *ev);

	void loadGameInfo(const char *what);

	void setDirectory(const char *path);

	/* Inherited */
	void updateTheme();

	void runLogic();

	void draw(SDL_Surface *where);

protected:
	DiscMenu *menu;
	GameInfoBox *gameInfo;
	SDL_Surface *bg;
	SDL_Surface *infobox;
	SDL_Surface *disc_info;
};


class DiscMenu : public FileBrowser, TimeoutHandler
{
	friend class DiscView;

public:
	DiscMenu(Font *font, GuiView *parent) :
		FileBrowser(game_exts, font, parent), TimeoutHandler()
	{
	}

	~DiscMenu()
	{
	}

	virtual void selectCallback(int which)
	{
		printf("entry %d selected: %s\n", which, this->pp_msgs[which]);
		Gui::gui->timerController->disarm(this);
	}

	virtual void hoverCallback(int which)
	{
		Gui::gui->timerController->arm(this, 10);
	}

	virtual void timeoutCallback()
	{
		printf("Hovering timed out over %s\n",
				this->pp_msgs[this->cur_sel]);
		((DiscView*)this->parent)->loadGameInfo(this->pp_msgs[this->cur_sel]);
	}

	virtual void escapeCallback(int which)
	{
		Gui::gui->timerController->disarm(this);
		Gui::gui->exitMenu();
	}
};

class GameInfoBox : public Menu
{
public:
	GameInfoBox(Font *font) : Menu(font)
	{
		this->gi = NULL;
		memset(this->gi_messages, 0, sizeof(this->gi_messages));
	}

	void loadGameInfo(const char *what)
	{
		memset(this->gi_messages, 0, sizeof(this->gi_messages));
		this->setText(this->gi_messages);

		if (this->gi)
		{
			delete this->gi;
			this->gi = NULL;
		}

		/* No need to do this for directories or the special "None" field */
		if (strcmp(what, "None") == 0 ||
				what[0] == '[')
		{
			/* Reset the game info */
			if (this->gi)
				delete this->gi;
			this->gi = NULL;
			return;
		}

		size_t len = strlen(Gui::gui->metadata_base_path) + strlen(what) + 6;
		char *tmp = (char*)xmalloc(len);
		sprintf(tmp, "%s/%s.lra", Gui::gui->metadata_base_path, what);

		/* Might return NULL, but that's OK */
		this->gi = GameInfo::loadFromFile(tmp);
		if (this->gi)
		{
			this->gi_messages[0] = "Game:";
			this->gi_messages[1] = this->gi->name;
			this->gi_messages[2] = "Author:";
			this->gi_messages[3] = this->gi->author;
		}
		this->setText(this->gi_messages);

		free(tmp);
	}

	virtual void selectCallback(int which)
	{
	}
	virtual void hoverCallback(int which)
	{
	}
	virtual void escapeCallback(int which)
	{
	}

	void draw(SDL_Surface *where, int x, int y, int w, int h)
	{
		if (!this->gi)
			return;
		if (!this->gi->screenshot)
			return;

		SDL_Rect dst;

		/* Blit the backgrounds */
		dst = (SDL_Rect){x + w / 2 - this->gi->screenshot->w / 2, y, w, h};
		SDL_BlitSurface(this->gi->screenshot, NULL, where, &dst);

		Menu::draw(where, x, y + this->gi->screenshot->h + 10,
				w, h - this->gi->screenshot->h - 10);
	}

private:
	const char *gi_messages[6];
	GameInfo *gi;
};

DiscView::DiscView() : GuiView()
{
	this->menu = new DiscMenu(NULL, this);
	this->gameInfo = new GameInfoBox(NULL);

	this->bg = NULL;
	this->infobox = NULL;
	this->disc_info = NULL;
}

DiscView::~DiscView()
{
	delete this->menu;
	delete this->gameInfo;
}

void DiscView::loadGameInfo(const char *what)
{
	this->gameInfo->loadGameInfo(what);
}

void DiscView::setDirectory(const char *path)
{
	this->menu->setDirectory(path);
}

void DiscView::updateTheme()
{
	this->bg = Gui::gui->main_menu_bg;
	this->infobox = Gui::gui->infobox;
	this->disc_info = Gui::gui->disc_info;

	this->gameInfo->setFont(Gui::gui->small_font);
	this->menu->setFont(Gui::gui->default_font);
	this->menu->setSelectedBackground(Gui::gui->bg_left, Gui::gui->bg_middle,
			Gui::gui->bg_right, Gui::gui->bg_submenu_left,
			Gui::gui->bg_submenu_middle, Gui::gui->bg_submenu_right);
}

void DiscView::runLogic()
{
	this->menu->runLogic();
}

void DiscView::pushEvent(SDL_Event *ev)
{
	this->menu->pushEvent(ev);
}

void DiscView::draw(SDL_Surface *where)
{
	SDL_Rect dst;

	/* Blit the backgrounds */
	dst = (SDL_Rect){20,45,300,400};
	SDL_BlitSurface(this->bg, NULL, where, &dst);

	dst = (SDL_Rect){350,13,0,0};
	SDL_BlitSurface(this->disc_info, NULL, where, &dst);

	this->menu->draw(where, 50, 70, 280, 375);
	this->gameInfo->draw(where, 360, 55, 262, 447);
}

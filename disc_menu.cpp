#include "menu.hh"
#include "file_browser.hh"

static const char *game_exts[] = {".d64", ".D64", ".t64", ".T64",
	".prg",".PRG", ".p00", ".P00", NULL};

class DiscView;
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
	}

	virtual void escapeCallback(int which)
	{
		Gui::gui->timerController->disarm(this);
		Gui::gui->exitMenu();
	}
};


class DiscView : public GuiView
{
public:
	DiscView() : GuiView()
	{
		this->menu = new DiscMenu(NULL, this);

		this->bg = NULL;
		this->infobox = NULL;
		this->disc_info = NULL;
	}

	~DiscView()
	{
		delete this->menu;
	}

	void setDirectory(const char *path)
	{
		this->menu->setDirectory(path);
	}

	void updateTheme()
	{
		this->bg = Gui::gui->main_menu_bg;
		this->infobox = Gui::gui->infobox;
		this->disc_info = Gui::gui->disc_info;

		this->menu->setFont(Gui::gui->default_font);
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
		 SDL_BlitSurface(this->bg, NULL, where, &dst);

		 dst = (SDL_Rect){350,13,0,0};
		 SDL_BlitSurface(this->disc_info, NULL, where, &dst);

		 this->menu->draw(where, 50, 70, 300, 400);
	}

protected:
	DiscMenu *menu;
	SDL_Surface *bg;
	SDL_Surface *infobox;
	SDL_Surface *disc_info;
};

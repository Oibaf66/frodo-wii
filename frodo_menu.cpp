#include <SDL_image.h>
#include <SDL_ttf.h>

#include "menu.hh"
#include "frodo_menu.hh"
#include "menu_messages.hh"
#include "sdl_ttf_font.hh"
#include "utils.hh"

extern SDL_Surface *screen;

class Gui;

const char *get_theme_path(const char *dir, const char *what)
{
	static char buf[255];

	memset(buf, 0, sizeof(buf));
	snprintf(buf, 254, "%s/%s",
			dir, what);

	return buf;
}

class HelpMenu : public Menu
{
public:
	HelpMenu(Font *font, const char ***all_messages) : Menu(font)
	{
		this->all_messages = all_messages;
	}

	void updateHelpMessage(int which)
	{
		this->setText(this->all_messages[which]);
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

protected:
	const char ***all_messages;
};

class MainMenu : public Menu
{
public:
	MainMenu(Font *font, HelpMenu *help, GuiView *parent) : Menu(font)
	{
		this->parent = parent;
		this->help = help;
	}

	virtual void selectCallback(int which)
	{
		printf("entry %d selected: %s\n", which, this->pp_msgs[which]);
		if (which == 11)
			exit(0);
	}

	virtual void hoverCallback(int which)
	{
		printf("entry %d hover over: %s\n", which, this->pp_msgs[which]);
		this->help->updateHelpMessage(which);
	}

	virtual void escapeCallback(int which)
	{
		printf("entry %d escaped: %s\n", which, this->pp_msgs[which]);
		this->parent->parent->exitMenu();
	}

private:
	GuiView *parent;
	HelpMenu *help;
};


class MainView : public GuiView
{
public:
	MainView(Gui *parent) : GuiView(parent)
	{
		this->help = new HelpMenu(NULL, main_menu_help);
		this->menu = new MainMenu(NULL, this->help, this);
		this->menu->setText(main_menu_messages);
		this->bg = NULL;
		this->infobox = NULL;
		this->textbox = NULL;
	}

	void updateTheme()
	{
		this->bg = parent->main_menu_bg;
		this->infobox = parent->infobox;
		this->textbox = parent->textbox;

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
	}

protected:
	MainMenu *menu;
	HelpMenu *help;
	SDL_Surface *bg;
	SDL_Surface *infobox;
	SDL_Surface *textbox;
};

Gui::Gui()
{
	this->focus = NULL;

	this->bg_left = NULL;
	this->bg_middle = NULL;
	this->bg_right = NULL;
	this->bg_submenu_left = NULL;
	this->bg_submenu_middle = NULL;
	this->bg_submenu_right = NULL;
	this->background = NULL;
	this->main_menu_bg = NULL;
	this->infobox = NULL;
	this->textbox = NULL;
	this->default_font = NULL;
	this->small_font = NULL;

	this->n_views = 0;
	this->views = NULL;

	/* Create the views */
	MainView *mv = new MainView(this);
	this->pushView(mv);
}


bool Gui::setTheme(const char *path)
{
	this->bg_left = this->loadThemeImage(path, "bg_left.png");
	this->bg_middle = this->loadThemeImage(path, "bg_middle.png");
	this->bg_right = this->loadThemeImage(path, "bg_right.png");
	this->bg_submenu_left = this->loadThemeImage(path, "bg_submenu_left.png");
	this->bg_submenu_middle = this->loadThemeImage(path, "bg_submenu_middle.png");
	this->bg_submenu_right = this->loadThemeImage(path, "bg_submenu_right.png");

	this->background = this->loadThemeImage(path, "background.png");
	this->main_menu_bg = this->loadThemeImage(path, "main_menu_bg.png");
	this->infobox = this->loadThemeImage(path, "infobox.png");
	this->textbox = this->loadThemeImage(path, "textbox.png");

	this->default_font = this->loadThemeFont(path, "font.ttf", 18);
	this->small_font = this->loadThemeFont(path, "font.ttf", 16);

	if (!this->bg_left || !this->bg_right || !this->bg_middle ||
			!this->bg_submenu_left || !this->bg_submenu_right ||
			!this->bg_submenu_middle ||
			!this->default_font ||
			!this->small_font)
	{
		SDL_FreeSurface(this->bg_left);
		SDL_FreeSurface(this->bg_middle);
		SDL_FreeSurface(this->bg_right);
		SDL_FreeSurface(this->bg_submenu_left);
		SDL_FreeSurface(this->bg_submenu_middle);
		SDL_FreeSurface(this->bg_submenu_right);
		SDL_FreeSurface(this->background);
		SDL_FreeSurface(this->main_menu_bg);
		SDL_FreeSurface(this->infobox);
		SDL_FreeSurface(this->textbox);

		if (this->default_font)
			delete this->default_font;
		if (this->small_font)
			delete this->small_font;

		return false;
	}

	for (int i = 0; i < this->n_views; i++)
		this->views[i]->updateTheme();

	return true;
}

void Gui::runLogic(void)
{
	GuiView *cur_view = this->peekView();

	if (!this->is_active || !cur_view)
		return;
	cur_view->runLogic();
}


void Gui::pushView(GuiView *view)
{
	int cur = this->n_views;

	this->n_views++;
	this->views = (GuiView**)xrealloc(this->views,
			sizeof(GuiView*) * this->n_views);
	this->views[cur] = view;
}

GuiView *Gui::popView()
{
	this->n_views--;
	if (this->n_views <= 0)
	{
		this->n_views = 0;
		free(this->views);
		return NULL;
	}

	this->views = (GuiView**)xrealloc(this->views,
			sizeof(GuiView*) * this->n_views);
	return this->views[this->n_views - 1];
}

void Gui::exitMenu()
{
	printf("Exiting the menu system\n");
	free(this->views);
	this->views = NULL;
}

void Gui::pushEvent(SDL_Event *ev)
{
	GuiView *cur_view = this->peekView();

	if (!this->is_active || !cur_view)
		return;
	cur_view->pushEvent(ev);
}

void Gui::draw(SDL_Surface *where)
{
	GuiView *cur_view = this->peekView();

	if (!this->is_active || !cur_view)
		return;

	 SDL_BlitSurface(this->background, NULL, screen, NULL);
	 cur_view->draw(where);
}

void Gui::activate()
{
	this->is_active = true;
}

void Gui::deActivate()
{
	this->is_active = false;
}

SDL_Surface *Gui::loadThemeImage(const char *dir, const char *what)
{
	return IMG_Load(get_theme_path(dir, what));
}

Font *Gui::loadThemeFont(const char *dir, const char *what, int size)
{
	TTF_Font *fnt;

	fnt = read_and_alloc_font(get_theme_path(dir, what), size);
	if (!fnt)
		return NULL;

	return new Font_TTF(fnt, 255,255,255);
}

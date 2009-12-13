#include <SDL_image.h>
#include <SDL_ttf.h>

#include "menu.hh"
#include "gui.hh"
#include "menu_messages.hh"
#include "help_box.hh"
#include "dialogue_box.hh"
#include "sdl_ttf_font.hh"
#include "utils.hh"

extern SDL_Surface *screen;

#define THEME_ROOT_PATH "themes"

static const char *get_theme_path(const char *dir, const char *what)
{
	static char buf[255];

	memset(buf, 0, sizeof(buf));
	snprintf(buf, 254, "%s/%s/%s",
			THEME_ROOT_PATH, dir, what);

	return buf;
}

/* These are a bit of special cases... */
#include "disc_menu.cpp"
#include "main_menu.cpp"

GuiView::GuiView()
{
}

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
	this->dialogue_bg = NULL;
	this->small_font = NULL;

	this->n_views = 0;
	this->views = NULL;

	/* Create the views */
	this->mv = new MainView();
	this->dv = new DiscView();
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
	this->dialogue_bg = this->loadThemeImage(path, "dialogue_box.png");

	this->default_font = this->loadThemeFont(path, "font.ttf", 18);
	this->small_font = this->loadThemeFont(path, "font.ttf", 16);

	if (!this->bg_left || !this->bg_right || !this->bg_middle ||
			!this->bg_submenu_left || !this->bg_submenu_right ||
			!this->bg_submenu_middle ||
			!this->dialogue_bg ||
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
		SDL_FreeSurface(this->dialogue_bg);
		SDL_FreeSurface(this->textbox);

		if (this->default_font)
			delete this->default_font;
		if (this->small_font)
			delete this->small_font;

		return false;
	}

	this->mv->updateTheme();
	this->dv->updateTheme();

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
	GuiView *cur = this->peekView();

	if (cur)
		delete cur;

	this->n_views--;
	if (this->n_views <= 0)
	{
		free(this->views);
		this->views = NULL;
		this->n_views = 0;
		/* Deactivate when no views are left */
		this->is_active = false;

		return NULL;
	}

	this->views = (GuiView**)xrealloc(this->views,
			sizeof(GuiView*) * this->n_views);
	return this->views[this->n_views - 1];
}

void Gui::exitMenu()
{
	printf("Exiting the menu system\n");

	/* Pop all views */
	while (this->popView())
		;
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


/* The singleton/factory stuff */
Gui *Gui::gui;
void Gui::init()
{
	Gui::gui = new Gui();

	/* Set the default theme */
	panic_if(!Gui::gui->setTheme("default"),
			"Setting default theme failed\n");
}

#ifndef __GUI_HH__
#define __GUI_HH__

#include <SDL.h>

#include "menu.hh"
#include "font.hh"

class Gui;

class GuiView
{
public:
	GuiView();

	virtual void pushEvent(SDL_Event *ev) = 0;

	virtual void runLogic() = 0;

	virtual void updateTheme() = 0;

	virtual void draw(SDL_Surface *where) = 0;
};

class Gui
{
public:
	Gui();

	~Gui();

	bool setTheme(const char *path);

	void activate();

	void deActivate();

	void runLogic(void);

	void pushEvent(SDL_Event *ev);

	void draw(SDL_Surface *where);

	void pushView(GuiView *view);

	GuiView *popView();

	GuiView *peekView()
	{
		if (!this->views)
			return NULL;
		return this->views[this->n_views-1];
	}

	void exitMenu();

	/* These are private, keep off! */
	const char *getThemePath(const char *dir, const char *what);

	SDL_Surface *loadThemeImage(const char *dir, const char *what);

	Font *loadThemeFont(const char *dir, const char *what, int size);

	bool is_active;
	Menu *focus; /* Where the focus goes */
	Menu *main_menu;

	SDL_Surface *background;
	SDL_Surface *main_menu_bg;
	SDL_Surface *infobox;
	SDL_Surface *textbox;
	SDL_Surface *dialogue_bg;
	SDL_Surface *bg_left, *bg_right, *bg_middle,
		*bg_submenu_left, *bg_submenu_right, *bg_submenu_middle;

	Font *default_font;
	Font *small_font;

	GuiView **views;
	int n_views;


	/* Singleton */
	static void init();
	static Gui *gui;
};

#endif /* GUI_HH */

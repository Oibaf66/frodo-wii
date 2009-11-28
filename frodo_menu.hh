#ifndef __GUI_HH__
#define __GUI_HH__

#include <SDL.h>

#include "menu.hh"
#include "font.hh"

enum GuiView
{
	main_menu,
	insert_disc,
	select_state,
	virtual_keyboard,
	networking,
	options,
	help,
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

	void setView(GuiView view);

	void pushEvent(SDL_Event *ev);

	void draw(SDL_Surface *where);

private:
	const char *getThemePath(const char *dir, const char *what);

	SDL_Surface *loadThemeImage(const char *dir, const char *what);

	Font *loadThemeFont(const char *dir, const char *what);

	bool is_active;
	Menu *focus; /* Where the focus goes */

	SDL_Surface *background;
	Menu *main_menu;

	SDL_Surface *bg_left, *bg_right, *bg_middle,
		*bg_submenu_left, *bg_submenu_right, *bg_submenu_middle;

	Font *main_font;
};

#endif /* GUI_HH */

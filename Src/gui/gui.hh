#ifndef __GUI_HH__
#define __GUI_HH__

#include <SDL.h>
#include <timer.hh>

#include "menu.hh"
#include "font.hh"
#include "gui_view.hh"

/* Frodo stuff */
#include <sysdeps.h>
#include <main.h>
#include <Prefs.h>

class DialogueBox;
class StatusBar;
class NetworkServerMessages;
class GameInfo;

class MainView;
class BindKeysView;
class DiscView;
class SaveGameView;
class OptionsView;
class NetworkView;
class ThemeView;
class GameInfoView;
class NetworkUserView;
class NetworkRegionView;

class VirtualKeyboard;

class Gui
{
public:
	Gui();

	~Gui();

	bool setTheme(const char *path);

	void activate();

	void runLogic(void);

	void pushJoystickEvent(event_t ev);

	void pushEvent(event_t ev);

	void pushEvent(SDL_Event *ev);

	void draw(SDL_Surface *where);

	void pushView(GuiView *view);

	void pushVirtualKeyboard(VirtualKeyboard *kbd);

	void pushDialogueBox(DialogueBox *dlg);

	DialogueBox *popDialogueBox();

	GuiView *popView();

	GuiView *peekView()
	{
		if (!this->views)
			return NULL;
		return this->views[this->n_views-1];
	}

	void updateGameInfo(GameInfo *gi);

	void saveGameInfo(const char *base_path, const char *name);

	void exitMenu();

	/* These are private, keep off! */
	const char *getThemePath(const char *dir, const char *what);

	SDL_Surface *loadThemeImage(const char *dir, const char *what);

	Font *loadThemeFont(const char *dir, const char *what, int size);

	bool is_active;
	Menu *focus; /* Where the focus goes */
	Menu *main_menu;
	SDL_Surface *screenshot;

	SDL_Surface *background;
	SDL_Surface *main_menu_bg;
	SDL_Surface *status_bar_bg;
	SDL_Surface *infobox;
	SDL_Surface *bind_key_box;
	SDL_Surface *network_message_box;
	SDL_Surface *keyboard;
	SDL_Surface *dialogue_bg;
	SDL_Surface *disc_info;
	SDL_Surface *network_info;
	SDL_Surface *bg_left, *bg_right, *bg_middle,
		*bg_submenu_left, *bg_submenu_right, *bg_submenu_middle;
	SDL_Surface *highlighted_key;
	SDL_Surface *selected_key;

	Font *default_font;
	Font *small_font;
	TimerController *controller;

	/* Handled specially */
	VirtualKeyboard *kbd;
	DialogueBox *dlg;
	StatusBar *status_bar;
	NetworkServerMessages *server_msgs;

	MainView *mv;
	DiscView *dv;
	SaveGameView *sgv;
	OptionsView *ov;
	NetworkView *nv;
	GameInfoView *giv;
	ThemeView *tv;
	BindKeysView *bkv;
	NetworkUserView *nuv;
	NetworkRegionView *nrv;

	GuiView **views;
	int n_views;

	const char *metadata_base_path;
	const char *theme_base_path;
	const char *game_base_path;
	const char *tmp_path;
	const char *save_game_path;

	GameInfo *cur_gameInfo;
	bool gameInfoChanged;

	/* New preferences */
	Prefs cur_prefs;
	Prefs *np;

	/* Singleton */
	static void init();
	static Gui *gui;
};

#endif /* GUI_HH */

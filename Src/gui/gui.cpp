#include <SDL_image.h>
#include <SDL_ttf.h>
#include <arpa/inet.h>

#include "menu.hh"
#include "gui.hh"
#include "menu_messages.hh"
#include "help_box.hh"
#include "dialogue_box.hh"
#include "sdl_ttf_font.hh"
#include "utils.hh"
#include "virtual_keyboard.hh"

extern SDL_Surface *screen;

#define THEME_ROOT_PATH "themes"
#define METADATA_ROOT_PATH "metadata"
#define GAME_ROOT_PATH "discs"
#define TMP_ROOT_PATH "tmp"
#define SAVE_GAME_ROOT_PATH "saves"

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
#include "save_game_menu.cpp"
#include "bind_keys_menu.cpp"
#include "theme_menu.cpp"
#include "options_menu.cpp"
#include "network_menu.cpp"
#include "game_info_menu.cpp"
#include "main_menu.cpp"

GuiView::GuiView()
{
}

void GuiView::updateTheme()
{
}

void GuiView::viewPushCallback()
{
}

void GuiView::viewPopCallback()
{
}

Gui::Gui()
{
	this->np = NULL;

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
	this->status_bar_bg = NULL;
	this->default_font = NULL;
	this->dialogue_bg = NULL;
	this->small_font = NULL;

	this->n_views = 0;
	this->views = NULL;
	this->timerController = new TimerController();

	VirtualKeyboard::kbd = new VirtualKeyboard(NULL);

	this->theme_base_path = THEME_ROOT_PATH;
	this->metadata_base_path = METADATA_ROOT_PATH;
	this->game_base_path = GAME_ROOT_PATH;
	this->tmp_path = TMP_ROOT_PATH;
	this->save_game_path = SAVE_GAME_ROOT_PATH;

	this->cur_gameInfo = new GameInfo();
	this->gameInfoChanged = false;

	this->dlg = NULL;
	this->kbd = NULL;

	this->mv = NULL;
	this->dv = NULL;
	this->sgv = NULL;
	this->ov = NULL;
	this->nv = NULL;
	this->tv = NULL;
	this->giv = NULL;
	this->bkv = NULL;
}

Gui::~Gui()
{
	delete this->mv;
	delete this->dv;
	delete this->sgv;
	delete this->ov;
	delete this->nv;
	delete this->tv;
	delete this->giv;
	delete this->bkv;

	delete this->cur_gameInfo;
	delete this->timerController;

	if (this->status_bar)
		delete this->status_bar;

	if (this->dlg)
		delete this->dlg;

	if (VirtualKeyboard::kbd)
		delete VirtualKeyboard::kbd;

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
	SDL_FreeSurface(this->disc_info);
	SDL_FreeSurface(this->textbox);
	SDL_FreeSurface(this->selected_key);
	SDL_FreeSurface(this->highlighted_key);
	SDL_FreeSurface(this->status_bar_bg);

	if (this->default_font)
		delete this->default_font;
	if (this->small_font)
		delete this->small_font;
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
	this->status_bar_bg = this->loadThemeImage(path, "status_bar.png");
	this->infobox = this->loadThemeImage(path, "infobox.png");
	this->textbox = this->loadThemeImage(path, "textbox.png");
	this->dialogue_bg = this->loadThemeImage(path, "dialogue_box.png");
	this->disc_info = this->loadThemeImage(path, "disc_info.png");

	this->highlighted_key = this->loadThemeImage(path, "highlighted_key.png");
	this->selected_key = this->loadThemeImage(path, "selected_key.png");

	this->default_font = this->loadThemeFont(path, "font.ttf", 18);
	this->small_font = this->loadThemeFont(path, "font.ttf", 16);

	if (!this->bg_left || !this->bg_right || !this->bg_middle ||
			!this->bg_submenu_left || !this->bg_submenu_right ||
			!this->bg_submenu_middle ||
			!this->dialogue_bg ||
			!this->disc_info ||
			!this->selected_key ||
			!this->highlighted_key ||
			!this->status_bar_bg ||
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
		SDL_FreeSurface(this->disc_info);
		SDL_FreeSurface(this->textbox);
		SDL_FreeSurface(this->selected_key);
		SDL_FreeSurface(this->highlighted_key);
		SDL_FreeSurface(this->status_bar_bg);

		if (this->default_font)
			delete this->default_font;
		if (this->small_font)
			delete this->small_font;

		return false;
	}

	/* Create the views */
	if (!this->mv)
	{
		this->status_bar = new StatusBar();

		this->mv = new MainView();
		this->dv = new DiscView();
		this->sgv = new SaveGameView();
		this->ov = new OptionsView();
		this->nv = new NetworkView();
		this->tv = new ThemeView();
		this->bkv = new BindKeysView();
		this->giv = new GameInfoView();
	}

	VirtualKeyboard::kbd->updateTheme();

	return true;
}

void Gui::runLogic(void)
{
	GuiView *cur_view = this->peekView();

	this->status_bar->runLogic();
	this->timerController->tick();

	if (!this->is_active || !cur_view)
		return;
	if (this->dlg)
		this->dlg->runLogic();
	else if (this->kbd)
		this->kbd->runLogic();
	else
		cur_view->runLogic();
}

void Gui::pushView(GuiView *view)
{
	int cur = this->n_views;

	this->n_views++;
	this->views = (GuiView**)xrealloc(this->views,
			sizeof(GuiView*) * this->n_views);
	this->views[cur] = view;
	view->viewPushCallback();
}

void Gui::pushDialogueBox(DialogueBox *dlg)
{
	this->dlg = dlg;
}

DialogueBox *Gui::popDialogueBox()
{
	DialogueBox *out = this->dlg;
	this->dlg = NULL;

	return out;
}

GuiView *Gui::popView()
{
	GuiView *cur = this->peekView();

	if (!cur)
		return NULL;
	cur->viewPopCallback();

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
	return cur;
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
	if (this->dlg)
		this->dlg->pushEvent(ev);
	else if (this->kbd)
		this->kbd->pushEvent(ev);
	else
		cur_view->pushEvent(ev);
}

void Gui::draw(SDL_Surface *where)
{
	GuiView *cur_view = this->peekView();

	if (!this->is_active || !cur_view)
	{
		this->status_bar->draw(where);
		return;
	}

	 SDL_BlitSurface(this->background, NULL, where, NULL);
	 cur_view->draw(where);
	 if (this->kbd)
		 this->kbd->draw(where);
	 if (this->dlg)
		 this->dlg->draw(where);
	 this->status_bar->draw(where);
}

void Gui::activate()
{
	this->is_active = true;
	/* FIXME! TMP! TMP! */
	this->np = new Prefs();
	this->pushView(this->mv);
}

SDL_Surface *Gui::loadThemeImage(const char *dir, const char *what)
{
	SDL_Surface *img = IMG_Load(get_theme_path(dir, what));
	SDL_Surface *out;

	if (!img)
		return NULL;
	out = SDL_DisplayFormatAlpha(img);
	SDL_FreeSurface(img);

	return out;
}

Font *Gui::loadThemeFont(const char *dir, const char *what, int size)
{
	TTF_Font *fnt;

	fnt = read_and_alloc_font(get_theme_path(dir, what), size);
	if (!fnt)
		return NULL;

	return new Font_TTF(fnt, 255,255,255);
}

void Gui::updateGameInfo(GameInfo *gi)
{
	panic_if(!gi, "gi must be set\n");
	delete this->cur_gameInfo;
	this->cur_gameInfo = gi;
	this->gameInfoChanged = true;
}

void Gui::saveGameInfo()
{
	struct game_info *p = this->cur_gameInfo->dump();

	if (p)
	{
		char buf[255];
		FILE *fp;

		snprintf(buf, sizeof(buf), "%s/%s",
				METADATA_ROOT_PATH, this->cur_gameInfo->filename);
		fp = fopen(buf, "w");
		if (!fp)
		{
			warning("Could not open %s for writing\n", buf);
			return;
		}
		int n = fwrite((const void*)p, 1, p->sz, fp);
		if (n != (int)p->sz)
			warning("Could only write %d bytes of %s\n", n, buf);
		fclose(fp);
	}

	this->gameInfoChanged = false;
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

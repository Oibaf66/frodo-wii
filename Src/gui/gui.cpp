#include <SDL_image.h>
#include <SDL_ttf.h>
#if defined(GEKKO)
# include <network.h>
#else
# include <arpa/inet.h>
#endif

#include <utils.hh>

#include "menu.hh"
#include "gui.hh"
#include "menu_messages.hh"
#include "help_box.hh"
#include "dialogue_box.hh"
#include "sdl_ttf_font.hh"
#include "virtual_keyboard.hh"

#include "network_server_messages.hh"

extern SDL_Surface *screen;

#if defined(GEKKO)
#define THEME_ROOT_PATH "/frodo/themes"
#define METADATA_ROOT_PATH "/frodo/metadata"
#define GAME_ROOT_PATH "/frodo/images"
#define TMP_ROOT_PATH "/frodo/tmp"
#define SAVE_GAME_ROOT_PATH "/frodo/saves"
#else
#define THEME_ROOT_PATH "themes"
#define METADATA_ROOT_PATH "metadata"
#define GAME_ROOT_PATH "images"
#define TMP_ROOT_PATH "tmp"
#define SAVE_GAME_ROOT_PATH "saves"
#endif

static const char *get_theme_path(const char *dir, const char *what)
{
	static char buf[255];

	memset(buf, 0, sizeof(buf));
	snprintf(buf, 254, "%s/%s/%s",
			THEME_ROOT_PATH, dir, what);

	return buf;
}

/* These are a bit of special cases... */
#include "network_user_menu.cpp"
#include "disc_menu.cpp"
#include "save_game_menu.cpp"
#include "bind_keys_menu.cpp"
#include "theme_menu.cpp"
#include "options_menu.cpp"
#include "network_region_menu.cpp"
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
	this->is_active = false;
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
	this->network_message_box = NULL;
	this->status_bar_bg = NULL;
	this->keyboard = NULL;
	this->default_font = NULL;
	this->dialogue_bg = NULL;
	this->network_info = NULL;
	this->bind_key_box = NULL;
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
	this->nuv = NULL;
	this->nrv = NULL;
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
	delete this->nuv;
	delete this->nrv;

	delete this->cur_gameInfo;
	delete this->timerController;

	if (this->status_bar)
		delete this->status_bar;
	if (this->server_msgs)
		delete this->server_msgs;

	if (this->dlg)
		delete this->dlg;

	if (VirtualKeyboard::kbd)
		delete VirtualKeyboard::kbd;

	SDL_FreeSurface(this->bg_left);
	SDL_FreeSurface(this->keyboard);
	SDL_FreeSurface(this->bg_middle);
	SDL_FreeSurface(this->bg_right);
	SDL_FreeSurface(this->bg_submenu_left);
	SDL_FreeSurface(this->bg_submenu_middle);
	SDL_FreeSurface(this->bg_submenu_right);
	SDL_FreeSurface(this->background);
	SDL_FreeSurface(this->main_menu_bg);
	SDL_FreeSurface(this->bind_key_box);
	SDL_FreeSurface(this->infobox);
	SDL_FreeSurface(this->dialogue_bg);
	SDL_FreeSurface(this->disc_info);
	SDL_FreeSurface(this->network_info);
	SDL_FreeSurface(this->network_message_box);
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
	this->keyboard = this->loadThemeImage(path, "keyboard.png");
	this->network_message_box = this->loadThemeImage(path, "network_message_box.png");
	this->dialogue_bg = this->loadThemeImage(path, "dialogue_box.png");
	this->disc_info = this->loadThemeImage(path, "disc_info.png");
	this->network_info = this->loadThemeImage(path, "network_info.png");
	this->bind_key_box = this->loadThemeImage(path, "bind_key_box.png");

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
			!this->bind_key_box ||
			!this->network_info ||
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
		SDL_FreeSurface(this->network_info);
		SDL_FreeSurface(this->infobox);
		SDL_FreeSurface(this->dialogue_bg);
		SDL_FreeSurface(this->disc_info);
		SDL_FreeSurface(this->network_message_box);
		SDL_FreeSurface(this->selected_key);
		SDL_FreeSurface(this->keyboard);
		SDL_FreeSurface(this->bind_key_box);
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
		this->server_msgs = new NetworkServerMessages();

		this->mv = new MainView();
		this->dv = new DiscView();
		this->sgv = new SaveGameView();
		this->ov = new OptionsView();
		this->nv = new NetworkView();
		this->tv = new ThemeView();
		this->bkv = new BindKeysView();
		this->giv = new GameInfoView();
		this->nuv = new NetworkUserView();
		this->nrv = new NetworkRegionView();
	}

	VirtualKeyboard::kbd->updateTheme();

	/* FIXME! Remove! */
	{
		static bool viewed = false;

		if (!viewed)
		{
			this->activate();
			this->pushDialogueBox(new DialogueBox(needs_help));
			viewed = true;
		}
	}

	return true;
}

void Gui::runLogic(void)
{
	GuiView *cur_view = this->peekView();

	this->status_bar->runLogic();
	this->timerController->tick();
	if (this->kbd)
		this->kbd->runLogic();

	if (!this->is_active || !cur_view)
		return;
	Gui::gui->server_msgs->runLogic();
	if (this->dlg)
		this->dlg->runLogic();
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
	if (this->peekView())
		this->peekView()->viewPushCallback();

	return cur;
}

void Gui::exitMenu()
{
	/* Pop all views */
	while (this->popView())
		;
	TheC64->NewPrefs(this->np);
	ThePrefs = *this->np;

	this->saveGameInfo(this->metadata_base_path, this->cur_gameInfo->filename);
}


void Gui::pushJoystickEvent(event_t ev)
{
	static event_t last = EVENT_NONE;
	static Uint32 last_ticks;
	Uint32 cur_ticks;

	if (last == ev)
		return;

	cur_ticks = SDL_GetTicks();
	if (cur_ticks - last_ticks < 150)
		return;
	last_ticks = cur_ticks;

	this->pushEvent(ev);
	last = ev;
}

void Gui::pushEvent(event_t ev)
{
	GuiView *cur_view = this->peekView();

	if (ev == KEY_ENTER_MENU)
	{
		this->activate();
		return;
	}

	if (!this->is_active || !cur_view)
	{
		if (this->kbd)
			this->kbd->pushEvent(ev);
		return;
	}

	if (this->dlg)
		this->dlg->pushEvent(ev);
	else if (this->kbd)
		this->kbd->pushEvent(ev);
	else
		cur_view->pushEvent(ev);
}

void Gui::pushEvent(SDL_Event *ev)
{
	if (this->kbd)
	{
		this->kbd->pushEvent(ev);
		return;
	}

	switch(ev->type)
	{
	case SDL_KEYDOWN:
		switch (ev->key.keysym.sym)
		{
		case SDLK_UP:
			this->pushEvent(KEY_UP);
			break;
		case SDLK_DOWN:
			this->pushEvent(KEY_DOWN);
			break;
		case SDLK_LEFT:
			this->pushEvent(KEY_LEFT);
			break;
		case SDLK_RIGHT:
			this->pushEvent(KEY_RIGHT);
			break;
		case SDLK_PAGEDOWN:
			this->pushEvent(KEY_PAGEDOWN);
			break;
		case SDLK_PAGEUP:
			this->pushEvent(KEY_PAGEUP);
			break;
		case SDLK_RETURN:
		case SDLK_SPACE:
			this->pushEvent(KEY_SELECT);
			break;
		case SDLK_HOME:
		case SDLK_ESCAPE:
			this->pushEvent(KEY_ESCAPE);
			break;
		default:
			break;
		}
		default:
			break;
	case SDL_QUIT:
		TheC64->TheDisplay->quit_requested = true;
		break;
	}
}

void Gui::draw(SDL_Surface *where)
{
	GuiView *cur_view = this->peekView();

	if (!this->is_active || !cur_view)
	{
		this->status_bar->draw(where);
		if (this->kbd)
			this->kbd->draw(where);
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

	this->cur_prefs = ThePrefs;
	this->np = &cur_prefs;

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
	/* Ignore if empty */
	if(!gi)
		return;

	/* Store the current game info */
	this->saveGameInfo(this->metadata_base_path, this->cur_gameInfo->filename);
	delete this->cur_gameInfo;

	this->cur_gameInfo = gi;
	this->gameInfoChanged = true;
}

void Gui::saveGameInfo(const char *base_path, const char *name)
{
	/* Don't save unset games */
	if (strcmp(this->cur_gameInfo->filename, "unknown") == 0)
		return;

	struct game_info *p = this->cur_gameInfo->dump();

	if (p)
	{
		size_t sz = ntohl(p->sz);

		char *new_name = (char *)xmalloc(strlen(base_path) +
				8 + strlen(name));
		FILE *fp;

		sprintf(new_name, "%s/%s.lra", base_path,
				name);
		fp = fopen(new_name, "w");
		if (fp)
		{
			int n = fwrite((const void*)p, 1, sz, fp);

			if (ferror(fp))
				warning("Write error on %s\n", new_name);
			else if ((size_t)n != sz)
				warning("Could only write %d bytes of %d for %s\n",
						n, sz, new_name);
			fclose(fp);
		}
		else
			warning("Could not open %s for writing\n", new_name);

		free(new_name);
	}
	free(p);

	this->gameInfoChanged = false;
}


/* The singleton/factory stuff */
Gui *Gui::gui;
void Gui::init()
{
	Gui::gui = new Gui();

	if (!Gui::gui->setTheme(ThePrefs.Theme))
	{
		/* Set the default theme */
		panic_if (!Gui::gui->setTheme("default"),
				"Setting default theme failed\n");
	}
}

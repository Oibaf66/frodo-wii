#include <SDL_image.h>
#include <SDL_ttf.h>

#include "menu.hh"
#include "frodo_menu.hh"
#include "sdl_ttf_font.hh"
#include "utils.hh"

extern SDL_Surface *screen;

class MainMenu : public Menu
{
public:
	MainMenu(Font *font) : Menu(font)
	{
		static const char *messages[] = {
				/*00*/          "File",
		                /*01*/          "^|Insert|Start",
		                /*02*/          "States",
		                /*03*/          "^|Load|Save|Delete",
		                /*04*/          "Keyboard",
		                /*05*/          "^|Type|Macro|Bind",
		                /*06*/          " ",
		                /*07*/          "Reset the C=64",
		                /*08*/          "Networking",
		                /*09*/          "Options",
		                /*10*/          "Advanced Options",
		                /*11*/          "Help",
		                /*12*/          "Quit",
		                NULL
		};

		this->setText(messages);
	}

	virtual void selectCallback(int which)
	{
		printf("entry %d selected: %s\n", which, this->pp_msgs[which]);
		if (which == 12)
			exit(0);
	}

	virtual void escapeCallback(int which)
	{
		printf("entry %d escaped: %s\n", which, this->pp_msgs[which]);
	}
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

	this->main_font = NULL;
	this->main_menu = new MainMenu(NULL);
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

	this->main_font = this->loadThemeFont(path, "font.ttf");

	if (!this->bg_left || !this->bg_right || !this->bg_middle ||
			!this->bg_submenu_left || !this->bg_submenu_right ||
			!this->bg_submenu_middle ||
			!this->main_font)
	{
		SDL_FreeSurface(this->bg_left);
		SDL_FreeSurface(this->bg_middle);
		SDL_FreeSurface(this->bg_right);
		SDL_FreeSurface(this->bg_submenu_left);
		SDL_FreeSurface(this->bg_submenu_middle);
		SDL_FreeSurface(this->bg_submenu_right);
		SDL_FreeSurface(this->background);
		SDL_FreeSurface(this->main_menu_bg);

		if (this->main_font)
			delete this->main_font;

		return false;
	}
	this->main_menu->setSelectedBackground(bg_left, bg_middle, bg_right,
			bg_submenu_left, bg_submenu_middle, bg_submenu_right);
	this->main_menu->setFont(this->main_font);
	this->focus = this->main_menu;

	return true;
}

void Gui::runLogic(void)
{
	if (!this->is_active)
		return;
	this->main_menu->runLogic();
}

void Gui::setView(GuiView view)
{

}

void Gui::pushEvent(SDL_Event *ev)
{
	if (this->is_active && this->focus)
		this->focus->pushEvent(ev);
}

void Gui::draw(SDL_Surface *where)
{
	SDL_Rect dst;

	if (!this->is_active)
		return;

	 SDL_BlitSurface(this->background, NULL, screen, NULL);
	 dst = (SDL_Rect){20,45,300,400};
	 SDL_BlitSurface(this->main_menu_bg, NULL, screen, &dst);
	 this->main_menu->draw(where, 50, 70, 300, 400);
}

void Gui::activate()
{
	this->is_active = true;
}

void Gui::deActivate()
{
	this->is_active = false;
}

const char *Gui::getThemePath(const char *dir, const char *what)
{
	static char buf[255];

	memset(buf, 0, sizeof(buf));
	snprintf(buf, 254, "%s/%s",
			dir, what);

	return buf;
}

SDL_Surface *Gui::loadThemeImage(const char *dir, const char *what)
{
	return IMG_Load(this->getThemePath(dir, what));
}

Font *Gui::loadThemeFont(const char *dir, const char *what)
{
	TTF_Font *fnt;

	fnt = read_and_alloc_font(this->getThemePath(dir, what), 18);
	if (!fnt)
		return NULL;

	return new Font_TTF(fnt, 255,255,255);
}

#include "menu.hh"
#include "file_browser.hh"

class ThemeMenu;

class ThemeView : public GuiView
{
public:
	ThemeView();

	~ThemeView();

	void pushEvent(event_t ev);

	void setDirectory(const char *path);

	/* Inherited */
	void runLogic();

	void draw(SDL_Surface *where);

protected:
	ThemeMenu *menu;
};


class ThemeMenu : public FileBrowser
{
	friend class ThemeView;

public:
	ThemeMenu(Font *font) :
		FileBrowser(NULL, font)
	{
	}

	virtual void selectCallback(int which)
	{
		const char *msg = this->pp_msgs[this->cur_sel];

		if (strcmp(msg, "None") != 0)
		{
			char *p = xstrdup(msg);

			p[strlen(p) - 1] = '\0';
			strcpy(Gui::gui->np->Theme, p + 1);
			if (!Gui::gui->setTheme(p + 1))
			{
				/* Something is wrong, reset to default */
				Gui::gui->setTheme("default");
				Gui::gui->pushDialogueBox(new DialogueBox(broken_theme_dlg));
				strcpy(Gui::gui->np->Theme, "default");
			}
			free(p);
		}
		Gui::gui->popView();
	}

	virtual void hoverCallback(int which)
	{
	}

	virtual void escapeCallback(int which)
	{
		Gui::gui->popView();
	}
};


ThemeView::ThemeView() : GuiView()
{
	this->menu = new ThemeMenu(Gui::gui->default_font);
}

ThemeView::~ThemeView()
{
	delete this->menu;
}

void ThemeView::setDirectory(const char *path)
{
	this->menu->setDirectory(path);
}

void ThemeView::runLogic()
{
	this->menu->runLogic();
}

void ThemeView::pushEvent(event_t ev)
{
	this->menu->pushEvent(ev);
}

void ThemeView::draw(SDL_Surface *where)
{
	SDL_Rect dst;

	/* Blit the backgrounds */
	dst = (SDL_Rect){20,45,300,400};
	SDL_BlitSurface(Gui::gui->main_menu_bg, NULL, where, &dst);

	dst = (SDL_Rect){350,13,0,0};
	SDL_BlitSurface(Gui::gui->infobox, NULL, where, &dst);

	this->menu->draw(where, 50, 70, 280, 375);
}

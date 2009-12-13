#include "menu.hh"

class DiscView;
class DiscMenu : public Menu
{
	friend class DiscView;

public:
	DiscMenu(Font *font, GuiView *parent) : Menu(font)
	{
		this->parent = parent;
		this->path = NULL;

		/* If nothing else: Set the default list */
		this->setDefaultFileList();
	}

	~DiscMenu()
	{
		this->freeFileList();
	}

	virtual void selectCallback(int which)
	{
		printf("entry %d selected: %s\n", which, this->pp_msgs[which]);
	}

	void setDirectory(const char *path)
	{
		const char *exts[] = {".d64", ".D64", ".t64", ".T64", ".prg",
				".PRG", ".p00", ".P00"};

		this->freeFileList();
		this->file_list = get_file_list(path, exts);
		if (!this->file_list)
			this->setDefaultFileList();
		this->setText(this->file_list);
	}

	virtual void hoverCallback(int which)
	{
	}

	virtual void escapeCallback(int which)
	{
		Gui::gui->exitMenu();
	}

private:
	void setDefaultFileList()
	{
		this->file_list = (const char **)xmalloc(2 * sizeof(char*));
		this->file_list[0] = xstrdup("None");
	}

	void freeFileList()
	{
		if (!this->file_list)
			return;
		for (int i = 0; this->file_list[i]; i++)
			free((void*)this->file_list[i]);
		free(this->file_list);
	}

	const char *path;
	const char **file_list;
	GuiView *parent;
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

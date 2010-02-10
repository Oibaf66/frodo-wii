#include "gui.hh"
#include "menu.hh"
#include "help_box.hh"
#include "status_bar.hh"
#include "data_store.hh"

#include <Network.h>

class NetworkUserView;

class PeerInfo
{
public:
	PeerInfo(const char *name, int scr_key)
	{
		this->name = (const char*)xstrdup(name);
		this->scr = NULL;
		this->region = 0;
		this->scr_key = scr_key;
	}

	~PeerInfo()
	{
		SDL_FreeSurface(this->scr);
		free((void*)this->name);
	}

	SDL_Surface *getScreenshot()
	{
		struct ds_data *data;

		if (this->scr)
			return this->scr;
		/* Look it up and store it when done */
		data = DataStore::ds->getData(this->scr_key);
		this->scr = sdl_surface_from_data(data->data, data->sz);

		return this->scr;
	}

	SDL_Surface *scr;
	const char *name;
	int region;
	int scr_key;
};

class NetworkUserMenu : public Menu
{
	friend class NetworkUserView;

public:
	NetworkUserMenu(Font *font) : Menu(font)
	{
		this->setText(NULL);
		this->peers = NULL;
		this->n_peers = 0;
	}

	~NetworkUserMenu()
	{
		this->freePeers();
	}

	void setPeers(NetworkUpdateListPeers *peerList)
	{
		NetworkUpdatePeerInfo *ps = peerList->peers;
		const char **messages;

		if (ps == NULL)
			return;

		this->freePeers();
		messages = (const char **)xmalloc( (peerList->n_peers + 1) *
				sizeof(const char*));

		for (unsigned i = 0; i < peerList->n_peers; i++)
		{
			messages[i] = (const char*)xstrdup((char*)ps->name);
			this->peers[i] = new PeerInfo(messages[i], ps->screenshot_key);
		}
		this->setText(messages);
		free((void*)messages);
	}

	virtual void selectCallback(int which)
	{
		Gui::gui->popView();
	}

	virtual void hoverCallback(int which)
	{
	}

	virtual void escapeCallback(int which)
	{
		Gui::gui->popView();
	}

private:
	void freePeers()
	{
		for (unsigned i = 0; i < this->n_peers; i++)
			delete this->peers[i];
		free(this->peers);
	}

	PeerInfo **peers;
	unsigned int n_peers;
};


class NetworkUserView : public GuiView
{
public:
	NetworkUserView() : GuiView()
	{
		this->menu = new NetworkUserMenu(Gui::gui->default_font);
	}

	~NetworkUserView()
	{
		delete this->menu;
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
		 SDL_BlitSurface(Gui::gui->main_menu_bg, NULL, where, &dst);

		 dst = (SDL_Rect){350,13,0,0};
		 SDL_BlitSurface(Gui::gui->disc_info, NULL, where, &dst);

		 this->menu->draw(where, 50, 70, 280, 375);
	}

protected:
	NetworkUserMenu *menu;
};

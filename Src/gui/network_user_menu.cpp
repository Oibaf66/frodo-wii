#include "gui.hh"
#include "menu.hh"
#include "help_box.hh"
#include "status_bar.hh"
#include "data_store.hh"
#include "network_user_menu.hh"

#include <Network.h>
#include <C64.h>

class NetworkUserView;

class PeerInfo
{
public:
	PeerInfo(NetworkUpdatePeerInfo *pi)
	{
		this->name = (const char*)xstrdup((char*)pi->name);
		this->scr = NULL;
		this->region = pi->region;
		if (this->region > REGION_ANTARTICA)
			this->region = REGION_UNKNOWN;
		this->scr_key = pi->screenshot_key;
		this->public_port = pi->public_port;
		this->private_port = pi->private_port;
		this->server_id = pi->server_id;

		this->hostname = ip_to_str(pi->public_ip);
	}

	~PeerInfo()
	{
		SDL_FreeSurface(this->scr);
		free((void*)this->name);
		free((void*)this->hostname);
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

	const char *getRegion()
	{
		return region_to_str(this->region);
	}

	SDL_Surface *scr;
	const char *name;
	const char *hostname;
	uint16_t public_port, private_port;
	uint32_t server_id;
	int region;
	int scr_key;
};

class PeerInfoBox : public Menu
{
public:
	PeerInfoBox(Font *font) : Menu(font)
	{
		this->pi = NULL;
		memset(this->pi_messages, 0, sizeof(this->pi_messages));
		this->setSelectedBackground(NULL, NULL, NULL, NULL, NULL, NULL);
	}

	void setPeerInfo(PeerInfo *pi)
	{
		this->pi = pi;
		this->updateMessages();
	}


	virtual void selectCallback(int which) { }
	virtual void hoverCallback(int which) { }
	virtual void escapeCallback(int which) { }

	void draw(SDL_Surface *where, int x, int y, int w, int h)
	{
		SDL_Surface *screenshot;
		SDL_Rect dst;

		if (!this->pi)
			return;

		screenshot = this->pi->getScreenshot();
		if (!screenshot)
		{
			Menu::draw(where, x, y + 10, w, h - 10);
			return;
		}

		/* Blit the screenshot */
		dst = (SDL_Rect){x + w / 2 - screenshot->w / 2, y, w, h};
		SDL_BlitSurface(screenshot, NULL, where, &dst);

		Menu::draw(where, x + 20, y + screenshot->h + 10, w - 20, h - screenshot->h - 10);

	}

	void updateMessages()
	{
		this->setText(NULL);
		memset(this->pi_messages, 0, sizeof(this->pi_messages));

		this->pi_messages[0] = " ";
		this->pi_messages[1] = "Name:";
		this->pi_messages[2] = " ";
		this->pi_messages[3] = "Region:";
		this->pi_messages[4] = " ";
		this->pi_messages[5] = " "; /* Maybe add something here later */

		if (this->pi)
		{
			this->pi_messages[1] = this->pi->name;
			this->pi_messages[3] = this->pi->getRegion();
			this->pi_messages[5] = " ";
		}

		this->setText(this->pi_messages);
	}

	const char *pi_messages[8];
	PeerInfo *pi;
};

class NetworkUserMenu : public Menu
{
	friend class NetworkUserView;

public:
	NetworkUserMenu(Font *font, PeerInfoBox *infoBox) : Menu(font)
	{
		this->setText(NULL);
		this->peers = NULL;
		this->n_peers = 0;
		this->infoBox = infoBox;
	}

	~NetworkUserMenu()
	{
		this->freePeers();
	}

	void setPeers(NetworkUpdateListPeers *peerList)
	{
		NetworkUpdatePeerInfo *ps = peerList->peers;
		const char **messages;

		if (ps == NULL || peerList->n_peers == 0)
			return;

		this->freePeers();
		this->n_peers = peerList->n_peers;
		messages = (const char **)xmalloc( (peerList->n_peers + 2) *
				sizeof(const char*));
		this->peers = (PeerInfo**)xrealloc((void*)this->peers,
				peerList->n_peers * sizeof(PeerInfo*));

		messages[0] = (const char *)xstrdup("None");
		for (unsigned i = 0; i < peerList->n_peers; i++)
		{
			messages[i + 1] = (const char*)xstrdup((char*)ps->name);
			this->peers[i] = new PeerInfo(&peerList->peers[i]);
		}
		this->setText(messages);
		free((void*)messages);
	}

	virtual void selectCallback(int which)
	{
		if (which > 0)
		{
			PeerInfo *peer = this->peers[which - 1];

			TheC64->network->SelectPeer(peer->hostname,
					peer->public_port, peer->server_id);
		}
		else
			TheC64->network->CancelPeerSelection();
		Gui::gui->exitMenu();
	}

	virtual void hoverCallback(int which)
	{
		if (which <= 0 || which > (int)this->n_peers)
			return;

		this->infoBox->setPeerInfo(this->peers[which - 1]);
	}

	virtual void escapeCallback(int which)
	{
		TheC64->network->CancelPeerSelection();
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
	PeerInfoBox *infoBox;
	unsigned int n_peers;
};


NetworkUserView::NetworkUserView() : GuiView()
			{
	this->peerInfo = new PeerInfoBox(Gui::gui->default_font);
	this->menu = new NetworkUserMenu(Gui::gui->default_font, this->peerInfo);
			}

NetworkUserView::~NetworkUserView()
{
	delete this->menu;
}

void NetworkUserView::runLogic()
{
	this->menu->runLogic();
}

void NetworkUserView::pushEvent(event_t ev)
{
	this->menu->pushEvent(ev);
}

void NetworkUserView::setPeers(NetworkUpdateListPeers *peerList)
{
	this->menu->setPeers(peerList);
}

void NetworkUserView::draw(SDL_Surface *where)
{
	SDL_Rect dst;

	/* Blit the backgrounds */
	dst = (SDL_Rect){20,45,300,400};
	SDL_BlitSurface(Gui::gui->main_menu_bg, NULL, where, &dst);

	dst = (SDL_Rect){350,13,0,0};
	SDL_BlitSurface(Gui::gui->network_info, NULL, where, &dst);

	this->menu->draw(where, 50, 70, 280, 375);
	this->peerInfo->draw(where, 390, 55, 242, 447);
}

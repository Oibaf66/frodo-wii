#ifndef NETWORK_USER_MENU_HH
#define NETWORK_USER_MENU_HH

#include <SDL.h>
#include <Network.h>

class GuiView;
class NetworkUserMenu;
class PeerInfoBox;

class NetworkUserView : public GuiView
{
public:
	NetworkUserView();

	~NetworkUserView();

	void runLogic();

	void pushEvent(SDL_Event *ev);

	void setPeers(NetworkUpdateListPeers *peerList);

	void draw(SDL_Surface *where);

protected:
	NetworkUserMenu *menu;
	PeerInfoBox *peerInfo;
};

#endif /* NETWORK_USER_MENU_HH */


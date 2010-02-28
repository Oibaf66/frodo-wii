#ifndef NETWORK_SERVER_MESSAGES_HH
#define NETWORK_SERVER_MESSAGES_HH

#include <SDL.h>

#include "status_bar.hh"

class NetworkServerMessages : public StatusBar
{
public:
	NetworkServerMessages();

	~NetworkServerMessages();

	virtual void draw(SDL_Surface *where);

protected:
	virtual void timeoutCallback();

private:
	char *flowed_messages[8];
};

#endif /* NETWORK_SERVER_MESSAGES_HH */

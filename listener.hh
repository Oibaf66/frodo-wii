#ifndef __LISTENER_HH__
#define __LISTENER_HH__

#include <string.h>

class Listener
{
	/* Implemented by the child */
};

class ListenerManager
{
public:
	ListenerManager();

	void registerListener(Listener *l);

	void unregisterListener(Listener *l);

	void flushListeners()
	{
		memset(this->listeners, 0, sizeof(this->listeners));
	}

	int nListeners()
	{
		return sizeof(this->listeners) / sizeof(*this->listeners);
	}

protected:
	Listener *listeners[8];
};

#endif /* __LISTENER_HH__ */

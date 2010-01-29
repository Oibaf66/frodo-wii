#include <utils.hh>

#include "listener.hh"

ListenerManager::ListenerManager()
{
	printf("Flushing listeners\n");
	this->flushListeners();
}

void ListenerManager::registerListener(Listener *kl)
{
	int n_listeners = sizeof(this->listeners) / sizeof(*this->listeners);
	int i;

	/* Don't register already registered listeners */
	for (i = 0; i < n_listeners; i++)
		if (this->listeners[i] == kl)
			return;
	/* Find a free spot */
	for (i = 0; i < n_listeners; i++)
		if (!this->listeners[i])
			break;

	panic_if(i == n_listeners,
			"No free listeners!\n");
	this->listeners[i] = kl;
}

void ListenerManager::unregisterListener(Listener *kl)
{
	int n_listeners = sizeof(this->listeners) / sizeof(*this->listeners);

	for (int i = 0; i < n_listeners; i++)
	{
		if (this->listeners[i] == kl)
			this->listeners[i] = NULL;
	}
}

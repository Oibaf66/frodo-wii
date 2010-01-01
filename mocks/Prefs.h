#ifndef __MOCK_PREFS_HH__
#define __MOCK_PREFS_HH__

#include <string.h>

class Prefs
{
public:
	Prefs()
	{
		strcpy(this->NetworkName, "Unset name");
		strcpy(this->NetworkServer, "play.c64-network.org");
		this->NetworkPort = 46214;
	}

	char NetworkName[32];
	char NetworkServer[128];
	int NetworkPort;
};

#endif /* __MOCK_PREFS_HH__ */

#ifndef __MOCK_PREFS_HH__
#define __MOCK_PREFS_HH__

#include <string.h>

#define SPEED_95 30
#define SPEED_100 20
#define SPEED_110 18

class Prefs
{
public:
	Prefs()
	{
		strcpy(this->NetworkName, "Unset name");
		strcpy(this->NetworkServer, "play.c64-network.org");
		this->NetworkPort = 46214;

		this->Emul1541Proc = 0;
		this->ShowLEDs = 0;
		this->DisplayOption = 0;
		this->MsPerFrame = SPEED_100;
	}

	char NetworkName[32];
	char NetworkServer[128];
	int NetworkPort;
	int Emul1541Proc;
	int ShowLEDs;
	int DisplayOption;
	unsigned int MsPerFrame;
};

#endif /* __MOCK_PREFS_HH__ */

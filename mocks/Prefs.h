#ifndef __MOCK_PREFS_HH__
#define __MOCK_PREFS_HH__

#include <string.h>

#define SPEED_95 30
#define SPEED_100 20
#define SPEED_110 18

enum
{
	/* ASCII values before these */
	JOY_NONE = 0,
	JOY_HORIZ = 256,
	JOY_VERT = 258,
	JOY_FIRE = 259,
};

/* Insanely high, but the Wii has insanely many of these */
#define MAX_JOYSTICK_AXES 32
#define MAX_JOYSTICK_BUTTONS 32
#define MAX_JOYSTICK_HATS 8

class Prefs
{
public:
	Prefs()
	{
		/* Set to NONE by default */
		memset(this->JoystickAxes, 0, sizeof(this->JoystickAxes));
		memset(this->JoystickButtons, 0, sizeof(this->JoystickButtons));
		memset(this->JoystickHats, 0, sizeof(this->JoystickHats));

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

	/* This is borrowed from UAE */
	int JoystickAxes[MAX_JOYSTICK_AXES];
	int JoystickHats[MAX_JOYSTICK_HATS];
	int JoystickButtons[MAX_JOYSTICK_BUTTONS];
};

#endif /* __MOCK_PREFS_HH__ */

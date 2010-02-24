/*
 *  Prefs.cpp - Global preferences
 *
 *  Frodo (C) 1994-1997,2002-2005 Christian Bauer
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
#include <string.h>

#include "sysdeps.h"

#include "Prefs.h"
#include "Display.h"
#include "C64.h"
#include "main.h"
#include "gui/widget.hh"


// These are the active preferences
Prefs ThePrefs;

// These are the preferences on disk
Prefs ThePrefsOnDisk;


/*
 *  Constructor: Set up preferences with defaults
 */

Prefs::Prefs()
{
	NormalCycles = 63;
	BadLineCycles = 23;
	CIACycles = 63;
	FloppyCycles = 64;
	SkipFrames = 1;
	LatencyMin = 80;
	LatencyMax = 120;
	LatencyAvg = 280;
	ScalingNumerator = 2;
	ScalingDenominator = 2;

#if defined(GEKKO)
	strcpy(BasePath, "/frodo/");
	strcpy(PrefsPath, "/frodo/frodorc");
#else
	strcpy(BasePath, "");
	strcpy(PrefsPath, "frodorc");
#endif

	strcpy(DrivePath[0], "64prgs");
	strcpy(DrivePath[1], "");
	strcpy(DrivePath[2], "");
	strcpy(DrivePath[3], "");

	strcpy(ViewPort, "Default");
	strcpy(DisplayMode, "Default");

	SIDType = SIDTYPE_DIGITAL;
	REUSize = REU_NONE;
	DisplayType = DISPTYPE_WINDOW;
	Joystick1Port = 1; /* Default to on */
	Joystick2Port = 1;

	SpritesOn = true;
	SpriteCollisions = true;
	JoystickSwap = false;
	LimitSpeed = false;
	FastReset = false;
	CIAIRQHack = false;
	MapSlash = true;
	Emul1541Proc = false;
	SIDFilters = true;
	DoubleScan = true;
	HideCursor = false;
	DirectSound = true;	
	ExclusiveSound = false;
	AutoPause = false;
	PrefsAtStartup = false;
	SystemMemory = false;
	AlwaysCopy = false;
	SystemKeys = true;
	ShowLEDs = true;

	this->SetupJoystickDefaults();

#ifdef HAVE_SDL
	this->DisplayOption = 0;
	this->MsPerFrame = SPEED_100;
#endif
	this->NetworkKey = rand() % 0xffff;
	this->NetworkAvatar = 0;
	snprintf(this->NetworkName, 32, "Unset name");
	snprintf(this->NetworkServer, 64, "play.c64-network.org");
	this->NetworkPort = 46214;
	this->NetworkRegion = REGION_UNKNOWN;
}


void Prefs::SetupJoystickDefaults()
{
	for (int i = 0; i < MAX_JOYSTICK_AXES; i++)
		this->JoystickAxes[i] = JOY_NONE;

	for (int i = 0; i < MAX_JOYSTICK_HATS; i++)
	{
		this->JoystickHats[i] = HAT_PLAIN;
		this->MenuJoystickHats[i] = HAT_PLAIN;
	}
	for (int i = 0; i < MAX_JOYSTICK_BUTTONS; i++)
	{
		this->JoystickButtons[i] = JOY_NONE;
		this->MenuJoystickButtons[i] = EVENT_NONE;
	}

	if (SDL_NumJoysticks() > 0)
	{
		const char *name = SDL_JoystickName(0);

		if (strncmp(name, "Wiimote", 7) == 0)
		{
			/* Wiimote/Classic hat */
			this->JoystickHats[0] = HAT_ROTATED_90;
			this->MenuJoystickHats[0] = HAT_ROTATED_90;

			/* Nunchuk/classic analogue */
			this->JoystickAxes[0] = JOY_HORIZ;
			this->JoystickAxes[1] = JOY_VERT;
			this->JoystickAxes[2] = JOY_HORIZ;
			this->JoystickAxes[3] = JOY_VERT;

			/* Wiimote 1, Nunchuk Z, classic a, b as fire */
			this->JoystickButtons[3] = 0x50;
			this->JoystickButtons[7] = 0x50;
			this->JoystickButtons[9] = 0x50;
			this->JoystickButtons[10] = 0x50;
			this->MenuJoystickButtons[3] = KEY_SELECT;
			this->MenuJoystickButtons[7] = KEY_SELECT;
			this->MenuJoystickButtons[9] = KEY_SELECT;
			this->MenuJoystickButtons[10] = KEY_SELECT;

			/* Wiimote A, Classic Zr, Zl as space */
			this->JoystickButtons[0] = (7 << 3) | 4;
			this->JoystickButtons[15] = (7 << 3) | 4;
			this->JoystickButtons[16] = (7 << 3) | 4;

			/* Wiimote B, classic x, y as R/S and menu escape */
			this->JoystickButtons[1] = (7 << 3) | 7;
			this->JoystickButtons[11] = (7 << 3) | 7;
			this->JoystickButtons[12] = (7 << 3) | 7;
			this->MenuJoystickButtons[1] = KEY_ESCAPE;
			this->MenuJoystickButtons[11] = KEY_ESCAPE;
			this->MenuJoystickButtons[12] = KEY_ESCAPE;

			/* Wiimote, classic Home as enter menu */
			this->MenuJoystickButtons[6] = ENTER_MENU;
			this->MenuJoystickButtons[19] = ENTER_MENU;
		}
		/* Saitek P380 */
		else if (strcmp(name, "Jess Tech Dual Analog Pad") == 0)
		{
			/* Pad */
			this->JoystickHats[0] = HAT_PLAIN;
			this->MenuJoystickHats[0] = HAT_PLAIN;

			/* Analogue parts */
			this->JoystickAxes[0] = JOY_HORIZ;
			this->JoystickAxes[1] = JOY_VERT;
			this->JoystickAxes[2] = JOY_HORIZ;
			this->JoystickAxes[3] = JOY_VERT;

			/* Button 4 Fire */
			this->JoystickButtons[3] = 0x50;

			/* Upper left front button R/S */
			this->JoystickButtons[4] = (7 << 3) | 7;
			/* Lower left front button space */
			this->JoystickButtons[5] = (7 << 3) | 4;

			/* Upper right 1, lower right 2 */
			this->JoystickButtons[6] = (7 << 3) | 0;
			this->JoystickButtons[7] = (7 << 3) | 3;

			/* Start to enter the menu */
		}

	}
}

/*
 *  Check if two Prefs structures are equal
 */

bool Prefs::operator==(const Prefs &rhs) const
{
	for (int i = 0; i < MAX_JOYSTICK_AXES; i++)
	{
		if (this->JoystickAxes[i] != rhs.JoystickAxes[i])
			return false;
	}
	for (int i = 0; i < MAX_JOYSTICK_HATS; i++)
	{
		if (this->JoystickHats[i] != rhs.JoystickHats[i])
			return false;
		if (this->MenuJoystickHats[i] != rhs.MenuJoystickHats[i])
			return false;
	}
	for (int i = 0; i < MAX_JOYSTICK_BUTTONS; i++)
	{
		if (this->JoystickButtons[i] != rhs.JoystickButtons[i])
			return false;
		if (this->MenuJoystickButtons[i] != rhs.MenuJoystickButtons[i])
			return false;
	}

	return (1
		&& NormalCycles == rhs.NormalCycles
		&& BadLineCycles == rhs.BadLineCycles
		&& CIACycles == rhs.CIACycles
		&& FloppyCycles == rhs.FloppyCycles
		&& SkipFrames == rhs.SkipFrames
		&& LatencyMin == rhs.LatencyMin
		&& LatencyMax == rhs.LatencyMax
		&& LatencyAvg == rhs.LatencyAvg
		&& ScalingNumerator == rhs.ScalingNumerator
		&& ScalingDenominator == rhs.ScalingNumerator
		&& strcmp(DrivePath[0], rhs.DrivePath[0]) == 0
		&& strcmp(DrivePath[1], rhs.DrivePath[1]) == 0
		&& strcmp(DrivePath[2], rhs.DrivePath[2]) == 0
		&& strcmp(DrivePath[3], rhs.DrivePath[3]) == 0
		&& strcmp(ViewPort, rhs.ViewPort) == 0
		&& strcmp(DisplayMode, rhs.DisplayMode) == 0
		&& SIDType == rhs.SIDType
		&& REUSize == rhs.REUSize
		&& DisplayType == rhs.DisplayType
		&& SpritesOn == rhs.SpritesOn
		&& SpriteCollisions == rhs.SpriteCollisions
		&& Joystick1Port == rhs.Joystick1Port
		&& Joystick2Port == rhs.Joystick2Port
		&& JoystickSwap == rhs.JoystickSwap
		&& LimitSpeed == rhs.LimitSpeed
		&& FastReset == rhs.FastReset
		&& CIAIRQHack == rhs.CIAIRQHack
		&& MapSlash == rhs.MapSlash
		&& Emul1541Proc == rhs.Emul1541Proc
		&& SIDFilters == rhs.SIDFilters
		&& DoubleScan == rhs.DoubleScan
		&& HideCursor == rhs.HideCursor
		&& DirectSound == rhs.DirectSound
		&& ExclusiveSound == rhs.ExclusiveSound
		&& AutoPause == rhs.AutoPause
		&& PrefsAtStartup == rhs.PrefsAtStartup
		&& SystemMemory == rhs.SystemMemory
		&& AlwaysCopy == rhs.AlwaysCopy
		&& SystemKeys == rhs.SystemKeys
		&& ShowLEDs == rhs.ShowLEDs
#ifdef HAVE_SDL
		&& this->DisplayOption == rhs.DisplayOption
		&& this->MsPerFrame == rhs.MsPerFrame
#endif
		&& this->NetworkKey == rhs.NetworkKey
		&& this->NetworkPort == rhs.NetworkPort
		&& this->NetworkRegion == rhs.NetworkRegion
		&& strcmp(this->NetworkServer, rhs.NetworkServer) == 0
		&& strcmp(this->NetworkName, rhs.NetworkName) == 0
		&& this->NetworkAvatar == rhs.NetworkAvatar
	);
}

bool Prefs::operator!=(const Prefs &rhs) const
{
	return !operator==(rhs);
}


/*
 *  Check preferences for validity and correct if necessary
 */

void Prefs::Check(void)
{
	if (SkipFrames <= 0) SkipFrames = 1;

	if (SIDType < SIDTYPE_NONE || SIDType > SIDTYPE_SIDCARD)
		SIDType = SIDTYPE_NONE;

	if (REUSize < REU_NONE || REUSize > REU_512K)
		REUSize = REU_NONE;

	if (DisplayType < DISPTYPE_WINDOW || DisplayType > DISPTYPE_SCREEN)
		DisplayType = DISPTYPE_WINDOW;
}


/*
 *  Load preferences from file
 */

void Prefs::Load(const char *filename)
{
	FILE *file;
	char line[256], keyword[256], value[256];

	if ((file = fopen(filename, "r")) != NULL) {
		while(fgets(line, 255, file)) {
			if (sscanf(line, "%s = %s\n", keyword, value) == 2) {
				if (!strcmp(keyword, "NormalCycles"))
					NormalCycles = atoi(value);
				else if (!strcmp(keyword, "BadLineCycles"))
					BadLineCycles = atoi(value);
				else if (!strcmp(keyword, "CIACycles"))
					CIACycles = atoi(value);
				else if (!strcmp(keyword, "FloppyCycles"))
					FloppyCycles = atoi(value);
				else if (!strcmp(keyword, "SkipFrames"))
					SkipFrames = atoi(value);
				else if (!strcmp(keyword, "LatencyMin"))
					LatencyMin = atoi(value);
				else if (!strcmp(keyword, "LatencyMax"))
					LatencyMax = atoi(value);
				else if (!strcmp(keyword, "LatencyAvg"))
					LatencyAvg = atoi(value);
				else if (!strcmp(keyword, "ScalingNumerator"))
					ScalingNumerator = atoi(value);
				else if (!strcmp(keyword, "ScalingDenominator"))
					ScalingDenominator = atoi(value);
				else if (!strcmp(keyword, "DrivePath8"))
					strcpy(DrivePath[0], value);
				else if (!strcmp(keyword, "DrivePath9"))
					strcpy(DrivePath[1], value);
				else if (!strcmp(keyword, "DrivePath10"))
					strcpy(DrivePath[2], value);
				else if (!strcmp(keyword, "DrivePath11"))
					strcpy(DrivePath[3], value);
				else if (!strcmp(keyword, "ViewPort"))
					strcpy(ViewPort, value);
				else if (!strcmp(keyword, "DisplayMode"))
					strcpy(DisplayMode, value);
				else if (!strcmp(keyword, "SIDType"))
					if (!strcmp(value, "DIGITAL"))
						SIDType = SIDTYPE_DIGITAL;
					else if (!strcmp(value, "SIDCARD"))
						SIDType = SIDTYPE_SIDCARD;
					else
						SIDType = SIDTYPE_NONE;
				else if (!strcmp(keyword, "REUSize")) {
					if (!strcmp(value, "128K"))
						REUSize = REU_128K;
					else if (!strcmp(value, "256K"))
						REUSize = REU_256K;
					else if (!strcmp(value, "512K"))
						REUSize = REU_512K;
					else
						REUSize = REU_NONE;
				} else if (!strcmp(keyword, "DisplayType"))
					DisplayType = strcmp(value, "SCREEN") ? DISPTYPE_WINDOW : DISPTYPE_SCREEN;
				else if (!strcmp(keyword, "Joystick1Port"))
					Joystick1Port = atoi(value);
				else if (!strcmp(keyword, "Joystick2Port"))
					Joystick2Port = atoi(value);
				else if (!strcmp(keyword, "SpritesOn"))
					SpritesOn = !strcmp(value, "TRUE");
				else if (!strcmp(keyword, "SpriteCollisions"))
					SpriteCollisions = !strcmp(value, "TRUE");
				else if (!strcmp(keyword, "JoystickSwap"))
					JoystickSwap = !strcmp(value, "TRUE");
				else if (!strcmp(keyword, "LimitSpeed"))
					LimitSpeed = !strcmp(value, "TRUE");
				else if (!strcmp(keyword, "FastReset"))
					FastReset = !strcmp(value, "TRUE");
				else if (!strcmp(keyword, "CIAIRQHack"))
					CIAIRQHack = !strcmp(value, "TRUE");
				else if (!strcmp(keyword, "MapSlash"))
					MapSlash = !strcmp(value, "TRUE");
				else if (!strcmp(keyword, "Emul1541Proc"))
					Emul1541Proc = !strcmp(value, "TRUE");
				else if (!strcmp(keyword, "SIDFilters"))
					SIDFilters = !strcmp(value, "TRUE");
				else if (!strcmp(keyword, "DoubleScan"))
					DoubleScan = !strcmp(value, "TRUE");
				else if (!strcmp(keyword, "HideCursor"))
					HideCursor = !strcmp(value, "TRUE");
				else if (!strcmp(keyword, "DirectSound"))
					DirectSound = !strcmp(value, "TRUE");
				else if (!strcmp(keyword, "ExclusiveSound"))
					ExclusiveSound = !strcmp(value, "TRUE");
				else if (!strcmp(keyword, "AutoPause"))
					AutoPause = !strcmp(value, "TRUE");
				else if (!strcmp(keyword, "PrefsAtStartup"))
					PrefsAtStartup = !strcmp(value, "TRUE");
				else if (!strcmp(keyword, "SystemMemory"))
					SystemMemory = !strcmp(value, "TRUE");
				else if (!strcmp(keyword, "AlwaysCopy"))
					AlwaysCopy = !strcmp(value, "TRUE");
				else if (!strcmp(keyword, "SystemKeys"))
					SystemKeys = !strcmp(value, "TRUE");
				else if (!strcmp(keyword, "ShowLEDs"))
					ShowLEDs = !strcmp(value, "TRUE");
				else if (!strncmp(keyword, "JoystickAxes", strlen("JoystickAxes")))
				{
					int n = atoi(keyword + strlen("JoystickAxes"));

					if (n >= 0 && n < MAX_JOYSTICK_AXES)
						this->JoystickAxes[n] = atoi(value);
				}
				else if (!strncmp(keyword, "JoystickHats", strlen("JoystickHats")))
				{
					int n = atoi(keyword + strlen("JoystickHats"));

					if (n >= 0 && n < MAX_JOYSTICK_HATS)
						this->JoystickHats[n] = atoi(value);
				}
				else if (!strncmp(keyword, "JoystickButtons", strlen("JoystickButtons")))
				{
					int n = atoi(keyword + strlen("JoystickButtons"));

					if (n >= 0 && n < MAX_JOYSTICK_BUTTONS)
						this->JoystickButtons[n] = atoi(value);
				}
				else if (!strncmp(keyword, "MenuJoystickHats", strlen("MenuJoystickHats")))
				{
					int n = atoi(keyword + strlen("MenuJoystickHats"));

					if (n >= 0 && n < MAX_JOYSTICK_HATS)
						this->MenuJoystickHats[n] = atoi(value);
				}
				else if (!strncmp(keyword, "MenuJoystickButtons", strlen("MenuJoystickButtons")))
				{
					int n = atoi(keyword + strlen("MenuJoystickButtons"));

					if (n >= 0 && n < MAX_JOYSTICK_BUTTONS)
						this->MenuJoystickButtons[n] = atoi(value);
				}
				else if (!strcmp(keyword, "DisplayOption"))
					DisplayOption = atoi(value);
				else if (!strcmp(keyword, "MsPerFrame"))
					MsPerFrame = atoi(value);
				else if (!strcmp(keyword, "NetworkKey"))
					NetworkKey = atoi(value);
				else if (!strcmp(keyword, "NetworkName"))
					strcpy(NetworkName, value);
				else if (!strcmp(keyword, "NetworkServer"))
					strcpy(NetworkServer, value);
				else if (!strcmp(keyword, "NetworkPort"))
					NetworkPort = atoi(value);
				else if (!strcmp(keyword, "NetworkName"))
					strcpy(NetworkName, value);
				else if (!strcmp(keyword, "NetworkRegion"))
					NetworkRegion = atoi(value);
				else if (!strcmp(keyword, "NetworkAvatar"))
					NetworkAvatar = atoi(value);
			}
		}
		fclose(file);
	}
	Check();
	ThePrefsOnDisk = *this;
}


/*
 *  Save preferences to file
 *  true: success, false: error
 */

bool Prefs::Save(const char *filename)
{
	FILE *file;

	Check();
	if ((file = fopen(filename, "w")) != NULL) {
		fprintf(file, "NormalCycles = %d\n", NormalCycles);
		fprintf(file, "BadLineCycles = %d\n", BadLineCycles);
		fprintf(file, "CIACycles = %d\n", CIACycles);
		fprintf(file, "FloppyCycles = %d\n", FloppyCycles);
		fprintf(file, "SkipFrames = %d\n", SkipFrames);
		fprintf(file, "LatencyMin = %d\n", LatencyMin);
		fprintf(file, "LatencyMax = %d\n", LatencyMax);
		fprintf(file, "LatencyAvg = %d\n", LatencyAvg);
		fprintf(file, "ScalingNumerator = %d\n", ScalingNumerator);
		fprintf(file, "ScalingDenominator = %d\n", ScalingDenominator);
		for (int i=0; i<4; i++)
			fprintf(file, "DrivePath%d = %s\n", i+8, DrivePath[i]);
		fprintf(file, "ViewPort = %s\n", ViewPort);
		fprintf(file, "DisplayMode = %s\n", DisplayMode);
		fprintf(file, "SIDType = ");
		switch (SIDType) {
			case SIDTYPE_NONE:
				fprintf(file, "NONE\n");
				break;
			case SIDTYPE_DIGITAL:
				fprintf(file, "DIGITAL\n");
				break;
			case SIDTYPE_SIDCARD:
				fprintf(file, "SIDCARD\n");
				break;
		}
		fprintf(file, "REUSize = ");
		switch (REUSize) {
			case REU_NONE:
				fprintf(file, "NONE\n");
				break;
			case REU_128K:
				fprintf(file, "128K\n");
				break;
			case REU_256K:
				fprintf(file, "256K\n");
				break;
			case REU_512K:
				fprintf(file, "512K\n");
				break;
		};
		fprintf(file, "DisplayType = %s\n", DisplayType == DISPTYPE_WINDOW ? "WINDOW" : "SCREEN");
		fprintf(file, "Joystick1Port = %d\n", Joystick1Port);
		fprintf(file, "Joystick2Port = %d\n", Joystick2Port);
		fprintf(file, "SpritesOn = %s\n", SpritesOn ? "TRUE" : "FALSE");
		fprintf(file, "SpriteCollisions = %s\n", SpriteCollisions ? "TRUE" : "FALSE");
		fprintf(file, "JoystickSwap = %s\n", JoystickSwap ? "TRUE" : "FALSE");
		fprintf(file, "LimitSpeed = %s\n", LimitSpeed ? "TRUE" : "FALSE");
		fprintf(file, "FastReset = %s\n", FastReset ? "TRUE" : "FALSE");
		fprintf(file, "CIAIRQHack = %s\n", CIAIRQHack ? "TRUE" : "FALSE");
		fprintf(file, "MapSlash = %s\n", MapSlash ? "TRUE" : "FALSE");
		fprintf(file, "Emul1541Proc = %s\n", Emul1541Proc ? "TRUE" : "FALSE");
		fprintf(file, "SIDFilters = %s\n", SIDFilters ? "TRUE" : "FALSE");
		fprintf(file, "DoubleScan = %s\n", DoubleScan ? "TRUE" : "FALSE");
		fprintf(file, "HideCursor = %s\n", HideCursor ? "TRUE" : "FALSE");
		fprintf(file, "DirectSound = %s\n", DirectSound ? "TRUE" : "FALSE");
		fprintf(file, "ExclusiveSound = %s\n", ExclusiveSound ? "TRUE" : "FALSE");
		fprintf(file, "AutoPause = %s\n", AutoPause ? "TRUE" : "FALSE");
		fprintf(file, "PrefsAtStartup = %s\n", PrefsAtStartup ? "TRUE" : "FALSE");
		fprintf(file, "SystemMemory = %s\n", SystemMemory ? "TRUE" : "FALSE");
		fprintf(file, "AlwaysCopy = %s\n", AlwaysCopy ? "TRUE" : "FALSE");
		fprintf(file, "SystemKeys = %s\n", SystemKeys ? "TRUE" : "FALSE");
		fprintf(file, "ShowLEDs = %s\n", ShowLEDs ? "TRUE" : "FALSE");

		for (int i = 0; i < MAX_JOYSTICK_AXES; i++)
			fprintf(file, "JoystickAxes%d = %d\n", i, JoystickAxes[i]);
		for (int i = 0; i < MAX_JOYSTICK_HATS; i++)
		{
			fprintf(file, "JoystickHats%d = %d\n", i, JoystickHats[i]);
			fprintf(file, "MenuJoystickHats%d = %d\n", i, MenuJoystickHats[i]);
		}
		for (int i = 0; i < MAX_JOYSTICK_BUTTONS; i++)
		{
			fprintf(file, "JoystickButtons%d = %d\n", i, JoystickButtons[i]);
			fprintf(file, "MenuJoystickButtons%d = %d\n", i, MenuJoystickButtons[i]);
		}

		fprintf(file, "DisplayOption = %d\n", DisplayOption);
		fprintf(file, "MsPerFrame = %d\n", MsPerFrame);
		fprintf(file, "NetworkKey = %d\n", NetworkKey);
		fprintf(file, "NetworkAvatar = %d\n", NetworkAvatar);
		fprintf(file, "NetworkName = %s\n", NetworkName);
		fprintf(file, "NetworkServer = %s\n", NetworkServer);
		fprintf(file, "NetworkPort = %d\n", NetworkPort);
		fprintf(file, "NetworkRegion = %d\n", NetworkRegion);
		fclose(file);
		ThePrefsOnDisk = *this;
		return true;
	}
	return false;
}

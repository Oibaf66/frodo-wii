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

// These are the default preferences
Prefs TheDefaultPrefs;

static void maybe_write(FILE *fp, bool do_write, const char *fmt, ...)
{
     va_list ap;
     int r;

     if (!do_write)
	  return;

     va_start(ap, fmt);
     r = vfprintf(fp, fmt, ap);
     va_end(ap);
}

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
	Joystick1Port = 0; /* Default to on */
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

	this->MsPerFrame = SPEED_100;
	this->NetworkKey = rand() % 0xffff;
	this->NetworkAvatar = 0;
	snprintf(this->NetworkName, 32, "Unset name");
	snprintf(this->NetworkServer, 64, "play.c64-network.org");
	this->NetworkPort = 46214;
	this->NetworkRegion = REGION_UNKNOWN;

	strcpy(this->Theme, "default");
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
			this->JoystickHats[0] = HAT_PLAIN;
			this->MenuJoystickHats[0] = HAT_PLAIN;

			/* Nunchuk/classic analogue */
			this->JoystickAxes[0] = JOY_HORIZ;
			this->JoystickAxes[1] = JOY_VERT;
			this->JoystickAxes[2] = JOY_HORIZ;
			this->JoystickAxes[3] = JOY_VERT;

			/* Wiimote 2, Nunchuk Z, classic a, b as fire */
			this->JoystickButtons[3] = 0x50;
			this->JoystickButtons[7] = 0x50;
			this->JoystickButtons[9] = 0x50;
			this->JoystickButtons[10] = 0x50;

			this->MenuJoystickButtons[3] = KEY_SELECT;
			this->MenuJoystickButtons[7] = KEY_SELECT;
			this->MenuJoystickButtons[9] = KEY_SELECT;
			this->MenuJoystickButtons[10] = KEY_SELECT;

			/* Wiimote +/- as page up, page down */
			this->MenuJoystickButtons[4] = KEY_PAGEUP;
			this->MenuJoystickButtons[5] = KEY_PAGEDOWN;
			this->MenuJoystickButtons[17] = KEY_PAGEUP;
			this->MenuJoystickButtons[18] = KEY_PAGEDOWN;

			/* +/- as 1 and 2 */
			this->JoystickButtons[4] = (7 << 3) | 0;
			this->JoystickButtons[5] = (7 << 3) | 3;
			this->JoystickButtons[17] = (7 << 3) | 0;
			this->JoystickButtons[18] = (7 << 3) | 3;

			/* Wiimote B, Nunchuk C, Classic Zr, Zl as space */
			this->JoystickButtons[1] = (7 << 3) | 4;
			this->JoystickButtons[8] = (7 << 3) | 4;
			this->JoystickButtons[15] = (7 << 3) | 4;
			this->JoystickButtons[16] = (7 << 3) | 4;

			/* Wiimote A and Classic L as F1 */
			this->JoystickButtons[0] = (0 << 3) | 4;
			this->JoystickButtons[13] = (0 << 3) | 4;

			/* Classic R as F3 */
			this->JoystickButtons[16] = (0 << 3) | 5;

			/* Wiimote 1 as R/S, classic X/Y */
			this->JoystickButtons[2] = (7 << 3) | 7;
			this->JoystickButtons[11] = (7 << 3) | 7;
			this->JoystickButtons[12] = (7 << 3) | 7;

			/* Wiimote 1, classic x, y as menu escape */
			this->MenuJoystickButtons[2] = KEY_ESCAPE;
			this->MenuJoystickButtons[11] = KEY_ESCAPE;
			this->MenuJoystickButtons[12] = KEY_ESCAPE;

			/* Wiimote, classic Home as enter menu */
			this->MenuJoystickButtons[6] = KEY_ENTER_MENU;
			this->MenuJoystickButtons[19] = KEY_ENTER_MENU;
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

			this->JoystickButtons[0] = (0 << 3) | 4;
			this->JoystickButtons[1] = (0 << 3) | 5;

			/* Button 4 Fire */
			this->JoystickButtons[3] = 0x50;
			this->MenuJoystickButtons[3] = KEY_SELECT;

			/* Upper left front button R/S */
			this->JoystickButtons[4] = (7 << 3) | 7;
			/* Lower left front button space */
			this->JoystickButtons[5] = (7 << 3) | 4;

			/* Upper right 1, lower right 2 */
			this->JoystickButtons[6] = (7 << 3) | 0;
			this->JoystickButtons[7] = (7 << 3) | 3;

			/* Start to enter the menu */
			this->MenuJoystickButtons[0] = KEY_PAGEDOWN;
			this->MenuJoystickButtons[1] = KEY_PAGEUP;
			this->MenuJoystickButtons[8] = KEY_ESCAPE;
			this->MenuJoystickButtons[9] = KEY_ENTER_MENU;
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
		&& this->MsPerFrame == rhs.MsPerFrame
		&& this->NetworkKey == rhs.NetworkKey
		&& this->NetworkPort == rhs.NetworkPort
		&& this->NetworkRegion == rhs.NetworkRegion
		&& strcmp(this->NetworkServer, rhs.NetworkServer) == 0
		&& strcmp(this->NetworkName, rhs.NetworkName) == 0
		&& strcmp(this->Theme, rhs.Theme) == 0
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
				else if (!strcmp(keyword, "Theme"))
					strcpy(Theme, value);
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
		maybe_write(file, NormalCycles != TheDefaultPrefs.NormalCycles, "NormalCycles = %d\n", NormalCycles);
		maybe_write(file, BadLineCycles != TheDefaultPrefs.BadLineCycles, "BadLineCycles = %d\n", BadLineCycles);
		maybe_write(file, CIACycles != TheDefaultPrefs.CIACycles, "CIACycles = %d\n", CIACycles);
		maybe_write(file, FloppyCycles != TheDefaultPrefs.FloppyCycles, "FloppyCycles = %d\n", FloppyCycles);
		maybe_write(file, SkipFrames != TheDefaultPrefs.SkipFrames, "SkipFrames = %d\n", SkipFrames);
		maybe_write(file, LatencyMin != TheDefaultPrefs.LatencyMin, "LatencyMin = %d\n", LatencyMin);
		maybe_write(file, LatencyMax != TheDefaultPrefs.LatencyMax, "LatencyMax = %d\n", LatencyMax);
		maybe_write(file, LatencyAvg != TheDefaultPrefs.LatencyAvg, "LatencyAvg = %d\n", LatencyAvg);
		maybe_write(file, ScalingNumerator != TheDefaultPrefs.ScalingNumerator, "ScalingNumerator = %d\n", ScalingNumerator);
		maybe_write(file, ScalingDenominator != TheDefaultPrefs.ScalingDenominator, "ScalingDenominator = %d\n", ScalingDenominator);
		for (int i=0; i<4; i++) {
			maybe_write(file, strcmp(DrivePath[i], TheDefaultPrefs.DrivePath[i]) != 0, "DrivePath%d = %s\n", i+8, DrivePath[i]);
		}
		maybe_write(file, strcmp(ViewPort, TheDefaultPrefs.ViewPort) != 0, "ViewPort = %s\n", ViewPort);
		maybe_write(file, strcmp(DisplayMode, TheDefaultPrefs.DisplayMode) != 0, "DisplayMode = %s\n", DisplayMode);
		if (SIDType != TheDefaultPrefs.SIDType)
		{
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
		}
		if (REUSize != TheDefaultPrefs.REUSize)
		{
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
		}
		maybe_write(file, DisplayType != TheDefaultPrefs.DisplayType, "DisplayType = %s\n", DisplayType == DISPTYPE_WINDOW ? "WINDOW" : "SCREEN");
		maybe_write(file, Joystick1Port != TheDefaultPrefs.Joystick1Port, "Joystick1Port = %d\n", Joystick1Port);
		maybe_write(file, Joystick2Port != TheDefaultPrefs.Joystick2Port, "Joystick1Port = %d\n", Joystick2Port);
		maybe_write(file, SpritesOn != TheDefaultPrefs.SpritesOn, "SpritesOn = %s\n", SpritesOn ? "TRUE" : "FALSE");
		maybe_write(file, SpriteCollisions != TheDefaultPrefs.SpriteCollisions, "SpriteCollisions = %s\n", SpriteCollisions ? "TRUE" : "FALSE");
		maybe_write(file, JoystickSwap != TheDefaultPrefs.JoystickSwap, "JoystickSwap = %s\n", JoystickSwap ? "TRUE" : "FALSE");
		maybe_write(file, LimitSpeed != TheDefaultPrefs.LimitSpeed, "LimitSpeed = %s\n", LimitSpeed ? "TRUE" : "FALSE");
		maybe_write(file, FastReset != TheDefaultPrefs.FastReset, "FastReset = %s\n", FastReset ? "TRUE" : "FALSE");
		maybe_write(file, CIAIRQHack != TheDefaultPrefs.CIAIRQHack, "CIAIRQHack = %s\n", CIAIRQHack ? "TRUE" : "FALSE");
		maybe_write(file, MapSlash != TheDefaultPrefs.MapSlash, "MapSlash = %s\n", MapSlash ? "TRUE" : "FALSE");
		maybe_write(file, Emul1541Proc != TheDefaultPrefs.Emul1541Proc, "Emul1541Proc = %s\n", Emul1541Proc ? "TRUE" : "FALSE");
		maybe_write(file, SIDFilters != TheDefaultPrefs.SIDFilters, "SIDFilters = %s\n", SIDFilters ? "TRUE" : "FALSE");
		maybe_write(file, DoubleScan != TheDefaultPrefs.DoubleScan, "DoubleScan = %s\n", DoubleScan ? "TRUE" : "FALSE");
		maybe_write(file, HideCursor != TheDefaultPrefs.HideCursor, "HideCursor = %s\n", HideCursor ? "TRUE" : "FALSE");
		maybe_write(file, DirectSound != TheDefaultPrefs.DirectSound, "DirectSound = %s\n", DirectSound ? "TRUE" : "FALSE");
		maybe_write(file, ExclusiveSound != TheDefaultPrefs.ExclusiveSound, "ExclusiveSound = %s\n", ExclusiveSound ? "TRUE" : "FALSE");
		maybe_write(file, AutoPause != TheDefaultPrefs.AutoPause, "AutoPause = %s\n", AutoPause ? "TRUE" : "FALSE");
		maybe_write(file, PrefsAtStartup != TheDefaultPrefs.PrefsAtStartup, "PrefsAtStartup = %s\n", PrefsAtStartup ? "TRUE" : "FALSE");
		maybe_write(file, SystemMemory != TheDefaultPrefs.SystemMemory, "SystemMemory = %s\n", SystemMemory ? "TRUE" : "FALSE");
		maybe_write(file, AlwaysCopy != TheDefaultPrefs.AlwaysCopy, "AlwaysCopy = %s\n", AlwaysCopy ? "TRUE" : "FALSE");
		maybe_write(file, SystemKeys != TheDefaultPrefs.SystemKeys, "SystemKeys = %s\n", SystemKeys ? "TRUE" : "FALSE");
		maybe_write(file, ShowLEDs != TheDefaultPrefs.ShowLEDs, "ShowLEDs = %s\n", ShowLEDs ? "TRUE" : "FALSE");

		for (int i = 0; i < MAX_JOYSTICK_AXES; i++)
			maybe_write(file, JoystickAxes[i] != TheDefaultPrefs.JoystickAxes[i], "JoystickAxes%d = %d\n", i, JoystickAxes[i]);
		for (int i = 0; i < MAX_JOYSTICK_HATS; i++)
		{
			maybe_write(file, JoystickHats[i] != TheDefaultPrefs.JoystickHats[i], "JoystickHats%d = %d\n", i, JoystickHats[i]);
			maybe_write(file, MenuJoystickHats[i] != TheDefaultPrefs.MenuJoystickHats[i], "MenuJoystickHats%d = %d\n", i, MenuJoystickHats[i]);
		}
		for (int i = 0; i < MAX_JOYSTICK_BUTTONS; i++)
		{
			maybe_write(file, JoystickButtons[i] != TheDefaultPrefs.JoystickButtons[i], "JoystickButtons%d = %d\n", i, JoystickButtons[i]);
			maybe_write(file, MenuJoystickButtons[i] != TheDefaultPrefs.MenuJoystickButtons[i], "MenuJoystickButtons%d = %d\n", i, MenuJoystickButtons[i]);
		}

		maybe_write(file, MsPerFrame != TheDefaultPrefs.MsPerFrame, "MsPerFrame = %d\n", MsPerFrame);
		maybe_write(file, NetworkKey != TheDefaultPrefs.NetworkKey, "NetworkKey = %d\n", NetworkKey);
		maybe_write(file, NetworkAvatar != TheDefaultPrefs.NetworkAvatar, "NetworkAvatar = %d\n", NetworkAvatar);
		maybe_write(file, strcmp(NetworkName, TheDefaultPrefs.NetworkName) != 0, "NetworkName = %s\n", NetworkName);
		maybe_write(file, strcmp(NetworkServer, TheDefaultPrefs.NetworkServer) != 0, "NetworkServer = %s\n", NetworkServer);
		maybe_write(file, NetworkPort != TheDefaultPrefs.NetworkPort, "NetworkPort = %d\n", NetworkPort);
		maybe_write(file, NetworkRegion != TheDefaultPrefs.NetworkRegion, "NetworkRegion = %d\n", NetworkRegion);
		maybe_write(file, strcmp(Theme, TheDefaultPrefs.Theme) != 0, "Theme = %s\n", Theme);
		fclose(file);
		ThePrefsOnDisk = *this;
		return true;
	}
	return false;
}

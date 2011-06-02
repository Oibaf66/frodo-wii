/*
 *  Prefs.h - Global preferences
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

#ifndef _PREFS_H
#define _PREFS_H

#define SPEED_95 30
#define SPEED_100 25
#define SPEED_110 18

#define MAX_JOYSTICK_AXES 16
#define MAX_JOYSTICK_BUTTONS 32
#define MAX_JOYSTICK_HATS 8

// SID types
enum {
	SIDTYPE_NONE,		// SID emulation off
	SIDTYPE_DIGITAL,	// Digital SID emulation
	SIDTYPE_SIDCARD		// SID card
};


// REU sizes
enum {
	REU_NONE,		// No REU
	REU_128K,		// 128K
	REU_256K,		// 256K
	REU_512K		// 512K
};


// Display types
enum {
	DISPTYPE_WINDOW,	// Window
	DISPTYPE_SCREEN		// Fullscreen
};

enum {
 	/* ASCII values before these */
        JOY_NONE = 0,
        JOY_HORIZ = 256,
        JOY_VERT = 258,
	JOY_FIRE = 259,
};

enum {
	HAT_PLAIN = 0,
	HAT_ROTATED_90 = 1,
	HAT_ROTATED_180 = 2,
	HAT_ROTATED_270 = 2,
};

// Key bindings (WII)
enum {
	WIIMOTE_UP,
	WIIMOTE_DOWN,
	WIIMOTE_LEFT,
	WIIMOTE_RIGHT,
	WIIMOTE_2,
	WIIMOTE_1,
	WIIMOTE_A,
	WIIMOTE_B,
	WIIMOTE_PLUS,
	WIIMOTE_MINUS,
	CLASSIC_UP,
	CLASSIC_DOWN,
	CLASSIC_LEFT,
	CLASSIC_RIGHT,
	CLASSIC_A,
	CLASSIC_B,
	CLASSIC_X,
	CLASSIC_Y,
	CLASSIC_L,
	CLASSIC_R,
	CLASSIC_ZR,
	CLASSIC_ZL,
	N_WIIMOTE_BINDINGS
};



// Preferences data
class Prefs {
public:
	Prefs();
	bool ShowEditor(bool startup, char *prefs_name);
	void Check(void);
	void Load(const char *filename);
	bool Save(const char *filename);
	bool Save_game(const char *filename);

	bool operator==(const Prefs &rhs) const;
	bool operator!=(const Prefs &rhs) const;

	void SetupJoystickDefaults();

	char BasePath[256];		// Where theme data etc are found
	char PrefsPath[256];		// Where the prefs will be stored
	int NormalCycles;		// Available CPU cycles in normal raster lines
	int BadLineCycles;		// Available CPU cycles in Bad Lines
	int CIACycles;			// CIA timer ticks per raster line
	int FloppyCycles;		// Available 1541 CPU cycles per line
	int SkipFrames;			// Draw every n-th frame

	char DrivePath[4][256];	// Path for drive 8..11

	char ViewPort[256];		// Size of the C64 screen to display (Win32)
	char DisplayMode[256];	// Video mode to use for full screen (Win32)

	int SIDType;			// SID emulation type
	int REUSize;			// Size of REU
	int DisplayType;		// Display type (BeOS)
	int Joystick1Port;		// Port that joystick 1 is connected to (0 = no joystick, all other values are system dependant)
	int Joystick2Port;		// Port that joystick 2 is connected to
	int LatencyMin;			// Min msecs ahead of sound buffer (Win32)
	int LatencyMax;			// Max msecs ahead of sound buffer (Win32)
	int LatencyAvg;			// Averaging interval in msecs (Win32)
	int ScalingNumerator;	// Window scaling numerator (Win32)
	int ScalingDenominator;	// Window scaling denominator (Win32)

	bool SpritesOn;			// Sprite display is on
	bool SpriteCollisions;	// Sprite collision detection is on
	bool JoystickSwap;		// Swap joysticks 1<->2
	bool LimitSpeed;		// Limit speed to 100%
	bool FastReset;			// Skip RAM test on reset
	bool CIAIRQHack;		// Write to CIA ICR clears IRQ
	bool MapSlash;			// Map '/' in C64 filenames
	bool Emul1541Proc;		// Enable processor-level 1541 emulation
	bool SIDFilters;		// Emulate SID filters
	bool DoubleScan;		// Double scan lines (BeOS, if DisplayType == DISPTYPE_SCREEN)
	bool JoystickGeekPort;	// Enable GeekPort joystick adapter
	bool HideCursor;		// Hide mouse cursor when visible (Win32)
	bool DirectSound;		// Use direct sound (instead of wav) (Win32)
	bool ExclusiveSound;	// Use exclusive mode with direct sound (Win32)
	bool AutoPause;			// Auto pause when not foreground app (Win32)
	bool PrefsAtStartup;	// Show prefs dialog at startup (Win32)
	bool SystemMemory;		// Put view work surface in system mem (Win32)
	bool AlwaysCopy;		// Always use a work surface (Win32)
	bool SystemKeys;		// Enable system keys and menu keys (Win32)
	bool ShowLEDs;			// Show LEDs (Win32)

	uint32 MsPerFrame;

	int JoystickAxes[MAX_JOYSTICK_AXES];
	int JoystickHats[MAX_JOYSTICK_HATS];
	int JoystickButtons[MAX_JOYSTICK_BUTTONS];

	int MenuJoystickHats[MAX_JOYSTICK_HATS];
	int MenuJoystickButtons[MAX_JOYSTICK_BUTTONS];

	char NetworkName[32];
	char NetworkServer[64];
	int NetworkRegion;
	int NetworkPort;

	int NetworkKey;
	uint16 NetworkAvatar;
	char Theme[128];

	bool CursorKeysForJoystick;
};


// These are the active preferences
extern Prefs ThePrefs;

// Theses are the preferences on disk
extern Prefs ThePrefsOnDisk;

// Theses are the default preferences
extern Prefs TheDefaultPrefs;

#endif

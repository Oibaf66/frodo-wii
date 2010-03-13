/*
 *  C64.h - Put the pieces together
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
#ifndef _C64_H
#define _C64_H

#include "Network.h"
#include "Prefs.h"

/* Network connection type */
enum
{
	NONE,
	CONNECT,
	MASTER,
	CLIENT
};

// Sizes of memory areas
const size_t C64_RAM_SIZE = 0x10000;
const size_t COLOR_RAM_SIZE = 0x400;
const size_t BASIC_ROM_SIZE = 0x2000;
const size_t KERNAL_ROM_SIZE = 0x2000;
const size_t CHAR_ROM_SIZE = 0x1000;
const size_t DRIVE_RAM_SIZE = 0x800;
const size_t DRIVE_ROM_SIZE = 0x4000;


// false: Frodo, true: FrodoSC
extern bool IsFrodoSC;

class Prefs;
class C64Display;
class MOS6510;
class MOS6569;
class MOS6581;
class MOS6526_1;
class MOS6526_2;
class IEC;
class REU;
class MOS6502_1541;
class Job1541;
class CmdPipe;

class C64 {
public:
	C64();
	~C64();

	void Run(void);
	void Quit(void);
	void Pause(void);

	void Resume(void);

	void Reset(void);
	void NMI(void);
	void VBlank(bool draw_frame);
	void NewPrefs(Prefs *prefs);
	void PatchKernal(bool fast_reset, bool emul_1541_proc);
	void SaveRAM(char *filename);
	void SaveSnapshot(const char *filename);
	bool LoadSnapshot(const char *filename);
	int SaveCPUState(FILE *f);
	int Save1541State(FILE *f);
	bool Save1541JobState(FILE *f);
	bool SaveVICState(FILE *f);
	bool SaveSIDState(FILE *f);
	bool SaveCIAState(FILE *f);
	bool LoadCPUState(FILE *f);
	bool Load1541State(FILE *f);
	bool Load1541JobState(FILE *f);
	bool LoadVICState(FILE *f);
	bool LoadSIDState(FILE *f);
	bool LoadCIAState(FILE *f);

	uint8 *RAM, *Basic, *Kernal,
		  *Char, *Color;		// C64
	uint8 *RAM1541, *ROM1541;	// 1541

	C64Display *TheDisplay;

	MOS6510 *TheCPU;			// C64
	MOS6569 *TheVIC;
	MOS6581 *TheSID;
	MOS6526_1 *TheCIA1;
	MOS6526_2 *TheCIA2;
	IEC *TheIEC;
	REU *TheREU;

	MOS6502_1541 *TheCPU1541;	// 1541
	Job1541 *TheJob1541;

#ifdef FRODO_SC
	uint32 CycleCounter;
#endif
	bool IsPaused();

	void quit()
	{
		this->quit_thyself = true;
	}
private:
	void c64_ctor1(void);
	void c64_ctor2(void);
	void c64_dtor(void);
	void open_joystick(int port);
	void close_joystick(int port);
	uint8 poll_joystick(int port);
	uint8 poll_joystick_axes(int port, bool *has_event);
	uint8 poll_joystick_hats(int port, bool *has_event);
	uint8 poll_joystick_buttons(int port, uint8 *table, bool *has_event);
	void thread_func(void);

	bool thread_running;	// Emulation thread is running
	bool quit_thyself;		// Emulation thread shall quit
	bool have_a_break;		// Emulation thread shall pause

	int joy_minx[2], joy_maxx[2], joy_miny[2], joy_maxy[2]; // For dynamic joystick calibration
	uint8 joykey;			// Joystick keyboard emulation mask value

	uint8 orig_kernal_1d84,	// Original contents of kernal locations $1d84 and $1d85
		  orig_kernal_1d85;	// (for undoing the Fast Reset patch)

public:
	char server_hostname[255];
	int server_port;
	int network_connection_type;
	Network *network;
	int linecnt;

	bool fake_key_sequence;
	const char *fake_key_str;
	int fake_key_index;
	int fake_key_keytime;

	bool prefs_changed;
	char save_game_name[256];

	void network_vblank();

	void startFakeKeySequence(const char *str);
	void run_fake_key_sequence();
	void pushKeyCode(int kc, bool up);
};


#endif

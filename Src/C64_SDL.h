/*
 *  C64_x.i - Put the pieces together, X specific stuff
 *
 *  Frodo (C) 1994-1997,2002 Christian Bauer
 *  Unix stuff by Bernd Schmidt/Lutz Vieweg
 */

#include "main.h"

#include <sys/types.h>
#include <dirent.h>

#include "gui/gui.hh"
#include "gui/status_bar.hh"
#include "gui/virtual_keyboard.hh"

#if defined(GEKKO)
#include <wiiuse/wpad.h>
#include <ogc/lwp_watchdog.h>
#endif

#define C64_NETWORK_BROKER "c64-network.game-host.org"

/* TODO: */
extern char *network_server_connect;


static struct timeval tv_start;

/*
 *  Constructor, system-dependent things
 */
void C64::c64_ctor1(void)
{
	// Initialize joystick variables
#ifdef HAVE_LINUX_JOYSTICK_H
	joyfd[0] = joyfd[1] = -1;
	joy_minx = joy_miny = 32767;
	joy_maxx = joy_maxy = -32768;
#endif
	this->linecnt = 0;

	this->fake_key_sequence = false;
	this->fake_key_index = 0;
	this->fake_key_keytime = 4;
        this->fake_key_str = "\nLOAD \"*\",8,1\nRUN\n";

	this->prefs_changed = false;
	memset(this->save_game_name, 0, sizeof(this->save_game_name));
	strcpy(this->save_game_name, "unknown");

	strncpy(this->server_hostname, C64_NETWORK_BROKER,
			sizeof(this->server_hostname));
	this->server_port = 46214;
	this->network_connection_type = NONE;
	this->network = NULL;

	if (network_server_connect) {
		printf("Connecting to %s\n", network_server_connect);
		strcpy(this->server_hostname, network_server_connect);
		this->network = new Network(this->server_hostname, this->server_port);
		this->network_connection_type = CONNECT;
	}
}

void C64::c64_ctor2(void)
{
	gettimeofday(&tv_start, NULL);
}


/*
 *  Destructor, system-dependent things
 */

void C64::c64_dtor(void)
{
}


void C64::pushKeyCode(int kc, bool up)
{
	TheDisplay->UpdateKeyMatrix(kc, up, TheCIA1->KeyMatrix, TheCIA1->RevMatrix, NULL);
}

/* From dreamcast port but heavily modified */
void C64::run_fake_key_sequence()
{
	int kc = Gui::gui->kbd->charToKeycode(this->fake_key_str[this->fake_key_index]);

	TheDisplay->FakeKeyPress(kc, TheCIA1->KeyMatrix, TheCIA1->RevMatrix);

	this->fake_key_keytime --;
        if (this->fake_key_keytime == 0)
        {
                this->fake_key_keytime = 4;
                this->fake_key_index ++;
        	TheDisplay->FakeKeyPress(-1, TheCIA1->KeyMatrix, TheCIA1->RevMatrix);

		if (this->fake_key_str[this->fake_key_index] == '\0')
                {
                        this->fake_key_sequence = false;
                        this->fake_key_index = 0;
                        this->fake_key_keytime = 5;
                }
        }
}

void C64::startFakeKeySequence(const char *str)
{
	this->fake_key_str = str;
	this->fake_key_sequence = true;
}

/*
 *  Start main emulation thread
 */

void C64::Run(void)
{
	// Reset chips
	TheCPU->Reset();
	TheSID->Reset();
	TheCIA1->Reset();
	TheCIA2->Reset();
	TheCPU1541->Reset();

	// Patch kernal IEC routines
	orig_kernal_1d84 = Kernal[0x1d84];
	orig_kernal_1d85 = Kernal[0x1d85];
	PatchKernal(ThePrefs.FastReset, ThePrefs.Emul1541Proc);

	quit_thyself = false;
	thread_func();
}

void C64::network_vblank()
{
        static uint32_t last_time_update;
#if defined(GEKKO)
        Uint32 now = ticks_to_millisecs(gettime());
#else
        Uint32 now = SDL_GetTicks();
#endif

        if (this->network) {
        	Uint8 *master = this->TheDisplay->BitmapBase();
        	Network *remote = this->network;
		uint8 *js;
        	static bool has_throttled;

        	if (this->quit_thyself)
		{
        		remote->Disconnect();
        		delete remote;
			this->network = NULL;
			TheC64->network_connection_type = NONE;

			return;
		}

        	remote->Tick( now - last_time_update );
        	if (this->network_connection_type == MASTER) {
        		if (ThePrefs.JoystickSwap)
        			js = &TheCIA1->Joystick2;
        		else
        			js = &TheCIA1->Joystick1;
        	} else {
				/* Both are the same */
        			js = &TheCIA1->Joystick2;
        	}

		remote->ResetNetworkUpdate();
        	/* Has the peer sent any data? */
        	if (remote->ReceiveUpdate() == true)
        	{
        		if (remote->DecodeUpdate(this->TheDisplay,
        				js, this->TheSID) == false)
        		{
        			/* Disconnect or sending crap, remove this guy! */
        			Gui::gui->status_bar->queueMessage("Peer disconnected");
        			delete remote;
        			this->network = NULL;
        			if (this->network_connection_type == CLIENT)
        				this->Reset();
        			this->network_connection_type = NONE;
        			return;
        		}
                	if (this->network_connection_type == CLIENT)
                		this->TheDisplay->Update(remote->GetScreen());
        	}
		const char *msg = TheDisplay->GetTextMessage();
		if (msg && strlen(msg) > 0)
			remote->EncodeTextMessage(msg, TheDisplay->text_message_broadcast);
		free((void *)msg);

		if (this->network_connection_type == CONNECT)
        		return;

        	/* Encode and send updates to the other side (what is determined by 
        	 * if this is the master or not) */
		if (this->network_connection_type == MASTER)
		{
			/* Skip this frame if the data rate is too high */
			if (remote->ThrottleTraffic())
				has_throttled = true;
			else {
				remote->EncodeDisplay(master, remote->GetScreen());
				remote->FlushSound();
			}
        	}

		remote->EncodeJoystickUpdate(*js);

		if (remote->SendPeerUpdate() == false)
        	{
        		/* Disconnect or broken data */
        		printf("Could not send update\n");
        	}

		static uint32_t last_traffic_update;

		if (last_time_update - last_traffic_update > 300)
		{
			TheDisplay->NetworkTrafficMeter(remote->GetKbps() / (8 * 1024.0),
					has_throttled);
			last_traffic_update = now;
			has_throttled = false;
        	}
        }

        last_time_update = now;
}

/*
 *  Vertical blank: Poll keyboard and joysticks, update window
 */

void C64::VBlank(bool draw_frame)
{
	/* From Acorn port */
	static uint32_t lastFrame;
        uint32_t now;
        uint8 j1, j2;
        int joy_port_1 = 0;

        if (ThePrefs.JoystickSwap)
        	joy_port_1 = 1;

	SDL_JoystickUpdate();
	// Poll joysticks
	j1 = poll_joystick(!joy_port_1);
	j2 = poll_joystick(joy_port_1);

	// Poll keyboard
	TheDisplay->PollKeyboard(TheCIA1->KeyMatrix, TheCIA1->RevMatrix, &joykey);
	if (TheDisplay->quit_requested)
		quit_thyself = true;

	if (this->fake_key_sequence)
		this->run_fake_key_sequence();

	/* Keyboard joystick input */
	if (ThePrefs.JoystickSwap)
		j1 &= joykey;
	else
		j2 &= joykey;

	if (this->network_connection_type == CLIENT)
	{
		Uint8 which = j2;

		if (ThePrefs.JoystickSwap)
			which = j1;

		TheCIA1->Joystick1 = which;
		TheCIA1->Joystick2 = which;
	}
	else if (this->network_connection_type == MASTER)
	{
		/* The other comes from the network */
		if (ThePrefs.JoystickSwap)
			TheCIA1->Joystick1 = j1;
		else
			TheCIA1->Joystick2 = j2;
	}
	else
	{
		TheCIA1->Joystick1 = j1;
		TheCIA1->Joystick2 = j2;
	}

	// Count TOD clocks
	TheCIA1->CountTOD();
	TheCIA2->CountTOD();

	// Update window if needed
	if (draw_frame && this->network_connection_type != CLIENT) {
		TheDisplay->Update();
	}

	this->network_vblank();

	Gui::gui->runLogic();

	//if (this->quit_thyself)
	//	ThePrefs.Save(ThePrefs.PrefsPath);
#if defined(GEKKO)
	if (this->quit_thyself && Network::networking_started == true)
		SYS_ResetSystem(SYS_RETURNTOMENU, 0, 0);
        now = ticks_to_millisecs(gettime());
#else
        now = SDL_GetTicks();
#endif

        if ( (now - lastFrame) < ThePrefs.MsPerFrame) {
        	usleep( (ThePrefs.MsPerFrame - (now - lastFrame)) * 1000);
        }
        lastFrame = now;
}

/*
 * The emulation's main loop
 */

void C64::thread_func(void)
{
	int linecnt = 0;

#ifdef FRODO_SC
	while (!quit_thyself) {

		// The order of calls is important here
		if (TheVIC->EmulateCycle())
			TheSID->EmulateLine();
		/* No need to emulate anything for the client */
		if (!this->have_a_break && this->network_connection_type != CLIENT) {
			TheCIA1->CheckIRQs();
			TheCIA2->CheckIRQs();
			TheCIA1->EmulateCycle();
			TheCIA2->EmulateCycle();
			TheCPU->EmulateCycle();

			if (ThePrefs.Emul1541Proc) {
				TheCPU1541->CountVIATimers(1);
				if (!TheCPU1541->Idle)
					TheCPU1541->EmulateCycle();
			}
		}
		CycleCounter++;
#else
	while (!quit_thyself) {

		// The order of calls is important here
		int cycles = TheVIC->EmulateLine();
		TheSID->EmulateLine();
#if !PRECISE_CIA_CYCLES
		TheCIA1->EmulateLine(ThePrefs.CIACycles);
		TheCIA2->EmulateLine(ThePrefs.CIACycles);
#endif

		if (ThePrefs.Emul1541Proc) {
			int cycles_1541 = ThePrefs.FloppyCycles;
			TheCPU1541->CountVIATimers(cycles_1541);

			if (!TheCPU1541->Idle) {
				// 1541 processor active, alternately execute
				//  6502 and 6510 instructions until both have
				//  used up their cycles
				while (cycles >= 0 || cycles_1541 >= 0)
					if (cycles > cycles_1541)
						cycles -= TheCPU->EmulateLine(1);
					else
						cycles_1541 -= TheCPU1541->EmulateLine(1);
			} else
				TheCPU->EmulateLine(cycles);
		} else
			// 1541 processor disabled, only emulate 6510
			TheCPU->EmulateLine(cycles);
#endif
		linecnt++;
	}
}

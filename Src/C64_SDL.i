/*
 *  C64_x.i - Put the pieces together, X specific stuff
 *
 *  Frodo (C) 1994-1997,2002 Christian Bauer
 *  Unix stuff by Bernd Schmidt/Lutz Vieweg
 */

#include "main.h"

#include <sys/types.h>
#include <dirent.h>

#if defined(GEKKO)
#include <wiiuse/wpad.h>
#define FONT_PATH "/apps/frodo/FreeMono.ttf"
#else
#define FONT_PATH "FreeMono.ttf"
#endif
#define MS_PER_FRAME 40

static struct timeval tv_start;
static int MENU_SIZE_X, MENU_SIZE_Y;
static const char *main_menu_messages[] = {
		"Insert disc or tape", /* 0 */
		"Load disc or tape",   /* 1 */
		"Reset C64",           /* 2 */
		"Bind key to joystick",/* 3 */
		"Display options",     /* 4 */
		"Controller 1 joystick port", /* 5 */
		"^|1|2",
		"Save/Load state",     /* 7 */
		" ",
		"Quit",                /* 9 */
		NULL,
};

static const char *display_option_messages[] = {
		"1-1 resolution",      /* 0 */
		"double resolution, centered", /* 1 */
		"full-screen stretched", /* 2 */
		NULL,
};

static const char *bind_key_messages[] = {
		"Bind to A",           /* 0 */
		"Bind to B",           /* 1 */
		"Bind to Plus",        /* 2 */
		"Bind to Minus",       /* 3 */
		"Bind to 1",           /* 4 */
		NULL,
};

static const char *save_load_state_messages[] = {
		"Load saved state",    /* 0 */
		"Save current state",  /* 0 */
		"Delete state",        /* 0 */
		NULL,
};

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

#if defined(GEKKO)
	this->base_dir = "/apps/frodo/images";
#else
	this->base_dir = ".";
#endif

	this->fake_key_sequence = false;
	this->fake_key_index = 0;
	this->fake_key_keytime = 5;
	this->fake_key_type = 0;

	MENU_SIZE_X = FULL_DISPLAY_X;
	MENU_SIZE_Y = FULL_DISPLAY_Y - FULL_DISPLAY_Y / 4;

	SDL_RWops *rw;
	
	Uint8 *data = (Uint8*)malloc(1 * 1024*1024);
	FILE *fp = fopen(FONT_PATH, "r");
	if (!fp) {
		fprintf(stderr, "Could not open font\n");
		exit(1);
	}
	fread(data, 1, 1 * 1024 * 1024, fp);
	rw = SDL_RWFromMem(data, 1 * 1024 * 1024);
	if (!rw) {
		fprintf(stderr, "Could not create RW: %s\n", SDL_GetError());
		exit(1);
	}

	this->menu_font = TTF_OpenFontRW(rw, 1, 20);
	if (!this->menu_font)
	{
	        fprintf(stderr, "Unable to open font\n" );
	        exit(1);		
	}
	menu_init(&this->main_menu, this->menu_font, main_menu_messages,
			0, 0, MENU_SIZE_X, MENU_SIZE_Y);
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
	menu_fini(&this->main_menu);
}

void C64::select_disc(Prefs *np)
{
        DIR *d = opendir(this->base_dir);
	const char **file_list;
        int cur = 0;
        struct dirent *de;
        int cnt = 16;
        menu_t select_disc_menu;

        if (!d)
               	return;

        file_list = (const char**)malloc(cnt * sizeof(char*));
        file_list[cur++] = strdup("None"); 
        file_list[cur] = NULL;

        for (de = readdir(d);
             de;
             de = readdir(d))
	{
        	/* FIXME! Add directories */
                if (strstr(de->d_name, ".d64") || strstr(de->d_name, ".D64") ||
                		strstr(de->d_name, ".t64") || strstr(de->d_name, ".T64"))
                {
                        char *p;

                        p = strdup(de->d_name);
                        file_list[cur++] = p;
                        file_list[cur] = NULL;
                        if (cur > cnt - 2)
                        {
                        	cnt = cnt + 32;
                        	file_list = (const char**)realloc(file_list, cnt * sizeof(char*));
				if (!file_list)
					return;
                        }
                }
        }
        closedir(d);

	menu_init(&select_disc_menu, this->menu_font, file_list,
			0, 0, MENU_SIZE_X, MENU_SIZE_Y);
	int opt = menu_select(real_screen, &select_disc_menu, ~0, NULL);
	if (opt >= 0)
	{
		const char *name = file_list[opt];

		if (strcmp(file_list[opt], "None") == 0)
		{
			strcpy(np->DrivePath[0], "\0");
		}
		else
		{
			snprintf(np->DrivePath[0], 255, "%s/%s",
					this->base_dir, name);
			if (strstr(name, ".d64") || strstr(name, ".D64"))
				np->DriveType[0] = DRVTYPE_D64;
			else
				np->DriveType[0] = DRVTYPE_T64;
			NewPrefs(np);
			ThePrefs = *np;
		}
	}
        menu_fini(&select_disc_menu);

        /* Cleanup everything */
        for ( int i = 0; i < cur; i++ )
        	free((void*)file_list[i]);
        free(file_list);
}

#define MATRIX(a,b) (((a) << 3) | (b))

void C64::bind_key(Prefs *np)
{
        menu_t bind_key_menu;
        menu_t key_menu;
        static const char *keys[] = { "space", "Run/Stop", "return", "F1", "F3", "F5", "F7",
        		"0", "1", "2", "3", "4", "5", "6", "7", "8", "9", "A",
        		"B", "C", "D", "E", "F", "G", "H", "I", "J", "K", "L", "M",
        		"N", "O", "P", "Q", "R", "S", "T", "U", "V", "X", "Y", "Z",
        		NULL };
        int kcs[] = { MATRIX(7, 4), MATRIX(7, 7), MATRIX(0, 1), /* space, R/S, return */
        	MATRIX(0, 4), MATRIX(0, 5), MATRIX(0, 6), MATRIX(0, 3), MATRIX(4, 3), MATRIX(7, 0),
        	MATRIX(7, 3), MATRIX(1, 0), MATRIX(1, 3), MATRIX(2, 0), MATRIX(2, 3), MATRIX(3, 0),
        	MATRIX(3, 3), MATRIX(4, 0), MATRIX(1, 2), MATRIX(3, 4), MATRIX(2, 4), MATRIX(2, 2),
        	MATRIX(1, 6), MATRIX(2, 5), MATRIX(3, 2), MATRIX(3, 5), MATRIX(4, 1), MATRIX(4, 2),
        	MATRIX(4, 5), MATRIX(5, 2), MATRIX(4, 4), MATRIX(4, 7), MATRIX(4, 6), MATRIX(5, 1),
        	MATRIX(7, 6), MATRIX(2, 1), MATRIX(1, 5), MATRIX(2, 6), MATRIX(3, 6), MATRIX(3, 7),
        	MATRIX(1, 1), MATRIX(2, 7), MATRIX(3, 1), MATRIX(1, 4) };

        menu_init(&bind_key_menu, this->menu_font, bind_key_messages,
			0, 0, MENU_SIZE_X, MENU_SIZE_Y);
	int opt = menu_select(real_screen, &bind_key_menu, ~0, NULL);
	if (opt >= 0)
	{
	        menu_init(&key_menu, this->menu_font, keys,
				0, 0, MENU_SIZE_X, MENU_SIZE_Y);
		int key = menu_select(real_screen, &key_menu, ~0, NULL);

		np->JoystickKeyBinding[opt] = kcs[key];
	        menu_fini(&key_menu);
	}
        menu_fini(&bind_key_menu);
}

void C64::display_options(Prefs *np)
{
        menu_t display_menu;

        menu_init(&display_menu, this->menu_font, display_option_messages,
			0, 0, MENU_SIZE_X, MENU_SIZE_Y);
	int opt = menu_select(real_screen, &display_menu, ~0, NULL);
	if (opt >= 0)
		np->DisplayOption = opt;
        menu_fini(&display_menu);
}


void C64::save_load_state(Prefs *np)
{
        menu_t save_load_menu;
        menu_t select_saves_menu;

        menu_init(&save_load_menu, this->menu_font, save_load_state_messages,
			0, 0, MENU_SIZE_X, MENU_SIZE_Y);
	int opt = menu_select(real_screen, &save_load_menu, ~0, NULL);
	switch(opt)
	{
	case 1: /* save */
	{
		char buf[255];
		const char *name = "save";

		snprintf(buf, 255, "/apps/frodo/saves/%s.sav", name);

		this->SaveSnapshot(buf);
	} break;
	case 0: /* load/delete */
	case 2:
	{
		int save = menu_select(real_screen, &select_saves_menu, ~0, NULL);
	} break;
	default:
		break;
	}
        menu_fini(&save_load_menu);
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

/* From dreamcast port */
static const char *auto_seq[4] =
{
        "\nLOAD \"*\",8,1\nRUN\n",
	"\nLOAD \"*\",9,1\nRUN\n",
        "\nLOAD \"*\",10,1\nRUN\n",
        "\nLOAD \"*\",11,1\nRUN\n",
};
extern "C" int get_kc_from_char(char c_in, int *shifted);

/*
 *  Vertical blank: Poll keyboard and joysticks, update window
 */

void C64::VBlank(bool draw_frame)
{
	// Poll keyboard
	TheDisplay->PollKeyboard(TheCIA1->KeyMatrix, TheCIA1->RevMatrix, &joykey);
	if (TheDisplay->quit_requested)
		quit_thyself = true;

	if (this->fake_key_sequence)
	{
                int shifted;
                int kc = get_kc_from_char(auto_seq[this->fake_key_type][this->fake_key_index], &shifted);

		TheDisplay->FakeKeyPress(kc, shifted, TheCIA1->KeyMatrix,
				TheCIA1->RevMatrix);

		this->fake_key_keytime --;
                if (this->fake_key_keytime == 0)
                {
                        this->fake_key_keytime = 1;
                        this->fake_key_index ++;

			if (auto_seq[this->fake_key_type][this->fake_key_index] == '\0')
                        {
                                this->fake_key_sequence = false;
                                this->fake_key_index = 0;
                                this->fake_key_keytime = 5;
                        }
                }
	}
	// Poll joysticks
	TheCIA1->Joystick1 = poll_joystick(0);
	TheCIA1->Joystick2 = poll_joystick(1);
       
	// Count TOD clocks
	TheCIA1->CountTOD();
	TheCIA2->CountTOD();

	// Update window if needed
	if (draw_frame) {
		TheDisplay->Update();
#if 0
		// Calculate time between VBlanks, display speedometer
		struct timeval tv;
		gettimeofday(&tv, NULL);
		if ((tv.tv_usec -= tv_start.tv_usec) < 0) {
			tv.tv_usec += 1000000;
			tv.tv_sec -= 1;
		}
		tv.tv_sec -= tv_start.tv_sec;
		double elapsed_time = (double)tv.tv_sec * 1000000 + tv.tv_usec;
		speed_index = 20000 / (elapsed_time + 1) * 100;

		// Limit speed to 100% if desired
		if ((speed_index > 100)) {
			usleep((unsigned long)(20000 - elapsed_time));
			speed_index = 100;
		}

		gettimeofday(&tv_start, NULL);

		TheDisplay->Speedometer((int)speed_index);
#endif
	}
	if (this->have_a_break) {
		int submenus[1]; 
		int opt = menu_select(real_screen, &this->main_menu, ~0, submenus);

		Prefs *np = Frodo::reload_prefs();
		switch(opt)
		{
		case 0: /* Insert disc/tape */
			this->select_disc(np);
			break;
		case 1: /* Load disc/tape */
			this->fake_key_sequence = true;
			break;
		case 2: /* Reset */
			Reset();
			break;
		case 3: /* Bind keys to joystick */
			this->bind_key(np);
			break;
		case 4: /* Display options */
			this->display_options(np);
			break;
		case 5: /* Swap joysticks */
			break;
		case 7: /* Save / load game */
			this->save_load_state(np);
			break;
		case 9: /* Quit */
			quit_thyself = true;				
			break;
		case -1:
		default:
			break;
		}
		if (submenus[0] == 0)
			np->JoystickSwap = false;
		else
			np->JoystickSwap = true;

		this->NewPrefs(np);
		ThePrefs = *np;
		ThePrefs.Save(PREFS_PATH);

		this->have_a_break = false;
	}
	/* From Acorn port */
	static uint64_t lastFrame;
        uint32_t now = SDL_GetTicks();

        if ( (now - lastFrame) < MS_PER_FRAME ) {
          SDL_Delay( MS_PER_FRAME - (now - lastFrame) );
        }
        lastFrame = now;
}


/*
 *  Open/close joystick drivers given old and new state of
 *  joystick preferences
 */

void C64::open_close_joysticks(bool oldjoy1, bool oldjoy2, bool newjoy1, bool newjoy2)
{
#ifdef HAVE_LINUX_JOYSTICK_H
	if (oldjoy1 != newjoy1) {
		joy_minx = joy_miny = 32767;	// Reset calibration
		joy_maxx = joy_maxy = -32768;
		if (newjoy1) {
			joyfd[0] = open("/dev/js0", O_RDONLY);
			if (joyfd[0] < 0)
				fprintf(stderr, "Couldn't open joystick 1\n");
		} else {
			close(joyfd[0]);
			joyfd[0] = -1;
		}
	}

	if (oldjoy2 != newjoy2) {
		joy_minx = joy_miny = 32767;	// Reset calibration
		joy_maxx = joy_maxy = -32768;
		if (newjoy2) {
			joyfd[1] = open("/dev/js1", O_RDONLY);
			if (joyfd[1] < 0)
				fprintf(stderr, "Couldn't open joystick 2\n");
		} else {
			close(joyfd[1]);
			joyfd[1] = -1;
		}
	}
#endif
}


/*
 *  Poll joystick port, return CIA mask
 */

uint8 C64::poll_joystick(int port)
{
#ifdef GEKKO
	int controller = port;
	Uint32 held;
	uint8 j = 0xff;

	if (ThePrefs.JoystickSwap)
		controller = !port;

	WPAD_ScanPads();

	held =  WPAD_ButtonsHeld(controller);
	if (held & WPAD_BUTTON_UP)
		j &= 0xfb; // Left
	if (held & WPAD_BUTTON_DOWN)
		j &= 0xf7; // Right
	if (held & WPAD_BUTTON_RIGHT)
		j &= 0xfe; // Up
	if (held & WPAD_BUTTON_LEFT)
		j &= 0xfd; // Down
	if (held & WPAD_BUTTON_2)
		j &= 0xef; // Button
	if (held & WPAD_BUTTON_HOME)
		this->enter_menu();

	if ( (held & WPAD_BUTTON_A) && ThePrefs.JoystickKeyBinding[0])
		TheDisplay->FakeKeyPressRepeat(ThePrefs.JoystickKeyBinding[0],
				false, TheCIA1->KeyMatrix, TheCIA1->RevMatrix);
	if ( (held & WPAD_BUTTON_B) && ThePrefs.JoystickKeyBinding[1])
		TheDisplay->FakeKeyPressRepeat(ThePrefs.JoystickKeyBinding[1],
				false, TheCIA1->KeyMatrix, TheCIA1->RevMatrix);
	if ( (held & WPAD_BUTTON_PLUS) && ThePrefs.JoystickKeyBinding[2])
		TheDisplay->FakeKeyPressRepeat(ThePrefs.JoystickKeyBinding[2],
				false, TheCIA1->KeyMatrix, TheCIA1->RevMatrix);
	if ( (held & WPAD_BUTTON_MINUS) && ThePrefs.JoystickKeyBinding[3])
		TheDisplay->FakeKeyPressRepeat(ThePrefs.JoystickKeyBinding[3],
				false, TheCIA1->KeyMatrix, TheCIA1->RevMatrix);
	if ( (held & WPAD_BUTTON_1) && ThePrefs.JoystickKeyBinding[4])
		TheDisplay->FakeKeyPressRepeat(ThePrefs.JoystickKeyBinding[4],
				false, TheCIA1->KeyMatrix, TheCIA1->RevMatrix);


	return j;
#elif defined(HAVE_LINUX_JOYSTICK_H)
	JS_DATA_TYPE js;
	uint8 j = 0xff;

	if (joyfd[port] >= 0) {
		if (read(joyfd[port], &js, JS_RETURN) == JS_RETURN) {
			if (js.x > joy_maxx)
				joy_maxx = js.x;
			if (js.x < joy_minx)
				joy_minx = js.x;
			if (js.y > joy_maxy)
				joy_maxy = js.y;
			if (js.y < joy_miny)
				joy_miny = js.y;

			if (joy_maxx-joy_minx < 100 || joy_maxy-joy_miny < 100)
				return 0xff;

			if (js.x < (joy_minx + (joy_maxx-joy_minx)/3))
				j &= 0xfb;							// Left
			else if (js.x > (joy_minx + 2*(joy_maxx-joy_minx)/3))
				j &= 0xf7;							// Right

			if (js.y < (joy_miny + (joy_maxy-joy_miny)/3))
				j &= 0xfe;							// Up
			else if (js.y > (joy_miny + 2*(joy_maxy-joy_miny)/3))
				j &= 0xfd;							// Down

			if (js.buttons & 1)
				j &= 0xef;							// Button
		}
	}
	return j;
#else
	return 0xff;
#endif
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

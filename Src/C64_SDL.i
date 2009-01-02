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
#endif

static struct timeval tv_start;
static char *main_menu_messages[] = {
		"Insert disc or tape", /* 0 */
		"Load disc or tape",   /* 1 */
		"Reset C64",           /* 2 */
		"Bind key to joystick",/* 3 */
		"Display options",     /* 4 */
		"Swap joysticks",      /* 5 */
		" ",
		"Quit",                /* 7 */
		NULL,
};

static char *display_option_messages[] = {
		"1-1 resolution",      /* 0 */
		"double resolution, centered", /* 1 */
		"full-screen stretched", /* 2 */
		NULL,
};

static char *bind_key_messages[] = {
		"Bind to A",           /* 0 */
		"Bind to B",           /* 1 */
		"Bind to Plus",        /* 2 */
		"Bind to Minus",       /* 3 */
		"Bind to 1",           /* 4 */
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

	SDL_RWops *rw;
	
	Uint8 *data = (Uint8*)malloc(1 * 1024*1024);
	FILE *fp = fopen("/apps/frodo/FreeMono.ttf", "r");
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
			0, 0, FULL_DISPLAY_X, FULL_DISPLAY_Y);
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
	char **file_list;
        int cur = 0;
        struct dirent *de;
        int cnt = 16;
        menu_t select_disc_menu;

        if (!d)
               	return;

        file_list = (char**)malloc(cnt * sizeof(char*));
        file_list[cur++] = strdup("None"); 
        file_list[cur++] = strdup(".."); 
        file_list[cur] = NULL;

        for (de = readdir(d);
             de;
             de = readdir(d))
	{
        	/* FIXME! Add directories */
                if (strstr(de->d_name, ".d64") ||
                		strstr(de->d_name, ".t64"))
                {
                        char *p;

                        p = strdup(de->d_name);
                        file_list[cur++] = p;
                        file_list[cur] = NULL;
                        if (cur > cnt - 1)
                        {
                        	cnt = cnt + 32;
                        	file_list = (char**)realloc(file_list, cnt);
				if (!file_list)
					return;
                        }
                }
        }
        closedir(d);

	menu_init(&select_disc_menu, this->menu_font, file_list,
			0, 0, FULL_DISPLAY_X, FULL_DISPLAY_Y);
	int opt = menu_select(real_screen, &select_disc_menu, ~0, NULL);
	if (opt >= 0)
	{
		char *name = file_list[opt];

		if (strcmp(file_list[opt], "None") == 0)
		{
			strcpy(np->DrivePath[0], "\0");
		}
		else
		{
			strncpy(np->DrivePath[0], name, 255);
			if (strstr(name, ".d64"))
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
        	free(file_list[i]);
        free(file_list);
}

#define MATRIX(a,b) (((a) << 3) | (b))

void C64::bind_key(Prefs *np)
{
        menu_t bind_key_menu;
        menu_t key_menu;
        char *keys[] = { "space", "Run/Stop", "return", "F1", "F3", "F5", "F7",
        		"0", "1", "2", "3", "4", "5", "6", "7", "8", "9", "A",
        		"B", "C", "D", "E", "F", "G", "H", "I", "J", "K", "L", "M",
        		"N", "O", "P", "Q", "R", "S", "T", "U", "V", "X", "Y", "Z",
        		NULL };
        int kcs[] = { MATRIX(7, 4), MATRIX(7, 7), MATRIX(0, 1), /* space,R/S, return */
        	MATRIX(0, 4), MATRIX(0, 5), MATRIX(0, 6), MATRIX(0, 3), MATRIX(4, 3), MATRIX(7, 0),
        	MATRIX(7, 3), MATRIX(1, 0), MATRIX(1, 3), MATRIX(2, 0), MATRIX(2, 3), MATRIX(3, 0),
        	MATRIX(3, 3), MATRIX(4, 0), MATRIX(1, 2), MATRIX(3, 4), MATRIX(2, 4), MATRIX(2, 2),
        	MATRIX(1, 6), MATRIX(2, 5), MATRIX(3, 2), MATRIX(3, 5), MATRIX(4, 1), MATRIX(4, 2),
        	MATRIX(4, 5), MATRIX(5, 2), MATRIX(4, 4), MATRIX(4, 7), MATRIX(4, 6), MATRIX(5, 1),
        	MATRIX(7, 6), MATRIX(2, 1), MATRIX(1, 5), MATRIX(2, 6), MATRIX(3, 6), MATRIX(3, 7),
        	MATRIX(1, 1), MATRIX(2, 7), MATRIX(3, 1), MATRIX(1, 4) };

        menu_init(&bind_key_menu, this->menu_font, bind_key_messages,
			0, 0, FULL_DISPLAY_X, FULL_DISPLAY_Y);
        menu_init(&key_menu, this->menu_font, keys,
			0, 0, FULL_DISPLAY_X, FULL_DISPLAY_Y);
	int opt = menu_select(real_screen, &bind_key_menu, ~0, NULL);
	if (opt >= 0)
	{
		int key = menu_select(real_screen, &key_menu, ~0, NULL);

#if defined(GEKKO)
		np->JoystickKeyBinding[opt] = kcs[key];
#endif
	}
        menu_fini(&bind_key_menu);
        menu_fini(&key_menu);
}

void C64::display_options(Prefs *np)
{
        menu_t display_menu;

        menu_init(&display_menu, this->menu_font, display_option_messages,
			0, 0, FULL_DISPLAY_X, FULL_DISPLAY_Y);
	int opt = menu_select(real_screen, &display_menu, ~0, NULL);
	if (opt >= 0)
		np->DisplayOption = opt;
        menu_fini(&display_menu);
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
				TheCIA1->RevMatrix, &joykey);

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

	if (ThePrefs.JoystickSwap) {
		uint8 tmp = TheCIA1->Joystick1;
		TheCIA1->Joystick1 = TheCIA1->Joystick2;
		TheCIA1->Joystick2 = tmp;
	}

	// Joystick keyboard emulation
	if (TheDisplay->NumLock())
		TheCIA1->Joystick1 &= joykey;
	else
		TheCIA1->Joystick2 &= joykey;
       
	// Count TOD clocks
	TheCIA1->CountTOD();
	TheCIA2->CountTOD();

	// Update window if needed
	static uint64_t lastFrame;
	if (draw_frame) {
		TheDisplay->Update();
	}
        uint32_t now = SDL_GetTicks();

        if ( (now - lastFrame) < 20 ) {
          SDL_Delay( 20 - (now - lastFrame) );
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
	Uint32 held = WPAD_ButtonsHeld(port);
	uint8 j = 0xff;

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
		TheDisplay->FakeKeyPress(ThePrefs.JoystickKeyBinding[0],
				false, TheCIA1->KeyMatrix, TheCIA1->RevMatrix, NULL);
	if ( (held & WPAD_BUTTON_B) && ThePrefs.JoystickKeyBinding[1])
		TheDisplay->FakeKeyPress(ThePrefs.JoystickKeyBinding[1],
				false, TheCIA1->KeyMatrix, TheCIA1->RevMatrix, NULL);
	if ( (held & WPAD_BUTTON_PLUS) && ThePrefs.JoystickKeyBinding[2])
		TheDisplay->FakeKeyPress(ThePrefs.JoystickKeyBinding[2],
				false, TheCIA1->KeyMatrix, TheCIA1->RevMatrix, NULL);
	if ( (held & WPAD_BUTTON_MINUS) && ThePrefs.JoystickKeyBinding[3])
		TheDisplay->FakeKeyPress(ThePrefs.JoystickKeyBinding[3],
				false, TheCIA1->KeyMatrix, TheCIA1->RevMatrix, NULL);
	if ( (held & WPAD_BUTTON_1) && ThePrefs.JoystickKeyBinding[4])
		TheDisplay->FakeKeyPress(ThePrefs.JoystickKeyBinding[4],
				false, TheCIA1->KeyMatrix, TheCIA1->RevMatrix, NULL);


	return j;
#endif
#ifdef HAVE_LINUX_JOYSTICK_H
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
			{
				uint8 tmp = TheCIA1->Joystick1;
				TheCIA1->Joystick1 = TheCIA1->Joystick2;
				TheCIA1->Joystick2 = tmp;
			}	break;
			case 7: /* Quit */
				quit_thyself = true;				
				break;
			case -1:
			default:
				break;
			}

			ThePrefs = *np;

			this->have_a_break = false;
		}
	}
}

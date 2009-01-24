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
#include <ogc/lwp_watchdog.h>
#define FONT_PATH "/apps/frodo/FreeMono.ttf"
#define SAVES_PATH "/apps/frodo/saves"
#define IMAGE_PATH "/apps/frodo/images"
#define TMP_PATH "/apps/frodo/tmp"
#else
#define FONT_PATH "FreeMono.ttf"
#define SAVES_PATH "saves"
#define IMAGE_PATH "images"
#define TMP_PATH "tmp"
#endif

static struct timeval tv_start;
static const char *main_menu_messages[] = {
		"Invoke key sequence", /* 0 */
		"Insert disc or tape", /* 1 */
		"Reset C64",           /* 2 */
		"Bind key to joystick",/* 3 */
		"Other options",       /* 4 */
		"Controller 1 joystick port", /* 5 */
		"^|1|2",
		"Save/Load state",     /* 7 */
		" ",
		"Quit",                /* 9 */
		NULL,
};

static const char *other_options_messages[] = {
		"Display resolution", /* 0 */
		"^|double-center|stretched",
		"Speed (approx)",     /* 2 */
		"^|95|100|110",
		"Emulate 1541",       /* 4 */
		"^|On|Off",
		NULL,
};

static const char *save_load_state_messages[] = {
		"Load saved state",    /* 0 */
		"Save current state",  /* 1 */
		"Delete state",        /* 2 */
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

	this->fake_key_sequence = false;
	this->fake_key_index = 0;
	this->fake_key_keytime = 4;
        this->fake_key_str = "\nLOAD \"*\",8,1\nRUN\n";

	this->prefs_changed = false;
	memset(this->save_game_name, 0, sizeof(this->save_game_name));
	strcpy(this->save_game_name, "unknown");

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

	this->virtual_keyboard = new VirtualKeyboard(real_screen, this->menu_font);
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

static int cmpstringp(const void *p1, const void *p2)
{
    return strcmp(* (char * const *) p1, * (char * const *) p2);
}

/* Return true if name ends with ext (for filenames) */
static bool ext_matches(const char *name, const char *ext)
{
	int len = strlen(name);
	int ext_len = strlen(ext);

	if (len <= ext_len)
		return false;
	return (strcmp(name + len - ext_len, ext) == 0);
	
}

static const char **get_file_list(const char *base_dir)
{
	DIR *d = opendir(base_dir);
	const char **file_list;
	int cur = 0;
	struct dirent *de;
	int cnt = 16;

	if (!d)
		return NULL;

	file_list = (const char**)malloc(cnt * sizeof(char*));
	file_list[cur++] = strdup("None"); 
	file_list[cur] = NULL;

	for (de = readdir(d);
	de;
	de = readdir(d))
	{
		if (ext_matches(de->d_name, ".d64") || ext_matches(de->d_name, ".D64") ||
				ext_matches(de->d_name, ".prg") || ext_matches(de->d_name, ".PRG") ||
				ext_matches(de->d_name, ".p00") || ext_matches(de->d_name, ".P00") ||
				ext_matches(de->d_name, ".s00") || ext_matches(de->d_name, ".S00") ||
				ext_matches(de->d_name, ".t64") || ext_matches(de->d_name, ".T64") ||
				ext_matches(de->d_name, ".sav"))
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
					return NULL;
			}
		}
	}
	closedir(d);
        qsort(&file_list[1], cur-1, sizeof(const char *), cmpstringp);

        return file_list;
}

void C64::select_disc(Prefs *np)
{
	const char **file_list = get_file_list(IMAGE_PATH);

	if (file_list == NULL)
		return;

	int opt = menu_select(real_screen, this->menu_font,
			file_list, NULL);
	if (opt >= 0)
	{
		const char *name = file_list[opt];

		if (strcmp(file_list[opt], "None") == 0)
		{
			strcpy(np->DrivePath[0], "\0");
			strcpy(this->save_game_name, "unknown");
		}
		else
		{
			snprintf(np->DrivePath[0], 255, "%s/%s",
					IMAGE_PATH, name);
			strncpy(this->save_game_name, name, 255);
			if (strstr(name, ".prg") || strstr(name, ".PRG") ||
				 strstr(name, ".p00") || strstr(name, ".P00") ||
				 strstr(name, ".s00") || strstr(name, ".S00")) {
				FILE *src, *dst;

				/* Clean temp dir first (we only want one file) */
				unlink(TMP_PATH"/a");

				src = fopen(np->DrivePath[0], "r");
				if (src != NULL)
				{
					snprintf(np->DrivePath[0], 255, "%s", TMP_PATH);

					/* Special handling of .prg: Copy to TMP_PATH and
					 * load that as a dir */
					dst = fopen(TMP_PATH"/a", "w");
					if (dst)
					{
						Uint8 buf[1024];
						size_t v;

						do {
							v = fread(buf, 1, 1024, src);
							fwrite(buf, 1, v, dst);
						} while (v > 0);
						fclose(dst);
					}
					fclose(src);
				}
			}

			NewPrefs(np);
			ThePrefs = *np;
		}
		this->prefs_changed = true;
	}

        /* Cleanup everything */
        for ( int i = 0; file_list[i]; i++ )
        	free((void*)file_list[i]);
        free(file_list);
}


char *C64::bind_one_key(Prefs *np, int which)
{
	static const char *which_to_button_name[] = { "A", "B", "+", "-", "1",
			"classic X", "classic Y", "classic B", "classic L",
			"classic R", "classic ZR", "classic ZL" };
	static char strs[N_WIIMOTE_BINDINGS][255];
	char *out = strs[which];
	int cur = np->JoystickKeyBinding[which];

	snprintf(out, 255, "Bind to %s (now %s)", which_to_button_name[which],
			this->virtual_keyboard->keycode_to_string(cur));

	return out;
}

void C64::bind_keys(Prefs *np)
{
	const char *bind_key_messages[N_WIIMOTE_BINDINGS + 1];
	bool has_classic_controller = false;

#if defined(GEKKO)
        WPADData *wpad, *wpad_other;

        wpad = WPAD_Data(0);
        wpad_other = WPAD_Data(1);

        if (wpad->exp.type == WPAD_EXP_CLASSIC ||
        		wpad_other->exp.type == WPAD_EXP_CLASSIC)
        	has_classic_controller = true;
#endif

        memset(bind_key_messages, 0, sizeof(const char*) * (N_WIIMOTE_BINDINGS + 1));

        for (int i = 0; i < (has_classic_controller ? N_WIIMOTE_BINDINGS : 5); i++)
        	bind_key_messages[i] = this->bind_one_key(np, i);

	int opt = menu_select(real_screen, this->menu_font,
			bind_key_messages, NULL);
	if (opt >= 0)
	{
	        int key;
	        bool shifted;

		key = this->virtual_keyboard->get_key();
		/* -2 means abort */
		if (key != np->JoystickKeyBinding[opt] && key != -2)
		{
			this->prefs_changed = true;
			np->JoystickKeyBinding[opt] = key;
		}
	}
}

void C64::other_options(Prefs *np)
{
        int submenus[3] = { np->DisplayOption, 0, !np->Emul1541Proc };

#define SPEED_95 33
#define SPEED_100 28
#define SPEED_110 25

        switch (np->MsPerFrame)
        {
        case SPEED_95:
        	submenus[1] = 0; break;
        case SPEED_110:
        	submenus[1] = 2; break;
        default:
        	/* If it has some other value... */
        	submenus[1] = 1; break;
        }
	int opt = menu_select(real_screen, this->menu_font,
			other_options_messages, submenus);
	if (opt >= 0)
	{
		np->DisplayOption = submenus[0];
		np->Emul1541Proc = submenus[2] == 0 ? true : false;

		switch(submenus[1])
		{
		case 0:
			np->MsPerFrame = SPEED_95; break;
		case 1:
			np->MsPerFrame = SPEED_100; break;
		case 2:
		default:
			np->MsPerFrame = SPEED_110; break;
		}
		this->prefs_changed = true;
	}
}

/* From dreamcast port but heavily modified */
void C64::run_fake_key_sequence()
{
	int kc = this->virtual_keyboard->char_to_keycode(this->fake_key_str[this->fake_key_index]);

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

void C64::start_fake_key_sequence(const char *str)
{
	this->fake_key_str = str;
	this->fake_key_sequence = true;
}

void C64::select_fake_key_sequence(Prefs *np)
{
	static const char *fake_key_sequences[] = {
			"\nLOAD \"*\",8,1\nRUN\n",
			"\nLOAD \"$\",8\n",
			"\nLIST\n",
			NULL};
	const char *fake_key_messages[] = {
			"LOAD \"*\",8,1  and  RUN",
			"LOAD \"$\",8",
			"LIST",
			"Type with virtual keyboard",
			NULL};
	int opt;

	opt = menu_select(real_screen, this->menu_font,
			fake_key_messages, NULL);
	if (opt < 0)
		return;

	if (opt == 3)
	{
		const char *seq = this->virtual_keyboard->get_string();

		if (seq != NULL)
			this->start_fake_key_sequence(seq);
	}
	else
		this->start_fake_key_sequence(fake_key_sequences[opt]);
}

void C64::save_load_state(Prefs *np)
{
	int opt = menu_select(real_screen, this->menu_font,
			save_load_state_messages, NULL);
	switch(opt)
	{
	case 1: /* save */
	{
		char save_buf[255];
		char prefs_buf[255];

		snprintf(save_buf, 255, "%s/%s.sav", SAVES_PATH,
				this->save_game_name);
		snprintf(prefs_buf, 255, "%s.prefs", save_buf);

		this->SaveSnapshot(save_buf);
		np->Save(prefs_buf);
	} break;
	case 0: /* load/delete */
	case 2:
	{
		const char **file_list = get_file_list(SAVES_PATH);

		if (file_list == NULL)
			break;
		int save = menu_select(real_screen, this->menu_font,
				file_list, NULL);
		if (save >= 0)
		{
			char save_buf[255];
			char prefs_buf[255];

			snprintf(save_buf, 255, "%s/%s", SAVES_PATH,  file_list[save]);
			snprintf(prefs_buf, 255, "%s.prefs", save_buf);
			if (opt == 2)
			{
				unlink(save_buf);
				unlink(prefs_buf);
			}
			else /* Load the snapshot */
			{
				this->LoadSnapshot(save_buf);
				np->Load(prefs_buf);
				this->prefs_changed = true;
			}
		}

		/* Cleanup everything */
		for ( int i = 0; file_list[i]; i++ )
			free((void*)file_list[i]);
		free(file_list);
	} break;
	default:
		break;
	}
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

/*
 *  Vertical blank: Poll keyboard and joysticks, update window
 */

void C64::VBlank(bool draw_frame)
{
#if defined(GEKKO)
	WPAD_ScanPads();
#endif

	// Poll joysticks
	TheCIA1->Joystick1 = poll_joystick(0);
	TheCIA1->Joystick2 = poll_joystick(1);

	// Poll keyboard
	TheDisplay->PollKeyboard(TheCIA1->KeyMatrix, TheCIA1->RevMatrix, &joykey);
	if (TheDisplay->quit_requested)
		quit_thyself = true;

	if (this->fake_key_sequence)
		this->run_fake_key_sequence();
#ifndef GEKKO
	// Joystick keyboard emulation
	if (TheDisplay->NumLock())
		TheCIA1->Joystick1 &= joykey;
	else
		TheCIA1->Joystick2 &= joykey;
#endif

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
		int opt;
		int old_swap = ThePrefs.JoystickSwap == true ? 1 : 0; 

		Prefs np = ThePrefs;
		this->prefs_changed = false;

		TheSID->PauseSound();
		submenus[0] = old_swap;
		opt = menu_select(real_screen, this->menu_font,
				main_menu_messages, submenus);

		switch(opt)
		{
		case 0: /* Load disc/tape */
			this->select_fake_key_sequence(&np);
			break;
		case 1: /* Insert disc/tape */
			this->select_disc(&np);
			break;
		case 2: /* Reset */
			Reset();
			break;
		case 3: /* Bind keys to joystick */
			this->bind_keys(&np);
			break;
		case 4: /* Other options */
			this->other_options(&np);
			break;
		case 5: /* Swap joysticks */
			break;
		case 7: /* Save / load game */
			this->save_load_state(&np);
			break;
		case 9: /* Quit */
			quit_thyself = true;				
			break;
		case -1:
		default:
			break;
		}
		if (submenus[0] == 0)
			np.JoystickSwap = false;
		else
			np.JoystickSwap = true;
		if (submenus[0] != old_swap)
			this->prefs_changed = true;

		if (this->prefs_changed)
		{
			this->NewPrefs(&np);
			ThePrefs = np;
		}
		TheDisplay->FakeKeyPress(-1, TheCIA1->KeyMatrix,
				TheCIA1->RevMatrix);

		this->have_a_break = false;
		if (this->quit_thyself)
			ThePrefs.Save(PREFS_PATH);
	}
	/* From Acorn port */
	static uint64_t lastFrame;
#if defined(GEKKO)
        uint32_t now = ticks_to_millisecs(gettime());
#else
        uint32_t now = SDL_GetTicks();
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

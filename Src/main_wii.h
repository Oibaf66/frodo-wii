/*
 *  main_wii.i - Main program, Wii specific stuff
 *
 *  Frodo (C) 1994-1997,2002 Christian Bauer
 */

#include "Version.h"

#include <SDL.h>
#include <fat.h>
#include <wiiuse/wpad.h>

#include "menu.h"

extern int init_graphics(void);

/*
 *  Create application object and start it
 */

char *fixme_tmp_network_client = 0;
int fixme_tmp_network_server = 0;

extern "C" int main(int argc, char **argv)
{
	Frodo *the_app;

	timeval tv;
	gettimeofday(&tv, NULL);
	srand(tv.tv_usec);

	printf("%s by Christian Bauer\n", VERSION_STRING);
	if (!init_graphics())
	{
		fprintf(stderr, "Could not initialize graphics\n");
		return 0;
	}
	fflush(stdout);
	fatInitDefault();

	// Init SDL
	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		fprintf(stderr, "Couldn't initialize SDL (%s)\n", SDL_GetError());
		return 0;
	}
	if (TTF_Init() < 0)
	{
	        fprintf(stderr, "Unable to init TTF: %s\n", TTF_GetError() );
	        return 0;		
	}
	menu_init();

	if (WPAD_Init() != WPAD_ERR_NONE)
	{
		fprintf(stderr, "Failed initializing controllers\n");
		return 0;
	}

	the_app = new Frodo();
	the_app->ArgvReceived(argc, argv);
	the_app->ReadyToRun();
	delete the_app;

//	SYS_ResetSystem(SYS_RETURNTOMENU, 0, 0);

	return 0;
}


/*
 *  Constructor: Initialize member variables
 */

Frodo::Frodo()
{
	TheC64 = NULL;
}


/*
 *  Process command line arguments
 */

void Frodo::ArgvReceived(int argc, char **argv)
{
}


/*
 *  Arguments processed, run emulation
 */

void Frodo::ReadyToRun(void)
{
	getcwd(AppDirPath, 256);

	ThePrefs.Load((char*)PREFS_PATH);

	// Create and start C64
	TheC64 = new C64;
	load_rom_files();
	TheC64->Run();
	delete TheC64;
}


Prefs *Frodo::reload_prefs(void)
{
	static Prefs newprefs;
	newprefs.Load((char*)PREFS_PATH);
	return &newprefs;
}

/*
 *  Determine whether path name refers to a directory
 */

bool IsDirectory(const char *path)
{
	struct stat st;
	return stat(path, &st) == 0 && S_ISDIR(st.st_mode);
}

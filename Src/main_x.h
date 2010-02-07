/*
 *  main_x.h - Main program, Unix specific stuff
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

#include "Version.h"

#include <SDL.h>
#include <SDL_ttf.h>
#include "gui/gui.hh"

extern int init_graphics(void);


// Global variables
char Frodo::prefs_path[256] = "";


/*
 *  Create application object and start it
 */

int main(int argc, char **argv)
{
	timeval tv;
	gettimeofday(&tv, NULL);
	srand(tv.tv_usec);

	// Init SDL
        if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER) < 0) {
                fprintf(stderr, "Couldn't initialize SDL (%s)\n", SDL_GetError());
                return 1;
	}
        if (TTF_Init() < 0)
        {
                fprintf(stderr, "Unable to init TTF: %s\n", TTF_GetError() );
		return 1;
        }

	if (!init_graphics())
		return 1;
	fflush(stdout);

	Frodo *the_app = new Frodo();
	the_app->ArgvReceived(argc, argv);
	the_app->ReadyToRun();
	delete the_app;

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
char *fixme_tmp_network_server = 0;

void Frodo::ArgvReceived(int argc, char **argv)
{
	if (argc == 2)
		fixme_tmp_network_server = argv[1];
}


/*
 *  Arguments processed, run emulation
 */

void Frodo::ReadyToRun(void)
{
	getcwd(AppDirPath, 256);

	// Load preferences
	if (!prefs_path[0]) {
		char *home = getenv("HOME");
		if (home != NULL && strlen(home) < 240) {
			strncpy(prefs_path, home, 200);
			strcat(prefs_path, "/");
		}
		strcat(prefs_path, ".frodorc");
	}
	ThePrefs.Load(prefs_path);

	// Create and start C64
	TheC64 = new C64;
	Gui::init();
	load_rom_files();
	TheC64->Run();
	delete TheC64;
}

/*
 *  Determine whether path name refers to a directory
 */

bool IsDirectory(const char *path)
{
	struct stat st;
	return stat(path, &st) == 0 && S_ISDIR(st.st_mode);
}

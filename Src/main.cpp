/*
 *  main.cpp - Main program
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
#include <SDL.h>
#include <SDL_ttf.h>

#include "sysdeps.h"

#include "main.h"
#include "C64.h"
#include "Display.h"
#include "Prefs.h"
#include "SAM.h"
#include "gui/gui.hh"
#include "data_store.hh"
#include "utils.hh"

#if defined(GEKKO)
#include <gccore.h>
#include <fat.h>
#include <ogc/usbstorage.h>
#endif


// Global variables
extern int init_graphics(void);
extern void CloseShare (bool silent);


// Global variables
C64 *TheC64 = NULL;		// Global C64 object
char AppDirPath[1024];	// Path of application directory
bool usbismount = false;
bool smbismount = false; 
bool sdismount = false; 

#ifndef GEKKO
bool networkisinit = true;
#else
bool networkisinit = false;
#endif

// ROM file names
#ifndef DATADIR
#define DATADIR
#endif

#ifdef __riscos__
#define BASIC_ROM_FILE	"FrodoRsrc:Basic_ROM"
#define KERNAL_ROM_FILE	"FrodoRsrc:Kernal_ROM"
#define CHAR_ROM_FILE	"FrodoRsrc:Char_ROM"
#define DRIVE_ROM_FILE	"FrodoRsrc:1541_ROM"
#elif GEKKO
#define WII_FRODO_BASE_PATH "/apps/frodo/"
#define BASIC_ROM_FILE	WII_FRODO_BASE_PATH"Basic_ROM"
#define KERNAL_ROM_FILE	WII_FRODO_BASE_PATH"Kernal_ROM"
#define CHAR_ROM_FILE	WII_FRODO_BASE_PATH"Char_ROM"
#define DRIVE_ROM_FILE	WII_FRODO_BASE_PATH"1541_ROM"
#else
#define BASIC_ROM_FILE DATADIR "Basic ROM"
#define KERNAL_ROM_FILE DATADIR "Kernal ROM"
#define CHAR_ROM_FILE DATADIR "Char ROM"
#define DRIVE_ROM_FILE DATADIR "1541 ROM"
#endif


// Builtin ROMs
#include "Basic_ROM.h"
#include "Kernal_ROM.h"
#include "Char_ROM.h"
#include "1541_ROM.h"



 
 #if defined(GEKKO)
 
//init and deinit USB device functions


bool InitUSB()
{ 
	printf("Initializing USB FAT subsytem ...\n\n");
	fatUnmount("usb:");
	
	// This should wake up the drive
	bool isMounted = fatMountSimple("usb", &__io_usbstorage);
	
	bool isInserted = __io_usbstorage.isInserted();
	if (!isInserted) 
	{
	printf("USB device not found\n\n");
	return false;
	}
 
	// USB Drive may be "sleeeeping" 
	// We need to try Mounting a few times to wake it up
	int retry = 10;
	while (retry && !isMounted)
	{
		sleep(1);
		isMounted = fatMountSimple("usb", &__io_usbstorage);
		retry--; 
	}
	return isMounted;
 }
 
 void DeInitUSB()
{
	fatUnmount("usb:");
	__io_usbstorage.shutdown(); 
}

bool InitNetwork()
{
        char myIP[16];

        memset(myIP, 0, sizeof(myIP));
	printf("Getting IP address via DHCP...\n\n");

	if (if_config(myIP, NULL, NULL, true) < 0) {
	        	printf("No DHCP reply\n\n");
	        	return false;
        }
	printf("Got an address: %s\n",myIP);
	return true;
}

#endif

/*
 *  Load C64 ROM files
 */

void Frodo::load_rom(const char *which, const char *path, uint8 *where, size_t size, const uint8 *builtin)
{
	FILE *f = fopen(path, "rb");
	if (f) {
		size_t actual = fread(where, 1, size, f);
		fclose(f);
		if (actual == size)
			return;
	}

	// Use builtin ROM
	printf("%s ROM file (%s) not readable, using builtin.\n", which, path);
	memcpy(where, builtin, size);
}

void Frodo::load_rom_files()
{
	load_rom("Basic", BASIC_ROM_FILE, TheC64->Basic, BASIC_ROM_SIZE, builtin_basic_rom);
	load_rom("Kernal", KERNAL_ROM_FILE, TheC64->Kernal, KERNAL_ROM_SIZE, builtin_kernal_rom);
	load_rom("Char", CHAR_ROM_FILE, TheC64->Char, CHAR_ROM_SIZE, builtin_char_rom);
	load_rom("1541", DRIVE_ROM_FILE, TheC64->ROM1541, DRIVE_ROM_SIZE, builtin_drive_rom);
}

/*
 *  Create application object and start it
 */

extern "C" int main(int argc, char **argv)
{
	timeval tv;
	gettimeofday(&tv, NULL);
	srand(tv.tv_usec);
#if defined(GEKKO)
	DIR *dir_tmp;
#endif

	// Init SDL
        if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER | SDL_INIT_JOYSTICK) < 0) {
                fprintf(stderr, "Couldn't initialize SDL (%s)\n", SDL_GetError());
                return 1;
	}
        if (TTF_Init() < 0)
        {
                fprintf(stderr, "Unable to init TTF: %s\n", TTF_GetError() );
		return 1;
        }

	fflush(stdout);

	#ifdef GEKKO
	
	printf("\x1b[2;0H");

	//initialize libfat library
	if (fatInitDefault())
		printf("FAT subsytem initialized\n\n");
	else
		{
		printf("Couldn't initialize FAT subsytem\n\n");
		sleep(3);
		exit(0);
		}
		
	DIR *dp;
    
	dp = opendir ("sd:/");
	if (dp) sdismount = 1; else sdismount = 0;
	
	if (sdismount)
		printf("SD FAT subsytem initialized\n\n");
	else
		printf("Couldn't initialize SD fat subsytem\n\n");
 	
	if (sdismount) closedir (dp);
	
	usbismount = InitUSB();
	if (usbismount) 
		printf("USB FAT subsytem initialized\n\n");
	else
		printf("Impossible to initialize USB FAT subsytem\n\n");
	
	networkisinit = InitNetwork();	
		
	sleep(2);
	
	//create tmp directory if it does not exist
	dir_tmp = opendir("/frodo/tmp");	
	if (!dir_tmp) {mkdir("/frodo/tmp",0777);printf("Making tmp directory\n");sleep(2);} else closedir(dir_tmp);
	
	//Cancel the old a file
	unlink ("/frodo/tmp/a");
	
	
	#endif

	Frodo *the_app = new Frodo();
	the_app->ArgvReceived(argc, argv);
	the_app->ReadyToRun();
	delete the_app;
	
	if (smbismount) CloseShare (true);

	#ifdef GEKKO
	DeInitUSB();
	fatUnmount(0);
	#endif

	return 0;
}


/*
 *  Constructor: Initialize member variables
 */

Frodo::Frodo()
{
	Prefs p;

	TheC64 = NULL;
	ThePrefs = p; /* Silly workaround to fix joystick bug */
	TheDefaultPrefs = p;
}


/*
 *  Process command line arguments
 */
char *network_server_connect = 0;
char *floppy8 = 0;

void Frodo::ArgvReceived(int argc, char **argv)
{
	if (argc == 2) floppy8 = argv[1];
	else if (argc == 3) network_server_connect = argv[2];	 
}

const char *try_path(const char *path, const char *file)
{
	if (path == NULL || file == NULL)
		return NULL;

	char *what = (char *)xmalloc(strlen(path) + strlen(file) + 3);
	struct stat st;
	const char *out = NULL;

	sprintf(what, "%s/%s", path, file);
	if (stat(what, &st) == 0)
		out = what;

	return out;
}

void Frodo::LoadFrodorc()
{
	const char *paths[] = {
			"/frodo", // Wii
			".",
			"/apps/frodo", // Wii
			"frodo",
			NULL, // Filled in below
			NULL, // also filled in below
			"/usr/share/frodo",
			NULL,
	};
	const char *prefs_path = NULL;
	const char *prefs_name = NULL;
	const char *total_name = NULL;
	char home_1[255];
	char home_2[255];
	int i;

	if (getenv("HOME"))
	{
		snprintf(home_1, sizeof(home_1), "%s/.frodo", getenv("HOME"));
		snprintf(home_2, sizeof(home_2), "%s/frodo", getenv("HOME"));
	}
	else
	{
		strcpy(home_1, "");
		strcpy(home_2, "");
	}
	paths[4] = home_1;
	paths[5] = home_2;

	for (i = 0; paths[i]; i++)
	{
		const char *p;
		const char *name = "frodorc";

		p = try_path(paths[i], name);

		if (p)
		{
			prefs_path = paths[i];
			prefs_name = name;
			total_name = p;
			break;
		}
		free((void*)p);
	}

	// Load preferences
	if (total_name)
	{
		ThePrefs.Load(total_name);
		strncpy(ThePrefs.PrefsPath, total_name, sizeof(ThePrefs.PrefsPath));
	} else
		printf("No frodorc, using default\n");

	free((void*)total_name);
}

/*
 *  Arguments processed, run emulation
 */


extern const char *prg_exts[];

void Frodo::ReadyToRun(void)
{
	if (getcwd(AppDirPath, 256) == NULL)
		strcpy(AppDirPath, "");

	this->LoadFrodorc();
	if (network_server_connect)
		strncpy(ThePrefs.NetworkServer, network_server_connect,
				sizeof(ThePrefs.NetworkServer));
		
//Mount the floppy if passed as argument
if (floppy8)  
	{
	strncpy(ThePrefs.DrivePath[0], floppy8, sizeof(ThePrefs.DrivePath[0]));
	
	char *filename;
	filename = strrchr(floppy8, '/');
	if (!filename) filename++;
	
	if (ext_matches_list(filename, prg_exts)) {
                	char *tmp_filename;
                        FILE *src, *dst;

                        tmp_filename = (char *)xmalloc(strlen(TMP_ROOT_PATH) + 4);
                        sprintf(tmp_filename, "%s/ab", TMP_ROOT_PATH);

                        /* Clean temp dir first (we only want one file) */
                        unlink(tmp_filename);

                        src = fopen(ThePrefs.DrivePath[0], "r");
                        if (src != NULL)
                        {
                                snprintf(ThePrefs.DrivePath[0], sizeof(ThePrefs.DrivePath[0]),
                                		"%s", TMP_ROOT_PATH);

                                /* Special handling of .prg: Copy to TMP_PATH and
                                 * load that as a dir */
                                dst = fopen(tmp_filename, "w");
                                if (dst)
                                {
                                        Uint8 buf[1024];
                                        size_t v;

                                        do {
                                                v = fread(buf, 1, sizeof(buf), src);
                                                fwrite(buf, 1, v, dst);
                                        } while (v > 0);
                                        fclose(dst);
                                }
                                fclose(src);
                        }
                        free(tmp_filename);
			}
	}
			
	panic_if (!init_graphics(),
			"Can't initialize graphics!\n");

	// Create and start C64
	TheC64 = new C64;
	DataStore::ds = new DataStore();
	TimerController::init();
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

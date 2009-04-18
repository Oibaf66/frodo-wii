#include <stdlib.h>

#include "menutexts.h"

const char *welcome[] = {
		/*01*/		"#1             WELCOME TO FRODO ON WII               ",
		/*02*/		" ------------------------------------------------- ",
		/*03*/		" In the system, hit HOME on the Wiimote to get to  ", 
		/*04*/		" the config-page. Load or autostart a D64, T64, or ", 
		/*05*/		" PRG image. Use the virtual keyboard or assign the ", 
		/*06*/		" key strokes to buttons on the Wiimote.            ",
		/*07*/		" Save the game state in the main menu of the game. ", 
		/*08*/		" Next time you can load that state instead of the  ",
		/*09*/		" game to have all configured stuff along with that ",
		/*10*/		" game.                                             ",
		/*11*/		"                                                   ",
		/*12*/		" This version features USB-keyboard support and    ", 
		/*13*/		" D64 content listing. Use first Wiimote as port 1  ", 
		/*14*/		" joystick and the second as port 2 joystick.       ",
		/*15*/		"                                                   ", 
		/*16*/		".-------------------------------------------------.",
		/*17*/		"^|       Enter Frodo       |    Don't show again    ", 
		NULL
};

const char *new_main_menu_messages[] = {
		/*00*/		"#1.MAIN MENU                       ",
		/*01*/		"#1-------------------------------------", 
		/*02*/		".Image", 
		/*03*/		"^|Load|Start|Remove", 
		/*04*/		".States",     
		/*05*/		"^|Load|Save|Delete|Rename ",     
		/*06*/		".Keyboard", 
		/*07*/		"^|Type|Macro|Bind",
		/*08*/		"#1-------------------------------------",
		/*09*/		".Reset the C=64",           
		/*10*/  	".Options",
		/*11*/		".Advanced Options", 
		/*12*/		".Controls",                
		/*13*/		"#1-------------------------------------",
		/*14*/		".Quit",                
		/*15*/		"#1-------------------------------------",
		/*16*/		"#1'2'=SELECT, '1'=CANCEL",                
		NULL
};
const char *new_options_menu_messages[] = {
		/*00*/		"#1.OPTIONS MENU                    ",
		/*01*/		"#1-------------------------------------", 
		/*02*/  	".Map Wiimote 1 to:",
		/*03*/		"^|Port 1|Port 2",
		/*04*/		" ", 
		/*05*/		".True 1541 emulation",       
		/*06*/		"^|NO|YES",
		/*07*/		" ", 
		/*08*/	  ".1541 Floppy Drive LED", /* 0 */
		/*09*/	  "^|OFF|ON",
		/*10*/		"#1-------------------------------------",
		/*11*/		"#1'2'=SELECT, '1'=CANCEL",                
		NULL
};
const char *new_advanced_options_menu_messages[] = {
		/*00*/		"#1.ADVANCED OPTIONS MENU           ",
		/*01*/		"#1-------------------------------------", 
		/*02*/		".Display resolution", /* 0 */
		/*03*/		"^|double-center|stretched",
		/*04*/		" ", 
		/*05*/		".Speed (approx.)",     /* 2 */
		/*06*/		"^|95|100|110",
		/*07*/		" ", 
		/*08*/		".Sprite collisions", /* 0 */
		/*09*/		"^|OFF|ON",
		/*10*/		" ", 
		/*11*/		".Autostart", /* 0 */
		/*12*/		"^|Save|Clear", /* 0 */
		/*13*/		"#1-------------------------------------",
		/*14*/		"#1'2'=SELECT, '1'=CANCEL",                
		NULL
};
const char *new_help_menu_messages[] = {
		/*00*/		"#1.CONTROLS MENU                        ",
		/*01*/		"#1-------------------------------------", 
		/*02*/		".Wiimote key mappings", /* 0 */
		/*03*/		"^|Wiimote Info|Set Default", /* 0 */
		/*04*/		" ", 
		/*05*/		".Show USB-keyboard layout", /* 0 */
		/*06*/		"#1-------------------------------------",
		/*07*/		"#1'2'=SELECT, '1'=CANCEL",                
		NULL
};
const char *layout_messages[] = {
		"#1.USB-Keyboard Layout             ",
		"#1-------------------------------------", 
		"#2ESC = Run/Stop",
		"#2F9  = QuickSave",
		"#2F10 = QuickLoad",
		"#2F11 = Restore",
		"#2TAB = CTRL",
		"#2INS = Clr/Home",
		"#2PGU = ¤",
		"#2AGR = C=",
		"#2HOM = Menu",
		"#1-------------------------------------", 
		"Back",
		NULL,
};

const char *main_menu_messages[] = {
		"Invoke key sequence", /* 0 */
		"Insert disc or tape", /* 1 */
		"Bind key to joystick",/* 2 */
		"Other options",       /* 3 */
		"Networking",          /* 4 */
		"Controller 1 joystick port", /* 5 */
		"^|1|2",
		"Save/Load state",     /* 7 */
		" ",
		"Quit",                /* 9 */
		NULL,
};

const char *save_load_state_messages[] = {
		"Load saved state",    /* 0 */
		"Save current state",  /* 1 */
		"Delete state",        /* 2 */
		NULL,
};

const char *other_options_messages[] = {
		"Display resolution", /* 0 */
		"^|double-center|stretched",
		"Speed (approx)",     /* 2 */
		"^|95|100|110",
		"Emulate 1541",       /* 4 */
		"^|On|Off",
		"Reset C64",         /* 6 */
		NULL,
};

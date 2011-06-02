#include <stdlib.h>

#include "menu_messages.hh"

const char *exit_dialogue_messages[8] = {
	/*00*/          "Do you really want to exit",
	/*01*/		"Frodo?",
	/*02*/		"#", /* Empty line */
	/*03*/		"#",
	/*04*/		"#",
	/*05*/		"#",
	/*06*/          "^|Yes|Cancel",
	NULL
};

const char *save_prefs_done[8] = {
	/*00*/          "Preferences saved!",
	/*01*/		"#",
	/*02*/		"#", /* Empty line */
	/*03*/		"#",
	/*04*/		"#",
	/*05*/		"#",
	/*06*/          "^|OK",
	NULL
};

const char *save_state_done[8] = {
	/*00*/          "Game state saved!",
	/*01*/		"#",
	/*02*/		"#", /* Empty line */
	/*03*/		"#",
	/*04*/		"#",
	/*05*/		"#",
	/*06*/          "^|OK",
	NULL
};

const char *network_port_dialogue_messages[8] = {
	/*00*/          "Please supply a number as",
	/*01*/		"network port.",
	/*02*/		"#", /* Empty line */
	/*03*/		"#",
	/*04*/		"#",
	/*05*/		"#",
	/*06*/          "^|OK",
	NULL
};

const char *network_unset_name_dlg[8] = {
	/*00*/          "Please setup a name to use",
	/*01*/		"on network connections.",
	/*02*/		"#", /* Empty line */
	/*03*/		"#",
	/*04*/		"#",
	/*05*/		"#",
	/*06*/          "^|OK",
	NULL
};

const char *network_need_connection[8] = {
	/*00*/          "You need to be connected",
	/*01*/		"to the C64 network to",
	/*02*/		"use this feature",
	/*03*/		"#",
	/*04*/		"#",
	/*05*/		"#",
	/*06*/          "^|OK",
	NULL
};

const char *network_need_peer[8] = {
	/*00*/          "You need to be connected",
	/*01*/		"to a peer on the network",
	/*02*/		"to use this feature",
	/*03*/		"#",
	/*04*/		"#",
	/*05*/		"#",
	/*06*/          "^|OK",
	NULL
};

const char *broken_theme_dlg[8] = {
	/*00*/          "The selected theme cannot be",
	/*01*/		"loaded, probably some file",
	/*02*/		"is missing or broken in it.",
	/*03*/		"The default theme has been",
	/*04*/		"reverted.",
	/*05*/		"#",
	/*06*/          "^|OK",
	NULL
};

const char *game_info_bad_year_dlg[8] = {
	/*00*/          "Impossible year selected,",
	/*01*/		"try starting with 1981!",
	/*02*/		"#", /* Empty line */
	/*03*/		"#",
	/*04*/		"#",
	/*05*/		"#",
	/*06*/          "^|OK",
	NULL
};

const char *game_info_bad_number_dlg[8] = {
	/*00*/          "Please enter a number!",
	/*01*/		"#",
	/*02*/		"#", /* Empty line */
	/*03*/		"#",
	/*04*/		"#",
	/*05*/		"#",
	/*06*/          "^|OK",
	NULL
};

const char *select_analogue_dlg[8] = {
	/*00*/          "Select axis of analogue",
	/*01*/		"joystick to bind.",
	/*02*/		"#",
	/*03*/		"#",
	/*04*/		"#",
	/*05*/		"#",
	/*06*/          "^|None|Horiz|Vert",
	NULL
};

const char *frodo_help[11] = {
	/*00*/          "Welcome to the C64 network!",
	/*01*/		"#",
	/*02*/		"Key bindings:",
	/*03*/		"Home - Enter menu",
	/*04*/		"F10 - Type network message",
	/*05*/		"F12 - Reset the C64",
	/*06*/		"Ctrl - Fire",
	/*07*/		"Arrows - Joystick",
	/*08*/		"#",
	/*09*/          "^|OK",
	NULL
};


const char *main_menu_messages[14] = {
	/*00*/          "File",
	/*01*/          "^|Start|Insert",
	/*02*/          "States",
	/*03*/          "^|Load|Save|Delete",
	/*04*/          "Keyboard",
	/*05*/          "^|Type|Bind|Toggle crsr",
	/*06*/          " ",
	/*07*/          "Game info",
	/*08*/          "Networking",
	/*09*/          "Options",
	/*10*/		"Save prefs",
	/*11*/          "Quit",
	NULL
};

const char **main_menu_help[14] = {
		(const char*[]){
				"Insert a disc/tape or",
				"start it",
				NULL,
		},
		NULL,
		(const char*[]){
				"Load/save or delete game",
				"states",
				NULL,
		},
		NULL,
		(const char*[]){
				"Bind keyboard keys to the",
				"joysticks, toggle cursor",
				"key settings (joystick or",
				"cursors) or type with the",
				"virtual keyboard",
				NULL,
		},
		NULL,
		NULL,
		(const char*[]){
				"View and configure game",
				"information (author,",
				"screenshots etc)",
				NULL,
		},
		(const char*[]){
				"Network setup for playing",
				"C64 games against other",
				"players online.",
				NULL,
		},
		(const char*[]){
				"Configure Frodo",
				NULL,
		},
		(const char*[]){
				"Save general preferences",
				NULL,
		},	
		(const char*[]){
				"Quit Frodo",
				NULL,
		},
		NULL,
};


const char *options_menu_messages[14] = {
                /*00*/          "Help",
                /*01*/          " ",
                /*02*/          "Map Controller 1 to:",
                /*03*/          "^|Port 1|Port 2",
                /*04*/          "True 1541 emulation",
                /*05*/          "^|ON|OFF",
                /*06*/          "Display",
                /*07*/          "^|window|fullscreen",
                /*08*/          "Speed (approx. %)",
                /*09*/          "^|95|100|110",
        	/*10*/          "Reset the C=64",
                /*11*/          " ",
                /*12*/          "Setup GUI theme",
                NULL
};

const char *bind_key_menu_messages[13] = {
                /*00*/          "Wiimote",
                /*01*/          "^|A|B|1|2|+|-",
                /*02*/          "Nunchuk",
                /*03*/          "^|Z|C",
                /*04*/          "Classic",
                /*05*/          "^|a|b|x|y|L|R|Zl|Zr|+|-",
                /*06*/          " ",
                /*07*/          "Reset to defaults",
                NULL
};

const char **options_menu_help[14] = {
		(const char*[]){
				"Help and keyboard.",
				"shortcuts",
				NULL,
		},
		NULL,
		(const char*[]){
				"Switch controller to",
				"C64 joystick port",
				"mapping.",
				NULL,
		},
		NULL,
		(const char*[]){
				"Turn on or off true 1541",
				"floppy emulation. Might",
				"be needed in some games",
				"but slows down emulation.",
				NULL,
		},
		NULL,
		(const char*[]){
				"Select display settings.",
				"Fullscreen runs in",
				"double size mode, while",
				"window in streched mode.",
				NULL,
		},
		NULL,
		(const char*[]){
				"Setup speed factor (in %).",
				"Should normally be 100",
				"unless Simon did some bad",
				"mistake.",
				NULL,
		},
		NULL,
		(const char*[]){
				"Reset the c64.",
				NULL,
		},
		NULL,
		(const char*[]){
				"Setup theme for the Frodo",
				"menus.",
				NULL,
		},
		NULL,
};

const char **network_menu_help[9] = {
		(const char*[]){
				"Setup username to use on",
				"the C64 network. Must be",
				"set before connceting.",
				NULL,
		},
		(const char*[]){
				"Setup server hostname.",
				"Only for debugging, so",
				"leave as it is.",
				NULL,
		},
		(const char*[]){
				"Setup the region of the",
				"world where you live.",
				NULL,
		},
		NULL,
		(const char*[]){
				"Connect to the C64",
				"network, or disconnect if",
				"you are already connected.",
				NULL,
		},
		NULL,
		(const char*[]){
				"Post message to everyone",
				"connected to the C64",
				"network server. You must",
				"be connected to use this.",
				NULL,
		},
		(const char*[]){
				"Post message to the peer",
				"you are playing with. You",
				"must be connected to use",
				"this feature.",
				" ",
				"You can also press F10 on",
				"the keyboard to activate",
				"this.",
				NULL,
		},
		NULL,
};



const char *game_info_menu_messages[11] = {
	/*00*/          "Capture game screenshot",
	/*01*/          " ",
	/*02*/          "Set game name",
	/*03*/          "Set publisher",
	/*04*/          "Set creator/programmer",
	/*05*/          "Set musician",
	/*06*/          "Set graphics artist",
	/*07*/          "Set publishing year",
	/*08*/          "Set genre",
	/*09*/          "Set number of players",
	NULL
};



const char *genre_dlg[8] = {
	/*00*/          "Action",
	/*01*/		"Adventure",
	/*02*/		"Simulation",
	/*03*/		"Puzzle",
	/*04*/		"Platform",
	/*05*/		"Strategy",
	/*06*/          "Role playing",
	NULL
};

const char *players_dlg[5] = {
	/*00*/          "#  1",
	/*01*/		"#  2",
	/*02*/		"#  3",
	/*03*/		"#  4",
	NULL
};

const char *needs_help[9] = {
	/*00*/          "c64-network.org needs",
	/*01*/		"graphics and web design",
	/*02*/		"help! See",
	/*02*/		"#",
	/*03*/		"http://www.c64-network.org",
	/*04*/		"for more information.",
	/*05*/		"#",
	/*06*/          "^|I'll help!",
	NULL
};

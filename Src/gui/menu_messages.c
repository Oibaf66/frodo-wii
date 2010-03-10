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
	/*00*/          NULL, /* Setup dynamically */
	/*01*/          " ",
	/*02*/          "File",
	/*03*/          "^|Start|Insert",
	/*04*/          "States",
	/*05*/          "^|Load|Save|Delete",
	/*06*/          "Keyboard",
	/*07*/          "^|Type|Bind",
	/*08*/          " ",
	/*09*/          "Game info",
	/*10*/          "Networking",
	/*11*/          "Options",
	/*12*/          "Quit",
	NULL
};

const char **main_menu_help[14] = {
		(const char*[]){
				"Pause or resume the C64",
				"emulation. Not available",
				"when running in networked",
				"mode.",
				NULL,
		},
		NULL,
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
				"joysticks, or type with",
				"the virtual keyboard",
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
                /*01*/          "^|Up|Down|Left|Right|A|B|1|2|+|-",
                /*02*/          "Nunchuk",
                /*03*/          "^|Horiz|Vert|Z|C",
                /*04*/          "Classic",
                /*05*/          "^|Up|Down|Left|a|b|x|y|Zl|Zr|+|-",
                /*06*/          "Classic (left analogue)",
                /*07*/          "^|Horiz|Vert",
                /*08*/          "Classic(right analogue)",
                /*09*/          "^|Horiz|Vert",
                /*10*/          " ",
                /*11*/          "Reset to defaults",
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
				"Display 1541 drive LED to",
				"show if the emulation got",
				"stuck or is running.",
				NULL,
		},
		NULL,
		(const char*[]){
				"Select display settings.",
				"Fullscreen attemts to run",
				"in fullscreen mode, while.",
				"windowed uses a window.",
				"Activated on next restart.",
				" ",
				"On the Wii, fullscreen is",
				"always used",
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


const char *game_info_menu_messages[6] = {
	/*00*/          "Capture game screenshot",
	/*01*/          " ",
	/*02*/          "Set game name",
	/*03*/          "Set game author",
	/*04*/          "Set publishing year",
	NULL
};


const char *needs_help[9] = {
	/*00*/          "c64-network.org needs",
	/*01*/		"grapichs and web design",
	/*02*/		"help! See",
	/*02*/		"#",
	/*03*/		"http://www.c64-network.org",
	/*04*/		"for more information.",
	/*05*/		"#",
	/*06*/          "^|I'll help!",
	NULL
};

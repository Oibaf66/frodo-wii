#include <stdlib.h>

#include "menu_messages.hh"

const char **exit_dialogue_messages = (const char*[]){
	/*00*/          "Do you really want to exit",
	/*01*/		"Frodo?",
	/*02*/		"#", /* Empty line */
	/*03*/		"#",
	/*04*/		"#",
	/*05*/		"#",
	/*06*/          "^|Yes|Cancel",
	NULL
};

const char **main_menu_messages = (const char*[]){
	/*00*/          "File",
	/*01*/          "^|Insert|Start",
	/*02*/          "States",
	/*03*/          "^|Load|Save|Delete",
	/*04*/          "Keyboard",
	/*05*/          "^|Type|Macro|Bind",
	/*06*/          " ",
	/*07*/          "Reset the C=64",
	/*08*/          "Networking",
	/*09*/          "Options",
	/*10*/          "Help",
	/*11*/          "Quit",
	NULL
};

const char **main_menu_help[] = {
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
				"joysticks, use pre-defined",
				"macros, or type with the",
				"virtual keyboard",
				NULL,
		},
		NULL,
		NULL,
		NULL,
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
		NULL,
		NULL,
};


const char **options_menu_messages = (const char*[]){
                /*00*/          "Map Wiimote 1 to:",
                /*01*/          "^|Port 1|Port 2",
                /*03*/          "True 1541 emulation",
                /*04*/          "^|ON|OFF",
                /*06*/          "1541 Floppy Drive LED",
                /*07*/          "^|ON|OFF",
                /*09*/          "Display resolution",
                /*10*/          "^|double-center|stretched",
                /*12*/          "Speed (approx. %)",
                /*13*/          "^|95|100|110",
                NULL
};

const char **options_menu_help[] = {
		(const char*[]){
				"Switch controller to",
				"C64 joystick port mapping",
				NULL,
		},
		NULL,
		(const char*[]){
				"Turn on or off true 1541",
				"floppy emulation. Might",
				"be needed in some games",
				"but slows down emulation",
				NULL,
		},
		NULL,
		(const char*[]){
				"Display 1541 drive LED to",
				"show if the emulation got",
				"stuck or is running",
				NULL,
		},
		NULL,
		(const char*[]){
				"Select display resolution",
				"mapping. Double-center",
				"removes some pixels on the",
				"borders, stretched fills",
				"the display but is slower",
				NULL,
		},
		NULL,
		(const char*[]){
				"Setup speed factor",
				NULL,
		},
		NULL,
};

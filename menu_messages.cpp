#include <stdlib.h>

#include "menu_messages.hh"

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

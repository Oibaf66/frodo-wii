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

const char **network_port_dialogue_messages = (const char*[]){
	/*00*/          "Please supply a number as",
	/*01*/		"network port.",
	/*02*/		"#", /* Empty line */
	/*03*/		"#",
	/*04*/		"#",
	/*05*/		"#",
	/*06*/          "^|OK",
	NULL
};

const char **network_unset_name_dlg = (const char*[]){
	/*00*/          "Please setup a name to use",
	/*01*/		"on network connections.",
	/*02*/		"#", /* Empty line */
	/*03*/		"#",
	/*04*/		"#",
	/*05*/		"#",
	/*06*/          "^|OK",
	NULL
};

const char **broken_theme_dlg = (const char*[]){
	/*00*/          "The selected theme cannot be",
	/*01*/		"loaded, probably some file",
	/*02*/		"is missing or broken in it.",
	/*03*/		"The default theme has been",
	/*04*/		"reverted.",
	/*05*/		"#",
	/*06*/          "^|OK",
	NULL
};

const char **select_analogue_dlg = (const char*[]){
	/*00*/          "Select axis of analogue",
	/*01*/		"joystick to bind.",
	/*02*/		"#",
	/*03*/		"#",
	/*04*/		"#",
	/*05*/		"#",
	/*06*/          "^|None|Horiz|Vert",
	NULL
};



const char **main_menu_messages = (const char*[]){
	/*00*/          NULL, /* Setup dynamically */
	/*01*/          " ",
	/*02*/          "File",
	/*03*/          "^|Insert|Start",
	/*04*/          "States",
	/*05*/          "^|Load|Save|Delete",
	/*06*/          "Keyboard",
	/*07*/          "^|Type|Macro|Bind",
	/*08*/          " ",
	/*09*/          "Reset the C=64",
	/*10*/          "Networking",
	/*11*/          "Options",
	/*12*/          "Quit",
	NULL
};

const char **main_menu_help[] = {
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
		(const char*[]){
				"Quit Frodo",
				NULL,
		},
		NULL,
};


const char **options_menu_messages = (const char*[]){
                /*00*/          "Map Controller 1 to:",
                /*01*/          "^|Port 1|Port 2",
                /*02*/          "True 1541 emulation",
                /*03*/          "^|ON|OFF",
                /*04*/          "1541 Floppy Drive LED",
                /*05*/          "^|ON|OFF",
                /*06*/          "Display resolution",
                /*07*/          "^|double-center|stretched",
                /*08*/          "Speed (approx. %)",
                /*09*/          "^|95|100|110",
                /*10*/          " ",
                /*11*/          "Setup GUI theme",
                NULL
};

const char **bind_key_menu_messages = (const char*[]){
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
                NULL
};

const char **options_menu_help[] = {
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
				"Select display resolution",
				"mapping. Double-center",
				"removes some pixels on the",
				"borders, stretched fills",
				"the display but is slower.",
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
		NULL,
		(const char*[]){
				"Setup theme for the Frodo",
				"menus.",
				NULL,
		},
		NULL,
};

const char **network_menu_help[] = {
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
				"UDP port to use. Only for",
				"debugging, so leave as",
				"it is",
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
				"Post message to the C64",
				"network server. You must",
				"be connected to use this.",
				NULL,
		},
		NULL,
};


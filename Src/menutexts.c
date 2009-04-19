#include <stdlib.h>

#include "menutexts.h"

const char *welcome[] = {
		/*03*/		"In the system, hit HOME on the Wiimote to get to  ", 
		/*04*/		"the config-page. Load or autostart a D64, T64,    ", 
		/*05*/		"or PRG image. Use the virtual keyboard or assign  ", 
		/*06*/		"key strokes to buttons on the Wiimote.            ",
		/*07*/		"You can save the game state in the main menu.     ", 
		/*08*/		"The next time you can load that state instead     ",
		/*09*/		"and restore the game and everything configured.   ",
		/*10*/		"                                                  ",
		/*11*/		"In the networking menu, you can host a game over  ",
		/*12*/		"the network or browse for active games. During    ", 
		/*13*/		"network play, you can send messages to your peer  ", 
		/*14*/		"by pressing ScrLk on the USB keyboard.            ",
		/*10*/		"                                                  ",
		/*10*/		"In each menu, the '2' button selects and entry    ",
		/*10*/		"and '1' cancels.                                  ",
		NULL
};

const char *new_main_menu_messages[] = {
		/*02*/		"File", 
		/*03*/		"^|Insert|Start", 
		/*04*/		"States",     
		/*05*/		"^|Load|Save|Delete",     
		/*06*/		"Keyboard", 
		/*07*/		"^|Type|Macro|Bind",
		/*08*/		"#1-------------------------------------",
		/*09*/		"Reset the C=64",           
		/*10*/  	"Networking",
		/*11*/  	"Options",
		/*12*/		"Advanced Options", 
		/*13*/		"Help",                
		/*15*/		"Quit",                
		NULL
};
const char *new_options_menu_messages[] = {
		/*02*/  	"Map Wiimote 1 to:",
		/*03*/		"^|Port 1|Port 2",
		/*04*/		" ", 
		/*05*/		"True 1541 emulation",       
		/*06*/		"^|NO|YES",
		/*07*/		" ", 
		/*08*/		"1541 Floppy Drive LED", /* 0 */
		/*09*/	  	"^|OFF|ON",
		/*10*/		"#1-------------------------------------",
		/*11*/		"#1'2'=SELECT, '1'=CANCEL",                
		NULL
};
const char *new_advanced_options_menu_messages[] = {
		/*02*/		"Display resolution",
		/*03*/		"^|double-center|stretched",
		/*04*/		" ", 
		/*05*/		"Speed (approx.)",
		/*06*/		"^|95|100|110",
		/*07*/		" ",
		/*08*/		"Sprite collisions",
		/*09*/		"^|OFF|ON",
		/*13*/		"#1-------------------------------------",
		/*14*/		"#1'2'=SELECT, '1'=CANCEL",                
		NULL
};
const char *new_help_menu_messages[] = {
		/*00*/		"#1.HELP MENU                           ",
		/*01*/		"#1-------------------------------------", 
		/*02*/		"Wiimote key mappings", /* 0 */
		/*03*/		"^|Wiimote Info|Set Default", /* 0 */
		/*04*/		" ", 
		/*05*/		"Show USB-keyboard layout", /* 0 */
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

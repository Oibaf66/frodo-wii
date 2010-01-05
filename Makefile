OBJS=menu.oo main.oo utils.oo gui.oo dialogue_box.oo menu_messages.oo \
	timer.oo game_info.oo widget.oo virtual_keyboard.oo listener.oo

all: menu

%.oo: %.cpp
	g++ -Imocks -Wall -g -c `sdl-config --cflags` -o $@ $<

menu.oo: menu.cpp menu.hh utils.hh font.hh widget.hh Makefile

widget.oo: widget.cpp widget.hh

gui.oo: gui.cpp gui.hh Makefile font.hh menu.hh sdl_ttf_font.hh \
	dialogue_box.hh help_box.hh main_menu.cpp disc_menu.cpp \
	file_browser.hh timer.hh game_info.hh widget.hh options_menu.cpp \
	network_menu.cpp theme_menu.cpp mocks/Prefs.h mocks/C64.h

virtual_keyboard.oo: virtual_keyboard.hh virtual_keyboard.cpp widget.hh listener.hh

game_info.oo: game_info.hh game_info.cpp

utils.oo: utils.cpp utils.hh Makefile

timer.oo: timer.cpp timer.hh utils.hh Makefile

dialogue_box.oo: dialogue_box.cpp dialogue_box.hh menu.hh listener.hh

listener.oo: listener.cpp listener.hh

main.oo: menu.hh utils.hh sdl_ttf_font.hh Makefile

menu: $(OBJS)
	g++ `sdl-config --libs` -lSDL -lSDL_image -lSDL_ttf -o $@ $+

clean:
	rm -f *.oo menu *~

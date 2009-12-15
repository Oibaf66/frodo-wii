OBJS=menu.oo main.oo utils.oo gui.oo dialogue_box.oo menu_messages.oo

all: menu

%.oo: %.cpp
	g++ -Wall -g -c `sdl-config --cflags` -o $@ $<

menu.oo: menu.cpp menu.hh utils.hh font.hh Makefile

gui.oo: Makefile gui.cpp gui.hh font.hh menu.hh sdl_ttf_font.hh \
	dialogue_box.hh help_box.hh main_menu.cpp disc_menu.cpp \
	file_browser.hh

utils.oo: utils.cpp utils.hh Makefile

dialogue_box.oo: dialogue_box.cpp dialogue_box.hh menu.hh Makefile

main.oo: menu.hh utils.hh sdl_ttf_font.hh Makefile

menu: $(OBJS)
	g++ `sdl-config --libs` -lSDL -lSDL_image -lSDL_ttf -o $@ $+

clean:
	rm -f *.oo menu *~

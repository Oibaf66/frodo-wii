OBJS=menu.oo main.oo utils.oo frodo_menu.oo dialogue_box.oo menu_messages.oo

all: menu

%.oo: %.cpp
	g++ -Wall -g -c `sdl-config --cflags` -o $@ $<

menu.oo: menu.cpp menu.hh utils.hh font.hh Makefile

frodo_menu.oo: frodo_menu.cpp frodo_menu.hh font.hh menu.hh Makefile

utils.oo: utils.cpp utils.hh Makefile

dialogue_box.oo: dialogue_box.cpp dialogue_box.hh menu.hh Makefile

main.oo: menu.hh utils.hh sdl_ttf_font.hh Makefile

menu: $(OBJS)
	g++ `sdl-config --libs` -lSDL -lSDL_image -lSDL_ttf -o $@ $+

clean:
	rm -f *.oo menu *~

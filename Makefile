OBJS=menu.oo main.oo utils.oo

all: menu

%.oo: %.cpp
	g++ -Wall -g -c `sdl-config --cflags` -o $@ $<

menu.oo: menu.cpp menu.hh utils.hh font.hh Makefile

utils.oo: utils.cpp utils.hh Makefile

main.oo: menu.hh utils.hh sdl_ttf_font.hh Makefile

menu: $(OBJS)
	g++ `sdl-config --libs` -lSDL -lSDL_image -lSDL_ttf -o $@ $+

clean:
	rm -f *.oo menu *~

OBJS=menu.oo main.oo

all: menu

%.oo: %.cpp
	g++ -c `sdl-config --cflags` -o $@ $<


menu: $(OBJS)
	g++ `sdl-config --libs` -lSDL -lSDL_image -lSDL_ttf -o $@ $+

clean:
	rm -f *.oo

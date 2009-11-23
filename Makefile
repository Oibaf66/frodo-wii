OBJS=menu.oo

%.oo: %.cpp
	g++ `sdl-config --cflags` -o $@ $<


menu: $(OBJS)
	g++ `sdl-config --libs` -lsdl -lsdl_image -lsdl_ttf -o $@ $+

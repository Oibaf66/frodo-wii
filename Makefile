OBJS=menu.o

menu: $(OBJS)
	g++ -lsdl -lsdl_image -lsdl_ttf -o $@ $+

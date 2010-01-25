CXX = g++
CC  = gcc
LD  = g++
CPP = cpp

ERROR_FILTER := 2>&1 | sed -e 's/\(.[a-zA-Z]\+\):\([0-9]\+\):/\1(\2):/g'

CFLAGS ?=-ggdb -Iinclude -Wall  `sdl-config --cflags` -Imocks
DEFINES =
LDFLAGS ?= $(GCOV) `sdl-config --libs` -lSDL_ttf -lSDL_image


CPP_SRCS=menu.cpp main.cpp utils.cpp gui.cpp dialogue_box.cpp menu_messages.cpp \
	timer.cpp game_info.cpp widget.cpp virtual_keyboard.cpp listener.cpp \
	status_bar.cpp

OBJS=$(patsubst %.cpp,objs/%.o,$(CPP_SRCS)) $(patsubst %.c,objs/%.o,$(C_SRCS))
DEPS=$(patsubst %.cpp,deps/%.d,$(CPP_SRCS)) $(patsubst %.c,deps/%.d,$(C_SRCS))

TARGET=menu


all: $(DEPS) $(TARGET)


deps/%.d: %.cpp
	@echo makedep $(notdir $<)
	@install -d deps/$(dir $<)
	@$(CPP) -M -MT objs/$(patsubst %.cpp,%.o,$<) $(DEFINES) $(CFLAGS) -o $@ $<

objs/%.o: %.cpp
	@echo CXX $(notdir $<)
	@install -d objs/$(dir $<)
	@$(CXX) $(CFLAGS) $(DEFINES) -c -o $@ $< $(ERROR_FILTER)

clean:
	rm -rf $(TARGET) *~ objs deps


$(TARGET): $(OBJS)
	@echo LD $@
	@$(LD) $(LDFLAGS) -o $@ $+

-include $(DEPS)

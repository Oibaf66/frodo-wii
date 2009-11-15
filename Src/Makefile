# Makefile.in for Frodo (generic Unix/X11)

## Version information
VERSION  = 4
REVISION = 3

## System specific configuration

SHELL = /bin/sh

prefix = /usr/local
exec_prefix = ${prefix}
bindir = ${exec_prefix}/bin
datadir = ${prefix}/share

DESTDIR =

CXX = g++
CC = gcc
CFLAGS =-DFRODO_SC -O2 -g -I/usr/include/SDL -I$(DEVKITPRO)/libogc/include -D_GNU_SOURCE=1 -D_REENTRANT -DHAVE_SDL -DORBIT2=1 -pthread -I/usr/include/libglade-2.0 -I/usr/include/gtk-2.0 -I/usr/include/libxml2 -I/usr/lib/gtk-2.0/include -I/usr/include/atk-1.0 -I/usr/include/cairo -I/usr/include/pango-1.0 -I/usr/include/glib-2.0 -I/usr/lib/glib-2.0/include -I/usr/include/freetype2 -I/usr/include/libpng12 -I/usr/include/pixman-1 -I/usr/include/libgnomeui-2.0 -I/usr/include/libart-2.0 -I/usr/include/gconf/2 -I/usr/include/gnome-keyring-1 -I/usr/include/libgnome-2.0 -I/usr/include/libbonoboui-2.0 -I/usr/include/libgnomecanvas-2.0 -I/usr/include/gnome-vfs-2.0 -I/usr/lib/gnome-vfs-2.0/include -I/usr/include/orbit-2.0 -I/usr/include/libbonobo-2.0 -I/usr/include/bonobo-activation-2.0 -I/usr/include/gail-1.0 -Igui/guichan/include  -I./ -DKBD_LANG=0
DEFS = -DHAVE_CONFIG_H -DDATADIR=\"$(datadir)/frodo/\" -DBINDIR=\"$(bindir)/\" -DHAVE_SDL
LDFLAGS =
LIBS =  -L/usr/lib -lSDL -lSDL_ttf -pthread -lSDL_image
INSTALL = /usr/bin/install -c
INSTALL_PROGRAM = ${INSTALL}
INSTALL_DATA = ${INSTALL} -m 644

## Files
GUIOBJS = gui/guichan/actionevent.o gui/guichan/basiccontainer.o gui/guichan/cliprectangle.o gui/guichan/color.o \
	gui/guichan/defaultfont.o gui/guichan/event.o gui/guichan/exception.o gui/guichan/focushandler.o gui/guichan/font.o \
	gui/guichan/genericinput.o gui/guichan/graphics.o gui/guichan/guichan.o gui/guichan/gui.o gui/guichan/image.o \
	gui/guichan/imagefont.o gui/guichan/inputevent.o gui/guichan/key.o gui/guichan/keyevent.o gui/guichan/keyinput.o \
	gui/guichan/mouseevent.o gui/guichan/mouseinput.o gui/guichan/rectangle.o gui/guichan/sdl/sdl.o \
	gui/guichan/sdl/sdlgraphics.o gui/guichan/sdl/sdlimage.o gui/guichan/sdl/sdlimageloader.o gui/guichan/sdl/sdlinput.o\
	gui/guichan/selectionevent.o gui/guichan/widget.o gui/guichan/widgets/button.o gui/guichan/widgets/checkbox.o \
	gui/guichan/widgets/container.o gui/guichan/widgets/dropdown.o gui/guichan/widgets/icon.o \
	gui/guichan/widgets/imagebutton.o gui/guichan/widgets/label.o gui/guichan/widgets/listbox.o \
	gui/guichan/widgets/radiobutton.o gui/guichan/widgets/scrollarea.o gui/guichan/widgets/slider.o \
	gui/guichan/widgets/tabbedarea.o gui/guichan/widgets/tab.o gui/guichan/widgets/textbox.o \
	gui/guichan/widgets/textfield.o gui/guichan/widgets/window.o gui/sdlwidgets.o

OBJS    = $(GUIOBJS) main.o Prefs.o SID.o REU.o IEC.o 1541fs.o \
          1541d64.o 1541t64.o 1541job.o SAM.o CmdPipe.o menu.o Network.o \
          VirtualKeyboard.o menutexts.o d64-read.o
SLOBJS  = $(OBJS) C64.o CPUC64.o VIC.o CIA.o CPU1541.o Display.o 
SLFLAGS = -DPRECISE_CPU_CYCLES=1 -DPRECISE_CIA_CYCLES=1 -DPC_IS_POINTER=0
SCOBJS  = $(OBJS) C64_SC.o CPUC64_SC.o VIC_SC.o CIA_SC.o CPU1541_SC.o CPU_common.o Display_SC.o
SCFLAGS = -DFRODO_SC
GUISRCS = 
SRCS    = $(GUISRCS) main.cpp Display.cpp Prefs.cpp SID.cpp REU.cpp IEC.cpp 1541fs.cpp \
          1541d64.cpp 1541t64.cpp 1541job.cpp SAM.cpp CmdPipe.cpp C64.cpp \
          C64_SC.cpp CPUC64.cpp CPUC64_SC.cpp VIC.cpp VIC_SC.cpp CIA.cpp \
          CIA_SC.cpp CPU1541.cpp CPU1541_PC.cpp CPU1541_SC.cpp CPU_common.cpp \
          menu.cpp

## Rules
.PHONY: install installdirs uninstall clean distclean depend dep
.SUFFIXES:
.SUFFIXES: .o .cpp .h

all: sysconfig.h FrodoSC

Frodo: $(SLOBJS)
	$(CXX) -o Frodo $(LDFLAGS) $(SLOBJS) $(LIBS)

FrodoSC: $(SCOBJS)
	$(CXX) -o FrodoSC $(LDFLAGS) $(SCOBJS) $(LIBS)

install: Frodo FrodoSC installdirs
	$(INSTALL_PROGRAM) Frodo $(DESTDIR)$(bindir)/Frodo
	$(INSTALL_PROGRAM) FrodoSC $(DESTDIR)$(bindir)/FrodoSC
	$(INSTALL_PROGRAM) Frodo_GUI.tcl $(DESTDIR)$(bindir)/Frodo_GUI.tcl
	$(INSTALL_DATA) ../Kernal\ ROM $(DESTDIR)$(datadir)/frodo/Kernal\ ROM
	$(INSTALL_DATA) glade/Frodo.glade $(DESTDIR)$(datadir)/frodo/Frodo.glade
	$(INSTALL_DATA) maemo/frodo.desktop $(DESTDIR)$(datadir)/applications/hildon/frodo.desktop
	$(INSTALL_DATA) maemo/frodo.service $(DESTDIR)$(datadir)/dbus-1/services/frodo.service
	$(INSTALL_DATA) maemo/frodo.service $(DESTDIR)$(datadir)/dbus-1/services/frodo.service
	$(INSTALL_DATA) maemo/Frodo_26_26.png $(DESTDIR)$(datadir)/icons/hicolor/26x26/hildon/frodo.png
	$(INSTALL_DATA) maemo/Frodo_40_40.png $(DESTDIR)$(datadir)/icons/hicolor/40x40/hildon/frodo.png
	$(INSTALL_DATA) maemo/Frodo_64_64.png $(DESTDIR)$(datadir)/icons/hicolor/scalable/hildon/frodo.png

installdirs:
	$(SHELL) mkinstalldirs $(DESTDIR)$(bindir) $(DESTDIR)$(datadir)/frodo $(DESTDIR)$(datadir)/applications/hildon $(DESTDIR)$(datadir)/dbus-1/services $(DESTDIR)$(datadir)/icons/hicolor/26x26/hildon  $(DESTDIR)$(datadir)/icons/hicolor/40x40/hildon $(DESTDIR)$(datadir)/icons/hicolor/scalable/hildon

uninstall:
	rm -f $(DESTDIR)$(bindir)/Frodo
	rm -f $(DESTDIR)$(bindir)/FrodoSC
	rm -f $(DESTDIR)$(bindir)/Frodo_GUI.tcl
	rm -f $(DESTDIR)$(datadir)/frodo/Kernal\ ROM
	rmdir $(DESTDIR)$(datadir)/frodo

clean:
	rm -f $(SLOBJS) $(SCOBJS)
	rm -f Frodo FrodoSC
	rm -f core* *.core *~ *.bak

distclean: clean
	rm -rf autom4te.cache
	rm -f Makefile sysconfig.h
	rm -f config.cache config.log config.status config.h

depend dep:
	makedepend $(CPPFLAGS) -Y. $(SRCS) 2>/dev/null

.cpp.o:
	$(CXX) $(DEFS) $(CFLAGS) -o $@ -c $*.cpp

%.o: %.c
	$(CC) $(DEFS) $(CFLAGS) -o $@ -c $*.c

C64.o: C64.cpp
	$(CXX) $(DEFS) $(CFLAGS) $(SLFLAGS) -o $@ -c $*.cpp

C64_SC.o: C64_SC.cpp
	$(CXX) $(DEFS) $(CFLAGS) $(SCFLAGS) -o $@ -c $*.cpp

CPUC64.o: CPUC64.cpp
	$(CXX) $(DEFS) $(CFLAGS) $(SLFLAGS) -o $@ -c $*.cpp

CPUC64_SC.o: CPUC64_SC.cpp
	$(CXX) $(DEFS) $(CFLAGS) $(SCFLAGS) -o $@ -c $*.cpp

Display_SC.o: Display.cpp
	$(CXX) $(DEFS) $(CFLAGS) $(SCFLAGS) -o $@ -c $<

CPU1541.o: CPU1541.cpp
	$(CXX) $(DEFS) $(CFLAGS) $(SLFLAGS) -o $@ -c $*.cpp

CPU1541_SC.o: CPU1541_SC.cpp
	$(CXX) $(DEFS) $(CFLAGS) $(SCFLAGS) -o $@ -c $*.cpp

VIC_SC.o: VIC_SC.cpp
	$(CXX) $(DEFS) $(CFLAGS) $(SCFLAGS) -o $@ -c $*.cpp

CIA_SC.o: CIA_SC.cpp
	$(CXX) $(DEFS) $(CFLAGS) $(SCFLAGS) -o $@ -c $*.cpp

sysconfig.h: sysconfig.h.Host-SDL
	cp $< $@

#-------------------------------------------------------------------------
# DO NOT DELETE THIS LINE -- make depend depends on it.

main.o: sysdeps.h sysconfig.h main.h C64.h Display.h Prefs.h SAM.h
main.o: Basic_ROM.h Kernal_ROM.h Char_ROM.h 1541_ROM.h
Display.o: sysdeps.h sysconfig.h Display.h main.h Prefs.h Display_SDL.h
Display_SC.o: sysdeps.h sysconfig.h Display.h main.h Prefs.h Display_SDL.h
Prefs.o: sysdeps.h sysconfig.h Prefs.h Display.h C64.h main.h
SID.o: sysdeps.h sysconfig.h SID.h Prefs.h SID_linux.h
REU.o: sysdeps.h sysconfig.h REU.h CPUC64.h C64.h Prefs.h
IEC.o: sysdeps.h sysconfig.h IEC.h 1541fs.h 1541d64.h 1541t64.h Prefs.h
IEC.o: Display.h main.h
1541fs.o: sysdeps.h sysconfig.h 1541fs.h IEC.h main.h Prefs.h
1541d64.o: sysdeps.h sysconfig.h 1541d64.h IEC.h Prefs.h C64.h main.h debug.h
1541t64.o: sysdeps.h sysconfig.h 1541t64.h IEC.h Prefs.h debug.h
1541job.o: sysdeps.h sysconfig.h 1541job.h CPU1541.h CIA.h Prefs.h C64.h
SAM.o: sysdeps.h sysconfig.h SAM.h C64.h CPUC64.h CPU1541.h CIA.h Prefs.h
SAM.o: VIC.h SID.h
CmdPipe.o: CmdPipe.h
C64.o: sysdeps.h sysconfig.h C64.h CPUC64.h CPU1541.h CIA.h Prefs.h VIC.h C64_SDL.h
C64.o: SID.h REU.h IEC.h 1541job.h Display.h
C64_SC.o: C64.cpp sysdeps.h sysconfig.h C64.h CPUC64.h CPU1541.h CIA.h C64_SDL.h
C64_SC.o: Prefs.h VIC.h SID.h REU.h IEC.h 1541job.h Display.h
CPUC64.o: sysdeps.h sysconfig.h CPUC64.h C64.h VIC.h SID.h CIA.h Prefs.h
CPUC64.o: REU.h IEC.h Display.h Version.h CPU_emulline.h
Network.o: Network.h NetworkUnix.h
CPUC64_SC.o: sysdeps.h sysconfig.h CPUC64.h C64.h CPU_common.h VIC.h SID.h
CPUC64_SC.o: CIA.h Prefs.h REU.h IEC.h Display.h Version.h CPU_emulcycle.h
VIC.o: sysdeps.h sysconfig.h VIC.h C64.h CPUC64.h Display.h Prefs.h
VIC_SC.o: sysdeps.h sysconfig.h VIC.h C64.h CPUC64.h Display.h Prefs.h
CIA.o: sysdeps.h sysconfig.h CIA.h Prefs.h CPUC64.h C64.h CPU1541.h VIC.h
CIA_SC.o: sysdeps.h sysconfig.h CIA.h Prefs.h CPUC64.h C64.h CPU1541.h VIC.h
CPU1541.o: sysdeps.h sysconfig.h CPU1541.h CIA.h Prefs.h C64.h 1541job.h
CPU1541.o: Display.h CPU_emulline.h
CPU1541_SC.o: sysdeps.h sysconfig.h CPU1541.h CIA.h Prefs.h C64.h
CPU1541_SC.o: CPU_common.h 1541job.h Display.h CPU_emulcycle.h
CPU_common.o: sysdeps.h sysconfig.h CPU_common.h
menu.o: menu.h
gui/sdlwidgets.o: gui/sdl.hpp gui/sdlwidgets.cpp gui/widgets.hpp

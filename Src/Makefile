# Makefile.in for Frodo (generic Unix/X11)
include $(DEVKITPPC)/gamecube_rules
## Version information
VERSION  = 4
REVISION = 3

## System specific configuration

#SHELL = /bin/sh

#prefix = /usr/local
#exec_prefix = ${prefix}
#bindir = ${exec_prefix}/bin
#datadir = ${prefix}/share

DESTDIR =



## Files
GUIOBJS = sdlgui.o file.o dlgFileSelect.o dlgMain.o dlgDrives.o dlgOptions.o dlgVideoSound.o dlgAdvanced.o dlgInput.o
OBJS    = $(GUIOBJS) main.o Display.o Prefs.o SID.o REU.o IEC.o 1541fs.o dirent.o\
1541d64.o 1541t64.o 1541job.o SAM.o
SLOBJS  = $(OBJS) C64.o CPUC64.o VIC.o CIA.o CPU1541.o
SLFLAGS = -DPRECISE_CPU_CYCLES=1 -DPRECISE_CIA_CYCLES=1 -DPC_IS_POINTER=0
SCOBJS  = $(OBJS) C64_SC.o CPUC64_SC.o VIC_SC.o CIA_SC.o CPU1541_SC.o CPU_common.o
SCFLAGS = -DFRODO_SC
GUISRCS = sdlgui.cpp file.cpp dlgFileSelect.cpp dlgMain.cpp dlgDrives.cpp dlgOptions.cpp dlgVidoSound.cpp dlgAdvanced.cpp dlgInput.cpp
SRCS    = $(GUISRCS) main.cpp Display.cpp Prefs.cpp SID.cpp REU.cpp IEC.cpp 1541fs.cpp \
          1541d64.cpp 1541t64.cpp 1541job.cpp SAM.cpp CmdPipe.cpp C64.cpp \
          C64_SC.cpp CPUC64.cpp CPUC64_SC.cpp VIC.cpp VIC_SC.cpp CIA.cpp \
          CIA_SC.cpp CPU1541.cpp CPU1541_PC.cpp CPU1541_SC.cpp CPU_common.cpp

#CXX = g++

CFLAGS = $(SLFLAGS) -g -O2 -fomit-frame-pointer -fno-exceptions $(MACHDEP) -I. -I$(DEVKITPRO)/libogc/include/ -I$(DEVKITPRO)/libogc/include/SDL  -DHAVE_SDL -DWII -DR14 -I./ -DKBD_LANG=0
DEFS = -DHAVE_CONFIG_H -DDATADIR=\"/frodo/\" -DBINDIR=\"/frodo/\"
LDFLAGS = -g $(MACHDEP) -Wl,-Map,$(notdir $@).map 
LIBS =  -L$(DEVKITPRO)/libogc/lib/cube -lSDL -logc -lz -lm 
CXXFLAGS   =   $(CFLAGS)

## Rules
.PHONY: install installdirs uninstall clean distclean depend dep
.SUFFIXES:
.SUFFIXES: .o .cpp .h

all: boot.dol #FrodoSC

Frodo.elf: $(SLOBJS)
	$(CXX) -o Frodo.elf $(CFLAGS) $(LDFLAGS) $(SLOBJS) $(LIBS)
#@powerpc-gekko-objcopy -O binary $< $@

FrodoSC: $(SCOBJS)
	$(CXX) -o FrodoSC $(LDFLAGS) $(SCOBJS) $(LIBS)

boot.dol: Frodo.dol
	cp Frodo.dol boot.dol

Frodo.dol: Frodo.elf

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

#-------------------------------------------------------------------------
# DO NOT DELETE THIS LINE -- make depend depends on it.

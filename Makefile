#
# Makefile.nix
#
# This is a *bare minimum* makefile for building gnuboy on *nix systems.
# If you have trouble with the configure script you can try using this,
# but *please* try the configure script first. This file is mostly
# unmaintained and may break.
#
# If you *do* insist on using this makefile, you at least need to check
# SYS_DEFS below and uncomment -DIS_LITTLE_ENDIAN if your system is
# little endian. Also, you may want to enable the OSS sound module if
# your system supports it.
#

prefix = /usr/local
bindir = /bin

CC = gcc
AS = $(CC)
LD = $(CC)

CFLAGS = -O3 
LDFLAGS = -s -lSDLmain -lSDL
ASFLAGS = 

SYS_DEFS = -DIS_LITTLE_ENDIAN
ASM_OBJS = 
#SND_OBJS = sys/oss/oss.o
# SND_OBJS = sys/dummy/nosound.o
# JOY_OBJS = sys/dummy/nojoy.o

TARGETS = gnuboy

SYS_OBJS = $(ASM_OBJS) $(SND_OBJS) sys/windows/windows.o
SYS_INCS = -I/usr/local/include -I./sys/windows

SDL_OBJS = sys/sdl/sdl.o sys/sdl/keymap.o sys/sdl/sdl-joystick.o sys/sdl/sdl-audio.o
SDL_LIBS = -lSDL

all: $(TARGETS)

include Rules

gnuboy: $(OBJS) $(SYS_OBJS) $(SDL_OBJS)
	$(LD) $(LDFLAGS) $(OBJS) $(SYS_OBJS) $(SDL_OBJS) -o $@ $(SDL_LIBS)

clean:
	rm -f *gnuboy gmon.out *.o sys/*.o sys/*/*.o asm/*/*.o


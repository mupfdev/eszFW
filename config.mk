PROJECT=RainbowJoe

ifeq ($(OS),Windows_NT)
	OUT=$(PROJECT).exe
	TOOLCHAIN=i686-w64-mingw32
	CC=$(TOOLCHAIN)-cc
else
	OUT=$(PROJECT)
	TOOLCHAIN=local
	UNAME_S := $(shell uname -s)
endif

LIBS=\
	-lSDL2\
	-lSDL2_image\
	-lSDL2_mixer\
	-lSDL2_ttf\
	-lxml2 -lz -llzma -lm

CFLAGS=\
	-D_REENTRANT\
	-DSDL_MAIN_HANDLED\
	-DWANT_ZLIB\
	-Isrc/inih\
	-Isrc/tmx/src\
	-isystem /usr/$(TOOLCHAIN)/include/libxml2\
	-O3\
	-pedantic-errors\
	-std=c99\
	-Wall\
	-Werror\
	-Wextra

SRCS=\
	$(wildcard src/*.c)\
	$(wildcard src/tmx/src/*.c)\
	$(wildcard src/inih/*.c)

OBJS=$(patsubst %.c, %.o, $(SRCS))

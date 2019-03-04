PROJECT=libeszFW

ifeq ($(OS),Windows_NT)
	OUT=lib/$(PROJECT).lib
	TMX_OUT=lib/libtmx.lib
	TOOLCHAIN=i686-w64-mingw32
	AR=ar
	CC=$(TOOLCHAIN)-gcc
else
	OUT=lib/$(PROJECT).a
	TMX_OUT=lib/libtmx.a
	TOOLCHAIN=local
	UNAME_S := $(shell uname -s)
endif

LIBS=\
	-lSDL2\
	-lSDL2_image\
	-lSDL2_mixer\
	-lSDL2_ttf\
	-lxml2 -lz -llzma

CFLAGS=\
	-D_REENTRANT\
	-DWANT_ZLIB\
	-Isrc\
	-Iexternal/tmx/src\
	-isystem /mingw32/include/libxml2\
	-isystem /mingw32/include/SDL2\
	-O2\
	-pedantic-errors\
	-std=c99\
	-Wall\
	-Werror\
	-Wextra

SRCS=$(wildcard src/*.c)
OBJS=$(patsubst %.c, %.o, $(SRCS))

TMX_SRCS=$(wildcard external/tmx/src/*.c)
TMX_OBJS=$(patsubst %.c, %.o, $(TMX_SRCS))

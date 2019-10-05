![eszFW](.media/logo.png "eszFW")

[![Build status](https://ci.appveyor.com/api/projects/status/0t2yt05ngahfa5jr?svg=true)](https://ci.appveyor.com/project/mupfelofen-de/eszfw)
[![Documentation](https://codedocs.xyz/mupfelofen-de/eszFW.svg?svg=true)](https://codedocs.xyz/mupfelofen-de/eszFW/)

## About

eszFW is a cross-platform game development framework / library written
in C.  It's mainly aimed at platformer games but it can probably used
for anything else too.  Furthermore, this project is the logical
continuation of my older projects [Rainbow
Joe](https://github.com/mupfelofen-de/rainbow-joe) and [Boondock
Sam](https://github.com/mupfelofen-de/boondock-sam), which I wrote to
familiarise myself with the basics of game programming.

It is written in C using SDL2 and the [TMX Map
Format](http://doc.mapeditor.org/en/stable/reference/tmx-map-format/)
(Tile Map XML).

## Documentation

The documentation can be generated using Doxygen:
```
doxygen
```

The documentation is generated automatically and can be found
[here](https://codedocs.xyz/mupfelofen-de/eszFW/).

### Status

This project is in development.  I will enhance it gradually, mainly as
a tool to my own use.  To see it in action, take a look at [Tau
Ceti](https://github.com/mupfelofen-de/TauCeti): a cross-platform 2D
sci-fi themed tech demo which I develop to test this framework.

An Android version of Tau Ceti is available on Google Play:

[![Tau Ceti](.media/badge_new.png)](https://play.google.com/store/apps/details?id=de.mupfelofen.TauCeti)

### Trivia

The abbreviation esz is a tribute to my best friend [Ertugrul
Söylemez](https://github.com/esoeylemez), who suddenly passed away on
May 12th 2018.  We all miss you deeply.

## Why C?

A question I hear alot is why I'm using C for this project instead of a
modern language like Rust, Haskell or `[insert your preferred language
here]`, because writing games in C is neither a easy thing to do nor
very common these days (even though many popular and critical acclaimed
games are written in C e.g. Doom, Quake and Neverwinter Nights).

The main reason is simple: to get exercise in C programming.  And
writing a game in C seemed like a fun way of getting some.  Even though
I am currently trying to get on with Haskell, I still consider C a fun
and challenging language which I also have to use professionally.

Anyway: I hope you find the code of this project useful.  If so, feel
free to use it in any way you want. Just consider buying me a beer in
case we meet someday.

## Dependencies

The program has been successfully compiled with the following libraries:
```
libxml2    2.9.8
sdl2       2.0.8
sdl2_image 2.0.3
sdl2_mixer 2.0.2
zlib       1.2.11
```

#### MacOS

All libraries can be installed normally via `brew` given the above formula names.

Additionally, MacOS builds require `icu4c` (installed with `brew install icu4c`).

Further, `icu4c` and `libxml2` will need to be force-linked using
`brew link --force icu4c libxml2`.

## Compiling

First clone the repository including the submodules:
```
git clone --recurse-submodules -j2 https://github.com/mupfelofen-de/eszFW.git
cd eszFW
```

To compile _eszFW_ under Linux use CMake as follows:
```
mkdir build
cd build
cmake ..
make
```

## Licence and Credits

This project is licenced under the "THE BEER-WARE LICENCE".  See the
file [LICENCE.md](LICENCE.md) for details.

[TMX C Loader](https://github.com/baylej/tmx/) by Bayle Jonathan is
licenced under a BSD 2-Clause "Simplified" Licence.  See the file
[COPYING](https://raw.githubusercontent.com/baylej/tmx/master/COPYING)
for details.

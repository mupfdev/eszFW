# eszFW

## About

eszFW is a portable framework with the intention to simplify 2D game
development in C.  Furthermore, this project is the logical continuation
of my older projects [Rainbow
Joe](https://github.com/mupfelofen-de/rainbow-joe) and [Boondock
Sam](https://github.com/mupfelofen-de/boondock-sam), which I wrote to
familiarise myself with the basics of game programming.

It is written in C using SDL2 and the [TMX Map
Format](http://doc.mapeditor.org/en/stable/reference/tmx-map-format/)
(Tile Map XML).

### Status

The framework is in a very early stage of development.  I will enhance
it gradually and hope that one day it will be suitable for productive
use.  Until then, have fun with it.

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

To compile _eszFW_ under Linux simply enter:
```
git clone --recurse-submodules -j2 https://github.com/mupfelofen-de/eszFW.git
cd eszFW
make
```

If you're on NixOS enter:
```
nix-shell --command make
```

To compile the example applications:
```
cd examples
make
```

## Licence and Credits

This project is licenced under the "THE BEER-WARE LICENCE".  See the
file [LICENCE.md](LICENCE.md) for details.

[TMX C Loader](https://github.com/baylej/tmx/) by Bayle Jonathan is
licenced under a BSD 2-Clause "Simplified" Licence.  See the file
[COPYING](https://raw.githubusercontent.com/baylej/tmx/master/COPYING)
for details.

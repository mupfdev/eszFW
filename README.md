# eszFW

<p>
<a href="https://github.com/mupfelofen-de/eszFW">
  <img src="https://img.shields.io/badge/project-GitHub-blue?style=flat?svg=true" alt="GitHub project" />
</a>
<a href="https://github.com/mupfelofen-de/eszFW/blob/master/LICENSE.md">
  <img src="https://img.shields.io/badge/licence-MIT-blue?style=flat?svg=true" alt="Licence" />
</a>
<a href="https://ci.appveyor.com/project/mupfelofen-de/eszfw">
  <img src="https://ci.appveyor.com/api/projects/status/0t2yt05ngahfa5jr?svg=true" alt="Build status" />
</a>
<a href="https://www.codacy.com/manual/mupf/eszFW?utm_source=github.com&amp;utm_medium=referral&amp;utm_content=mupfelofen-de/eszFW&amp;utm_campaign=Badge_Grade">
  <img src=https://app.codacy.com/project/badge/Grade/999c4a824cba43dba68100819699fcfa alt="Codacy Badge" />
</a>
</p>

## About

eszFW is a cross-platform game engine written in C.  It's aimed at
platformer games. This project is the logical continuation of my older
projects [Rainbow Joe](https://github.com/mupfelofen-de/rainbow-joe) and
[Boondock Sam](https://github.com/mupfelofen-de/boondock-sam).

## Features

- It runs on all platforms [supported by SDL2](https://wiki.libsdl.org/Installation#Supported_platforms).
- Fully reentrant engine core.

## Documentation

The documentation can be generated using Doxygen:
```bash
doxygen
```

A automatically generated version of the documentation can be found
here:  [eszfw.de](https://eszfw.de)

### Status

**Update**: I started to write the entire engine from scratch.  Until
  the next release this project is in an incomplete state.

This project is in development.  I will enhance it gradually, mainly as
a tool to my own use.  To see it in action, take a look at [Tau
Ceti](https://github.com/mupfelofen-de/TauCeti): a cross-platform 2D
sci-fi themed tech demo which I develop to test this engine.

[![Tau Ceti](https://media.eszfw.de/tc-01-tn.png)](https://media.eszfw.de/tc-01.png?raw=true "Tau Ceti 1")
[![Tau Ceti](https://media.eszfw.de/tc-02-tn.png)](https://media.eszfw.de/tc-02.png?raw=true "Tau Ceti 2")

An Android version of Tau Ceti is available on Google Play:

[![Get it on Google Play](https://play.google.com/intl/en_us/badges/images/generic/en_badge_web_generic.png)](https://play.google.com/store/apps/details?id=de.mupfelofen.TauCeti)

### Trivia

The abbreviation esz is a tribute to my best friend [Ertugrul
Söylemez](https://github.com/esoeylemez), who suddenly passed away on
May 12th, 2018.  We all miss you deeply.

## C is dead, long live C

Even though hardly any games are written in C nowadays, there are a few
noteworthy titles that meet this criterion e.g. Doom, Quake, Quake II,
and Neverwinter Nights.

This project should show that it is still possible and that C (and
procedural programming in general) is often underestimated.

With that in mind: C is dead, long live C!

## Dependencies

The program has been successfully compiled and tested with the following libraries:
```text
SDL2       2.0.12
libxml2    2.9.10 (optional)
zlib       1.2.11 (optional)
```

## Compiling

First clone the repository including the submodules:
```bash
git clone --recurse-submodules -j2 https://github.com/mupfelofen-de/eszFW.git
cd eszFW
```

To compile _eszFW_ under Linux use CMake as follows:
```bash
mkdir build
cd build
cmake ..
make
```

## Licence and Credits

This project is licensed under the "The MIT License".  See the file
[LICENSE.md](LICENSE.md) for details.

[cwalk](https://github.com/likle/cwalk) by Leonard Iklé is licensed
under the MIT License.  See the file
[LICENSE.md](https://github.com/likle/cwalk/blob/master/LICENSE.md) for
details.

[TMX C Loader](https://github.com/baylej/tmx/) by Bayle Jonathan is
licensed under a BSD 2-Clause "Simplified" Licence.  See the file
[COPYING](https://raw.githubusercontent.com/baylej/tmx/master/COPYING)
for details.

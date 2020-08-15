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
  <a href="https://www.codetriage.com/mupfelofen-de/eszfw">
    <img src="https://www.codetriage.com/mupfelofen-de/eszfw/badges/users.svg" alt="Open Source Helpers" />
  </a>
</p>

## About

eszFW is a cross-platform game engine written in C.  It's aimed at
platformer games. This project is the logical continuation of my older
projects [Rainbow Joe](https://github.com/mupfelofen-de/rainbow-joe) and
[Boondock Sam](https://github.com/mupfelofen-de/boondock-sam).

## Features

- It runs on all platforms [supported by
  SDL2](https://wiki.libsdl.org/Installation#Supported_platforms).

- Fully reentrant engine core.

- The dependencies can be limited to SDL2.

- Support for two different TMX loaders:
  [libTMX](https://github.com/baylej/tmx) and
  [cute_tiled](https://github.com/RandyGaul/cute_headers).

## Documentation

The documentation can be generated using Doxygen:
```bash
doxygen
```

A automatically generated version of the documentation can be found
here:  [eszfw.de](https://eszfw.de)

## Code style

You are invited to contribute to this project. But to ensure a uniform
formatting of the source code, you will find some rules here:

- Follow the C11 standard.
- Do not use tabs and use a consistent 4 space indentation style.
- Use lower snake_case for both function and variable names.
- Try to use a consistent style.  Use the existing code as a guideline.

### Status

This project currently undergoes a complete overhaul.

If you wanna see the previous version in action, take a look at [Tau
Ceti](https://github.com/mupfelofen-de/TauCeti): a sci-fi themed tech
demo which I develop to test this engine.

[![Tau Ceti](https://media.eszfw.de/tc-01-tn.png)](https://media.eszfw.de/tc-01.png?raw=true "Tau Ceti 1")
[![Tau Ceti](https://media.eszfw.de/tc-02-tn.png)](https://media.eszfw.de/tc-02.png?raw=true "Tau Ceti 2")

An Android version of Tau Ceti is available on Google Play:

[![Get it on Google Play](https://play.google.com/intl/en_us/badges/images/generic/en_badge_web_generic.png)](https://play.google.com/store/apps/details?id=de.mupfelofen.TauCeti)

## C is dead, long live C

Even though hardly any games are written in C nowadays, there are a few
noteworthy titles that meet this criterion e.g. Doom, Quake, Quake II,
and Neverwinter Nights.

This project should show that it is still possible and that C (and
procedural programming in general) is often underestimated.

With that in mind: C is dead, long live C!

### Trivia

The abbreviation esz is a tribute to my best friend [Ertugrul
Söylemez](https://github.com/esoeylemez), who suddenly passed away on
May 12th, 2018.  We all miss you deeply.

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

You can also use _eszFW_ with _libTMX_ instead of _cute_tiled_.  Just
enable the respective CMake option:

```bash
cmake -DUSE_LIBTMX=ON ..
```

## Licence and Credits

[cute_tiled](https://github.com/RandyGaul/cute_headers) by Randy Gaul is
licensed under the zlib licence.

[TMX C Loader](https://github.com/baylej/tmx/) by Bayle Jonathan is
licensed under a BSD 2-Clause "Simplified" Licence.

This project and all further listed libraries are licensed under the
"The MIT License".  See the file [LICENSE.md](LICENSE.md) for details.

[buffer](https://github.com/clibs/buffer) by TJ Holowaychuk.

[cwalk](https://github.com/likle/cwalk) by Leonard Iklé.

[picolog](https://github.com/picojs/picolog) by James McLean.

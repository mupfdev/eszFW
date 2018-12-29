{ pkgs ? import <nixpkgs> {} }:

with pkgs;
with stdenv.lib;

let
    makeSDLCFlags = lib.concatMap (p: [ "-I${getDev p}/include/SDL2" "-I${getDev p}/include" ]);
    makeSDLDFlags = lib.concatMap (p: [ "-L${getLib p}/lib" ]);
    SDLlibs = [ SDL2 SDL2_image SDL2_mixer SDL2_ttf ];

in pkgs.stdenv.mkDerivation {
    name = "eszFW";
    src = ./.;

    NIX_CFLAGS_COMPILE = (makeSDLCFlags SDLlibs) ++ [ "-I${getDev zlib}/include" "-I${getDev libxml2}/include/libxml2" ];
    NIX_CFLAGS_LINK    = (makeSDLDFlags SDLlibs) ++ [ "-L${getLib zlib}/lib" "-L${getLib libxml2}/lib" "-L${getLib lzma}/lib" "-L${getLib icu}/lib" ];

    installPhase = ''
        install -Dsm 755 RainbowJoe $out/bin/RainbowJoe
    '';
}

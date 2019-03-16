{
  nixpkgs ? <nixpkgs>,
  pkgs ? import nixpkgs {}
}:

with pkgs;

let
  extraIncludes = ps: builtins.concatStringsSep " " (
    builtins.map (p: "-I${stdenv.lib.getDev p}/include/SDL2") ps
  );
in stdenv.mkDerivation {
  name = "eszfw";
  src = ./.;

  nativeBuildInputs = [ cmake pkgconfig ];
  buildInputs = [ libxml2 SDL2 SDL2_image SDL2_mixer SDL2_ttf ];

  NIX_CFLAGS_COMPILE = extraIncludes [ SDL2_ttf SDL2_mixer ];

  installPhase = ''
    mkdir -p $out/bin
    cp libeszFW.so $out/lib/libeszFW.so
  '';
}

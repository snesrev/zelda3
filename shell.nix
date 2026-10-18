{ pkgs ? import <nixpkgs> {} }:
  pkgs.mkShell {
    nativeBuildInputs = with pkgs.buildPackages; 
      [ gcc 
        clang 
        SDL2.dev 
        gnumake 
        (python311.withPackages(ps: with ps; [ pillow pyyaml ]))
      ];

   # To build, make sure you have the ROM in place, then run `make -j16`
}

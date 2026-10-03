{ pkgs ? import <nixpkgs> {} }:

pkgs.mkShell {
  nativeBuildInputs = [
    pkgs.gnumake
    pkgs.pkg-config
  ];

  buildInputs = [
    pkgs.SDL2
    pkgs.libGL
    (pkgs.python3.withPackages (ps: [
      ps.pillow
      ps.pyyaml
    ]))
  ];

  # Project uses pre-C23 empty parameter lists; GCC 15 defaults to C23
  CFLAGS = "-O2 -Werror -std=gnu17";
}

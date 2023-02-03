with (import <nixpkgs> {});

let
	nixgl = import (builtins.fetchTarball https://github.com/guibou/nixGL/archive/main.tar.gz) {};
	my-py-pkgs = py-pkgs: with py-pkgs; [
		pillow
		pyyaml
	];
	py-packaged = python3.withPackages my-py-pkgs;
	libPath = with pkgs; lib.makeLibraryPath [
		libGL
		xorg.libX11
		xorg.libX11.dev
		xorg.libXcursor
		xorg.libXrender
		xorg.libXfixes
		xorg.libXi
		xorg.libxcb
		xorg.libXScrnSaver
		xorg.libpthreadstubs
		xorg.libXext
		xorg.libXrandr
		xorg.libXau
		xorg.libXdmcp
		libxkbcommon
	];
in
mkShell {
	buildInputs = [
		py-packaged
		SDL2
		SDL2.dev
		xorg.libX11
		xorg.libX11.dev
		xorg.libXcursor
		xorg.libXScrnSaver
		xorg.libXi
		xorg.libXrandr
		xorg.libxcb
		libxkbcommon
		libGL
		gcc
		nixgl.auto.nixGLDefault
	];
	LD_LIBRARY_PATH = libPath;
	LDFLAGS = "-lpthread -lrt";
}

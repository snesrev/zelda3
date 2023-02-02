with (import <nixpkgs> {});

let
	nixgl = import (builtins.fetchTarball https://github.com/guibou/nixGL/archive/main.tar.gz) {};
	my-py-pkgs = py-pkgs: with py-pkgs; [
		pillow
		pyyaml
	];
	py-packaged = python3.withPackages my-py-pkgs;
in
mkShell {
	buildInputs = [
		py-packaged
		SDL2
		SDL2.dev
		clang
		nixgl.auto.nixGLDefault
	];
}

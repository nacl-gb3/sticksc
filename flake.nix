{
  description = "A basic flake with a shell";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixpkgs-unstable";
    flake-utils.url = "github:numtide/flake-utils";
  };

  outputs = { nixpkgs, flake-utils, ... }:
    flake-utils.lib.eachDefaultSystem (system:
      let
        pkgs = nixpkgs.legacyPackages.${system};
      in
      {
        devShells.default = pkgs.mkShell {
          packages = [];
          nativeBuildInputs = [
            pkgs.gcc
            # pkgs.clang
            pkgs.gdb
            pkgs.valgrind
            pkgs.gnumake
            # pkgs.cmake
            # pkgs.bear
          ];
          buildInputs = [ ];
        };
      });
}

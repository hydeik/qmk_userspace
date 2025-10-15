{
  description = "My QMK keymap firmware for multiple keyboards";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-25.05";
    flake-parts = {
      url = "github:hercules-ci/flake-parts";
      inputs.nixpkgs-lib.follows = "nixpkgs";
    };
    devshell = {
      url = "github:numtide/devshell";
      inputs.nixpkgs.follows = "nixpkgs";
    };
    systems.url = "github:nix-systems/default";
  };

  outputs =
    inputs:
    inputs.flake-parts.lib.mkFlake { inherit inputs; } {
      imports = [ inputs.devshell.flakeModule ];
      systems = import inputs.systems;
      perSystem =
        { pkgs, ... }:
        {
          devshells.default = {
            devshell.packages = with pkgs; [
              clang-tools
              qmk
            ];
          };
        };
    };
}

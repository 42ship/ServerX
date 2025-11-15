{
  pkgs ? import <nixpkgs> { },
}:
pkgs.mkShell {
  buildInputs = with pkgs; [
  ];
  nativeBuildInputs = with pkgs; [
  ];
  packages = with pkgs; [
    gdb
    compiledb
    doxygen
    valgrind
    gnumake
    perf
    apacheHttpd
    flamegraph
  ];
}

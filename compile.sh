#!/bin/sh

# Check folders
if [ ! -d source ]; then
  echo "Source code missing";
  exit 1
fi
if [ ! -d include ]; then
  echo "Source code headers missing";
  exit 2
fi
if [ ! -f 3gx.ld ]; then
  echo "Plugin linker specification missing";
  exit 3
fi
if [ ! -f cookbook.mk ]; then
  echo "Makefile missing";
  exit 3
fi

make -f cookbook.mk clean
if test "$1" = "rectrpf" || test ! -d CTRPF/ctrpluginframework; then
cd CTRPF
./fetchAndPatch $2
cd ..
fi

make -f cookbook.mk
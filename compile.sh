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
    exit 4
fi

make -f cookbook.mk clean
if make -f cookbook.mk $1 -j4; then
    python3 resources/mkHookList.py -i resources/ptcEUR-3.6.0.ini -o build/map_EUR.cyxmap
    python3 resources/mkHookList.py -i resources/ptcUSA-3.6.0.ini -o build/map_USA.cyxmap
    python3 resources/mkHookList.py -i resources/ptcJPN-3.6.3.ini -o build/map_JPN.cyxmap
    if [ -d release ]; then
        mkdir -p release/sdmc/PTC3PLG/resources
        rm release/sdmc/PTC3PLG/resources/*.*
        cp *.3gx release/sdmc/PTC3PLG/resources
        cp build/*.cyxmap release/sdmc/PTC3PLG/resources
        rm -rf release/sdmc/luma/plugins
        mkdir -p release/sdmc/luma/plugins
        mkdir release/sdmc/luma/plugins/0004000000117200
        cp *.3gx release/sdmc/luma/plugins/0004000000117200
        mkdir release/sdmc/luma/plugins/000400000016DE00
        cp *.3gx release/sdmc/luma/plugins/000400000016DE00
        mkdir release/sdmc/luma/plugins/00040000001A1C00
        cp *.3gx release/sdmc/luma/plugins/00040000001A1C00
    fi
fi

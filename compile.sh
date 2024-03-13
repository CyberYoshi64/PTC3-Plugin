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
if [ ! -f Makefile ]; then
    echo "Makefile missing";
    exit 4
fi

make clean
if make $1 -j$(nproc); then
    python3 resources/mkHookList.py -i resources/hooks/ptcEUR-3.6.0.ini -o build/map_EUR.cyxmap
    python3 resources/mkHookList.py -i resources/hooks/ptcUSA-3.6.0.ini -o build/map_USA.cyxmap
    python3 resources/mkHookList.py -i resources/hooks/ptcJPN-3.6.3.ini -o build/map_JPN.cyxmap
    python3 resources/mkStrArc.py resources/strings/csv build
    if [ -d release ]; then
        mkdir -p release/PTC3PLG/resources/lang
        rm -f release/PTC3PLG/resources/*.*
        cp build/lang/*.* release/PTC3PLG/resources/lang
        cp output/*.3gx release/PTC3PLG/resources
        cp build/*.cyxmap release/PTC3PLG/resources
        rm -rf release/luma/plugins
        mkdir -p release/luma/plugins
        mkdir release/luma/plugins/0004000000117200
        cp output/*.3gx release/luma/plugins/0004000000117200
        mkdir release/luma/plugins/000400000016DE00
        cp output/*.3gx release/luma/plugins/000400000016DE00
        mkdir release/luma/plugins/00040000001A1C00
        cp output/*.3gx release/luma/plugins/00040000001A1C00
        if [ -f PTC3PLG.zip ]; then
        	rm PTC3PLG.zip
        fi
        zip -9qur PTC3PLG.zip release/*
    fi
fi

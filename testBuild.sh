#!/bin/bash

SENDFILEPY="./sendfile.py"
PLUGINFILE="ptc3plg.3gx"
FTP_IP="192.168.2.180"
FTP_PORT="5000"

if ./compile.sh $1; then
    $SENDFILEPY $PLUGINFILE /PTC3PLG/resources $FTP_IP $FTP_PORT
    $SENDFILEPY $PLUGINFILE /luma/plugins/0004000000117200 $FTP_IP $FTP_PORT
    $SENDFILEPY $PLUGINFILE /luma/plugins/000400000016DE00 $FTP_IP $FTP_PORT
    $SENDFILEPY $PLUGINFILE /luma/plugins/00040000001A1C00 $FTP_IP $FTP_PORT
    $SENDFILEPY build/map_EUR.cyxmap /PTC3PLG/resources $FTP_IP $FTP_PORT
    $SENDFILEPY build/map_USA.cyxmap /PTC3PLG/resources $FTP_IP $FTP_PORT
    $SENDFILEPY build/map_JPN.cyxmap /PTC3PLG/resources $FTP_IP $FTP_PORT
    $SENDFILEPY build/map_JPN.cyxmap /PTC3PLG/resources $FTP_IP $FTP_PORT
    $SENDFILEPY build/lang/ENG.bin /PTC3PLG/resources/lang $FTP_IP $FTP_PORT
    ./toCitra.sh
else
    echo "Damnit..."
fi

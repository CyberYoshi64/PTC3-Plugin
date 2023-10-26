#!/usr/bin/python3

import sys
import ftplib
import datetime
from ftplib import FTP

# Usage:
# 	recvfile.py "filename" "ftppath" "host" "ip"

def printf(string):
    print(datetime.datetime.strftime(datetime.datetime.now(), '%Y-%m-%d %H:%M:%S') + " : " + string);

if __name__ == '__main__':
    print("");
    printf("FTP File Sender\n")
    try:
        filename = sys.argv[1]
        path = sys.argv[2]
        host = sys.argv[3]
        port = int(sys.argv[4])

        ftp = FTP()
        printf("Connecting to " + host + ":" + str(port));
        ftp.connect(host, port);
        printf("Connected");

        printf("Opening " + filename);
        file = open(sys.argv[1], "wb");
        printf("Success");

        printf("Moving to: ftp:/" + path);
        ftp.cwd(path);
        printf("Receive file");
        ftp.retrbinary('RETR '+ filename, file.write);
        printf("Done")

        file.close();

        ftp.quit();
        printf("Disconnected");

    except IOError as e:
        printf("/!\ An error occured. /!\ ");


#!/usr/bin/python3

import os, shutil, sys, struct, io, json, glob

class StringArchiveEntry:
    _STRUCT = "< I 20s I H H"

    _ENCODINGS = [
        "ascii",
        "utf-8",
        "utf-16-le"
    ]

    def __init__(self) -> None:
        self.stringID = 0
        self.stringName = b''
        self.fileOffset = 0
        self.length = 0
        self.data = b'' # length is derived
        self.encoding = 0

    def set(self, id:int, name:str, data:str|bytes, encoding="utf-8"):
        self.stringID = id
        self.stringName = name.encode("utf-8","replace")
        self.data = data if (type(data)==bytes) else data.encode(encoding, "replace")
        self.length = len(self.data)
        self.data += b'\0' * (4 - (len(self.data)%4))
        self.encoding = self._ENCODINGS.index(encoding)
    
    def packEntryHead(self, fd:io.BufferedWriter, off:int):
        fd.write(struct.pack(self._STRUCT, self.stringID, self.stringName, off, self.length, self.encoding))
        return len(self.data)

    def packData(self, fd:io.BufferedWriter):
        fd.write(self.data)

class StringArchive:
    _STRUCT = "< 4s I I 20x"

    magic = b'CY5$'
    version = 1

    def __init__(self) -> None:
        self.entries = []
    
    def addEntryRaw(self, e):
        self.entries.append(e)
    
    def addEntry(self, *s):
        e = StringArchiveEntry()
        e.set(*s)
        self.entries.append(e)
    
    def pack(self, fd:io.BufferedWriter):
        fd.write(struct.pack(self._STRUCT, self.magic, self.version, len(self.entries)))
        offset = struct.calcsize(self._STRUCT) + struct.calcsize(StringArchiveEntry._STRUCT) * len(self.entries)
        for i in self.entries:
            offset += i.packEntryHead(fd, offset)
        for i in self.entries:
            i.packData(fd)

def parseStr(s:str):
    while True:
        i = s.find(r"\x")
        if i<0: break
        s = s[:i] + chr(int("0x"+s[i+2:i+4],0)) + s[i+4:]
    while True:
        i = s.find(r"\u")
        if i<0: break
        s = s[:i] + chr(int("0x"+s[i+2:i+6],0)) + s[i+6:]
    while True:
        i = s.find(r"\d")
        if i<0: break
        s = s[:i] + chr(int(s[i+2:i+5])) + s[i+5:]
    while True:
        i = s.find(r"\n")
        if i<0: break
        s = s[:i] + "\n" + s[i+2:]
    while True:
        i = s.find(r"\r")
        if i<0: break
        s = s[:i] + "\r" + s[i+2:]
    while True:
        i = s.find(r"\t")
        if i<0: break
        s = s[:i] + "\t" + s[i+2:]
    while True:
        i = s.find(r"\"")
        if i<0: break
        s = s[:i] + '"' + s[i+2:]
    while True:
        i = s.find(r"\'")
        if i<0: break
        s = s[:i] + "'" + s[i+2:]
    while True:
        i = s.find(r"\\")
        if i<0: break
        s = s[:i] + "\\" + s[i+2:]
    return s

indir = sys.argv[1]
g = glob.glob(os.path.join(indir, "*.csv"))

outdir = os.path.join(sys.argv[2], "lang")

if os.path.isdir(outdir):
    shutil.rmtree(outdir)
os.makedirs(outdir, exist_ok=True)

for i in g:
    print(i)
    n = i[i.rfind(os.sep)+1:i.rfind(".")]
    
    strarc = StringArchive()
    
    with open(i, "r", encoding="utf-8", errors="replace", newline="\n") as f:
        l = f.readlines()
        for i in range(len(l)):
            l[i] = l[i].split("\t")
            for j in range(len(l[i])):
                l[i][j] = l[i][j].strip("\n ")
            l[i][2] = parseStr(l[i][2])
        l.pop(0)
    
    for i in l:
        strarc.addEntry(int(i[0],0), i[1], i[2], i[3])

    with open(
        os.path.join(outdir, "%s.bin"%n),
        "w+b"
    ) as f:
        strarc.pack(f)
#!/usr/bin/python3

import os, sys, struct

l = {
    0: "DMY",
    1000000: "INIT",
    1000001: "EXIT",
    1000010: "CFGGET",
    1000011: "CFGSET",
    1000020: "FILES",
    1000021: "FOPEN",
    1000022: "FMODE",
    1000023: "FREAD",
    1000024: "FWRITE",
    1000025: "FCLOSE",
    1000026: "FSEEK",
    1000027: "VALIDATE",
    1000030: "CHKFILE",
    1000031: "MKFILE",
    1000032: "MVFILE",
    1000033: "RMFILE",
    1000040: "CHKDIR",
    1000041: "MKDIR",
    1000042: "MVDIR",
    1000043: "RMDIR",
    1100000: "UNIXTIME",
    1900000: "OSD",
    1900001: "CRASH",
    1900002: "FILLGRP",
    1900003: "SYSGCOPY",
    1900004: "SETUP_CLIP"
}

with open("build/funcMap.bin","wb") as f:
    f.write(struct.pack("<2I", 1, len(l)))
    for id,name in l.items():
        f.write(struct.pack("<IH", id, len(name)) + name.encode("ascii","replace").replace(b'?', b'_'))

l = {
    0: "DMY",
    1000000: "SAFEDIR",
    1000001: "PRJ_ACCESS",
    1000002: "SD_ACCESS",
    2000000: "DIRECTMODE",
    2000001: "PTC_VER",
    2000010: "LANG",
    2000011: "LANGSTR",
    2000012: "REGION",
    2000013: "REGIONSTR",
    2000020: "SYS_VERSTR",
    2000021: "SYS_REGION",
    2000022: "SYS_REGIONSTR",
    2000023: "SYS_MODEL",
    2000030: "ISCITRA",
    2000040: "VOLSLIDER",
    2000041: "HEADSET",
    2000050: "3DSLIDER",
    2000060: "WIFILEVEL",
    2000070: "BAT_LEVEL_RAW",
    2000071: "BAT_LEVEL",
    2000072: "BAT_CHARGING",
    2000073: "BAT_CHARGER",
    2000080: "CAN_SLEEP",
    2000090: "NETWORKSTATE",
    2010000: "PARENTALFLAGS",
    2100000: "FIRM_VER",
    2100001: "KERNEL_VER",
    3000000: "CYX_VER",
    3000001: "CYX_VERSTR",
    3000002: "CYX_COMMIT",
    3000003: "CYX_BUILDSTR",
    4000000: "SDMCCLUSTER",
    4000001: "SDMCSECTOR",
    4000002: "SDMCFREE",
    4000003: "SDMCFREE_C",
    4000004: "SDMCTOTAL",
    4000005: "SDMCTOTAL_C"
}

with open("build/funcCfgMap.bin","wb") as f:
    f.write(struct.pack("<2I", 1, len(l)))
    for id,name in l.items():
        f.write(struct.pack("<IH", id, len(name)) + name.encode("ascii","replace").replace(b'?', b'_'))

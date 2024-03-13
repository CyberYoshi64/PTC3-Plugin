#!/usr/bin/python3
import os, argparse, configparser

class HookAddresses:
    list = []
    def __init__(self, name) -> None:
        cfgprs = configparser.ConfigParser()
        cfgprs.read(name)
        for i in cfgprs.items("Hooks"):
            if i[0]=="version":
                if i[1]!="2": raise Exception("Incompatible version")
        self.list = cfgprs.items("Hooks.Keys")
        del cfgprs
    def get(self, k:str, d=None):
        s = k.lower()
        for i in self.list:
            if i[0]==s: return i[1]
        return d

class HookListFile:
    hooks = []
    hookOrder = (
        "VersionInt",
        "BootText",
        "ConfigBuf",
        "EditorData",
        "GraphicStructs",
        "ActiveProjStr",
        "HelpPagePal",
        "HelpPageDefCol",
        "HelpPageDefPal",
        "ConsoleTextPal",
        "FuncController",
        "BasicInterpretRun",
        "BasicCommandRun",
        "BasicIsDirectMode",
        "FontMapBuf",
        "FuncFontGetOff",
        "ColorKeybBack",
        "ColorSearchBack",
        "ColorFileCreatorBack",
        "ColorFileDescBack",
        "ColorActiveProjLbl",
        "ColorSetSmToolLbl",
        "ColorSetKeyRep",
        "ColorFileKeyLbl",
        "ServerLoad2_1",
        "ServerLoad2_2",
        "ServerSave3_1",
        "ServerSave3_2",
        "ServerShow2",
        "ServerList2",
        "ServerInfo2",
        "ServerDelete2",
        "ServerShoplist2",
        "ServerPrepurchase2",
        "ServerPurchase2",
        "PetcFileHMACKey",
        "PetcSessionTokenFunc",
        "OAuthOutToken",
        "nnActConnectRequired",
        "nnActNetworkTimeValidated",
        "nnActIsNetworkAccountFunc",
        "nnSndSoundThreadEntry1",
        "nnSndSoundThreadEntry2",
        "screenBuf",
        "funcBsaStringAlloc",
        "funcPrintCon",
        "funcMainEntry",
        "funcMainLoop1",
        "funcSendKeyCode",
        "funcBSAError",
        "varBsaResult",
    )
    _MGC = b"CY$X"
    version = 3

    def __init__(self, hooklist:HookAddresses=None)->None:
        if hooklist!=None:
            for i in self.hookOrder:
                r = hooklist.get(i)
                if r == None: raise Exception("Provided hooklist doesn't have a key for '%s'."%i)
                try:
                    r = int(r,0)
                    self.hooks.append(r)
                except: raise Exception("Key '%s' is malformed."%i)

    def export(self, name:str=None):
        b = self._MGC + int.to_bytes(self.version, 4, "little")
        for i in self.hooks:
            b += int.to_bytes(i, 4, "little")
        if name!=None:
            if os.path.isdir(name): raise Exception("Can't output to a folder.")
            if os.path.exists(name): os.remove(name)
            with open(name, "wb") as f:
                f.write(b)

        
args = argparse.ArgumentParser(description="Make a hook list for CYX")

args.add_argument("-i", "--input", required=True, help="Input file (.INI containing hook addresses)")
args.add_argument("-o", "--output", required=True, help="Output file (binary)")
args.add_argument("-v", "--verbose", action="store_true", help="Print what's being done")

arg = args.parse_args()

if not arg.verbose: print(arg.output)
if arg.verbose: print("Obtaining hook addresses from '%s'" % arg.input,end="...",flush=True)
h = HookAddresses(arg.input)
if arg.verbose: print("done.\r\nParsing address list",end="...",flush=True)
hf = HookListFile(h)
if arg.verbose: print("done.\r\nSaving hook map to '%s'" % arg.output,end="...",flush=True)
hf.export(arg.output)
if arg.verbose: print("done.\r\nSuccess!")
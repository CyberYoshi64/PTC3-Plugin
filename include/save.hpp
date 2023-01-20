#pragma once
#include <CTRPluginFramework.hpp>

#define MAX_SAVE_ENTRIES 50

namespace CTRPluginFramework {

	typedef struct archEntry_s {
		char archName[0x10];
		u64 archHandle;
		u8 type;
		u8 finished;
	} archEntry;
	typedef struct archData_s {
		u32 numEntries;
		archEntry entries[MAX_SAVE_ENTRIES];
	} archData;

	enum ArchTypes
	{
		ARCH_ROMFS = 1,
		ARCH_SAVE = 2,
		ARCH_EXTDATA = 4
	};
	#define ARCH_RW		ARCH_SAVE|ARCH_EXTDATA
	class OnionSave
	{
	public:
		static archData save;
		static void addArchive(u8* arch, u64 handle);
		static bool getArchive(u16* arch, u8* mode, bool isReadOnly);
		static void initDebug();
		static void debugAppend(std::string);
		static int existArchiveu16(u16* arch);
		static int existArchiveHnd(u64 hnd);
		static void addArchiveHnd(u64 handle, u32 archid);
		static int existArchiveu8(u8* arch);
		static void setupPackPaths();
		static u16 romPath[50];
		static u16 dataPath[50];
		static u16 extPath[50];
		static File* debugFile;
	};
}
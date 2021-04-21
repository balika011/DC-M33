
#ifndef MODULE_ENTRY_H
#define MODULE_ENTRY_H

class CModuleEntry
{
public:
	CModuleEntry(void);
	~CModuleEntry();

	int ParseBinary(char *buffer);
	int ParseText(char *file, char *key, char *flags, char *modNames, int &namePos);

	int WriteBinary(FILE *fd);
	int WriteText(FILE *fd, char *modNames);

	void Print(char *modNames);
	
	int GetFlags()
	{
		return mFlags;
	}

private:
#pragma pack(push, 1)
	struct ModEntryHeader
	{
		int ModNameOffset;	// Module name offset 
		int zero0;
		int Flags;			// 0001 - vsh mode, 
							// 0002 - game (presume)
							// 0004 - update
							// 0008 - pops
							// 0010 - licensegame
							// 0040 - app 
		int zero1;
		int key[4];			// Hash of some description
	};
#pragma pop(push)

	int mFlags;
	int mNameOffset;
	int mKey[4];
};

#endif

